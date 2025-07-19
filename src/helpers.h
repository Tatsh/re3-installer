#ifndef RE3_INSTALLER_HELPERS_H
#define RE3_INSTALLER_HELPERS_H

#include <stdbool.h>

bool copy_tree(const char *src, const char *dest);
bool ends_with_dll(const char *);
bool ends_with_exe(const char *);
bool ends_with_url(const char *);
bool exists(const char *);
bool is_directory(const char *);
bool is_dir_empty(const char *path);
bool is_file(const char *);
bool is_iso(const char *);
bool remove_tree(const char *path);
bool validate_args(int argc, char *const argv[]);
char *get_installation_dir();

#endif // RE3_INSTALLER_HELPERS_H
