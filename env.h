#ifndef env_h
#define env_h

#ifdef __APPLE__
#include <crt_externs.h>
#include <sysdir.h>
#define environ (*_NSGetEnviron())
#else
extern char **environ;
#endif

char *env(const char *);

#endif /* env_h */
