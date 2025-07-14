#include "helpers.h"

#include <string.h>

#include <sys/stat.h>

bool is_file(const char *path) {
    struct stat statbuf;
    int ret = stat(path, &statbuf);
    if (ret < 0) {
        return false;
    }
    return (statbuf.st_mode & S_IFMT) == S_IFREG;
}

bool is_directory(const char *path) {
    struct stat statbuf;
    int ret = stat(path, &statbuf);
    if (ret < 0) {
        return false;
    }
    return (statbuf.st_mode & S_IFMT) == S_IFDIR;
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
