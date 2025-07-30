#include <stdarg.h>
#include <stdio.h>

enum log_level {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_ERROR,
};

#ifdef NDEBUG
enum log_level g_log_level = LOG_LEVEL_INFO;
#else
enum log_level g_log_level = LOG_LEVEL_DEBUG;
#endif

void log_debug(const char *msg, ...) {
    // LCOV_EXCL_START
    if (g_log_level != LOG_LEVEL_DEBUG) {
        return;
    }
    // LCOV_EXCL_STOP
    va_list ap;
    va_start(ap, msg);
    vfprintf(stderr, msg, ap);
    va_end(ap);
}

void log_error(const char *msg, ...) {
    va_list ap;
    va_start(ap, msg);
    vfprintf(stderr, msg, ap);
    va_end(ap);
}

void log_info(const char *msg, ...) {
    // LCOV_EXCL_START
    if (g_log_level > LOG_LEVEL_INFO) {
        return;
    }
    // LCOV_EXCL_STOP
    va_list ap;
    va_start(ap, msg);
    vfprintf(stdout, msg, ap);
    va_end(ap);
}
