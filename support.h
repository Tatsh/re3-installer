#ifndef support_h
#define support_h

#ifdef _MSC_VER
#include <direct.h>
#define mkdir _mkdir
#define strncasecmp _strnicmp
#endif

#endif /* support_h */
