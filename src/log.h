#ifndef RE3_INSTALLER_LOG_H
#define RE3_INSTALLER_LOG_H

#ifdef __GNUC__
#define ATTR_FORMAT_PRINTF_1_2 __attribute__((format(printf, 1, 2)))
#else
#define ATTR_FORMAT_PRINTF_1_2
#endif

void log_debug(const char *msg, ...) ATTR_FORMAT_PRINTF_1_2;
void log_error(const char *msg, ...) ATTR_FORMAT_PRINTF_1_2;
void log_info(const char *msg, ...) ATTR_FORMAT_PRINTF_1_2;

#endif
