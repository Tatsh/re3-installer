#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>

#include "installer.h"

bool __wrap_is_dir_empty(const char *path) {
    check_expected_ptr(path);
    return mock_type(bool);
}

bool __wrap_is_iso(const char *path) {
    check_expected_ptr(path);
    return mock_type(bool);
}

bool __wrap_extract_iso_to_temp(const char *iso_path, char **out_dir) {
    check_expected_ptr(iso_path);
    check_expected_ptr(out_dir);
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

    expect_string(__wrap_is_dir_empty, path, "/installation/dir");
    will_return(__wrap_is_dir_empty, true);

    expect_string(__wrap_is_iso, path, "/path/to/disc1");
    will_return(__wrap_is_iso, false);

    expect_string(__wrap_is_iso, path, "/path/to/disc2");
    will_return(__wrap_is_iso, false);

    expect_string(__wrap_unshield_extract, cab_path, "/path/to/disc1/data1.cab");
    expect_string(__wrap_unshield_extract, dest_dir, "/installation/dir");
    will_return(__wrap_unshield_extract, true);

    expect_string(__wrap_copy_tree, src, "/path/to/disc2/Audio");
    expect_string(__wrap_copy_tree, dest, "/installation/dir/Audio");
    will_return(__wrap_copy_tree, true);

    bool result = install_re3_game_data("/path/to/disc1", "/path/to/disc2", "/installation/dir");
    assert_false(result);
}

static void test_install_re3_game_data_non_empty_dir(void **state) {
    (void)state;

    expect_string(__wrap_is_dir_empty, path, "/installation/dir");
    will_return(__wrap_is_dir_empty, false);

    bool result = install_re3_game_data("/path/to/disc1", "/path/to/disc2", "/installation/dir");
    assert_true(result);
}

static void test_install_re3_game_data_failed_extract_iso(void **state) {
    (void)state;

    expect_string(__wrap_is_dir_empty, path, "/installation/dir");
    will_return(__wrap_is_dir_empty, true);

    expect_string(__wrap_is_iso, path, "/path/to/disc1");
    will_return(__wrap_is_iso, true);

    expect_string(__wrap_extract_iso_to_temp, iso_path, "/path/to/disc1");
    expect_any(__wrap_extract_iso_to_temp, out_dir);
    will_return(__wrap_extract_iso_to_temp, false);

    bool result = install_re3_game_data("/path/to/disc1", "/path/to/disc2", "/installation/dir");
    assert_true(result);
}

static void test_install_re3_game_data_failed_copy_tree(void **state) {
    (void)state;

    expect_string(__wrap_is_dir_empty, path, "/installation/dir");
    will_return(__wrap_is_dir_empty, true);

    expect_string(__wrap_is_iso, path, "/path/to/disc1");
    will_return(__wrap_is_iso, false);

    expect_string(__wrap_is_iso, path, "/path/to/disc2");
    will_return(__wrap_is_iso, false);

    expect_string(__wrap_unshield_extract, cab_path, "/path/to/disc1/data1.cab");
    expect_string(__wrap_unshield_extract, dest_dir, "/installation/dir");
    will_return(__wrap_unshield_extract, true);

    expect_string(__wrap_copy_tree, src, "/path/to/disc2/Audio");
    expect_string(__wrap_copy_tree, dest, "/installation/dir/Audio");
    will_return(__wrap_copy_tree, false);

    bool result = install_re3_game_data("/path/to/disc1", "/path/to/disc2", "/installation/dir");
    assert_true(result);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_install_re3_game_data_success),
        cmocka_unit_test(test_install_re3_game_data_non_empty_dir),
        cmocka_unit_test(test_install_re3_game_data_failed_extract_iso),
        cmocka_unit_test(test_install_re3_game_data_failed_copy_tree),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
