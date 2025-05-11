#include <check.h>
#include "../include/ots.h"
#include "../data.h"

START_TEST(test_seed_monero_create)
{
    ots_result_t* result = ots_random_32();
    ck_assert(ots_result_data_is_uint8(result));
    const uint8_t* random = ots_result_uint8_array(result);
    ck_assert_ptr_nonnull(random);
    ots_free_result(&result);
    result = ots_check_low_entropy(random, 32, 3.5);
    ck_assert(ots_result_is_boolean(result));
    ck_assert(ots_result_boolean(result, true) == false);
    ots_free_result(&result);
    result = ots_monero_seed_create(random, 0, 0, OTS_NETWORK_MAIN);
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    ck_assert_ptr_null(result);
    ck_assert_ptr_nonnull(seed);
    ots_free_handle(&seed);
}
END_TEST

START_TEST(test_seed_monero_generate)
{
    ots_result_t* result = ots_monero_seed_generate(0, 0, OTS_NETWORK_MAIN);
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    ck_assert_ptr_null(result);
    ck_assert_ptr_nonnull(seed);
    ots_free_handle(&seed);
}
END_TEST

START_TEST(test_seed_monero_decode)
{
    for(size_t i = 0; i < get_monero_seed_test_cases_count(); i++) {
        const char* phrase = get_monero_seed_test_case_phrase(i);
        uint64_t height = get_monero_seed_test_case_height(i);
        uint64_t time = get_monero_seed_test_case_time(i);
        int network = get_monero_seed_test_case_network(i);
        const char* password = get_monero_seed_test_case_password(i);
        bool valid = get_monero_seed_test_case_valid(i);
        ots_result_t* result = ots_monero_seed_decode(phrase, height, time, (OTS_NETWORK)network, password);
        if(!valid) {
            ck_assert(ots_is_error(result));
            ots_free_result(&result);
            continue;
        }
        ck_assert(ots_result_is_seed(result));
        ots_handle_t* seed = ots_result_handle(result);
        ots_free_result(&result);
        ck_assert_ptr_null(result);
        ck_assert_ptr_nonnull(seed);
        ots_free_handle(&seed);
    }
}
END_TEST

START_TEST(test_seed_monero_decode_indices)
{
    for(size_t i = 0; i < get_monero_seed_test_cases_count(); i++) {
        const char* phrase = get_monero_seed_test_case_phrase(i);
        uint64_t height = get_monero_seed_test_case_height(i);
        uint64_t time = get_monero_seed_test_case_time(i);
        int network = get_monero_seed_test_case_network(i);
        const char* password = get_monero_seed_test_case_password(i);
        if(!get_monero_seed_test_case_valid(i)) {
            continue;
        }
        ots_result_t* result = ots_monero_seed_decode(phrase, height, time, (OTS_NETWORK)network, password);
        ck_assert(ots_result_is_seed(result));
        ots_handle_t* seed = ots_result_handle(result);
        ots_free_result(&result);
        ck_assert_ptr_null(result);
        ck_assert_ptr_nonnull(seed);
        result = ots_seed_fingerprint(seed);
        ck_assert(ots_result_is_string(result));
        const char* fingerprint = ots_result_string_copy(result);
        ots_free_result(&result);
        for(size_t j = 0; j < 2; j++) {
            result = ots_seed_indices(seed, i==0?"":"password");
            ck_assert(ots_result_is_seed_indices(result));
            ots_handle_t* indices = ots_result_handle(result);
            ots_free_result(&result);
            result = ots_monero_seed_decode_indices(indices, height, time, (OTS_NETWORK)network, i==0?"":"password");
            ck_assert(ots_result_is_seed(result));
            ots_handle_t* seed1 = ots_result_handle(result);
            ots_free_result(&result);
            result = ots_seed_fingerprint(seed1);
            ck_assert(ots_result_is_string(result));
            const char* fingerprint1 = ots_result_string_copy(result);
            ck_assert_str_eq(fingerprint, fingerprint1);
            ots_free_result(&result);
            ots_free_handle(&seed1);
            ots_free_string((char **)&fingerprint1);
        }
        ots_free_handle(&seed);
    }
}
END_TEST

Suite* seed_monero_suite(void)
{
    Suite* s = suite_create("Monero Seed");
    // Core functionality
    TCase* tc_core = tcase_create("Core");
    tcase_add_test(tc_core, test_seed_monero_create);
    tcase_add_test(tc_core, test_seed_monero_generate);
    tcase_add_test(tc_core, test_seed_monero_decode);
    tcase_add_test(tc_core, test_seed_monero_decode_indices);
    suite_add_tcase(s, tc_core);
    return s;
}

