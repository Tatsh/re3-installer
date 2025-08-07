#include <errno.h>
#include <limits.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <cdio/iso9660.h>
#include <cmocka.h>
#include <libunshield.h>

#include "extractor.h"

bool ends_with_ext_gcda(const char *s) {
    return strlen(s) > 5 && !strcmp(s + strlen(s) - 5, ".gcda");
}

bool is_gcda(FILE *restrict stream) {
    if (stream == (FILE *)0x3) {
        return false;
    }
    char proc_lnk[32];
    ssize_t r;
    char filename[PATH_MAX];
    int fno = fileno(stream);
    if (fno >= 0) {
        sprintf(proc_lnk, "/proc/self/fd/%d", fno);
        r = readlink(proc_lnk, filename, PATH_MAX);
        if (r < 0) {
            return false;
        }
        filename[r] = '\0';
        return ends_with_ext_gcda(filename);
    }
    return false;
}

FILE *__real_fopen(const char *restrict path, const char *restrict mode);
FILE *__wrap_fopen(const char *restrict path, const char *restrict mode) {
    if (ends_with_ext_gcda(path)) {
        return __real_fopen(path, mode);
    }
    check_expected(path);
    check_expected(mode);
    return mock_type(FILE *);
}

int __real_fwrite(const void *ptr, size_t size, size_t count, FILE *stream);
int __wrap_fwrite(const void *ptr, size_t size, size_t count, FILE *stream) {
    if (is_gcda(stream)) {
        return __real_fwrite(ptr, size, count, stream);
    }
    check_expected_ptr(ptr);
    check_expected(size);
    check_expected(count);
    check_expected(stream);
    return mock_type(int);
}

int __real_fclose(FILE *stream);
int __wrap_fclose(FILE *stream) {
    if (is_gcda(stream)) {
        return __real_fclose(stream);
    }
    check_expected(stream);
    return mock();
}

int __real_ferror(FILE *stream);
int __wrap_ferror(FILE *stream) {
    if (is_gcda(stream)) {
        return __real_ferror(stream);
    }
    check_expected(stream);
    return mock_type(int);
}

void *__wrap_unshield_open(const char *filename) {
    check_expected_ptr(filename);
    return mock_ptr_type(void *);
}

void *__wrap_unshield_file_group_find(void *unshield, const char *group_name) {
    check_expected_ptr(unshield);
    check_expected_ptr(group_name);
    return mock_ptr_type(void *);
}

bool __wrap_unshield_file_is_valid(void *unshield, int index) {
    check_expected_ptr(unshield);
    check_expected(index);
    return mock_type(bool);
}

const char *__wrap_unshield_file_name(void *unshield, int index) {
    check_expected_ptr(unshield);
    check_expected(index);
    return mock_ptr_type(const char *);
}

const char *__wrap_unshield_directory_name(void *unshield, int index) {
    check_expected_ptr(unshield);
    check_expected(index);
    return mock_ptr_type(const char *);
}

bool __wrap_unshield_file_save(void *unshield, int index, const char *output_dir) {
    check_expected_ptr(unshield);
    check_expected(index);
    check_expected_ptr(output_dir);
    return mock_type(bool);
}

int __wrap_mkdir_p(const char *path) {
    check_expected_ptr(path);
    return mock_type(int);
}

char *__wrap_env(const char *var_name) {
    check_expected_ptr(var_name);
    return mock_ptr_type(char *);
}

char *__wrap_mkdtemp(char *template) {
    check_expected_ptr(template);
    return mock_ptr_type(char *);
}

void *__wrap_iso9660_open_ext(const char *filename, int options) {
    check_expected_ptr(filename);
    check_expected(options);
    return mock_ptr_type(void *);
}

int __wrap_iso9660_ifs_get_joliet_level(void *iso) {
    check_expected_ptr(iso);
    return mock_type(int);
}

bool __wrap_ends_with_exe(const char *filename) {
    check_expected_ptr(filename);
    return mock_type(bool);
}

bool __wrap_ends_with_dll(const char *filename) {
    check_expected_ptr(filename);
    return mock_type(bool);
}

bool __wrap_ends_with_url(const char *filename) {
    check_expected_ptr(filename);
    return mock_type(bool);
}

void __wrap_unshield_close(void *unshield) {
    check_expected_ptr(unshield);
}

