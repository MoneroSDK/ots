#include <stdbool.h>
#include <check.h>
#include "../include/ots.h"
#include "../include/ots-version.h"
#include <string.h>
#include <stdio.h>

// TODO: LLM generated placeholder unit test, so everything compiles for now and nothing explodes on running the `make test`.
//       Throw away this placeholder test and write real tests for the OTS library.
//
// Test version functions
START_TEST(test_version)
{
    ots_result_t result = ots_version();
    ck_assert_int_eq(result.error.code, 0);
    ck_assert_str_eq((char*)result.result.ptr, OTS_VERSION_STRING);
    ots_free_string((char*)result.result.ptr);

    result = ots_version_components();
    ck_assert_int_eq(result.error.code, 0);
    int* components = (int*)result.result.ptr;
    ck_assert_int_eq(components[0], OTS_VERSION_MAJOR);
    ck_assert_int_eq(components[1], OTS_VERSION_MINOR);
    ck_assert_int_eq(components[2], OTS_VERSION_PATCH);
    ots_free_array(components);
}
END_TEST

// Test error handling
START_TEST(test_error_handling)
{
    ots_error_t error;
    ots_error_init(&error);
    ck_assert_int_eq(error.code, 0);
    ck_assert_str_eq(error.message, "");
    
    ots_result_t result = ots_get_last_error();
    ck_assert_str_eq((char*)result.result.ptr, "Not implemented");
    ots_free_string((char*)result.result.ptr);
    
    result = ots_get_error_message(-1);
    ck_assert_str_eq((char*)result.result.ptr, "Not implemented");
    ots_free_string((char*)result.result.ptr);
}
END_TEST

// Create test suite
Suite* ots_suite(void)
{
    Suite* s = suite_create("OTS");
    
    // Core functionality
    TCase* tc_core = tcase_create("Core");
    tcase_add_test(tc_core, test_version);
    tcase_add_test(tc_core, test_error_handling);
    suite_add_tcase(s, tc_core);
    
    return s;
}

int main(void)
{
    int number_failed;
    Suite* s = ots_suite();
    SRunner* sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return number_failed;
}
