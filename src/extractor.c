#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <limits.h>

#include <archive.h>
#include <archive_entry.h>
#include <libunshield.h>

#include "env.h"
#include "extractor.h"
#include "helpers.h"
#include "mkdir_p.h"

bool extract_archive_to_temp(const char *archive_path, char **output_dir) {
    struct archive *archive = archive_read_new();
    struct archive_entry *entry;
    int r;
    bool ret = true;

    char *template = env("TMPDIR");
    if (!strlen(template)) {
        memset(template, 0, PATH_MAX);
        strcpy(template, "/tmp/re3.XXXXXX");
    } else {
        strcat(template, "/re3.XXXXXX");
    }
    *output_dir = mkdtemp(template);
    free(template);
    if (!output_dir) {
        fprintf(stderr, "Failed to create temporary directory.\n");
        perror(NULL);
        ret = false;
        goto cleanup;
    }

    archive_read_support_format_iso9660(archive);

    if ((r = archive_read_open_filename(archive, archive_path, 10240)) != ARCHIVE_OK) {
        fprintf(
            stderr, "Could not open archive %s: %s\n", archive_path, archive_error_string(archive));
        ret = false;
        goto cleanup;
    }

    while (archive_read_next_header(archive, &entry) == ARCHIVE_OK) {
        const char *current_file = archive_entry_pathname(entry);
        char full_path[PATH_MAX];

        snprintf(full_path, sizeof(full_path), "%s/%s", *output_dir, current_file);
        archive_entry_set_pathname(entry, full_path);

        if ((r = archive_write_header(archive, entry)) != ARCHIVE_OK) {
            fprintf(stderr,
                    "Could not write header for %s: %s\n",
                    current_file,
                    archive_error_string(archive));
            continue;
        }

        const void *buff;
        size_t size;
        la_int64_t offset;

        while ((r = archive_read_data_block(archive, &buff, &size, &offset)) == ARCHIVE_OK) {
            if (archive_write_data(archive, buff, size) < 0) {
                fprintf(stderr,
                        "Could not write data for %s: %s\n",
                        current_file,
                        archive_error_string(archive));
                break;
            }
        }
    }

cleanup:
    archive_read_free(archive);
    return ret;
}

static const char *FILE_GROUPS[] = {"App Executables", "Don't Delete", NULL};

bool unshield_extract(const char *cab_path, const char *installation_dir) {
    bool ret = true;
    unshield_set_log_level(UNSHIELD_LOG_LEVEL_ERROR);
    Unshield *unshield = unshield_open(cab_path);
    if (!unshield) {
        fprintf(stderr, "Failed to open %s.\n", cab_path);
        goto cleanup;
    }
    for (unsigned long group_index = 0;
         group_index < ((sizeof FILE_GROUPS / sizeof FILE_GROUPS[0]) - 1);
         group_index++) {
        UnshieldFileGroup *group = unshield_file_group_find(unshield, FILE_GROUPS[group_index]);
        if (!group) {
            fprintf(stderr, "Could not find %s group.\n", FILE_GROUPS[group_index]);
            ret = false;
            goto cleanup;
        }
        char *target_dir = malloc(PATH_MAX);
        for (unsigned long i = group->first_file; i <= group->last_file; i++) {
            if (unshield_file_is_valid(unshield, (int)i)) {
                const char *name = unshield_file_name(unshield, (int)i);
                if (ends_with_exe(name) || ends_with_dll(name) || ends_with_url(name)) {
                    continue;
                }
                char *dir = (char *)unshield_directory_name(
                    unshield, unshield_file_directory(unshield, (int)i));
                for (unsigned long j = 0; j < strlen(dir); j++) {
                    if (dir[j] == '\\') {
                        dir[j] = '/';
                    }
                }
                memset(target_dir, 0, PATH_MAX);
                if (group_index == 0 && !strcmp(dir, "audio")) { // Keep casing consistent
                    dir[0] = 'A';
                }
                int ret =
                    sprintf(target_dir, "%s%s%s", installation_dir, dir ? "/" : "", dir ? dir : "");
                assert(ret > 0);
                if (mkdir_p(target_dir) < 0) {
                    if (errno != EEXIST) {
                        fprintf(stderr, "Failed to create directory %s: ", target_dir);
                        perror(NULL);
                        ret = false;
                        goto cleanup;
                    }
                }
                char *target_path = malloc(PATH_MAX);
                memset(target_path, 0, PATH_MAX);
                ret = sprintf(target_path, "%s/%s", target_dir, name);
                assert(ret > 0);
                bool unshield_ret = unshield_file_save(unshield, (int)i, target_path);
                assert(unshield_ret);
                free(target_path);
            }
        }
        free(target_dir);
    }
cleanup:
    unshield_close(unshield);
    return ret;
}