const char *__wrap_unshield_file_directory(void *unshield, int index) {
    check_expected_ptr(unshield);
    check_expected(index);
    return mock_ptr_type(const char *);
}

void *__wrap__cdio_list_begin(void *list) {
    check_expected_ptr(list);
    return mock_ptr_type(void *);
}

void *__wrap__cdio_list_node_data(void *node) {
    check_expected_ptr(node);
    return mock_ptr_type(void *);
}

const char *__wrap_iso9660_name_translate_ext(const char *name, int joliet_level) {
    check_expected_ptr(name);
    check_expected(joliet_level);
    return mock_ptr_type(const char *);
}

ssize_t __wrap_iso9660_iso_seek_read(void *iso, void *ptr, size_t size, off_t offset) {
    check_expected_ptr(iso);
    check_expected_ptr(ptr);
    check_expected(size);
    check_expected(offset);
    return mock_type(ssize_t);
}

void *__wrap__cdio_list_node_next(void *node) {
    check_expected_ptr(node);
    return mock_ptr_type(void *);
}

void *__wrap_iso9660_ifs_readdir(void *iso, void *dir) {
    check_expected_ptr(iso);
    check_expected_ptr(dir);
    return mock_ptr_type(void *);
}

void __wrap_iso9660_close(void *iso) {
    check_expected_ptr(iso);
}

void __wrap_unshield_set_log_level(int level) {
    check_expected(level);
}

static bool should_extract_true(const char *filename) {
    (void)filename;
    return true;
}

static bool should_extract_false(const char *filename) {
    (void)filename;
    return false;
}

static void test_unshield_extract_failure_group_find(void **state) {
    (void)state;
    expect_string(__wrap_unshield_open, filename, "test.cab");
    will_return(__wrap_unshield_open, (void *)0x1);
    expect_value(__wrap_unshield_file_group_find, unshield, (void *)0x1);
    expect_string(__wrap_unshield_file_group_find, group_name, "App Executables");
    will_return(__wrap_unshield_file_group_find, NULL);
    expect_any(__wrap_unshield_set_log_level, level);
    expect_value(__wrap_unshield_close, unshield, (void *)0x1);
    bool result = unshield_extract("test.cab", "/installation");
    assert_false(result);
}

static void test_unshield_extract_failure_mkdir(void **state) {
    (void)state;

    expect_string(__wrap_unshield_open, filename, "test.cab");
    will_return(__wrap_unshield_open, (void *)0x1);

    expect_value(__wrap_unshield_file_group_find, unshield, (void *)0x1);
    expect_string(__wrap_unshield_file_group_find, group_name, "App Executables");
    UnshieldFileGroup *group = calloc(1, sizeof(UnshieldFileGroup));
    group->name = "App Executables";
    group->first_file = 0;
    group->last_file = 1;
    will_return(__wrap_unshield_file_group_find, group);

    expect_value(__wrap_unshield_file_is_valid, unshield, (void *)0x1);
    expect_value(__wrap_unshield_file_is_valid, index, 0);
    will_return(__wrap_unshield_file_is_valid, true);

    expect_value(__wrap_unshield_file_name, unshield, (void *)0x1);
    expect_value(__wrap_unshield_file_name, index, 0);
    will_return(__wrap_unshield_file_name, "test_file.txt");

    expect_string(__wrap_ends_with_dll, filename, "test_file.txt");
    will_return(__wrap_ends_with_dll, false);
    expect_string(__wrap_ends_with_exe, filename, "test_file.txt");
    will_return(__wrap_ends_with_exe, false);
    expect_string(__wrap_ends_with_url, filename, "test_file.txt");
    will_return(__wrap_ends_with_url, false);

    expect_value(__wrap_unshield_file_directory, unshield, (void *)0x1);
    expect_value(__wrap_unshield_file_directory, index, 0);
    will_return(__wrap_unshield_file_directory, 1);

    expect_value(__wrap_unshield_directory_name, unshield, (void *)0x1);
    expect_value(__wrap_unshield_directory_name, index, 1);
    will_return(__wrap_unshield_directory_name, "audio");

    expect_string(__wrap_mkdir_p, path, "/installation/Audio");
    will_return(__wrap_mkdir_p, -1);

    expect_any(__wrap_unshield_set_log_level, level);
    expect_value(__wrap_unshield_close, unshield, (void *)0x1);
    bool result = unshield_extract("test.cab", "/installation");
    assert_false(result);

    free(group);
}

