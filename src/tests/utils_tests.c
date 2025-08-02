#include <fcntl.h>
#include <limits.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cmocka.h>

#include "support.h"
#include "utils.h"

bool ends_with_ext_gcda(const char *s) {
    return strlen(s) > 5 && !strcmp(s + strlen(s) - 5, ".gcda");
}

bool is_gcda(int fno) {
    char proc_lnk[32];
    ssize_t r;
    char filename[PATH_MAX];
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

int __real_close(int fd);
int __wrap_close(int fd) {
    if (is_gcda(fd)) {
        return __real_close(fd);
    }
    check_expected(fd);
    return mock_type(int);
}

char *__wrap_env(const char *name) {
    check_expected_ptr(name);
    return mock_ptr_type(char *);
}

void *
__wrap_fts_open(char *const *path_argv, int options, int (*compar)(const void *, const void *)) {
    (void)compar;
    check_expected_ptr(path_argv);
    check_expected(options);
    return mock_ptr_type(void *);
}

void *__wrap_fts_read(void *fts) {
    check_expected_ptr(fts);
    return mock_ptr_type(void *);
}

int __wrap_fts_close(void *fts) {
    check_expected_ptr(fts);
    return mock_type(int);
}

int __wrap_mkdir_p(const char *path) {
    check_expected_ptr(path);
    return mock_type(int);
}

int __real_open(const char *path, int flags, ...);
int __wrap_open(const char *path, int flags, ...) {
    if (ends_with_ext_gcda(path)) {
        int real_fd = __real_open(path, flags);
        return real_fd;
    }
    check_expected_ptr(path);
    check_expected(flags);
    return mock_type(int);
}

int __wrap_rmdir(const char *path) {
    check_expected_ptr(path);
    return mock_type(int);
}

int __wrap_unlink(const char *path) {
    check_expected_ptr(path);
    return mock_type(int);
}

ssize_t __wrap_sendfile(int out_fd, int in_fd, off_t *offset, size_t count) {
    check_expected(out_fd);
    check_expected(in_fd);
    check_expected_ptr(offset);
    check_expected(count);
    return mock_type(ssize_t);
}

int __wrap_stat(const char *path, struct stat *buf) {
    check_expected_ptr(path);
    check_expected_ptr(buf);
    return mock_type(int);
}

static void test_is_directory_stat_returns_negative(void **state) {
    (void)state;
    // Expect stat to be called with the given path and return -1
    expect_string(__wrap_stat, path, "notadir");
    expect_any(__wrap_stat, buf);
    will_return(__wrap_stat, -1);

    bool result = is_directory("notadir");
    assert_false(result);
}

static void test_get_installation_dir_xdg_data_home_not_empty(void **state) {
    (void)state;
    // Expect env to be called with 'XDG_DATA_HOME' and return zero-length string
    expect_string(__wrap_env, name, "HOME");
    will_return(__wrap_env, strdup("/home/user"));

    expect_string(__wrap_env, name, "XDG_DATA_HOME");
    will_return(__wrap_env, strdup("/home/user/.local/share"));

    char *dir = get_installation_dir();
    assert_non_null(dir);
    // Should not be empty, as fallback is used
    assert_true(strlen(dir) > 0);
    assert_string_equal(dir, "/home/user/.local/share/re3");
    free(dir);
}

static void test_copy_tree_fts_open_returns_null(void **state) {
    (void)state;
    // Expect fts_open to be called with correct arguments
    expect_any(__wrap_fts_open, path_argv);
    expect_any(__wrap_fts_open, options);
    will_return(__wrap_fts_open, NULL);

    bool result = copy_tree("src_dir", "dst_dir");
    assert_false(result);
}

static void test_copy_tree_mkdir_p_returns_negative(void **state) {
    (void)state;
    // Expect fts_open to be called and return a non-null pointer
    FTS dummy_fts = {0};
    expect_any(__wrap_fts_open, path_argv);
    expect_any(__wrap_fts_open, options);
    will_return(__wrap_fts_open, &dummy_fts);

    // Expect fts_read to be called and return a non-null pointer
    expect_any(__wrap_fts_read, fts);
    FTSENT dummy_ftsent = {0};
    dummy_ftsent.fts_info = FTS_D; // Simulate a directory
    char dummy_path[] = "src_dir";
    dummy_ftsent.fts_path = dummy_path;
    will_return(__wrap_fts_read, &dummy_ftsent);

    // Expect mkdir_p to be called and return -1
    expect_any(__wrap_mkdir_p, path);
    will_return(__wrap_mkdir_p, -1);

    // Expect fts_close to be called
    expect_any(__wrap_fts_close, fts);
    will_return(__wrap_fts_close, 0);

    bool result = copy_tree("src_dir", "dst_dir");
    assert_false(result);
}

static void test_copy_tree_open_src_returns_negative(void **state) {
    (void)state;
    // Simulate open() returning -1
    FTS dummy_fts = {0};
    expect_any(__wrap_fts_open, path_argv);
    expect_any(__wrap_fts_open, options);
    will_return(__wrap_fts_open, &dummy_fts);

    // Simulate reading a file entry
    expect_any(__wrap_fts_read, fts);
    FTSENT dummy_ftsent = {0};
    dummy_ftsent.fts_info = FTS_F;
    char dummy_path[] = "src_file";
    dummy_ftsent.fts_path = dummy_path;
    will_return(__wrap_fts_read, &dummy_ftsent);

    expect_any(__wrap_open, path);
    expect_value(__wrap_open, flags, O_RDONLY);
    will_return(__wrap_open, -1);

    expect_any(__wrap_fts_close, fts);
    will_return(__wrap_fts_close, 0);

    bool result = copy_tree("src_dir", "dst_dir");
    free(dummy_ftsent.fts_statp);
    assert_false(result);
}

static void test_copy_tree_open_dest_returns_negative(void **state) {
    (void)state;
    FTS dummy_fts = {0};
    expect_any(__wrap_fts_open, path_argv);
    expect_any(__wrap_fts_open, options);
    will_return(__wrap_fts_open, &dummy_fts);

    // First file entry
    expect_any(__wrap_fts_read, fts);
    FTSENT first_ftsent = {0};
    first_ftsent.fts_info = FTS_F;
    first_ftsent.fts_statp = malloc(sizeof(struct stat));
    assert_non_null(first_ftsent.fts_statp);
    first_ftsent.fts_statp->st_mode = 0;
    char first_path[] = "src_file1";
    first_ftsent.fts_path = first_path;
    will_return(__wrap_fts_read, &first_ftsent);

    // First open succeeds
    expect_any(__wrap_open, path);
    expect_value(__wrap_open, flags, O_RDONLY);
    will_return(__wrap_open, 3);

    // Second open fails
    expect_any(__wrap_open, path);
    expect_any(__wrap_open, flags);
    will_return(__wrap_open, -1);

    expect_any(__wrap_fts_close, fts);
    will_return(__wrap_fts_close, 0);

    expect_any(__wrap_close, fd);
    will_return(__wrap_close, 0);

    bool result = copy_tree("src_dir", "dst_dir");
    free(first_ftsent.fts_statp);
    assert_false(result);
}

static void test_copy_tree_sendfile_returns_negative(void **state) {
    (void)state;
    FTS dummy_fts = {0};
    expect_any(__wrap_fts_open, path_argv);
    expect_any(__wrap_fts_open, options);
    will_return(__wrap_fts_open, &dummy_fts);

    expect_any(__wrap_fts_read, fts);
    FTSENT dummy_ftsent = {0};
    dummy_ftsent.fts_info = FTS_F;
    dummy_ftsent.fts_statp = malloc(sizeof(struct stat));
    assert_non_null(dummy_ftsent.fts_statp);
    dummy_ftsent.fts_statp->st_size = 1234;
    dummy_ftsent.fts_statp->st_mode = 0644;
    char dummy_path[] = "src_file";
    dummy_ftsent.fts_path = dummy_path;
    will_return(__wrap_fts_read, &dummy_ftsent);

    expect_any(__wrap_open, path);
    expect_value(__wrap_open, flags, O_RDONLY);
    will_return(__wrap_open, 3);

    expect_any(__wrap_open, path);
    expect_any(__wrap_open, flags);
    will_return(__wrap_open, 4);

    expect_any(__wrap_sendfile, out_fd);
    expect_any(__wrap_sendfile, in_fd);
    expect_any(__wrap_sendfile, offset);
    expect_any(__wrap_sendfile, count);
    will_return(__wrap_sendfile, -1);

    expect_any(__wrap_close, fd);
    will_return(__wrap_close, 0);

    expect_any(__wrap_close, fd);
    will_return(__wrap_close, 0);

    expect_any(__wrap_fts_close, fts);
    will_return(__wrap_fts_close, 0);

    bool result = copy_tree("src_dir", "dst_dir");
    free(dummy_ftsent.fts_statp);
    assert_false(result);
}

static void test_copy_tree_fts_read_returns_fts_err(void **state) {
    (void)state;
    FTS dummy_fts = {0};
    expect_any(__wrap_fts_open, path_argv);
    expect_any(__wrap_fts_open, options);
    will_return(__wrap_fts_open, &dummy_fts);

    expect_any(__wrap_fts_read, fts);
    FTSENT dummy_ftsent = {0};
    dummy_ftsent.fts_info = FTS_ERR;
    char dummy_path[] = "err_node";
    dummy_ftsent.fts_path = dummy_path;
    will_return(__wrap_fts_read, &dummy_ftsent);

    expect_any(__wrap_fts_close, fts);
    will_return(__wrap_fts_close, 0);

    bool result = copy_tree("src_dir", "dst_dir");
    assert_false(result);
}

static void test_copy_tree_fts_read_returns_fts_ns(void **state) {
    (void)state;
    FTS dummy_fts = {0};
    expect_any(__wrap_fts_open, path_argv);
    expect_any(__wrap_fts_open, options);
    will_return(__wrap_fts_open, &dummy_fts);

    expect_any(__wrap_fts_read, fts);
    FTSENT dummy_ftsent = {0};
    dummy_ftsent.fts_info = FTS_NS;
    char dummy_path[] = "ns_node";
    dummy_ftsent.fts_path = dummy_path;
    will_return(__wrap_fts_read, &dummy_ftsent);

    expect_any(__wrap_fts_close, fts);
    will_return(__wrap_fts_close, 0);

    bool result = copy_tree("src_dir", "dst_dir");
    assert_false(result);
}

static void test_remove_tree_fts_open_returns_null(void **state) {
    (void)state;
    expect_any(__wrap_fts_open, path_argv);
    expect_any(__wrap_fts_open, options);
    will_return(__wrap_fts_open, NULL);

    bool result = remove_tree("test_dir");
    assert_false(result);
}

static void test_remove_tree_rmdir_returns_negative(void **state) {
    (void)state;
    // Expect fts_open to be called and return a non-null pointer
    FTS dummy_fts = {0};
    expect_any(__wrap_fts_open, path_argv);
    expect_any(__wrap_fts_open, options);
    will_return(__wrap_fts_open, &dummy_fts);

    // Expect fts_read to be called and return a directory entry
    expect_any(__wrap_fts_read, fts);
    FTSENT dummy_ftsent = {0};
    dummy_ftsent.fts_info = FTS_DP; // Directory post-order
    char dummy_path[] = "test_dir";
    dummy_ftsent.fts_path = dummy_path;
    will_return(__wrap_fts_read, &dummy_ftsent);

    // Expect rmdir to be called and return -1
    expect_any(__wrap_rmdir, path);
    will_return(__wrap_rmdir, -1);

    // Expect fts_close to be called
    expect_any(__wrap_fts_close, fts);
    will_return(__wrap_fts_close, 0);

    bool result = remove_tree("test_dir");
    assert_false(result);
}

static void test_remove_tree_unlink_returns_negative(void **state) {
    (void)state;
    // Expect fts_open to be called and return a non-null pointer
    FTS dummy_fts = {0};
    expect_any(__wrap_fts_open, path_argv);
    expect_any(__wrap_fts_open, options);
    will_return(__wrap_fts_open, &dummy_fts);

    // Expect fts_read to be called and return a file entry
    expect_any(__wrap_fts_read, fts);
    FTSENT dummy_ftsent = {0};
    dummy_ftsent.fts_info = FTS_F; // File
    char dummy_path[] = "test_file";
    dummy_ftsent.fts_path = dummy_path;
    will_return(__wrap_fts_read, &dummy_ftsent);

    // Expect unlink to be called and return -1
    expect_any(__wrap_unlink, path);
    will_return(__wrap_unlink, -1);

    // Expect fts_close to be called
    expect_any(__wrap_fts_close, fts);
    will_return(__wrap_fts_close, 0);

    bool result = remove_tree("test_file");
    assert_false(result);
}

static void test_remove_tree_fts_read_returns_fts_err(void **state) {
    (void)state;
    // Expect fts_open to be called and return a non-null pointer
    FTS dummy_fts = {0};
    expect_any(__wrap_fts_open, path_argv);
    expect_any(__wrap_fts_open, options);
    will_return(__wrap_fts_open, &dummy_fts);

    // Expect fts_read to be called and return a node with FTS_ERR
    expect_any(__wrap_fts_read, fts);
    FTSENT dummy_ftsent = {0};
    dummy_ftsent.fts_info = FTS_ERR;
    char dummy_path[] = "err_node";
    dummy_ftsent.fts_path = dummy_path;
    will_return(__wrap_fts_read, &dummy_ftsent);

    // Expect fts_close to be called
    expect_any(__wrap_fts_close, fts);
    will_return(__wrap_fts_close, 0);

    bool result = remove_tree("err_node");
    assert_false(result);
}

static void test_remove_tree_fts_read_returns_fts_ns(void **state) {
    (void)state;
    // Expect fts_open to be called and return a non-null pointer
    FTS dummy_fts = {0};
    expect_any(__wrap_fts_open, path_argv);
    expect_any(__wrap_fts_open, options);
    will_return(__wrap_fts_open, &dummy_fts);

    // Expect fts_read to be called and return a node with FTS_NS
    expect_any(__wrap_fts_read, fts);
    FTSENT dummy_ftsent = {0};
    dummy_ftsent.fts_info = FTS_NS;
    char dummy_path[] = "ns_node";
    dummy_ftsent.fts_path = dummy_path;
    will_return(__wrap_fts_read, &dummy_ftsent);

    // Expect fts_close to be called
    expect_any(__wrap_fts_close, fts);
    will_return(__wrap_fts_close, 0);

    bool result = remove_tree("ns_node");
    assert_false(result);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_is_directory_stat_returns_negative),
        cmocka_unit_test(test_copy_tree_fts_open_returns_null),
        cmocka_unit_test(test_copy_tree_fts_read_returns_fts_err),
        cmocka_unit_test(test_copy_tree_fts_read_returns_fts_ns),
        cmocka_unit_test(test_copy_tree_mkdir_p_returns_negative),
        cmocka_unit_test(test_copy_tree_open_dest_returns_negative),
        cmocka_unit_test(test_copy_tree_open_src_returns_negative),
        cmocka_unit_test(test_copy_tree_sendfile_returns_negative),
        cmocka_unit_test(test_get_installation_dir_xdg_data_home_not_empty),
        cmocka_unit_test(test_remove_tree_fts_open_returns_null),
        cmocka_unit_test(test_remove_tree_fts_read_returns_fts_err),
        cmocka_unit_test(test_remove_tree_fts_read_returns_fts_ns),
        cmocka_unit_test(test_remove_tree_rmdir_returns_negative),
        cmocka_unit_test(test_remove_tree_unlink_returns_negative),
    };
    return cmocka_run_group_tests(tests, nullptr, nullptr);
}
