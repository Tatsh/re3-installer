#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "env.h"
#include "helpers.h"
#include "log.h"
#include "mkdir_p.h"
#include "support.h"

bool exists(const char *path) {
    struct stat statbuf;
    int ret = stat(path, &statbuf);
    if (ret < 0) {
        return false;
    }
    return S_ISREG(statbuf.st_mode) || S_ISDIR(statbuf.st_mode);
}

bool is_file(const char *path) {
    struct stat statbuf;
    int ret = stat(path, &statbuf);
    if (ret < 0) {
        return false;
    }
    return S_ISREG(statbuf.st_mode);
}

bool is_directory(const char *path) {
    struct stat statbuf;
    int ret = stat(path, &statbuf);
    if (ret < 0) {
        return false;
    }
    return S_ISDIR(statbuf.st_mode);
}

bool ends_with_iso(const char *path) {
    size_t len = strlen(path);
    if (len < 3) {
        return false;
    }
    return !strncasecmp(path + len - 3, "iso", 3);
}

bool ends_with_dll(const char *path) {
    size_t len = strlen(path);
    if (len < 3) {
        return false;
    }
    return !strncasecmp(path + len - 3, "dll", 3);
}

bool ends_with_exe(const char *path) {
    size_t len = strlen(path);
    if (len < 3) {
        return false;
    }
    return !strncasecmp(path + len - 3, "exe", 3);
}

bool ends_with_url(const char *path) {
    size_t len = strlen(path);
    if (len < 3) {
        return false;
    }
    return !strncasecmp(path + len - 3, "url", 3);
}

bool is_dir_empty(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) < 0) {
        return false;
    }
    if (!S_ISDIR(statbuf.st_mode)) {
        return false;
    }
    return statbuf.st_nlink <= 2;
}

bool validate_args(int argc, char *const argv[]) {
    if (argc < 3) {
        log_info("Usage: %s ISO_OR_DIR1 ISO_OR_DIR2 [INSTALL_DIR]\n\n", argv[0]);
        log_info("ISO_OR_DIR1 and ISO_OR_DIR2 must both be the appropriate path as an ISO image or "
                 "a path to its contents in a directory.\nISO_OR_DIR2 must contain the 'Audio' "
                 "directory from the second disc.\n");
        return false;
    }
    for (int i = 1; i < 3; i++) {
        if (!exists(argv[i])) {
            log_error("Argument '%s' does not exist.\n", argv[i]);
            return false;
        }
    }
    if (argc == 4) {
        if (is_file(argv[3])) {
            log_error("Argument '%s' is an existing file.\n", argv[3]);
            return false;
        }
        mkdir_p(argv[3]);
    }
    return true;
}

char *get_installation_dir() {
    char *install_dir = calloc(PATH_MAX, 1);
#ifdef __APPLE__
    char *home_dir = env("HOME");
    assert(strlen(home_dir) > 0);
    char path[PATH_MAX];
    sysdir_search_path_enumeration_state state = sysdir_start_search_path_enumeration(
        SYSDIR_DIRECTORY_APPLICATION_SUPPORT, SYSDIR_DOMAIN_MASK_USER);
    while ((state = sysdir_get_next_search_path_enumeration(state, path)) != 0) {
        if (state == 0) {
            log_error("Failed to get a valid directory.\n");
            return nullptr;
        }
        assert(path[0] != '\0');
        sprintf(install_dir, "%s%s/re3", home_dir, path + 1);
        break;
    }
#elif defined(_WIN32)
    char result[MAX_PATH - 5] = {0};
    wchar_t result_w[MAX_PATH - 5] = {0};
    SHGetFolderPath(nullptr, CSIDL_APPDATA | CSIDL_FLAG_CREATE, nullptr, 0, result_w);
    assert(result_w[0] != L'\0' && wcslen(result_w) < MAX_PATH - 5);
    int len = WideCharToMultiByte(CP_UTF8, 0, result_w, -1, result, MAX_PATH, nullptr, nullptr);
    assert(len > 0);
    sprintf(install_dir, "%s\\re3", result);
#else
    char *home_dir = env("HOME");
    assert(strlen(home_dir) > 0);
    char *xdg_data_home = env("XDG_DATA_HOME");
    if (strlen(xdg_data_home) == 0) {
        sprintf(install_dir, "%s/.local/share/re3", home_dir);
    } else {
        sprintf(install_dir, "%s/re3", xdg_data_home);
    }
    free(xdg_data_home);
#endif
    return install_dir;
}