static void test_unshield_extract_failure_file_save_ignored(void **state) {
    (void)state;

    expect_string(__wrap_unshield_open, filename, "test.cab");
    will_return(__wrap_unshield_open, (void *)0x1);

    expect_value(__wrap_unshield_file_group_find, unshield, (void *)0x1);
    expect_string(__wrap_unshield_file_group_find, group_name, "App Executables");
    UnshieldFileGroup *group = calloc(1, sizeof(UnshieldFileGroup));
    group->name = "App Executables";
    group->first_file = 0;
    group->last_file = 1;
    will_return(__wrap_unshield_file_group_find, group);

    expect_value(__wrap_unshield_file_group_find, unshield, (void *)0x1);
    expect_string(__wrap_unshield_file_group_find, group_name, "Don't Delete");
    will_return(__wrap_unshield_file_group_find, NULL);

    expect_value(__wrap_unshield_file_is_valid, unshield, (void *)0x1);
    expect_value(__wrap_unshield_file_is_valid, index, 0);
    will_return(__wrap_unshield_file_is_valid, true);

    expect_value(__wrap_unshield_file_is_valid, unshield, (void *)0x1);
    expect_value(__wrap_unshield_file_is_valid, index, 1);
    will_return(__wrap_unshield_file_is_valid, true);

    expect_value(__wrap_unshield_file_name, unshield, (void *)0x1);
    expect_value(__wrap_unshield_file_name, index, 0);
    will_return(__wrap_unshield_file_name, "test_file.txt");

    expect_value(__wrap_unshield_file_name, unshield, (void *)0x1);
    expect_value(__wrap_unshield_file_name, index, 1);
    will_return(__wrap_unshield_file_name, "test_file.exe");

    expect_string(__wrap_ends_with_dll, filename, "test_file.txt");
    will_return(__wrap_ends_with_dll, false);
    expect_string(__wrap_ends_with_exe, filename, "test_file.txt");
    will_return(__wrap_ends_with_exe, false);
    expect_string(__wrap_ends_with_url, filename, "test_file.txt");
    will_return(__wrap_ends_with_url, false);

    expect_string(__wrap_ends_with_exe, filename, "test_file.exe");
    will_return(__wrap_ends_with_exe, true);

    expect_value(__wrap_unshield_file_directory, unshield, (void *)0x1);
    expect_value(__wrap_unshield_file_directory, index, 0);
    will_return(__wrap_unshield_file_directory, 1);

    expect_value(__wrap_unshield_directory_name, unshield, (void *)0x1);
    expect_value(__wrap_unshield_directory_name, index, 1);
    will_return(__wrap_unshield_directory_name, "aud\\io");

    expect_string(__wrap_mkdir_p, path, "/installation/aud/io");
    will_return(__wrap_mkdir_p, 0);

    expect_value(__wrap_unshield_file_save, unshield, (void *)0x1);
    expect_value(__wrap_unshield_file_save, index, 0);
    expect_string(__wrap_unshield_file_save, output_dir, "/installation/aud/io/test_file.txt");
    will_return(__wrap_unshield_file_save, false);

    expect_any(__wrap_unshield_set_log_level, level);
    expect_value(__wrap_unshield_close, unshield, (void *)0x1);
    bool result = unshield_extract("test.cab", "/installation");
    assert_true(result);

    free(group);
}

