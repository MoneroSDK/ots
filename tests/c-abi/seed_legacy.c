#include <check.h>
#include "../../include/ots.h"
#include "../data.h"

START_TEST(test_seed_legacy_decode)
{
    for(size_t i = 0; get_legacy_seed_test_cases_count() > i; i++) {
        const char* phrase = get_legacy_seed_test_case_phrase(i);
        uint64_t height = get_legacy_seed_test_case_height(i);
        uint64_t time = get_legacy_seed_test_case_time(i);
        int network = get_legacy_seed_test_case_network(i);
        const char* fingerprint = get_legacy_seed_test_case_fingerprint(i);
        ots_result_t* result = ots_legacy_seed_decode(
            phrase,
            height,
            time,
            network
        );
        if(!get_legacy_seed_test_case_valid(i)) {
            ck_assert(ots_is_error(result));
            ots_free_result(&result);
            continue;
        }
        ck_assert(ots_is_result(result));
        ck_assert(ots_result_is_seed(result));
        ots_handle_t* seed1 = ots_result_handle(result);
        ots_free_result(&result);
        ck_assert_ptr_null(result);
        ck_assert_ptr_nonnull(seed1);
        result = ots_seed_fingerprint(seed1);
        ck_assert(ots_result_is_string(result));
        ck_assert_str_eq(ots_result_string(result), fingerprint);
        ck_assert_str_eq(ots_result_string(result), get_legacy_seed_test_case_fingerprint(i));
        ots_free_result(&result);
        ots_free_handle(&seed1);
    }
}
END_TEST

START_TEST(test_seed_legacy_decode_indices)
{
    ots_result_t* result = ots_legacy_seed_decode(
        get_legacy_seed_test_case_phrase(2),
        get_legacy_seed_test_case_height(2),
        get_legacy_seed_test_case_time(2),
        get_legacy_seed_test_case_network(2)
    );
    ck_assert(ots_is_result(result));
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* seed1 = ots_result_handle(result);
    ots_free_result(&result);
    ck_assert_ptr_null(result);
    ck_assert_ptr_nonnull(seed1);
    result = ots_seed_indices(seed1, "");
    ck_assert(ots_result_is_seed_indices(result));
    ots_handle_t* indices = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_legacy_seed_decode_indices(
        indices,
        get_legacy_seed_test_case_height(2),
        get_legacy_seed_test_case_time(2),
        get_legacy_seed_test_case_network(2)
    );
    ck_assert(ots_is_result(result));
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* seed2 = ots_result_handle(result);
    ots_free_result(&result);
    ck_assert_ptr_null(result);
    ck_assert_ptr_nonnull(seed2);
    result = ots_seed_fingerprint(seed2);
    ck_assert(ots_result_is_string(result));
    const char* fingerprint1 = (const char*)ots_result_string_copy(result);
    ots_free_result(&result);
    result = ots_seed_fingerprint(seed1);
    ck_assert(ots_result_is_string(result));
    const char* fingerprint2 = (const char*)ots_result_string_copy(result);
    ots_free_result(&result);
    ck_assert_str_eq(fingerprint1, fingerprint2);
    ck_assert_str_eq(fingerprint1, get_legacy_seed_test_case_fingerprint(2));
    ots_free_string((char**) &fingerprint1);
    ots_free_string((char**) &fingerprint2);
    ots_free_handle(&seed1);
    ots_free_handle(&seed2);
}
END_TEST

Suite* seed_legacy_suite(void)
{
    Suite* s = suite_create("Legacy Seed");
    // Core functionality
    TCase* tc_core = tcase_create("Core");
    tcase_add_test(tc_core, test_seed_legacy_decode);
    tcase_add_test(tc_core, test_seed_legacy_decode_indices);
    suite_add_tcase(s, tc_core);
    return s;
}

