#include "env.h"

#include <stdlib.h>
#include <string.h>

#include <limits.h>

char *env(const char *varName) {
  char **p = environ;
  char *value = malloc(PATH_MAX);
  memset(value, 0, PATH_MAX);
  size_t varNameLength = strlen(varName);
  char *search = malloc(varNameLength + 2);
  memset(search, 0, varNameLength + 2);
  strcpy(search, varName);
  strcat(search, "=");
  for (; *p; p++) {
    if (!strncmp(search, *p, varNameLength + 1)) {
      strcpy(value, *p + varNameLength + 1);
      break;
    }
  }
  free(search);
  return value;
}