static void test_extract_iso_to_temp_cdio_list_node_data_returns_dot_and_dot_dot(void **state) {
    (void)state;
    void *iso = (void *)0x1;

    expect_string(__wrap_env, var_name, "TMPDIR");
    char *tmpdir = calloc(PATH_MAX, 1);
    strcpy(tmpdir, "/tmp");
    will_return(__wrap_env, tmpdir);

    expect_string(__wrap_mkdtemp, template, "/tmp/re3.XXXXXX");
    will_return(__wrap_mkdtemp, "/tmp/re3.XXXXXX");

    expect_string(__wrap_iso9660_open_ext, filename, "test.iso");
    expect_value(__wrap_iso9660_open_ext, options, ISO_EXTENSION_ALL);

    will_return(__wrap_iso9660_open_ext, iso);
    expect_value(__wrap_iso9660_ifs_get_joliet_level, iso, iso);
    will_return(__wrap_iso9660_ifs_get_joliet_level, 1);

    CdioISO9660FileList_t *entlist = iso9660_filelist_new();

    expect_value(__wrap_iso9660_ifs_readdir, iso, iso);
    expect_string(__wrap_iso9660_ifs_readdir, dir, "");
    will_return(__wrap_iso9660_ifs_readdir, entlist);

    expect_value(__wrap__cdio_list_begin, list, entlist);
    will_return(__wrap__cdio_list_begin, (void *)0x2);

    iso9660_stat_t statbuf_dot = {0};
    strcpy(statbuf_dot.filename, ".");
    expect_any(__wrap__cdio_list_node_data, node);
    will_return(__wrap__cdio_list_node_data, &statbuf_dot);

    void *next_node = (void *)0x4;
    expect_any(__wrap__cdio_list_node_next, node);
    will_return(__wrap__cdio_list_node_next, next_node);

    iso9660_stat_t statbuf_dotdot = {0};
    strcpy(statbuf_dotdot.filename, "..");
    expect_any(__wrap__cdio_list_node_data, node);
    will_return(__wrap__cdio_list_node_data, &statbuf_dotdot);

    expect_any(__wrap__cdio_list_node_next, node);
    will_return(__wrap__cdio_list_node_next, NULL);

    expect_any(__wrap_iso9660_close, iso);

    char *output_dir = nullptr;
    bool result = extract_iso_to_temp("test.iso", &output_dir, should_extract_true);

    assert_true(result);
    assert_non_null(output_dir);

    free(tmpdir);
}

static void test_extract_iso_to_temp_iso_seek_read_not_blocksize(void **state) {
    (void)state;
    void *iso = (void *)0x1;

    expect_string(__wrap_env, var_name, "TMPDIR");
    char *tmpdir = calloc(PATH_MAX, 1);
    strcpy(tmpdir, "/tmp");
    will_return(__wrap_env, tmpdir);

    expect_string(__wrap_mkdtemp, template, "/tmp/re3.XXXXXX");
    will_return(__wrap_mkdtemp, "/tmp/re3.XXXXXX");

    expect_string(__wrap_iso9660_open_ext, filename, "test.iso");
    expect_value(__wrap_iso9660_open_ext, options, ISO_EXTENSION_ALL);
    will_return(__wrap_iso9660_open_ext, iso);

    expect_value(__wrap_iso9660_ifs_get_joliet_level, iso, iso);
    will_return(__wrap_iso9660_ifs_get_joliet_level, 1);

    CdioISO9660FileList_t *entlist = iso9660_filelist_new();

    expect_value(__wrap_iso9660_ifs_readdir, iso, iso);
    expect_string(__wrap_iso9660_ifs_readdir, dir, "");
    will_return(__wrap_iso9660_ifs_readdir, entlist);

    expect_value(__wrap__cdio_list_begin, list, entlist);
    void *node = (void *)0x2;
    will_return(__wrap__cdio_list_begin, node);

    iso9660_stat_t statbuf = {0};
    statbuf.type = _STAT_FILE;
#ifdef HAVE_ISO9660_STAT_T_TOTAL_SIZE
    statbuf.total_size = 123;
#else
    statbuf.size = 123;
#endif
    strcpy(statbuf.filename, "ff");
    expect_any(__wrap__cdio_list_node_data, node);
    will_return(__wrap__cdio_list_node_data, &statbuf);

    expect_string(__wrap_iso9660_name_translate_ext, name, "ff");
    expect_any(__wrap_iso9660_name_translate_ext, joliet_level);
    will_return(__wrap_iso9660_name_translate_ext, "ff");

    expect_any(__wrap_iso9660_close, iso);

    expect_any(__wrap_fopen, path);
    expect_string(__wrap_fopen, mode, "wb");
    will_return(__wrap_fopen, (FILE *)0x3);

    expect_value(__wrap_fclose, stream, (FILE *)0x3);
    will_return(__wrap_fclose, 0);

    expect_any(__wrap_iso9660_iso_seek_read, iso);
    expect_any(__wrap_iso9660_iso_seek_read, ptr);
    expect_any(__wrap_iso9660_iso_seek_read, size);
    expect_any(__wrap_iso9660_iso_seek_read, offset);
    will_return(__wrap_iso9660_iso_seek_read, 123); // Not ISO_BLOCKSIZE

    char *output_dir = nullptr;

    bool result = extract_iso_to_temp("test.iso", &output_dir, should_extract_true);

    assert_false(result);
    assert_non_null(output_dir);

    free(tmpdir);
}

