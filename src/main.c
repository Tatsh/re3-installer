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
#include "mkdir_p.h"
#include "support.h"

int main(int argc, char *argv[]) {
    if (!validate_args(argc, argv)) {
        return 1;
    }

    char *disc1_out_dir = nullptr;
    char *disc2_out_dir = nullptr;
    char *dir1 = strdup(argv[1]);
    char *dir2 = strdup(argv[2]);
    bool iso_mode = false;

    if (is_iso(argv[1]) && is_iso(argv[2])) {
        iso_mode = true;
        if (!extract_archive_to_temp(argv[1], &disc1_out_dir)) {
            fprintf(stderr, "Failed to extract disc 1.\n");
            return 1;
        }
        if (!extract_archive_to_temp(argv[2], &disc2_out_dir)) {
            fprintf(stderr, "Failed to extract disc 2.\n");
            return 1;
        }
    }

    // Extract data1.cab
    char *installation_dir = get_installation_dir();

    char *data1_cab_path = malloc(PATH_MAX);
    memset(data1_cab_path, 0, PATH_MAX);
    sprintf(data1_cab_path, "%s/data1.cab", iso_mode ? disc1_out_dir : dir1);

    unshield_extract(data1_cab_path, installation_dir);

    free(data1_cab_path);
    free(installation_dir);

    // Copy disc 2 Audio
    char *audioDir = malloc(PATH_MAX);
    sprintf(audioDir, "%s/Audio/", iso_mode ? disc2_out_dir : dir2);
    copy_tree(audioDir, "");
    // Clean up
    // if (iso_mode) {
    //     pid = fork();
    //     if (pid == 0) {
    //         execlp("rm", "rm", "-fR", dir2, NULL);
    //         _exit(EXIT_SUCCESS);
    //     } else if (pid < 0) {
    //         status = -1;
    //     } else {
    //         if (waitpid(pid, &status, 0) != pid) {
    //             perror(NULL);
    //         }
    //     }
    // }
    free(dir1);
    free(dir2);
    free(audioDir);
    free(data1_cab_path);
    return 0;
}