bool copy_tree(const char *src, const char *dest) {
#ifdef HAVE_COPYFILE
    if (copyfile(src, dest, nullptr, COPYFILE_ALL | COPYFILE_RECURSIVE) != 0) {
        return false;
    }
#elif defined(HAVE_FTS_OPEN) && defined(HAVE_SENDFILE)
    FTS *fts = nullptr;
    FTSENT *node = nullptr;
    char *paths[] = {(char *)src, nullptr};
    fts = fts_open(paths, FTS_NOCHDIR | FTS_PHYSICAL, nullptr);
    if (!fts) {
        return false;
    }
    while ((node = fts_read(fts)) != nullptr) {
        switch (node->fts_info) {
        case FTS_D:
            // Create directory in destination
            {
                char dest_path[PATH_MAX];
                snprintf(dest_path, PATH_MAX, "%s/%s", dest, node->fts_name);
                if (mkdir_p(dest_path) < 0) {
                    fts_close(fts);
                    return false;
                }
            }
            break;

        case FTS_F:
            // Copy file to destination
            {
                char dest_path[PATH_MAX];
                snprintf(dest_path, PATH_MAX, "%s/%s", dest, node->fts_path + strlen(src) + 1);
                log_debug("%s -> %s\n", node->fts_path, dest_path);
                int fd_src = open(node->fts_path, O_RDONLY);
                if (fd_src < 0) {
                    fts_close(fts);
                    return false;
                }
                int fd_dest =
                    open(dest_path, O_WRONLY | O_CREAT | O_TRUNC, node->fts_statp->st_mode);
                if (fd_dest < 0) {
                    close(fd_src);
                    fts_close(fts);
                    return false;
                }
                off_t offset = 0;
                ssize_t bytes_copied;
                while ((bytes_copied =
                            sendfile(fd_dest, fd_src, &offset, node->fts_statp->st_size)) > 0)
                    ;
                if (bytes_copied < 0) {
                    close(fd_src);
                    close(fd_dest);
                    fts_close(fts);
                    return false;
                }
                close(fd_src);
                close(fd_dest);
            }
            break;

        case FTS_ERR:
        case FTS_NS:
            log_error("Error accessing %s: %s\n", node->fts_path, strerror(node->fts_errno));
            fts_close(fts);
            return false;

        default:
            break;
        }
    }
    fts_close(fts);
#elif defined(_WIN32)
    wchar_t *src_w = nullptr;
    wchar_t *dest_w = nullptr;
    int src_w_len = MultiByteToWideChar(CP_UTF8, 0, src, -1, nullptr, 0);
    int dest_w_len = MultiByteToWideChar(CP_UTF8, 0, dest, -1, nullptr, 0);
    if (src_w_len == 0 || dest_w_len == 0) {
        log_error("Failed to calculate wide string lengths.\n");
        return false;
    }
    src_w = calloc(src_w_len + 3, sizeof(wchar_t));
    dest_w = calloc(dest_w_len + 2, sizeof(wchar_t));
    if (!src_w || !dest_w) {
        log_error("Failed to allocate memory for wide strings.\n");
        free(src_w);
        free(dest_w);
        return false;
    }
    if (MultiByteToWideChar(CP_UTF8, 0, src, -1, src_w, src_w_len) == 0) {
        log_error("Failed to convert source path to wide string.\n");
        free(src_w);
        free(dest_w);
        return false;
    }
    if (MultiByteToWideChar(CP_UTF8, 0, dest, -1, dest_w, dest_w_len) == 0) {
        log_error("Failed to convert destination path to wide string.\n");
        free(src_w);
        free(dest_w);
        return false;
    }
    wcscat(src_w, L"\\*");
    src_w[src_w_len + 1] = L'\0';
    src_w[src_w_len + 2] = L'\0';
    dest_w[dest_w_len] = L'\0';
    dest_w[dest_w_len + 1] = L'\0';
#ifndef NDEBUG
    wprintf(L"Copying '%ls' (length: %d) to '%ls' (length: %d).\n",
            src_w,
            wcslen(src_w),
            dest_w,
            wcslen(dest_w));
#endif
    SHFILEOPSTRUCT file_op = {0};
    file_op.wFunc = FO_COPY;
    file_op.pFrom = src_w;
    file_op.pTo = dest_w;
    file_op.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_NOCONFIRMMKDIR;
    int ret = SHFileOperation(&file_op);
    if (file_op.fAnyOperationsAborted) {
        log_error("Failed to copy '%s' to '%s'; ret = %d.\n", src, dest, ret);
        log_error("At least one file operation was aborted.\n");
        free(src_w);
        free(dest_w);
        return false;
    }
    if (ret != 0) {
        log_error("Failed to copy '%s' to '%s'; ret = %d.\n", src, dest, ret);
        free(src_w);
        free(dest_w);
        return false;
    }
    free(src_w);
    free(dest_w);
#else
    DIR *dir = opendir(src);
    struct stat statbuf;
    if (stat(dest, &statbuf) < 0) {
        if (mkdir_p(dest) < 0) {
            closedir(dir);
            return false;
        }
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
            continue;
        }
        char src_path[PATH_MAX];
        char dest_path[PATH_MAX];
        snprintf(src_path, PATH_MAX, "%s/%s", src, entry->d_name);
        snprintf(dest_path, PATH_MAX, "%s/%s", dest, entry->d_name);
        if (stat(src_path, &statbuf) < 0) {
            closedir(dir);
            return false;
        }
        if (S_ISDIR(statbuf.st_mode)) {
            if (!copy_tree(src_path, dest_path)) {
                closedir(dir);
                return false;
            }
        } else if (S_ISREG(statbuf.st_mode)) {
            FILE *src_file = fopen(src_path, "rb");
            if (!src_file) {
                log_error("Failed to open source file.");
                closedir(dir);
                return false;
            }
            FILE *dest_file = fopen(dest_path, "wb");
            if (!dest_file) {
                fclose(src_file);
                closedir(dir);
                return false;
            }
            char buffer[8192];
            size_t bytes;
            while ((bytes = fread(buffer, 1, sizeof(buffer), src_file)) > 0) {
                if (fwrite(buffer, 1, bytes, dest_file) != bytes) {
                    fclose(src_file);
                    fclose(dest_file);
                    closedir(dir);
                    return false;
                }
            }
            fclose(src_file);
            fclose(dest_file);
        }
    }
    closedir(dir);