static void test_extract_iso_to_temp_fopen_returns_null(void **state) {
    (void)state;
    void *iso = (void *)0x1;

    expect_string(__wrap_env, var_name, "TMPDIR");
    char *tmpdir = calloc(PATH_MAX, 1);
    strcpy(tmpdir, "/tmp");
    will_return(__wrap_env, tmpdir);

    expect_string(__wrap_mkdtemp, template, "/tmp/re3.XXXXXX");
    will_return(__wrap_mkdtemp, "/tmp/re3.XXXXXX");

    expect_string(__wrap_iso9660_open_ext, filename, "test.iso");
    expect_value(__wrap_iso9660_open_ext, options, ISO_EXTENSION_ALL);
    will_return(__wrap_iso9660_open_ext, iso);

    expect_value(__wrap_iso9660_ifs_get_joliet_level, iso, iso);
    will_return(__wrap_iso9660_ifs_get_joliet_level, 1);

    CdioISO9660FileList_t *entlist = iso9660_filelist_new();

    expect_value(__wrap_iso9660_ifs_readdir, iso, iso);
    expect_string(__wrap_iso9660_ifs_readdir, dir, "");
    will_return(__wrap_iso9660_ifs_readdir, entlist);

    expect_value(__wrap__cdio_list_begin, list, entlist);
    void *node = (void *)0x2;
    will_return(__wrap__cdio_list_begin, node);

    iso9660_stat_t statbuf = {0};
    statbuf.type = _STAT_FILE;
#ifdef HAVE_ISO9660_STAT_T_TOTAL_SIZE
    statbuf.total_size = ISO_BLOCKSIZE;
#else
    statbuf.size = ISO_BLOCKSIZE;
#endif
    strcpy(statbuf.filename, "ff");
    expect_any(__wrap__cdio_list_node_data, node);
    will_return(__wrap__cdio_list_node_data, &statbuf);

    expect_string(__wrap_iso9660_name_translate_ext, name, "ff");
    expect_any(__wrap_iso9660_name_translate_ext, joliet_level);
    will_return(__wrap_iso9660_name_translate_ext, "ff");

    expect_any(__wrap_fopen, path);
    expect_string(__wrap_fopen, mode, "wb");
    will_return(__wrap_fopen, NULL);

    expect_any(__wrap_iso9660_close, iso);

    char *output_dir = nullptr;
    bool result = extract_iso_to_temp("test.iso", &output_dir, should_extract_true);

    assert_false(result);
    assert_non_null(output_dir);

    free(tmpdir);
}

static void test_extract_iso_to_temp_ferror_nonzero(void **state) {
    (void)state;
    void *iso = (void *)0x1;

    expect_string(__wrap_env, var_name, "TMPDIR");
    char *tmpdir = calloc(PATH_MAX, 1);
    strcpy(tmpdir, "/tmp");
    will_return(__wrap_env, tmpdir);

    expect_string(__wrap_mkdtemp, template, "/tmp/re3.XXXXXX");
    will_return(__wrap_mkdtemp, "/tmp/re3.XXXXXX");

    expect_string(__wrap_iso9660_open_ext, filename, "test.iso");
    expect_value(__wrap_iso9660_open_ext, options, ISO_EXTENSION_ALL);
    will_return(__wrap_iso9660_open_ext, iso);

    expect_value(__wrap_iso9660_ifs_get_joliet_level, iso, iso);
    will_return(__wrap_iso9660_ifs_get_joliet_level, 1);

    CdioISO9660FileList_t *entlist = iso9660_filelist_new();

    expect_value(__wrap_iso9660_ifs_readdir, iso, iso);
    expect_string(__wrap_iso9660_ifs_readdir, dir, "");
    will_return(__wrap_iso9660_ifs_readdir, entlist);

    expect_value(__wrap__cdio_list_begin, list, entlist);
    void *node = (void *)0x2;
    will_return(__wrap__cdio_list_begin, node);

    iso9660_stat_t statbuf = {0};
    statbuf.type = _STAT_FILE;
#ifdef HAVE_ISO9660_STAT_T_TOTAL_SIZE
    statbuf.total_size = ISO_BLOCKSIZE;
#else
    statbuf.size = ISO_BLOCKSIZE;
#endif
    strcpy(statbuf.filename, "ff");
    expect_any(__wrap__cdio_list_node_data, node);
    will_return(__wrap__cdio_list_node_data, &statbuf);

    expect_string(__wrap_iso9660_name_translate_ext, name, "ff");
    expect_any(__wrap_iso9660_name_translate_ext, joliet_level);
    will_return(__wrap_iso9660_name_translate_ext, "ff");

    expect_any(__wrap_fopen, path);
    expect_string(__wrap_fopen, mode, "wb");
    FILE *mock_file = (FILE *)0x3;
    will_return(__wrap_fopen, mock_file);

    expect_any(__wrap_iso9660_iso_seek_read, iso);
    expect_any(__wrap_iso9660_iso_seek_read, ptr);
    expect_any(__wrap_iso9660_iso_seek_read, size);
    expect_any(__wrap_iso9660_iso_seek_read, offset);
    will_return(__wrap_iso9660_iso_seek_read, ISO_BLOCKSIZE);

    expect_any(__wrap_fwrite, ptr);
    expect_value(__wrap_fwrite, size, ISO_BLOCKSIZE);
    expect_value(__wrap_fwrite, count, 1);
    expect_value(__wrap_fwrite, stream, mock_file);
    will_return(__wrap_fwrite, ISO_BLOCKSIZE);

    expect_value(__wrap_ferror, stream, mock_file);
    will_return(__wrap_ferror, 1);

    expect_value(__wrap_fclose, stream, mock_file);
    will_return(__wrap_fclose, 0);

    expect_any(__wrap_iso9660_close, iso);

    char *output_dir = nullptr;
    bool result = extract_iso_to_temp("test.iso", &output_dir, should_extract_true);

    assert_false(result);
    assert_non_null(output_dir);

    free(tmpdir);
}

