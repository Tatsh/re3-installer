#ifndef RE3_INSTALLER_INSTALLER_H
#define RE3_INSTALLER_INSTALLER_H

#include "support.h"

bool install_re3_game_data(const char *disc1_path,
                           const char *disc2_path,
                           const char *install_dir) ATTR_NONNULL;

#endif
