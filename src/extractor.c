#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>

#include <cdio/cdio.h>
#include <cdio/iso9660.h>
#include <cdio/logging.h>
#include <libunshield.h>

#include "env.h"
#include "extractor.h"
#include "helpers.h"
#include "mkdir_p.h"
#include "support.h"

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef HAVE_ISO9660_STAT_T_TOTAL_SIZE
#define iso9660_stat_t_total_size(p_statbuf) ((p_statbuf)->size)
#else
#define iso9660_stat_t_total_size(p_statbuf) ((p_statbuf)->total_size)
#endif

// Taken from https://github.com/libcdio/libcdio/blob/master/example/extract.c
static int iso_extract_files(iso9660_t *p_iso,
                             const char *psz_path,
                             const char *psz_extract_dir,
                             uint8_t i_joliet_level) {
    FILE *fd = NULL;
    int i_length, r = 1;
    char psz_fullpath[PATH_MAX], *psz_basename;
    const char *psz_iso_name = &psz_fullpath[strlen(psz_extract_dir)];
    unsigned char buf[ISO_BLOCKSIZE];
    CdioListNode_t *p_entnode;
    iso9660_stat_t *p_statbuf;
    CdioISO9660FileList_t *p_entlist;
    size_t i;
    lsn_t lsn;
    int64_t i_file_length;
    if ((p_iso == NULL) || (psz_path == NULL))
        return 1;
    i_length = snprintf(psz_fullpath, sizeof(psz_fullpath), "%s%s/", psz_extract_dir, psz_path);
    if (i_length < 0)
        return 1;
    psz_basename = &psz_fullpath[i_length];
    p_entlist = iso9660_ifs_readdir(p_iso, psz_path);
    if (!p_entlist) {
        printf("Could not access %s\n", psz_path);
        return 1;
    }
    _CDIO_LIST_FOREACH(p_entnode, p_entlist) {
        p_statbuf = (iso9660_stat_t *)_cdio_list_node_data(p_entnode);
        /* Eliminate . and .. entries */
        if ((strcmp(p_statbuf->filename, ".") == 0) || (strcmp(p_statbuf->filename, "..") == 0))
            continue;
        iso9660_name_translate_ext(p_statbuf->filename, psz_basename, i_joliet_level);
        if (p_statbuf->type == _STAT_DIR) {
            mkdir(psz_fullpath, S_IRWXU);
            if (iso_extract_files(p_iso, psz_iso_name, psz_extract_dir, i_joliet_level))
                goto out;
        } else {
            fprintf(stderr, "Extracting: %s\n", psz_fullpath);
            fd = fopen(psz_fullpath, "wb");
            if (fd == NULL) {
                fprintf(stderr, "Unable to create file\n");
                goto out;
            }
            i_file_length = iso9660_stat_t_total_size(p_statbuf);
            for (i = 0; i_file_length > 0; i++) {
                memset(buf, 0, ISO_BLOCKSIZE);
                lsn = p_statbuf->lsn + i;
                if (iso9660_iso_seek_read(p_iso, buf, lsn, 1) != ISO_BLOCKSIZE) {
                    fprintf(stderr,
                            "Error reading ISO9660 file %s at LSN %lu\n",
                            psz_iso_name,
                            (long unsigned int)lsn);
                    goto out;
                }
                fwrite(buf, (size_t)MIN(i_file_length, ISO_BLOCKSIZE), 1, fd);
                if (ferror(fd)) {
                    fprintf(stderr, "Error writing file %s: %s\n", psz_iso_name, strerror(errno));
                    goto out;
                }
                i_file_length -= ISO_BLOCKSIZE;
            }
            fclose(fd);
            fd = NULL;
        }
    }
    r = 0;
out:
    if (fd != NULL)
        fclose(fd);
    iso9660_filelist_free(p_entlist);
    return r;
}

bool extract_iso_to_temp(const char *iso_path, char **output_dir) {
    bool ret = true;
    char *template = env("TMPDIR");
    if (!strlen(template)) {
        memset(template, 0, PATH_MAX);
        strcpy(template, "/tmp/re3.XXXXXX");
    } else {
        strcat(template, "/re3.XXXXXX");
    }
    *output_dir = mkdtemp(template);
    if (!output_dir) {
        fprintf(stderr, "Failed to create temporary directory.\n");
        return false;
    }
    fprintf(stderr, "Extracting ISO %s to '%s'.\n", iso_path, *output_dir);
    iso9660_t *iso = iso9660_open_ext(iso_path, ISO_EXTENSION_ALL);
    if (!iso) {
        fprintf(stderr, "Failed to open ISO file `%s`.\n", iso_path);
        return false;
    }
    uint8_t i_joliet_level = iso9660_ifs_get_joliet_level(iso);
    ret = !iso_extract_files(iso, "", *output_dir, i_joliet_level);
    iso9660_close(iso);
    return ret;
}

static const char *FILE_GROUPS[] = {"App Executables", "Don't Delete", nullptr};

bool unshield_extract(const char *cab_path, const char *installation_dir) {
    bool ret = true;
    unshield_set_log_level(UNSHIELD_LOG_LEVEL_ERROR);
    Unshield *unshield = unshield_open(cab_path);
    if (!unshield) {
        fprintf(stderr, "Failed to open %s.\n", cab_path);
        ret = false;
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
                int ret2 =
                    sprintf(target_dir, "%s%s%s", installation_dir, dir ? "/" : "", dir ? dir : "");
                assert(ret2 > 0);
                if (mkdir_p(target_dir) < 0) {
                    if (errno != EEXIST) {
                        fprintf(stderr, "Failed to create directory %s: ", target_dir);
                        ret = false;
                        goto cleanup;
                    }
                }
                char *target_path = malloc(PATH_MAX);
                memset(target_path, 0, PATH_MAX);
                ret2 = sprintf(target_path, "%s/%s", target_dir, name);
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
