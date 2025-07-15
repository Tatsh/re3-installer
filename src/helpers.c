#include <assert.h>
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#ifdef __linux__
#include <sys/sendfile.h>
#elif defined(__APPLE__)
#include <copyfile.h>
#elif defined(_WIN32)
#include <shellapi.h>
#endif
#include <sys/stat.h>
#include <unistd.h>

#include "env.h"
#include "helpers.h"
#include "mkdir_p.h"

bool exists(const char *path) {
    struct stat statbuf;
    int ret = stat(path, &statbuf);
    if (ret < 0) {
        perror(NULL);
        return false;
    }
    return S_ISREG(statbuf.st_mode) || S_ISDIR(statbuf.st_mode);
}

bool is_file(const char *path) {
    struct stat statbuf;
    int ret = stat(path, &statbuf);
    if (ret < 0) {
        perror(NULL);
        return false;
    }
    return S_ISREG(statbuf.st_mode);
}

bool is_directory(const char *path) {
    struct stat statbuf;
    int ret = stat(path, &statbuf);
    if (ret < 0) {
        perror(NULL);
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

bool validate_args(int argc, char *const argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s ISO1|DIR1 ISO2|DIR2\n", argv[0]);
        fprintf(stderr, "\nISO1 must be the first disc as an ISO image.\n");
        fprintf(stderr, "ISO2 must be the second disc as an ISO image.\n");
        fprintf(stderr,
                "DIR1 must be the directory containing data1.cab, data1.hdr, and data2.cab from "
                "the first disc.\n");
        fprintf(
            stderr,
            "DIR2 must be the directory containing the Audio directory from the second disc.\n");
        fprintf(stderr, "\nBoth arguments must be of the same type.\n");
        return false;
    }
    if ((!is_iso(argv[1]) && is_iso(argv[2])) ||
        (!is_directory(argv[1]) && is_directory(argv[2]))) {
        fprintf(stderr, "\nBoth arguments must be of the same type.\n");
        return false;
    }
    for (int i = 1; i < argc; i++) {
        if (!exists(argv[i])) {
            fprintf(stderr, "Argument '%s' does not exist.\n", argv[i]);
            return false;
        }
    }
    return true;
}

char *get_installation_dir() {
    char *install_dir = malloc(PATH_MAX);
    char *home_dir = env("HOME");
#if defined(__APPLE__)
    char path[PATH_MAX];
    sysdir_search_path_enumeration_state state = sysdir_start_search_path_enumeration(
        SYSDIR_DIRECTORY_APPLICATION_SUPPORT, SYSDIR_DOMAIN_MASK_USER);
    while ((state = sysdir_get_next_search_path_enumeration(state, path)) != 0) {
        if (state == 0) {
            fprintf(stderr, "Failed to get a valid directory.\n");
            return 1;
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

static int
copy_callback(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftw_buf) {
    char dest_path[PATH_MAX];
    snprintf(dest_path, PATH_MAX, "%s/%s", "", fpath + ftw_buf->base);

    if (typeflag == FTW_D && mkdir_p(dest_path) < 0) {
        return -1;
    } else if (typeflag == FTW_F) {
        int fd_src = open(fpath, O_RDONLY);
        if (fd_src < 0) {
            perror("Failed to open source file");
            return -1;
        }
        int fd_dest = open(dest_path, O_WRONLY | O_CREAT | O_TRUNC, sb->st_mode);
        if (fd_dest < 0) {
            perror(nullptr);
            close(fd_src);
            return -1;
        }
        off_t offset = 0;
        ssize_t bytes_copied;
        while ((bytes_copied = sendfile(fd_dest, fd_src, &offset, sb->st_size)) > 0)
            ;
        if (bytes_copied < 0) {
            perror(nullptr);
            close(fd_src);
            close(fd_dest);
            return -1;
        }
        close(fd_src);
        close(fd_dest);
    }
    return 0;
}

bool copy_tree(const char *src, const char *dest) {
#ifdef __linux__
    return nftw(src, copy_callback, 20, FTW_DEPTH) == 0;
#elif defined(__APPLE__)
    return copyfile(src, dest, NULL, COPYFILE_ALL | COPYFILE_RECURSIVE) == 0;
#elif defined(_WIN32)
    SHFILEOPSTRUCT file_op = {0};
    file_op.wFunc = FO_COPY;
    file_op.pFrom = src;
    file_op.pTo = dest;
    file_op.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
    int ret = SHFileOperation(&file_op);
    if (ret != 0) {
        fprintf(stderr, "Failed to copy directory: %d.\n", ret);
        return false;
    }
    return true;
#endif
}