static void test_extract_iso_to_temp_ok(void **state) {
    (void)state;
    void *iso = (void *)0x1;

    expect_string(__wrap_env, var_name, "TMPDIR");
    char *tmpdir = calloc(PATH_MAX, 1);
    strcpy(tmpdir, "/tmp");
    will_return(__wrap_env, tmpdir);

    expect_string(__wrap_mkdtemp, template, "/tmp/re3.XXXXXX");
    will_return(__wrap_mkdtemp, "/tmp/re3.XXXXXX");

    expect_string(__wrap_iso9660_open_ext, filename, "test.iso");
    expect_value(__wrap_iso9660_open_ext, options, ISO_EXTENSION_ALL);
    will_return(__wrap_iso9660_open_ext, iso);

    expect_value(__wrap_iso9660_ifs_get_joliet_level, iso, iso);
    will_return(__wrap_iso9660_ifs_get_joliet_level, 1);

    CdioISO9660FileList_t *entlist = iso9660_filelist_new();

    expect_value(__wrap_iso9660_ifs_readdir, iso, iso);
    expect_string(__wrap_iso9660_ifs_readdir, dir, "");
    will_return(__wrap_iso9660_ifs_readdir, entlist);

    expect_value(__wrap__cdio_list_begin, list, entlist);
    void *node = (void *)0x2;
    will_return(__wrap__cdio_list_begin, node);

    iso9660_stat_t statbuf = {0};
    statbuf.type = _STAT_FILE;
#ifdef HAVE_ISO9660_STAT_T_TOTAL_SIZE
    statbuf.total_size = ISO_BLOCKSIZE;
#else
    statbuf.size = ISO_BLOCKSIZE;
#endif
    strcpy(statbuf.filename, "ff");
    expect_any(__wrap__cdio_list_node_data, node);
    will_return(__wrap__cdio_list_node_data, &statbuf);

    expect_string(__wrap_iso9660_name_translate_ext, name, "ff");
    expect_any(__wrap_iso9660_name_translate_ext, joliet_level);
    will_return(__wrap_iso9660_name_translate_ext, "ff");

    expect_any(__wrap_fopen, path);
    expect_string(__wrap_fopen, mode, "wb");
    FILE *mock_file = (FILE *)0x3;
    will_return(__wrap_fopen, mock_file);

    expect_any(__wrap_iso9660_iso_seek_read, iso);
    expect_any(__wrap_iso9660_iso_seek_read, ptr);
    expect_any(__wrap_iso9660_iso_seek_read, size);
    expect_any(__wrap_iso9660_iso_seek_read, offset);
    will_return(__wrap_iso9660_iso_seek_read, ISO_BLOCKSIZE);

    expect_any(__wrap_fwrite, ptr);
    expect_value(__wrap_fwrite, size, ISO_BLOCKSIZE);
    expect_value(__wrap_fwrite, count, 1);
    expect_value(__wrap_fwrite, stream, mock_file);
    will_return(__wrap_fwrite, ISO_BLOCKSIZE);

    expect_value(__wrap_ferror, stream, mock_file);
    will_return(__wrap_ferror, 0);

    expect_value(__wrap_fclose, stream, mock_file);
    will_return(__wrap_fclose, 0);

    expect_value(__wrap__cdio_list_node_next, node, node);
    will_return(__wrap__cdio_list_node_next, NULL);

    expect_any(__wrap_iso9660_close, iso);

    char *output_dir = nullptr;
    bool result = extract_iso_to_temp("test.iso", &output_dir, should_extract_true);

    assert_true(result);
    assert_non_null(output_dir);

    free(tmpdir);
}

