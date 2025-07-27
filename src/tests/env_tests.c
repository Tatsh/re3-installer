#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <cmocka.h>

#include "env.h"

static void test_env_variable_exists(void **state) {
    (void)state; // Unused parameter
#ifdef _WIN32
    _putenv("TEST_ENV_VAR=TEST_VALUE");
#else
    setenv("TEST_ENV_VAR", "TEST_VALUE", 1);
#endif
    char *result = env("TEST_ENV_VAR");
    assert_non_null(result);
    assert_string_equal(result, "TEST_VALUE");
    free(result);
}

static void test_env_variable_does_not_exist(void **state) {
    (void)state; // Unused parameter
#ifdef _WIN32
    _putenv("TEST_ENV_VAR=");
#else
    unsetenv("TEST_ENV_VAR");
#endif
    char *result = env("TEST_ENV_VAR");
    assert_non_null(result);
    assert_string_equal(result, ""); // Expect an empty string if the variable doesn't exist
    free(result);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_env_variable_exists),
        cmocka_unit_test(test_env_variable_does_not_exist),
    };
    return cmocka_run_group_tests(tests, nullptr, nullptr);
}
