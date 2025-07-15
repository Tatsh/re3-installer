#include <stdlib.h>
#include <string.h>

#include <limits.h>

#include "env.h"

char *env(const char *var_name) {
    char **p = environ;
    char *value = malloc(PATH_MAX);
    memset(value, 0, PATH_MAX);
    size_t var_name_len = strlen(var_name);
    char *search = malloc(var_name_len + 2);
    memset(search, 0, var_name_len + 2);
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
}
