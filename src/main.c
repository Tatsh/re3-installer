#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <libunshield.h>

#include "env.h"
#include "extractor.h"
#include "helpers.h"
#include "installer.h"
#include "mkdir_p.h"
#include "support.h"

int main(int argc, char *argv[]) {
    if (!validate_args(argc, argv)) {
        return 1;
    }
    bool ok = false;
    bool specified_install_dir = argc == 4;
    char *installation_dir = specified_install_dir ? argv[3] : get_installation_dir();
    fprintf(stderr, "Installation directory: %s\n", installation_dir);
    ok = install_re3_game_data(argv[1], argv[2], installation_dir);
    if (!specified_install_dir) {
        free(installation_dir);
    }
    return ok ? 0 : 1;
}
