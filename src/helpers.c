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

bool is_iso(const char *path) {
    if (!strncmp(path, "iso", 3)) {
        return true;
    }
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
        perror("Failed to stat directory");
        return false;
    }
    if (!S_ISDIR(statbuf.st_mode)) {
        fprintf(stderr, "Path '%s' is not a directory.\n", path);
        return false;
    }
    return statbuf.st_nlink == 2;
}

bool validate_args(int argc, char *const argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s ISO_OR_DIR1 ISO_OR_DIR2 [INSTALL_DIR]\n\n", argv[0]);
        fprintf(stderr,
                "ISO_OR_DIR1 must be the first disc as an ISO image or its contents in a "
                "directory.\n");
        fprintf(stderr,
                "ISO_OR_DIR2 must be the second disc as an ISO image or its contents in a "
                "directory.\n");
        fprintf(stderr,
                "ISO_OR_DIR1 must contain data1.cab, data1.hdr, and data2.cab from first disc.\n");
        fprintf(stderr, "DIR2 must contain the 'Audio' directory from the second disc.\n");
        return false;
    }
    for (int i = 1; i < 3; i++) {
        if (!exists(argv[i])) {
            fprintf(stderr, "Argument '%s' does not exist.\n", argv[i]);
            return false;
        }
    }
    if (argc == 4) {
        if (is_file(argv[3])) {
            fprintf(stderr, "Argument '%s' is an existing file.\n", argv[3]);
            return false;
        }
        mkdir_p(argv[3]);
    }
    return true;
}

char *get_installation_dir() {
    char *install_dir = malloc(PATH_MAX);
    char *home_dir = env("HOME");
#ifdef __APPLE__
    char path[PATH_MAX];
    sysdir_search_path_enumeration_state state = sysdir_start_search_path_enumeration(
        SYSDIR_DIRECTORY_APPLICATION_SUPPORT, SYSDIR_DOMAIN_MASK_USER);
    while ((state = sysdir_get_next_search_path_enumeration(state, path)) != 0) {
        if (state == 0) {
            fprintf(stderr, "Failed to get a valid directory.\n");
            return nullptr;
        }
        assert(path[0] != '\0');
        sprintf(install_dir, "%s%s/re3", home_dir, path + 1);
        break;
    }
#else
    char *xdg_data_home = env("XDG_DATA_HOME");
    if (strlen(xdg_data_home) == 0) {
        assert(strlen(home_dir) > 0);
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
    return copyfile(src, dest, nullptr, COPYFILE_ALL | COPYFILE_RECURSIVE) == 0;
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
                snprintf(dest_path, PATH_MAX, "%s/%s", dest, node->fts_path + strlen(src) + 1);
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
            fprintf(stderr, "Error accessing %s: %s\n", node->fts_path, strerror(node->fts_errno));
            fts_close(fts);
            return false;

        default:
            break;
        }
    }

    fts_close(fts);

    return true;
#elif defined(_WIN32)
    SHFILEOPSTRUCT file_op = {0};
    file_op.wFunc = FO_COPY;
    file_op.pFrom = src;
    file_op.pTo = dest;
    file_op.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
    int ret = SHFileOperation(&file_op);
    if (ret != 0) {
        return false;
    }
    return true;
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
    while ((entry = readdir(dir)) != NULL) {
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
                perror("Failed to open source file");
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
    return true;
#endif
}

bool remove_tree(const char *path) {
#if defined(HAVE_FTS_OPEN)
    FTS *fts = NULL;
    FTSENT *node = NULL;
    char *paths[] = {(char *)path, NULL};

    fts = fts_open(paths, FTS_NOCHDIR | FTS_PHYSICAL | FTS_XDEV, NULL);
    if (!fts) {
        perror("fts_open");
        return false;
    }

    while ((node = fts_read(fts)) != NULL) {
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
            fprintf(stderr, "Error accessing %s: %s\n", node->fts_path, strerror(node->fts_errno));
            fts_close(fts);
            return false;

        default:
            break;
        }
    }

    fts_close(fts);
#elif defined(_WIN32)
    SHFILEOPSTRUCT file_op = {0};
    file_op.wFunc = FO_DELETE;
    file_op.pFrom = path;
    file_op.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT | FOF_ALLOWUNDO;
    int ret = SHFileOperation(&file_op);
    if (ret != 0) {
        fprintf(stderr, "Failed to remove directory: %s\n", strerror(ret));
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
    while ((entry = readdir(dir)) != NULL) {
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
