#include <check.h>
#include "../include/ots.h"

/**
 * @file wipeable_string.c
 * @note Tests for WipeableString are a bit redudant
 * @todo Should maybe restructure the tests, but all covert, but
 *       redundant, ugly but not hurting (restructure when time allows,
 *       so probably never?)
 */

START_TEST(test_ots_wipeable_string_create)
{
    char* test_str = "Wipeable Secret";
    ots_result_t* result = ots_wipeable_string_create(test_str);
    ck_assert(ots_result_is_wipeable_string(result));
    ots_handle_t* ws = ots_result_handle(result);
    ots_free_result(&result);
    ck_assert_str_eq(ots_wipeable_string_c_str(ws), test_str);
    ots_free_handle(&ws);
}
END_TEST

START_TEST(test_ots_wipeable_string_compare)
{
    char* test_strings[5];
    test_strings[0] = "AAAAAA";
    test_strings[1] = "BBBBBB";
    test_strings[2] = "CCCCCC";
    test_strings[3] = "DDDDDD";
    test_strings[4] = "EEEEEE";
    ots_handle_t* ws[5];
    for(size_t i = 0; i < 5; i++) {
        ots_result_t* result = ots_wipeable_string_create(test_strings[i]);
        ck_assert(ots_result_is_wipeable_string(result));
        ws[i] = ots_result_handle(result);
        ots_free_result(&result);
    }
    ots_result_t* result = ots_wipeable_string_create(ots_wipeable_string_c_str(ws[2]));
    ck_assert(ots_result_is_wipeable_string(result));
    ots_handle_t* ws_ref = ots_result_handle(result);
    ots_free_result(&result);
    for(size_t i = 0; i < 5; i++) {
        result = ots_wipeable_string_compare(ws[i], ws_ref);
        ck_assert(ots_result_is_comparison(result));
        ck_assert(ots_result_is_equal(result) == (i==2));
        switch(i) {
            case 0:
            case 1:
                ck_assert_int_lt(ots_result_comparison(result), 0);
                break;
            case 2:
                ck_assert_int_eq(ots_result_comparison(result), 0);
                break;
            case 3:
            case 4:
                ck_assert_int_gt(ots_result_comparison(result), 0);
                break;
        }
    }
}
END_TEST

START_TEST(test_ots_wipeable_string_c_str)
{
    char* test_str = "Wipeable Secret";
    ots_result_t* result = ots_wipeable_string_create(test_str);
    ck_assert(ots_result_is_wipeable_string(result));
    ots_handle_t* ws = ots_result_handle(result);
    ots_free_result(&result);
    const char* c_str = ots_wipeable_string_c_str(ws);
    ck_assert_str_eq(c_str, test_str);
    ots_free_handle(&ws); // wipes c_str, too
    ck_assert_str_ne(c_str, test_str);
}
END_TEST


START_TEST(test_wipeable_string)
{
    // define test strings
    const char* s1 = "Hello, World!";
    const char* s2 = "Hello, World!";
    const char* s3 = "Hello, Universe!";
    // Create a wipeable string ws1
    ots_result_t* result = ots_wipeable_string_create(s1);
    ck_assert(ots_result_is_wipeable_string(result));
    ots_handle_t* ws1 = ots_result_handle(result);
    ots_free_result(&result);
    // Create a wipeable string ws2
    result = ots_wipeable_string_create(s2);
    ck_assert(ots_result_is_wipeable_string(result));
    ots_handle_t* ws2 = ots_result_handle(result);
    ots_free_result(&result);
    // Compare ws1 and ws2
    result = ots_wipeable_string_compare(ws1, ws2);
    ck_assert(ots_result_is_comparison(result));
    ck_assert_int_eq(ots_result_comparison(result), 0);
    ck_assert_int_eq(ots_result_is_equal(result), true);
    ots_free_result(&result);
    // Change ws2
    ots_free_handle(&ws2);
    result = ots_wipeable_string_create(s3);
    ck_assert(ots_result_is_wipeable_string(result));
    ws2 = ots_result_handle(result);
    const char* ws2_str = ots_result_string_copy(result);
    ck_assert_str_eq(ws2_str, s3);
    ots_free_result(&result);
    ots_free_string((char **)&ws2_str); // allowed to free `const char**`
    // Compare ws1 and ws2 again
    result = ots_wipeable_string_compare(ws1, ws2);
    ck_assert(ots_result_is_comparison(result));
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
    tcase_add_test(tc_core, test_ots_wipeable_string_create);
    tcase_add_test(tc_core, test_ots_wipeable_string_compare);
    tcase_add_test(tc_core, test_ots_wipeable_string_c_str);
    suite_add_tcase(s, tc_core);
    return s;
}
