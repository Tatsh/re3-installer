#include <fcntl.h>
#include <limits.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cmocka.h>

#include "utils.h"

static char temp_dir[128];

static int setup(void **state) {
    (void)state;
    snprintf(temp_dir, sizeof(temp_dir), "/tmp/re3_test_%d", getpid());
    mkdir(temp_dir, 0700);
    return 0;
}

static int teardown(void **state) {
    (void)state;
    remove_tree(temp_dir);
    return 0;
}

static void test_exists_and_is_file(void **state) {
    (void)state;
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%s/testfile", temp_dir);
    FILE *f = fopen(file_path, "w");
    assert_non_null(f);
    fputs("test", f);
    fclose(f);
    assert_true(exists(file_path));
    assert_true(is_file(file_path));
    assert_false(is_directory(file_path));
}

static void test_is_directory_and_is_dir_empty(void **state) {
    (void)state;
    char dir_path[256];
    snprintf(dir_path, sizeof(dir_path), "%s/testdir", temp_dir);
    mkdir(dir_path, 0700);
    assert_true(exists(dir_path));
    assert_true(is_directory(dir_path));
    assert_false(is_file(dir_path));
    assert_true(is_dir_empty(dir_path));
    // Add a file, now not empty
    char file_path[PATH_MAX];
    snprintf(file_path, sizeof(file_path), "%s/file", dir_path);
    FILE *f = fopen(file_path, "w");
    assert_non_null(f);
    fclose(f);
    assert_false(is_dir_empty(dir_path));
}

static void test_ends_with_iso_dll_exe_url(void **state) {
    (void)state;
    assert_true(ends_with_iso("foo.iso"));
    assert_true(ends_with_iso("bar.ISO"));
    assert_false(ends_with_iso("foo.txt"));
    assert_true(ends_with_dll("lib.dll"));
    assert_true(ends_with_dll("LIB.DLL"));
    assert_false(ends_with_dll("lib.so"));
    assert_true(ends_with_exe("run.exe"));
    assert_true(ends_with_exe("RUN.EXE"));
    assert_false(ends_with_exe("run.sh"));
    assert_true(ends_with_url("link.url"));
    assert_true(ends_with_url("LINK.URL"));
    assert_false(ends_with_url("link.txt"));
    assert_false(ends_with_iso(""));
    assert_false(ends_with_dll(""));
    assert_false(ends_with_exe(""));
    assert_false(ends_with_url(""));
}

static void test_validate_args(void **state) {
    (void)state;
    char iso1[PATH_MAX], iso2[PATH_MAX], dir[PATH_MAX];
    snprintf(iso1, sizeof(iso1), "%s/iso1.iso", temp_dir);
    snprintf(iso2, sizeof(iso2), "%s/iso2.iso", temp_dir);
    snprintf(dir, sizeof(dir), "%s/install", temp_dir);
    FILE *f1 = fopen(iso1, "w"), *f2 = fopen(iso2, "w");
    assert_non_null(f1);
    assert_non_null(f2);
    fclose(f1);
    fclose(f2);
    char *const empty_argv[] = {(char *)"prog"};
    assert_false(validate_args(1, empty_argv));
    char *const argv1[] = {(char *)"prog", iso1, iso2};
    assert_true(validate_args(3, argv1));
    char *const argv2[] = {(char *)"prog", iso1, iso2, dir};
    assert_true(validate_args(4, argv2));
    // File as install dir should fail
    char *const argv3[] = {(char *)"prog", iso1, iso2, iso1};
    assert_false(validate_args(4, argv3));
    // Non-existent arg
    char *const argv4[] = {(char *)"prog", (char *)"/no/such/file", iso2};
    assert_false(validate_args(3, argv4));
}

static void test_get_installation_dir(void **state) {
    (void)state;
    char *dir = get_installation_dir();
    assert_non_null(dir);
    assert_true(strlen(dir) > 0);
    free(dir);
}

static void test_copy_tree_and_remove_tree(void **state) {
    (void)state;
    char src[256], dest[256];
    snprintf(src, sizeof(src), "%s/src", temp_dir);
    snprintf(dest, sizeof(dest), "%s/dest", temp_dir);
    mkdir(src, 0700);
    char file1[384], file2[384];
    snprintf(file1, sizeof(file1), "%s/file1.txt", src);
    snprintf(file2, sizeof(file2), "%s/file2.txt", src);
    FILE *f1 = fopen(file1, "w"), *f2 = fopen(file2, "w");
    assert_non_null(f1);
    assert_non_null(f2);
    fputs("abc", f1);
    fputs("def", f2);
    fclose(f1);
    fclose(f2);
    assert_true(copy_tree(src, dest));
    char dest_file1[512];
    snprintf(dest_file1, sizeof(dest_file1), "%s/file1.txt", dest);
    assert_true(exists(dest_file1));
    assert_true(remove_tree(dest));
    assert_false(exists(dest));
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_exists_and_is_file, setup, teardown),
        cmocka_unit_test_setup_teardown(test_is_directory_and_is_dir_empty, setup, teardown),
        cmocka_unit_test(test_ends_with_iso_dll_exe_url),
        cmocka_unit_test_setup_teardown(test_validate_args, setup, teardown),
        cmocka_unit_test(test_get_installation_dir),
        cmocka_unit_test_setup_teardown(test_copy_tree_and_remove_tree, setup, teardown),
    };
    return cmocka_run_group_tests(tests, nullptr, nullptr);
}
