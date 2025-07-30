#include <errno.h>
#include <limits.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cmocka.h>

#include "mkdir_p.h"

int __wrap_mkdir(const char *path, mode_t mode) {
    errno = mock_type(int);
    check_expected_ptr(path);
    check_expected(mode);
    return mock_type(int);
}

static void test_mkdir_p_simple(void **state) {
    (void)state;
    const char *path = "test_dir_simple";

    expect_string(__wrap_mkdir, path, path);
    expect_value(__wrap_mkdir, mode, S_IRWXU);
    will_return(__wrap_mkdir, 0);
    will_return(__wrap_mkdir, 0);

    assert_int_equal(mkdir_p(path), 0);
}

static void test_mkdir_p_nested(void **state) {
    (void)state;
    const char *path = "test_dir_nested/a/b/c";

    expect_string(__wrap_mkdir, path, "test_dir_nested");
    expect_value(__wrap_mkdir, mode, S_IRWXU);
    will_return(__wrap_mkdir, 0);
    will_return(__wrap_mkdir, 0);

    expect_string(__wrap_mkdir, path, "test_dir_nested/a");
    expect_value(__wrap_mkdir, mode, S_IRWXU);
    will_return(__wrap_mkdir, 0);
    will_return(__wrap_mkdir, 0);

    expect_string(__wrap_mkdir, path, "test_dir_nested/a/b");
    expect_value(__wrap_mkdir, mode, S_IRWXU);
    will_return(__wrap_mkdir, 0);
    will_return(__wrap_mkdir, 0);

    expect_string(__wrap_mkdir, path, "test_dir_nested/a/b/c");
    expect_value(__wrap_mkdir, mode, S_IRWXU);
    will_return(__wrap_mkdir, 0);
    will_return(__wrap_mkdir, 0);

    assert_int_equal(mkdir_p(path), 0);
}

static void test_mkdir_p_existing(void **state) {
    (void)state;
    const char *path = "test_dir_existing";

    expect_string(__wrap_mkdir, path, path);
    expect_value(__wrap_mkdir, mode, S_IRWXU);
    will_return(__wrap_mkdir, 0);
    will_return(__wrap_mkdir, 0);

    expect_string(__wrap_mkdir, path, path);
    expect_value(__wrap_mkdir, mode, S_IRWXU);
    will_return(__wrap_mkdir, 0);
    will_return(__wrap_mkdir, 0);

    assert_int_equal(mkdir_p(path), 0);
    // Call again, should succeed (EEXIST)
    assert_int_equal(mkdir_p(path), 0);
}

static void test_mkdir_p_long_path(void **state) {
    (void)state;
    char path[PATH_MAX];
    memset(path, 'a', sizeof(path) - 1);
    path[sizeof(path) - 1] = '\0';

    expect_string(__wrap_mkdir, path, path);
    expect_value(__wrap_mkdir, mode, S_IRWXU);
    will_return(__wrap_mkdir, ENAMETOOLONG);
    will_return(__wrap_mkdir, -1);

    int ret = mkdir_p(path);
    assert_true(ret == -1);
    assert_true(errno == ENAMETOOLONG);
}

static void test_mkdir_p_other_error(void **state) {
    (void)state;
    const char *path = "test_dir_other_error";

    expect_string(__wrap_mkdir, path, path);
    expect_value(__wrap_mkdir, mode, S_IRWXU);
    will_return(__wrap_mkdir, EACCES);
    will_return(__wrap_mkdir, -1);

    int ret = mkdir_p(path);
    assert_true(ret == -1);
    assert_true(errno == EACCES);
}

static void test_mkdir_p_nested_error(void **state) {
    (void)state;
    const char *path = "test_dir_nested_error/a/b/c";

    expect_string(__wrap_mkdir, path, "test_dir_nested_error");
    expect_value(__wrap_mkdir, mode, S_IRWXU);
    will_return(__wrap_mkdir, 0);
    will_return(__wrap_mkdir, 0);

    expect_string(__wrap_mkdir, path, "test_dir_nested_error/a");
    expect_value(__wrap_mkdir, mode, S_IRWXU);
    will_return(__wrap_mkdir, EACCES);
    will_return(__wrap_mkdir, -1);

    int ret = mkdir_p(path);
    assert_true(ret == -1);
    assert_true(errno == EACCES);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_mkdir_p_existing),
        cmocka_unit_test(test_mkdir_p_long_path),
        cmocka_unit_test(test_mkdir_p_nested),
        cmocka_unit_test(test_mkdir_p_simple),
        cmocka_unit_test(test_mkdir_p_other_error),
        cmocka_unit_test(test_mkdir_p_nested_error),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
