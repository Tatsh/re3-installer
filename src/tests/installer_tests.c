#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <cmocka.h>

#include "installer.h"

bool __wrap_exists(const char *path) {
    check_expected_ptr(path);
    return mock_type(bool);
}

bool __wrap_is_dir_empty(const char *path) {
    check_expected_ptr(path);
    return mock_type(bool);
}

bool __wrap_ends_with_iso(const char *path) {
    check_expected_ptr(path);
    return mock_type(bool);
}

bool __wrap_extract_iso_to_temp(const char *iso_path, char **out_dir) {
    check_expected_ptr(iso_path);
    check_expected_ptr(out_dir);
    *out_dir = mock_ptr_type(char *);
    return mock_type(bool);
}

bool __wrap_unshield_extract(const char *cab_path, const char *dest_dir) {
    check_expected_ptr(cab_path);
    check_expected_ptr(dest_dir);
    return mock_type(bool);
}

bool __wrap_copy_tree(const char *src, const char *dest) {
    check_expected_ptr(src);
    check_expected_ptr(dest);
    return mock_type(bool);
}

bool __wrap_remove_tree(const char *path) {
    check_expected_ptr(path);
    return mock_type(bool);
}

static void test_install_re3_game_data_success(void **state) {
    (void)state;

    expect_string(__wrap_exists, path, "/installation/dir");
    will_return(__wrap_exists, false);

    expect_string(__wrap_is_dir_empty, path, "/installation/dir");
    will_return(__wrap_is_dir_empty, true);

    expect_string(__wrap_ends_with_iso, path, "/path/to/disc1");
    will_return(__wrap_ends_with_iso, false);

    expect_string(__wrap_ends_with_iso, path, "/path/to/disc2");
    will_return(__wrap_ends_with_iso, false);

    expect_string(__wrap_unshield_extract, cab_path, "/path/to/disc1/data1.cab");
    expect_string(__wrap_unshield_extract, dest_dir, "/installation/dir");
    will_return(__wrap_unshield_extract, true);

    expect_string(__wrap_copy_tree, src, "/path/to/disc2/Audio");
    expect_string(__wrap_copy_tree, dest, "/installation/dir/Audio");
    will_return(__wrap_copy_tree, true);

    bool result = install_re3_game_data("/path/to/disc1", "/path/to/disc2", "/installation/dir");
    assert_true(result);
}

static void test_install_re3_game_data_non_empty_dir(void **state) {
    (void)state;
    expect_string(__wrap_exists, path, "/installation/dir");
    will_return(__wrap_exists, true);

    expect_string(__wrap_is_dir_empty, path, "/installation/dir");
    will_return(__wrap_is_dir_empty, false);

    bool result = install_re3_game_data("/path/to/disc1", "/path/to/disc2", "/installation/dir");
    assert_false(result);
}

static void test_install_re3_game_data_failed_extract_iso(void **state) {
    (void)state;
    expect_string(__wrap_exists, path, "/installation/dir");
    will_return(__wrap_exists, false);

    expect_string(__wrap_is_dir_empty, path, "/installation/dir");
    will_return(__wrap_is_dir_empty, true);

    expect_string(__wrap_ends_with_iso, path, "/path/to/disc1");
    will_return(__wrap_ends_with_iso, true);

    expect_string(__wrap_extract_iso_to_temp, iso_path, "/path/to/disc1");
    expect_any(__wrap_extract_iso_to_temp, out_dir);
    will_return(__wrap_extract_iso_to_temp, "temp_dir");
    will_return(__wrap_extract_iso_to_temp, false);

    bool result = install_re3_game_data("/path/to/disc1", "/path/to/disc2", "/installation/dir");
    assert_false(result);
}

static void test_install_re3_game_data_failed_copy_tree(void **state) {
    (void)state;
    expect_string(__wrap_exists, path, "/installation/dir");
    will_return(__wrap_exists, false);

    expect_string(__wrap_is_dir_empty, path, "/installation/dir");
    will_return(__wrap_is_dir_empty, true);

    expect_string(__wrap_ends_with_iso, path, "/path/to/disc1");
    will_return(__wrap_ends_with_iso, false);

    expect_string(__wrap_ends_with_iso, path, "/path/to/disc2");
    will_return(__wrap_ends_with_iso, false);

    expect_string(__wrap_unshield_extract, cab_path, "/path/to/disc1/data1.cab");
    expect_string(__wrap_unshield_extract, dest_dir, "/installation/dir");
    will_return(__wrap_unshield_extract, true);

    expect_string(__wrap_copy_tree, src, "/path/to/disc2/Audio");
    expect_string(__wrap_copy_tree, dest, "/installation/dir/Audio");
    will_return(__wrap_copy_tree, false);

    bool result = install_re3_game_data("/path/to/disc1", "/path/to/disc2", "/installation/dir");
    assert_false(result);
}

static void test_install_re3_game_data_fail_to_extract_iso_disc2(void **state) {
    (void)state;
    expect_string(__wrap_exists, path, "/installation/dir");
    will_return(__wrap_exists, false);

    expect_string(__wrap_is_dir_empty, path, "/installation/dir");
    will_return(__wrap_is_dir_empty, true);

    expect_string(__wrap_ends_with_iso, path, "/path/to/disc1");
    will_return(__wrap_ends_with_iso, false);

    expect_string(__wrap_ends_with_iso, path, "/path/to/disc2.iso");
    will_return(__wrap_ends_with_iso, true);

    expect_string(__wrap_extract_iso_to_temp, iso_path, "/path/to/disc2.iso");
    expect_any(__wrap_extract_iso_to_temp, out_dir);
    will_return(__wrap_extract_iso_to_temp, "temp_dir");
    will_return(__wrap_extract_iso_to_temp, false);

    bool result =
        install_re3_game_data("/path/to/disc1", "/path/to/disc2.iso", "/installation/dir");
    assert_false(result);
}

