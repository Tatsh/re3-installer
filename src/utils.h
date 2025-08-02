#ifndef RE3_INSTALLER_HELPERS_H
#define RE3_INSTALLER_HELPERS_H

#include "support.h"

bool ends_with_dll(const char *) ATTR_NONNULL;
bool ends_with_exe(const char *) ATTR_NONNULL;
bool ends_with_iso(const char *) ATTR_NONNULL;
bool ends_with_url(const char *) ATTR_NONNULL;
bool exists(const char *) ATTR_NONNULL;
bool is_dir_empty(const char *) ATTR_NONNULL;
bool is_directory(const char *) ATTR_NONNULL;
bool is_file(const char *) ATTR_NONNULL;
bool copy_tree(const char *src, const char *dest) ATTR_NONNULL;
bool remove_tree(const char *path) ATTR_NONNULL;
bool validate_args(int argc, char *const argv[]) ATTR_NONNULL;
char *get_installation_dir() ATTR_RETURNS_NONNULL;

#endif // RE3_INSTALLER_HELPERS_H
