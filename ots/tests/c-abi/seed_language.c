#include <check.h>
#include "../include/ots.h"
#include "../data.h"

#include <stdio.h>

START_TEST(test_seed_languages)
{
    ots_result_t* result = ots_seed_languages();
    ck_assert(ots_result_data_handle_is_seed_language(result));
    ck_assert_int_eq(ots_result_size(result), 15);
    ots_free_result(&result);
}
END_TEST

START_TEST(test_seed_languages_for_type)
{
    ots_result_t* result = ots_seed_languages_for_type(OTS_SEED_TYPE_MONERO);
    ck_assert(ots_result_data_handle_is_seed_language(result));
    ck_assert_int_eq(ots_result_size(result), 12);
    ots_free_result(&result);
    result = ots_seed_languages_for_type(OTS_SEED_TYPE_POLYSEED);
    ck_assert(ots_result_data_handle_is_seed_language(result));
    ck_assert_int_eq(ots_result_size(result), 10);
    ots_free_result(&result);
}
END_TEST

START_TEST(test_seed_language_for_code)
{
    ots_result_t* result = ots_seed_language_from_code("de");
    ck_assert(ots_result_is_seed_language(result));
    ots_handle_t* de = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_language_english_name(de);
    ck_assert(ots_result_is_string(result));
    ck_assert_str_eq(ots_result_string(result), "German");
    ots_free_result(&result);
    ots_free_handle(&de);
}
END_TEST

START_TEST(test_seed_language_for_name)
{
    ots_result_t* result = ots_seed_language_from_name("Deutsch");
    ck_assert(ots_result_is_seed_language(result));
    ots_handle_t* de = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_language_english_name(de);
    ck_assert(ots_result_is_string(result));
    ck_assert_str_eq(ots_result_string(result), "German");
    ots_free_result(&result);
    ots_free_handle(&de);
}
END_TEST

START_TEST(test_seed_language_for_english_name)
{
    ots_result_t* result = ots_seed_language_from_english_name("Spanish");
    ck_assert(ots_result_is_seed_language(result));
    ots_handle_t* es = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_language_code(es);
    ck_assert(ots_result_is_string(result));
    ck_assert_str_eq(ots_result_string(result), "es");
    ots_free_result(&result);
    ots_free_handle(&es);
}
END_TEST

START_TEST(test_seed_language_default)
{
    ots_result_t* result = ots_seed_language_default(OTS_SEED_TYPE_MONERO);
    ck_assert(ots_is_error(result)); // no default set
    result = ots_seed_language_from_code("ru");
    ck_assert(ots_result_is_seed_language(result));
    ots_handle_t* ru = ots_result_handle(result);
    result = ots_seed_language_set_default(OTS_SEED_TYPE_MONERO, ru);
    ck_assert(ots_result_is_seed_language(result));
    ots_handle_t* lang = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_language_english_name(lang);
    ck_assert(ots_result_is_string(result));
    ck_assert_str_eq(ots_result_string(result), "Russian");
    ots_free_result(&result);
    result = ots_seed_language_default(OTS_SEED_TYPE_MONERO);
    ck_assert(ots_result_is_seed_language(result));
    lang = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_language_english_name(lang);
    ck_assert(ots_result_is_string(result));
    ck_assert_str_eq(ots_result_string(result), "Russian");
    ots_free_result(&result);
    ots_free_handle(&lang);
}
END_TEST

START_TEST(test_seed_language_data)
{
    for(size_t i = 0; i < get_seed_language_test_cases_count(); i++) {
        if(!get_seed_language_test_case_valid(i))
            continue;
        const char* code = get_seed_language_test_case_code(i);
        const char* name = get_seed_language_test_case_name(i);
        const char* english_name = get_seed_language_test_case_english_name(i);
        bool monero_supported = get_seed_language_test_case_monero_supported(i);
        bool polyseed_supported = get_seed_language_test_case_polyseed_supported(i);
        ots_result_t* result = ots_seed_language_from_code(code);
        ck_assert(ots_result_is_seed_language(result));
        ots_handle_t* lang = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_seed_language_code(lang);
        ck_assert(ots_result_is_string(result));
        ck_assert_str_eq(ots_result_string(result), code);
        ots_free_result(&result);
        result = ots_seed_language_name(lang);
        ck_assert(ots_result_is_string(result));
        ck_assert_str_eq(ots_result_string(result), name);
        ots_free_result(&result);
        result = ots_seed_language_english_name(lang);
        ck_assert(ots_result_is_string(result));
        ck_assert_str_eq(ots_result_string(result), english_name);
        ots_free_result(&result);
        result = ots_seed_language_supported(lang, OTS_SEED_TYPE_MONERO);
        ck_assert(ots_result_is_boolean(result));
        ck_assert_int_eq(ots_result_boolean(result, false), monero_supported);
        ots_free_result(&result);
        result = ots_seed_language_supported(lang, OTS_SEED_TYPE_POLYSEED);
        ck_assert(ots_result_is_boolean(result));
        ck_assert_int_eq(ots_result_boolean(result, false), polyseed_supported);
        ots_free_result(&result);
        ots_free_handle(&lang);
    }
}
END_TEST

START_TEST(test_seed_languages_equals)
{
    ots_result_t* result = ots_seed_language_from_code("en");
    ck_assert(ots_result_is_seed_language(result));
    ots_handle_t* en = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_language_from_english_name("English");
    ck_assert(ots_result_is_seed_language(result));
    ots_handle_t* en2 = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_language_from_code("es");
    ck_assert(ots_result_is_seed_language(result));
    ots_handle_t* es = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_language_equals(en, en2);
    ck_assert(ots_result_is_boolean(result));
    ck_assert(ots_result_boolean(result, false));
    ots_free_result(&result);
    result = ots_seed_language_equals(en, es);
    ck_assert(ots_result_is_boolean(result));
    ck_assert(!ots_result_boolean(result, false));
    ots_free_result(&result);
    ots_free_handle(&en);
    ots_free_handle(&en2);
    ots_free_handle(&es);
}
END_TEST

START_TEST(test_seed_language_equals_code)
{
    for(size_t i = 0; i < get_seed_language_test_cases_count(); i++) {
        if(!get_seed_language_test_case_valid(i))
            continue;
        const char* code = get_seed_language_test_case_code(i);
        ots_result_t* result = ots_seed_language_from_code(code);
        ck_assert(ots_result_is_seed_language(result));
        ots_handle_t* lang = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_seed_language_equals_code(lang, code);
        ck_assert(ots_result_is_boolean(result));
        ck_assert(ots_result_boolean(result, false));
        ots_free_result(&result);
        result = ots_seed_language_equals_code(lang, "xx");
        ck_assert(ots_result_is_boolean(result));
        ck_assert(!ots_result_boolean(result, false));
        ots_free_result(&result);
        ots_free_handle(&lang);
    }
}


Suite* seed_language_suite(void)
{
    Suite* s = suite_create("Seed Language");
    // Core functionality
    TCase* tc_core = tcase_create("Core");
    tcase_add_test(tc_core, test_seed_languages);
    tcase_add_test(tc_core, test_seed_languages_for_type);
    tcase_add_test(tc_core, test_seed_language_for_code);
    tcase_add_test(tc_core, test_seed_language_for_name);
    tcase_add_test(tc_core, test_seed_language_for_english_name);
    tcase_add_test(tc_core, test_seed_language_default);
    tcase_add_test(tc_core, test_seed_language_data);
    tcase_add_test(tc_core, test_seed_languages_equals);
    tcase_add_test(tc_core, test_seed_language_equals_code);
    suite_add_tcase(s, tc_core);
    return s;
}