static void test_extract_iso_to_temp_should_extract_always_false(void **state) {
    (void)state;
    void *iso = (void *)0x1;

    expect_string(__wrap_env, var_name, "TMPDIR");
    char *tmpdir = calloc(PATH_MAX, 1);
    strcpy(tmpdir, "/tmp");
    will_return(__wrap_env, tmpdir);

    expect_string(__wrap_mkdtemp, template, "/tmp/re3.XXXXXX");
    will_return(__wrap_mkdtemp, "/tmp/re3.XXXXXX");

    expect_string(__wrap_iso9660_open_ext, filename, "test.iso");
    expect_value(__wrap_iso9660_open_ext, options, ISO_EXTENSION_ALL);
    will_return(__wrap_iso9660_open_ext, iso);

    expect_value(__wrap_iso9660_ifs_get_joliet_level, iso, iso);
    will_return(__wrap_iso9660_ifs_get_joliet_level, 1);

    CdioISO9660FileList_t *entlist = iso9660_filelist_new();

    expect_value(__wrap_iso9660_ifs_readdir, iso, iso);
    expect_string(__wrap_iso9660_ifs_readdir, dir, "");
    will_return(__wrap_iso9660_ifs_readdir, entlist);

    expect_value(__wrap__cdio_list_begin, list, entlist);
    void *node = (void *)0x2;
    will_return(__wrap__cdio_list_begin, node);

    iso9660_stat_t statbuf = {0};
    statbuf.type = _STAT_FILE;
#ifdef HAVE_ISO9660_STAT_T_TOTAL_SIZE
    statbuf.total_size = 123;
#else
    statbuf.size = 123;
#endif
    strcpy(statbuf.filename, "ff");
    expect_any(__wrap__cdio_list_node_data, node);
    will_return(__wrap__cdio_list_node_data, &statbuf);

    expect_string(__wrap_iso9660_name_translate_ext, name, "ff");
    expect_any(__wrap_iso9660_name_translate_ext, joliet_level);
    will_return(__wrap_iso9660_name_translate_ext, "ff");

    expect_value(__wrap__cdio_list_node_next, node, node);
    will_return(__wrap__cdio_list_node_next, NULL);

    expect_any(__wrap_iso9660_close, iso);

    char *output_dir = nullptr;

    bool result = extract_iso_to_temp("test.iso", &output_dir, should_extract_false);

    assert_true(result);
    assert_non_null(output_dir);

    free(tmpdir);
}

static void test_extract_iso_to_temp_failure_env(void **state) {
    (void)state;
    expect_string(__wrap_env, var_name, "TMPDIR");
    char *tmpdir = calloc(PATH_MAX, 1);
    will_return(__wrap_env, tmpdir);
    expect_string(__wrap_mkdtemp, template, "/tmp/re3.XXXXXX");
    will_return(__wrap_mkdtemp, NULL);
    char *output_dir = nullptr;
    bool result = extract_iso_to_temp("test.iso", &output_dir, should_extract_true);
    assert_false(result);
    assert_null(output_dir);
    free(tmpdir);
}

