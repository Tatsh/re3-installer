#ifndef RE3_INSTALLER_ENV_H
#define RE3_INSTALLER_ENV_H

#ifdef __APPLE__
#include <crt_externs.h>
#define environ (*_NSGetEnviron())
#elif !defined(WIN32) || defined(_MSC_VER)
extern char **environ;
#endif

char *env(const char *);

#endif // RE3_INSTALLER_ENV_H
