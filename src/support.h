#ifndef RE3_INSTALLER_SUPPORT_H
#define RE3_INSTALLER_SUPPORT_H

#ifndef HAVE_NULLPTR
#define nullptr NULL
#endif

#ifdef HAVE_STDBOOL
#include <stdbool.h>
#else
#define bool int
#define true 1
#define false 0
#endif

#ifdef HAVE_FTS_OPEN
#include <fts.h>
#endif

#ifdef HAVE_SENDFILE
#include <sys/sendfile.h>
#endif

#ifdef HAVE_COPYFILE
#include <copyfile.h>
#endif

#ifdef _WIN32
#include <windows.h>

#include <direct.h>
#include <io.h>
#include <rpc.h>
#include <shellapi.h>
#include <shlobj.h>
#include <wchar.h>
#define mkdir(p, _) _mkdir(p)
#define open _open
#define strncasecmp _strnicmp
#define stat _stat
#endif

#ifdef __APPLE__
#include <sysdir.h>
#endif

#endif // RE3_INSTALLER_SUPPORT_H
