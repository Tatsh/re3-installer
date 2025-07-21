#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <libunshield.h>

#include "env.h"
#include "extractor.h"
#include "helpers.h"
#include "log.h"
#include "mkdir_p.h"
#include "support.h"

static bool cd1_filter(const char *path) {
    return !strncmp(path, "data", 4);
}

static bool cd2_filter(const char *path) {
    return !strncmp(path, "Audio", 5);
}

bool install_re3_game_data(const char *disc1_path,
                           const char *disc2_path,
                           const char *installation_dir) {
    bool err = false;
    char *disc1_out_dir = nullptr;
    char *disc2_out_dir = nullptr;
    bool iso_mode_disc1 = false;
    bool iso_mode_disc2 = false;
    if (!exists(installation_dir)) {
        mkdir(installation_dir, S_IRWXU);
    }
    if (!is_dir_empty(installation_dir)) {
        log_error("Installation directory '%s' is not empty.\n", installation_dir);
        err = true;
        goto cleanup;
    }
    if (is_iso(disc1_path)) {
        iso_mode_disc1 = true;
        if (!extract_iso_to_temp(disc1_path, &disc1_out_dir, cd1_filter)) {
            log_error("Failed to extract disc 1.\n");
            return 1;
        }
    }
    if (is_iso(disc2_path)) {
        iso_mode_disc2 = true;
        if (!extract_iso_to_temp(disc2_path, &disc2_out_dir, cd2_filter)) {
            log_error("Failed to extract disc 2.\n");
            return 1;
        }
    }
    // Extract data1.cab
    log_info("Copying files from disc 1...\n");
    char data1_cab_path[PATH_MAX];
    sprintf(data1_cab_path, "%s/data1.cab", iso_mode_disc1 ? disc1_out_dir : disc1_path);
    log_debug("Using data1.cab: %s\n", data1_cab_path);
    if (!unshield_extract(data1_cab_path, installation_dir)) {
        log_error("Failed to extract data1.cab.\n");
        err = true;
        goto cleanup;
    }
    // Copy disc 2 Audio
    log_info("Copying audio files from disc 2...\n");
    char src_audio_dir[PATH_MAX];
    char dest_audio_dir[PATH_MAX];
    sprintf(dest_audio_dir, "%s/Audio", installation_dir);
    sprintf(src_audio_dir, "%s/Audio", iso_mode_disc2 ? disc2_out_dir : disc2_path);
    if (!copy_tree(src_audio_dir, dest_audio_dir)) {
        log_error("Failed to copy audio files from '%s' to '%s'.\n", src_audio_dir, dest_audio_dir);
        err = true;
        goto cleanup;
    }
cleanup:
    if (disc1_out_dir) {
        log_debug("Removing temporary directory for disc 1: %s\n", disc1_out_dir);
        if (!remove_tree(disc1_out_dir)) {
            log_error("Failed to remove temporary directory for disc 1: %s\n", disc1_out_dir);
            err = true;
        }
    }
    if (disc2_out_dir) {
        log_debug("Removing temporary directory for disc 2: %s\n", disc2_out_dir);
        if (!remove_tree(disc2_out_dir)) {
            log_error("Failed to remove temporary directory for disc 2: %s\n", disc2_out_dir);
            err = true;
        }
    }
    free(disc1_out_dir);
    free(disc2_out_dir);
    return err ? 1 : 0;
}
