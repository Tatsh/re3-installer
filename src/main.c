#include <stdlib.h>

#include "env.h"
#include "extractor.h"
#include "installer.h"
#include "log.h"
#include "mkdir_p.h"
#include "support.h"
#include "utils.h"

int main(int argc, char *argv[]) {
    if (!validate_args(argc, argv)) {
        return 1;
    }
    bool ok = false;
    bool specified_install_dir = argc == 4;
    char *installation_dir = specified_install_dir ? argv[3] : get_installation_dir();
#ifdef _WIN32
    if (specified_install_dir) {
        installation_dir = _fullpath(nullptr, argv[3], MAX_PATH);
    }
#endif
    log_info("Installation directory: %s\n", installation_dir);
    ok = install_re3_game_data(argv[1], argv[2], installation_dir);
#ifndef _WIN32
    if (!specified_install_dir) {
        free(installation_dir);
    }
#endif
    return ok ? 0 : 1;
}
