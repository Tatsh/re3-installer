#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "env.h"

char *env(const char *var_name) {
#ifdef _WIN32
    char *value = getenv(var_name);
    char *result = calloc(PATH_MAX, 1);
    if (!value) {
        return result;
    }
    strcpy(result, value);
    return result;
#else
    char **p = environ;
    char *value = calloc(PATH_MAX, 1);
    assert(value != nullptr);
    size_t var_name_len = strlen(var_name);
    char *search = calloc(var_name_len + 2, 1);
    strcpy(search, var_name);
    strcat(search, "=");
    for (; *p; p++) {
        if (!strncmp(search, *p, var_name_len + 1)) {
            strcpy(value, *p + var_name_len + 1);
            break;
        }
    }
    free(search);
    return value;
#endif
}
