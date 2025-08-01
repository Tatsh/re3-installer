#ifndef RE3_INSTALLER_EXTRACTOR_H
#define RE3_INSTALLER_EXTRACTOR_H

#include "support.h"

typedef bool (*should_extract_callback_t)(const char *);
bool extract_iso_to_temp(const char *archive_path,
                         char **output_dir,
                         should_extract_callback_t should_extract) ATTR_NONNULL;
bool unshield_extract(const char *cab_path, const char *installation_dir) ATTR_NONNULL;

#endif // RE3_INSTALLER_EXTRACTOR_H
