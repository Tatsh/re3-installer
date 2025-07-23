#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cdio/iso9660.h>
#include <libunshield.h>

#include "env.h"
#include "extractor.h"
#include "helpers.h"
#include "log.h"
#include "mkdir_p.h"

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
                             uint8_t i_joliet_level,
                             should_extract_callback_t should_extract) {
    FILE *fd = nullptr;
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
    if (p_iso == nullptr || psz_path == nullptr) {
        return 1;
    }
    i_length = snprintf(psz_fullpath, sizeof(psz_fullpath), "%s%s/", psz_extract_dir, psz_path);
    if (i_length < 0) {
        return 1;
    }
    psz_basename = &psz_fullpath[i_length];
    p_entlist = iso9660_ifs_readdir(p_iso, psz_path);
    if (!p_entlist) {
        log_error("Could not access %s\n.", psz_path);
        return 1;
    }
    _CDIO_LIST_FOREACH(p_entnode, p_entlist) {
        p_statbuf = (iso9660_stat_t *)_cdio_list_node_data(p_entnode);
        /* Eliminate . and .. entries */
        if (strcmp(p_statbuf->filename, ".") == 0 || strcmp(p_statbuf->filename, "..") == 0) {
            continue;
        }
        iso9660_name_translate_ext(p_statbuf->filename, psz_basename, i_joliet_level);
        if (!should_extract(psz_iso_name + 1)) {
            log_debug("Ignoring file: %s\n", psz_basename);
            continue;
        }
        if (p_statbuf->type == _STAT_DIR) {
            mkdir(psz_fullpath, S_IRWXU);
            if (iso_extract_files(
                    p_iso, psz_iso_name, psz_extract_dir, i_joliet_level, should_extract)) {
                goto out;
            }
        } else {
            log_debug("Extracting: %s\n", psz_iso_name + 1);
            fd = fopen(psz_fullpath, "wb");
            if (fd == nullptr) {
                log_error("Unable to create file.\n");
                goto out;
            }
            i_file_length = iso9660_stat_t_total_size(p_statbuf);
            for (i = 0; i_file_length > 0; i++) {
                memset(buf, 0, ISO_BLOCKSIZE);
                lsn = p_statbuf->lsn + i;
                if (iso9660_iso_seek_read(p_iso, buf, lsn, 1) != ISO_BLOCKSIZE) {
                    log_error("Error reading ISO9660 file %s at LSN %lu.\n",
                              psz_iso_name,
                              (long unsigned int)lsn);
                    goto out;
                }
                fwrite(buf, (size_t)MIN(i_file_length, ISO_BLOCKSIZE), 1, fd);
                if (ferror(fd)) {
                    log_error("Error writing file %s: %s\n", psz_iso_name, strerror(errno));
                    goto out;
                }
                i_file_length -= ISO_BLOCKSIZE;
            }
            fclose(fd);
            fd = nullptr;
        }
    }
    r = 0;
out:
    if (fd != nullptr)
        fclose(fd);
    iso9660_filelist_free(p_entlist);
    return r;
}

bool extract_iso_to_temp(const char *iso_path,
                         char **output_dir,
                         should_extract_callback_t should_extract) {
    bool ret = true;
#ifdef _WIN32
    wchar_t temp_path_w[MAX_PATH];
    GetTempPathW(MAX_PATH, temp_path_w);
    wchar_t *output_dir_w = _wtempnam(temp_path_w, L"re3");
    size_t req_size =
        (size_t)WideCharToMultiByte(CP_UTF8, 0, output_dir_w, -1, nullptr, 0, nullptr, nullptr);
    if (req_size == 0) {
        log_error("Failed to convert wide string to UTF-8 (req_size = 0).\n");
        return false;
    }
    *output_dir = malloc(req_size);
    memset(*output_dir, 0, req_size);
    if (!WideCharToMultiByte(
            CP_UTF8, 0, output_dir_w, -1, *output_dir, req_size, nullptr, nullptr)) {
        log_error("Failed to convert wide string to UTF-8.\n");
        return false;
    }
    if (!CreateDirectoryW(output_dir_w, nullptr)) {
        log_error("Failed to create temporary directory: %s\n", *output_dir);
        return false;
    }
    free(output_dir_w);
#else
    char *template = env("TMPDIR");
    if (!strlen(template)) {
        memset(template, 0, PATH_MAX);
        strcpy(template, "/tmp/re3.XXXXXX");
    } else {
        strcat(template, "/re3.XXXXXX");
    }
    *output_dir = mkdtemp(template);
#endif
    if (!output_dir) {
        log_error("Failed to create temporary directory.\n");
        return false;
    }
    log_debug("Extracting ISO to temporary directory: %s\n", *output_dir);
    iso9660_t *iso = iso9660_open_ext(iso_path, ISO_EXTENSION_ALL);
    if (!iso) {
        log_error("Failed to open ISO file `%s`.\n", iso_path);
        return false;
    }
    uint8_t i_joliet_level = iso9660_ifs_get_joliet_level(iso);
    ret = !iso_extract_files(iso, "", *output_dir, i_joliet_level, should_extract);
    iso9660_close(iso);
    return ret;
}

static const char *FILE_GROUPS[] = {"App Executables", "Don't Delete", nullptr};

bool unshield_extract(const char *cab_path, const char *installation_dir) {
    bool ret = true;
#ifdef NDEBUG
    unshield_set_log_level(UNSHIELD_LOG_LEVEL_ERROR);
#else
    unshield_set_log_level(UNSHIELD_LOG_LEVEL_LOWEST);
#endif
    Unshield *unshield = unshield_open(cab_path);
    if (!unshield) {
        log_error("Failed to open %s.\n", cab_path);
        ret = false;
        goto cleanup;
    }
    for (unsigned long group_index = 0;
         group_index < ((sizeof FILE_GROUPS / sizeof FILE_GROUPS[0]) - 1);
         group_index++) {
        UnshieldFileGroup *group = unshield_file_group_find(unshield, FILE_GROUPS[group_index]);
        if (!group) {
            log_error("Could not find %s group.\n", FILE_GROUPS[group_index]);
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
                char *dir = strdup(
                    unshield_directory_name(unshield, unshield_file_directory(unshield, (int)i)));
#ifndef _WIN32
                for (unsigned long j = 0; j < strlen(dir); j++) {
                    if (dir[j] == '\\') {
                        dir[j] = '/';
                    }
                }
#endif
                memset(target_dir, 0, PATH_MAX);
                if (group_index == 0 && !strcmp(dir, "audio")) { // Keep casing consistent
                    dir[0] = 'A';
                }
                int ret2 =
                    sprintf(target_dir, "%s%s%s", installation_dir, dir ? "/" : "", dir ? dir : "");
                assert(ret2 > 0);
                if (mkdir_p(target_dir) < 0) {
                    if (errno != EEXIST) {
                        log_error("Failed to create directory %s: ", target_dir);
                        ret = false;
                        goto cleanup;
                    }
                }
                char *target_path = malloc(PATH_MAX);
                memset(target_path, 0, PATH_MAX);
                ret2 = sprintf(target_path, "%s/%s", target_dir, name);
                assert(ret > 0);
                bool unshield_ret = unshield_file_save(unshield, (int)i, target_path);
                if (!unshield_ret) {
                    log_debug("Ignoring error saving file '%s'.\n", target_path);
                }
                free(target_path);
                free(dir);
            }
        }
        free(target_dir);
    }
cleanup:
    unshield_close(unshield);
    return ret;
}