static void test_extract_iso_to_temp_failure_mkdtemp(void **state) {
    (void)state;
    expect_string(__wrap_env, var_name, "TMPDIR");
    char *tmpdir = calloc(PATH_MAX, 1);
    strcpy(tmpdir, "/tmp");
    will_return(__wrap_env, tmpdir);
    expect_string(__wrap_mkdtemp, template, "/tmp/re3.XXXXXX");
    will_return(__wrap_mkdtemp, NULL);
    char *output_dir = nullptr;
    bool result = extract_iso_to_temp("test.iso", &output_dir, should_extract_true);
    assert_false(result);
    assert_null(output_dir);
    free(tmpdir);
}

static void test_extract_iso_to_temp_failure_iso_open(void **state) {
    (void)state;
    expect_string(__wrap_env, var_name, "TMPDIR");
    char *tmpdir = calloc(PATH_MAX, 1);
    strcpy(tmpdir, "/tmp");
    will_return(__wrap_env, tmpdir);
    expect_string(__wrap_mkdtemp, template, "/tmp/re3.XXXXXX");
    will_return(__wrap_mkdtemp, "/tmp/re3.XXXXXX");
    expect_string(__wrap_iso9660_open_ext, filename, "test.iso");
    expect_value(__wrap_iso9660_open_ext, options, ISO_EXTENSION_ALL);
    will_return(__wrap_iso9660_open_ext, NULL);
    char *output_dir = nullptr;
    bool result = extract_iso_to_temp("test.iso", &output_dir, should_extract_true);
    assert_false(result);
    assert_non_null(output_dir);
    free(tmpdir);
}

static void test_extract_iso_to_temp_ifs_readdir_fail(void **state) {
    (void)state;
    expect_string(__wrap_env, var_name, "TMPDIR");
    char *tmpdir = calloc(PATH_MAX, 1);
    strcpy(tmpdir, "/tmp");
    will_return(__wrap_env, tmpdir);
    expect_string(__wrap_mkdtemp, template, "/tmp/re3.XXXXXX");
    will_return(__wrap_mkdtemp, "/tmp/re3.XXXXXX");
    expect_string(__wrap_iso9660_open_ext, filename, "test.iso");
    expect_value(__wrap_iso9660_open_ext, options, ISO_EXTENSION_ALL);
    will_return(__wrap_iso9660_open_ext, (void *)0x1);
    expect_value(__wrap_iso9660_ifs_get_joliet_level, iso, (void *)0x1);
    will_return(__wrap_iso9660_ifs_get_joliet_level, 1);
    expect_value(__wrap_iso9660_ifs_readdir, iso, (void *)0x1);
    expect_string(__wrap_iso9660_ifs_readdir, dir, "");
    will_return(__wrap_iso9660_ifs_readdir, NULL);
    expect_value(__wrap_iso9660_close, iso, (void *)0x1);
    char *output_dir = nullptr;
    bool result = extract_iso_to_temp("test.iso", &output_dir, should_extract_true);
    assert_false(result);
    assert_non_null(output_dir);
    free(tmpdir);
}

static void test_unshield_open_failure(void **state) {
    (void)state;
    expect_string(__wrap_unshield_open, filename, "test.cab");
    expect_any(__wrap_unshield_set_log_level, level);
    expect_value(__wrap_unshield_close, unshield, NULL);
    will_return(__wrap_unshield_open, NULL);
    bool result = unshield_extract("test.cab", "/installation");
    assert_false(result);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_extract_iso_to_temp_cdio_list_node_data_returns_dot_and_dot_dot),
        cmocka_unit_test(test_extract_iso_to_temp_failure_env),
        cmocka_unit_test(test_extract_iso_to_temp_failure_iso_open),
        cmocka_unit_test(test_extract_iso_to_temp_failure_mkdtemp),
        cmocka_unit_test(test_extract_iso_to_temp_ferror_nonzero),
        cmocka_unit_test(test_extract_iso_to_temp_fopen_returns_null),
        cmocka_unit_test(test_extract_iso_to_temp_ifs_readdir_fail),
        cmocka_unit_test(test_extract_iso_to_temp_iso_seek_read_not_blocksize),
        cmocka_unit_test(test_extract_iso_to_temp_ok),
        cmocka_unit_test(test_extract_iso_to_temp_should_extract_always_false),
        cmocka_unit_test(test_unshield_extract_failure_file_save_ignored),
        cmocka_unit_test(test_unshield_extract_failure_group_find),
        cmocka_unit_test(test_unshield_extract_failure_mkdir),
        cmocka_unit_test(test_unshield_open_failure),
    };
    return cmocka_run_group_tests(tests, nullptr, nullptr);
}