#endif
    return true;
}

bool remove_tree(const char *path) {
#if defined(HAVE_FTS_OPEN)
    FTS *fts = nullptr;
    FTSENT *node = nullptr;
    char *paths[] = {(char *)path, nullptr};

    fts = fts_open(paths, FTS_NOCHDIR | FTS_PHYSICAL | FTS_XDEV, nullptr);
    if (!fts) {
        perror("fts_open");
        return false;
    }

    while ((node = fts_read(fts)) != nullptr) {
        switch (node->fts_info) {
        case FTS_DP: // Post-order directory
            if (rmdir(node->fts_path) < 0) {
                perror("rmdir");
                fts_close(fts);
                return false;
            }
            break;

        case FTS_F:       // Regular file
        case FTS_SL:      // Symbolic link
        case FTS_SLNONE:  // Broken symbolic link
        case FTS_DEFAULT: // Other file types
            if (unlink(node->fts_path) < 0) {
                perror("unlink");
                fts_close(fts);
                return false;
            }
            break;

        case FTS_ERR: // Error
        case FTS_NS:  // Stat failed
            log_error("Error accessing %s: %s\n", node->fts_path, strerror(node->fts_errno));
            fts_close(fts);
            return false;

        default:
            break;
        }
    }

    fts_close(fts);
#elif defined(_WIN32)
    wchar_t *path_w;
    size_t path_len = strlen(path);
    int req_size = MultiByteToWideChar(CP_UTF8, 0, path, path_len, nullptr, 0);
    path_w = calloc(req_size + 1, sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, path, path_len, path_w, req_size);
    path_w[req_size] = L'\0';
    path_w[req_size + 1] = L'\0';
    SHFILEOPSTRUCT file_op = {0};
    file_op.wFunc = FO_DELETE;
    file_op.pFrom = path_w;
    file_op.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
    int ret = SHFileOperation(&file_op);
    free(path_w);
    if (ret != 0) {
        log_error("Failed to remove directory: %d\n", ret);
        return false;
    }
#else
    DIR *dir = opendir(path);
    if (!dir) {
        return false;
    }
    struct dirent *entry;
    char full_path[PATH_MAX];
    struct stat statbuf;
    while ((entry = readdir(dir)) != nullptr) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
            continue;
        }
        snprintf(full_path, PATH_MAX, "%s/%s", path, entry->d_name);
        if (stat(full_path, &statbuf) < 0) {
            closedir(dir);
            return false;
        }
        if (S_ISDIR(statbuf.st_mode)) {
            if (!remove_tree(full_path)) {
                closedir(dir);
                return false;
            }
        } else {
            if (unlink(full_path) < 0) {
                perror("unlink");
                closedir(dir);
                return false;
            }
        }
    }
    closedir(dir);
    if (rmdir(path) < 0) {
        return false;
    }
#endif
    return true;
}
