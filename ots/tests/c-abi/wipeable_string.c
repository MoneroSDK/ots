#include <check.h>
#include "../include/ots.h"

START_TEST(test_wipeable_string)
{
    // define test strings
    const char* s1 = "Hello, World!";
    const char* s2 = "Hello, World!";
    const char* s3 = "Hello, Universe!";
    // Create a wipeable string ws1
    ots_result_t* result = ots_wipeable_string_create(s1);
    ck_assert_int_eq(ots_is_result(result), true);
    ck_assert_int_eq(ots_result_is_type(result, OTS_RESULT_HANDLE), true);
    ots_handle_t* ws1 = ots_result_handle(result);
    ots_free_result(&result);

    // Create a wipeable string ws2
    result = ots_wipeable_string_create(s2);
    ck_assert_int_eq(ots_is_result(result), true);
    ck_assert_int_eq(ots_result_is_type(result, OTS_RESULT_HANDLE), true);
    ots_handle_t* ws2 = ots_result_handle(result);
    ots_free_result(&result);

    // Compare ws1 and ws2
    result = ots_wipeable_string_compare(ws1, ws2);
    ck_assert_int_eq(ots_is_result(result), true);
    ck_assert_int_eq(ots_result_is_type(result, OTS_RESULT_COMPARISON), true);
    ck_assert_int_eq(ots_result_comparison(result), 0);
    ck_assert_int_eq(ots_result_is_equal(result), true);
    ots_free_result(&result);

    // Change ws2
    ots_free_handle(&ws2);
    result = ots_wipeable_string_create(s3);
    ck_assert_int_eq(ots_is_result(result), true);
    ck_assert_int_eq(ots_result_is_type(result, OTS_RESULT_HANDLE), true);
    ws2 = ots_result_handle(result);

    // Compare ws1 and ws2 again
    result = ots_wipeable_string_compare(ws1, ws2);
    ck_assert_int_eq(ots_is_result(result), true);
    ck_assert_int_eq(ots_result_is_type(result, OTS_RESULT_COMPARISON), true);
    ck_assert_int_ne(ots_result_comparison(result), 0);
    ck_assert_int_eq(ots_result_is_equal(result), false);
    ots_free_result(&result);

    // Get c_str from ws1
    const char* str1 = ots_wipeable_string_c_str(ws1);
    ck_assert_ptr_nonnull(str1);
    ck_assert_str_eq(str1, s1);

    // Free wipeable strings
    ots_free_handle(&ws1);
    str1 = ots_wipeable_string_c_str(ws1);
    ck_assert_ptr_null(str1);
    ots_free_handle(&ws2);
    ots_free_string((char **)&str1); // allowed to free `const char**`
    ck_assert_ptr_null(str1);
    const char* str2 = ots_wipeable_string_c_str(ws2);
    ck_assert_ptr_null(str2);
}
END_TEST

Suite* wipeable_string_suite(void)
{
    Suite* s = suite_create("Wipeable String");
    // Core functionality
    TCase* tc_core = tcase_create("Core");
    tcase_add_test(tc_core, test_wipeable_string);
    suite_add_tcase(s, tc_core);
    return s;
}
