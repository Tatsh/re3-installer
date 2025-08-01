#include <errno.h>
#include <limits.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "mkdir_p.h"

/** From
 * https://gist.github.com/JonathonReinhart/8c0d90191c38af2dcadb102c4e202950 */
int mkdir_p(const char *path) {
    /* Adapted from http://stackoverflow.com/a/2336245/119527 */
    const size_t len = strlen(path);
    char _path[PATH_MAX];
    char *p;

    errno = 0;

    /* Copy string so its mutable */
    if (len > sizeof(_path) - 1) {
        errno = ENAMETOOLONG;
        return -1;
    }
    strcpy(_path, path);

    /* Iterate the string */
    for (p = _path + 1; *p; p++) {
        if (*p == '/') {
            /* Temporarily truncate */
            *p = '\0';

            if (mkdir(_path, S_IRWXU) != 0 && errno != EEXIST) {
                return -1;
            }

            *p = '/';
        }
    }

    if (mkdir(_path, S_IRWXU) != 0 && errno != EEXIST) {
        return -1;
    }

    return 0;
}
