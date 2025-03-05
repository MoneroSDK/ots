#include <check.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "../include/ots.h"
#include "../include/ots-version.h"

START_TEST(test_ots_version)
{
    ots_result_t* result = ots_version();
    ck_assert_int_eq(result->error.code, 0);
    ck_assert_str_eq(ots_result_string(result), OTS_VERSION_STRING);
    ots_free_result(&result);

    result = ots_version_components();
    ck_assert_int_eq(result->error.code, 0);
    int* components = (int*)ots_result_array(result);
    ck_assert_int_eq(components[0], OTS_VERSION_MAJOR);
    ck_assert_int_eq(components[1], OTS_VERSION_MINOR);
    ck_assert_int_eq(components[2], OTS_VERSION_PATCH);
    ots_free_array((void*)&components, sizeof(int), 3);
    ots_free_result(&result);
}
END_TEST

Suite* ots_suite(void)
{
    Suite* s = suite_create("OTS");
    
    // Core functionality
    TCase* tc_core = tcase_create("Core");
    tcase_add_test(tc_core, test_ots_version);
    suite_add_tcase(s, tc_core);
    
    return s;
}
