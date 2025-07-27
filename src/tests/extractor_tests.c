#include <errno.h>
#include <limits.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cdio/iso9660.h>
#include <cmocka.h>

#include "extractor.h"

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

int __wrap_errno(void) {
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

int __wrap_iso_extract_files(void *iso, const char *output_dir) {
    check_expected_ptr(iso);
    check_expected_ptr(output_dir);
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

void __wrap_iso9660_filelist_free(void *filelist) {
    check_expected_ptr(filelist);
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

static void test_unshield_extract_success(void **state) {
    (void)state;
    will_return(__wrap_unshield_open, (void *)0x1);
    will_return(__wrap_unshield_file_group_find, (void *)0x1);
    will_return(__wrap_unshield_file_is_valid, true);
    will_return(__wrap_unshield_file_name, "test_file.txt");
    will_return(__wrap_unshield_directory_name, "test_dir");
    will_return(__wrap_unshield_file_save, true);
    will_return(__wrap_mkdir_p, 0);
    bool result = unshield_extract("test.cab", "/installation");
    assert_true(result);
}

static void test_unshield_extract_failure_open(void **state) {
    (void)state;
    will_return(__wrap_unshield_open, NULL);
    bool result = unshield_extract("test.cab", "/installation");
    assert_false(result);
}

static void test_unshield_extract_failure_group_find(void **state) {
    (void)state;
    will_return(__wrap_unshield_open, (void *)0x1);
    will_return(__wrap_unshield_file_group_find, NULL);
    bool result = unshield_extract("test.cab", "/installation");
    assert_false(result);
}

static void test_unshield_extract_failure_mkdir(void **state) {
    (void)state;
    will_return(__wrap_unshield_open, (void *)0x1);
    will_return(__wrap_unshield_file_group_find, (void *)0x1);
    will_return(__wrap_unshield_file_is_valid, true);
    will_return(__wrap_unshield_file_name, "test_file.txt");
    will_return(__wrap_unshield_directory_name, "test_dir");
    will_return(__wrap_mkdir_p, -1);
    will_return(__wrap_errno, EEXIST);
    bool result = unshield_extract("test.cab", "/installation");
    assert_false(result);
}

static void test_extract_iso_to_temp_success(void **state) {
    (void)state;
    will_return(__wrap_env, "/tmp");
    will_return(__wrap_mkdtemp, "/tmp/re3.XXXXXX");
    will_return(__wrap_iso9660_open_ext, (void *)0x1);
    will_return(__wrap_iso9660_ifs_get_joliet_level, 1);
    will_return(__wrap_iso_extract_files, 0);
    char *output_dir = nullptr;
    bool result = extract_iso_to_temp("test.iso", &output_dir, nullptr);
    assert_true(result);
    assert_string_equal(output_dir, "/tmp/re3.XXXXXX");
    free(output_dir);
}

static void test_extract_iso_to_temp_failure_env(void **state) {
    (void)state;
    expect_string(__wrap_env, var_name, "TMPDIR");
    char *tmpdir = calloc(PATH_MAX, 1);
    will_return(__wrap_env, tmpdir);
    expect_string(__wrap_mkdtemp, template, "/tmp/re3.XXXXXX");
    will_return(__wrap_mkdtemp, NULL);
    char *output_dir = nullptr;
    bool result = extract_iso_to_temp("test.iso", &output_dir, nullptr);
    assert_false(result);
    assert_null(output_dir);
    free(tmpdir);
}

static void test_extract_iso_to_temp_failure_mkdtemp(void **state) {
    (void)state;
    expect_string(__wrap_env, var_name, "TMPDIR");
    will_return(__wrap_env, "/tmp");
    will_return(__wrap_mkdtemp, NULL);
    char *output_dir = nullptr;
    bool result = extract_iso_to_temp("test.iso", &output_dir, nullptr);
    assert_false(result);
    assert_null(output_dir);
}

static void test_extract_iso_to_temp_failure_iso_open(void **state) {
    (void)state;
    will_return(__wrap_env, "/tmp");
    will_return(__wrap_mkdtemp, "/tmp/re3.XXXXXX");
    will_return(__wrap_iso9660_open_ext, NULL);
    char *output_dir = nullptr;
    bool result = extract_iso_to_temp("test.iso", &output_dir, nullptr);
    assert_false(result);
    assert_non_null(output_dir);
    free(output_dir);
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
    bool result = extract_iso_to_temp("test.iso", &output_dir, nullptr);
    assert_false(result);
    assert_non_null(output_dir);
    free(tmpdir);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_extract_iso_to_temp_failure_env),
        cmocka_unit_test(test_extract_iso_to_temp_ifs_readdir_fail),
        // cmocka_unit_test(test_extract_iso_to_temp_failure_iso_open),
        // cmocka_unit_test(test_extract_iso_to_temp_failure_mkdtemp),
        // cmocka_unit_test(test_extract_iso_to_temp_success),
        // cmocka_unit_test(test_unshield_extract_failure_group_find),
        // cmocka_unit_test(test_unshield_extract_failure_mkdir),
        // cmocka_unit_test(test_unshield_extract_failure_open),
        // cmocka_unit_test(test_unshield_extract_success),
    };
    return cmocka_run_group_tests(tests, nullptr, nullptr);
}
