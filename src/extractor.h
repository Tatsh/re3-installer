#ifndef RE3_INSTALLER_EXTRACTOR_H
#define RE3_INSTALLER_EXTRACTOR_H

#include "support.h"

bool extract_iso_to_temp(const char *archive_path, char **output_dir);
bool unshield_extract(const char *cab_path, const char *installation_dir);

#endif // RE3_INSTALLER_EXTRACTOR_H