static void test_install_re3_unshield_extractor_failure(void **state) {
    (void)state;
    expect_string(__wrap_exists, path, "/installation/dir");
    will_return(__wrap_exists, false);

    expect_string(__wrap_is_dir_empty, path, "/installation/dir");
    will_return(__wrap_is_dir_empty, true);

    expect_string(__wrap_ends_with_iso, path, "/path/to/disc1");
    will_return(__wrap_ends_with_iso, false);

    expect_string(__wrap_ends_with_iso, path, "/path/to/disc2");
    will_return(__wrap_ends_with_iso, false);

    expect_string(__wrap_unshield_extract, cab_path, "/path/to/disc1/data1.cab");
    expect_string(__wrap_unshield_extract, dest_dir, "/installation/dir");
    will_return(__wrap_unshield_extract, false);

    bool result = install_re3_game_data("/path/to/disc1", "/path/to/disc2", "/installation/dir");
    assert_false(result);
}

static void test_install_re3_clean_up_disc1_temp_dir(void **state) {
    (void)state;
    expect_string(__wrap_exists, path, "/installation/dir");
    will_return(__wrap_exists, false);

    expect_string(__wrap_is_dir_empty, path, "/installation/dir");
    will_return(__wrap_is_dir_empty, true);

    expect_string(__wrap_ends_with_iso, path, "/path/to/disc1.iso");
    will_return(__wrap_ends_with_iso, true);

    expect_string(__wrap_ends_with_iso, path, "/path/to/disc2");
    will_return(__wrap_ends_with_iso, false);

    expect_string(__wrap_extract_iso_to_temp, iso_path, "/path/to/disc1.iso");
    expect_any(__wrap_extract_iso_to_temp, out_dir);
    char *temp_dir = calloc(9, 1);
    strncpy(temp_dir, "temp_dir", 9);
    will_return(__wrap_extract_iso_to_temp, temp_dir);
    will_return(__wrap_extract_iso_to_temp, true);

    expect_any(__wrap_unshield_extract, cab_path);
    expect_string(__wrap_unshield_extract, dest_dir, "/installation/dir");
    will_return(__wrap_unshield_extract, true);

    expect_any(__wrap_copy_tree, src);
    expect_any(__wrap_copy_tree, dest);
    will_return(__wrap_copy_tree, true);

    expect_string(__wrap_remove_tree, path, "temp_dir");
    will_return(__wrap_remove_tree, false);

    bool result =
        install_re3_game_data("/path/to/disc1.iso", "/path/to/disc2", "/installation/dir");
    assert_false(result);
}

static void test_install_re3_clean_up_disc2_temp_dir(void **state) {
    (void)state;
    expect_string(__wrap_exists, path, "/installation/dir");
    will_return(__wrap_exists, false);

    expect_string(__wrap_is_dir_empty, path, "/installation/dir");
    will_return(__wrap_is_dir_empty, true);

    expect_string(__wrap_ends_with_iso, path, "/path/to/disc1");
    will_return(__wrap_ends_with_iso, false);

    expect_string(__wrap_ends_with_iso, path, "/path/to/disc2.iso");
    will_return(__wrap_ends_with_iso, true);

    expect_string(__wrap_extract_iso_to_temp, iso_path, "/path/to/disc2.iso");
    expect_any(__wrap_extract_iso_to_temp, out_dir);
    char *temp_dir = calloc(9, 1);
    strncpy(temp_dir, "temp_dir", 9);
    will_return(__wrap_extract_iso_to_temp, temp_dir);
    will_return(__wrap_extract_iso_to_temp, true);

    expect_any(__wrap_unshield_extract, cab_path);
    expect_string(__wrap_unshield_extract, dest_dir, "/installation/dir");
    will_return(__wrap_unshield_extract, true);

    expect_any(__wrap_copy_tree, src);
    expect_any(__wrap_copy_tree, dest);
    will_return(__wrap_copy_tree, true);

    expect_string(__wrap_remove_tree, path, "temp_dir");
    will_return(__wrap_remove_tree, false);

    bool result =
        install_re3_game_data("/path/to/disc1", "/path/to/disc2.iso", "/installation/dir");
    assert_false(result);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_install_re3_game_data_success),
        cmocka_unit_test(test_install_re3_game_data_non_empty_dir),
        cmocka_unit_test(test_install_re3_game_data_failed_extract_iso),
        cmocka_unit_test(test_install_re3_game_data_failed_copy_tree),
        cmocka_unit_test(test_install_re3_game_data_fail_to_extract_iso_disc2),
        cmocka_unit_test(test_install_re3_unshield_extractor_failure),
        cmocka_unit_test(test_install_re3_clean_up_disc1_temp_dir),
        cmocka_unit_test(test_install_re3_clean_up_disc2_temp_dir),
    };
    return cmocka_run_group_tests(tests, nullptr, nullptr);
}
