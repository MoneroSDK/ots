#include <check.h>
#include <stdint.h>
#include "../include/ots.h"
#include "../data.h"

START_TEST(test_seed_polyseed_create)
{
    // Test the creation of a Polyseed
    ots_result_t* result = ots_random_bytes(19);
    ck_assert(ots_result_data_is_uint8(result));
    const uint8_t* random = ots_result_uint8_array(result);
    ots_free_result(&result);
    result = ots_polyseed_create(random, OTS_NETWORK_MAIN, 0, "passphrase");
    ck_assert(ots_result_is_handle(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_fingerprint(seed);
    ck_assert(ots_result_is_string(result));
    ck_assert_str_ne(ots_result_string(result), "");
    ck_assert_int_eq(ots_result_size(result), 6);
    const char* fingerprint = ots_result_string_copy(result);
    ots_free_result(&result);
    result = ots_seed_language_from_code("en");
    ck_assert(ots_result_is_handle(result));
    ots_handle_t* en = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_phrase(seed, en, "password");
    ck_assert(ots_result_is_wipeable_string(result));
    const char* phrase = ots_result_string_copy(result);
    ck_assert_str_ne(phrase, "");
    ots_free_result(&result);
    result = ots_polyseed_decode(phrase, OTS_NETWORK_MAIN, "password", "passphrase");
    ck_assert(ots_result_is_handle(result));
    ots_handle_t* decoded_seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_fingerprint(decoded_seed);
    ck_assert(ots_result_is_string(result));
    ck_assert_str_eq(ots_result_string(result), fingerprint);
    ots_free_result(&result);
    ots_free_handle(&decoded_seed);
    ots_free_handle(&seed);
    ots_free_string((char**)&fingerprint);
    ots_free_string((char**)&phrase);
    ots_free_handle(&en);
}
END_TEST

START_TEST(test_seed_polyseed_generate)
{
    // Test the generation of a Polyseed
    ots_result_t* result = ots_polyseed_generate(OTS_NETWORK_MAIN, 0, "passphrase");
    ck_assert(ots_result_is_handle(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_fingerprint(seed);
    ck_assert(ots_result_is_string(result));
    ck_assert_str_ne(ots_result_string(result), "");
    ck_assert_int_eq(ots_result_size(result), 6);
    const char* fingerprint = ots_result_string_copy(result);
    ots_free_result(&result);
    result = ots_seed_language_from_code("en");
    ck_assert(ots_result_is_handle(result));
    ots_handle_t* en = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_phrase(seed, en, "password");
    ck_assert(ots_result_is_wipeable_string(result));
    const char* phrase = ots_result_string_copy(result);
    ck_assert_str_ne(phrase, "");
    ots_free_result(&result);
    result = ots_polyseed_decode(phrase, OTS_NETWORK_MAIN, "password", "passphrase");
    ck_assert(ots_result_is_handle(result));
    ots_handle_t* decoded_seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_fingerprint(decoded_seed);
    ck_assert(ots_result_is_string(result));
    ck_assert_str_eq(ots_result_string(result), fingerprint);
    ots_free_result(&result);
    ots_free_handle(&decoded_seed);
    ots_free_handle(&seed);
    ots_free_string((char**)&fingerprint);
    ots_free_string((char**)&phrase);
    ots_free_handle(&en);
}
END_TEST

START_TEST(test_seed_polyseed_decode)
{
    for(size_t i = 0; i < get_polyseed_test_cases_count(); i++) {
        const char* phrase = get_polyseed_test_case_phrase(i);
        const char* passphrase = get_polyseed_test_case_passphrase(i);
        const char* password = get_polyseed_test_case_password(i);
        const char* fingerprint = get_polyseed_test_case_fingerprint(i);
        const char* address = get_polyseed_test_case_address(i);
        ots_result_t* result = ots_polyseed_decode(phrase, OTS_NETWORK_MAIN, password, passphrase);
        if(!get_polyseed_test_case_valid(i)) {
            ck_assert(ots_is_error(result));
            continue;
        }
        ck_assert(ots_result_is_handle(result));
        ots_handle_t* seed = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_seed_fingerprint(seed);
        ck_assert(ots_result_is_string(result));
        ck_assert_str_eq(ots_result_string(result), fingerprint);
        ots_free_result(&result);
        result = ots_seed_address(seed);
        ck_assert(ots_result_is_handle(result));
        ots_handle_t* address_handle = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_address_base58_string(address_handle);
        ck_assert(ots_result_is_string(result));
        ck_assert_str_eq(ots_result_string(result), address);
        ots_free_result(&result);
        ots_free_handle(&seed);
    }
}
END_TEST

START_TEST(test_seed_polyseed_decode_with_language)
{
    ots_result_t* result = ots_polyseed_generate(OTS_NETWORK_MAIN, 0, "");
    ck_assert(ots_result_is_handle(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_fingerprint(seed);
    ck_assert(ots_result_is_string(result));
    const char* fingerprint = ots_result_string_copy(result);
    ots_free_result(&result);
    result = ots_seed_languages_for_type(OTS_SEED_TYPE_POLYSEED);
    ck_assert(ots_result_data_handle_is_seed_language(result));
    ots_handle_t* languages = ots_result_handle_array(result);
    size_t languages_count = ots_result_size(result);
    ots_free_result(&result);
    for(size_t i = 0; i < languages_count; i++) {
        result = ots_seed_phrase(seed, &languages[i], "password");
        ck_assert(ots_result_is_wipeable_string(result));
        const char* phrase = ots_result_string_copy(result);
        ck_assert_str_ne(phrase, "");
        ots_free_result(&result);
        result = ots_polyseed_decode_with_language(phrase, &languages[i], OTS_NETWORK_MAIN, "password", "");
        ck_assert(ots_result_is_handle(result));
        ots_handle_t* decoded_seed = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_seed_fingerprint(decoded_seed);
        ck_assert(ots_result_is_string(result));
        ck_assert_str_eq(ots_result_string(result), fingerprint);
        ots_free_result(&result);
        ots_free_handle(&decoded_seed);
    }
}
END_TEST

START_TEST(test_seed_polyseed_decode_with_language_code)
{
    ots_result_t* result = ots_polyseed_generate(OTS_NETWORK_MAIN, 0, "");
    ck_assert(ots_result_is_handle(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_fingerprint(seed);
    ck_assert(ots_result_is_string(result));
    const char* fingerprint = ots_result_string_copy(result);
    ots_free_result(&result);
    result = ots_seed_languages_for_type(OTS_SEED_TYPE_POLYSEED);
    ck_assert(ots_result_data_handle_is_seed_language(result));
    ots_handle_t* languages = ots_result_handle_array(result);
    size_t languages_count = ots_result_size(result);
    ots_free_result(&result);
    for(size_t i = 0; i < languages_count; i++) {
        result = ots_seed_language_code(&languages[i]);
        ck_assert(ots_result_is_string(result));
        const char* code = ots_result_string_copy(result);
        ots_free_result(&result);
        result = ots_seed_phrase_for_language_code(seed, code, "password");
        ck_assert(ots_result_is_wipeable_string(result));
        const char* phrase = ots_result_string_copy(result);
        ck_assert_str_ne(phrase, "");
        ots_free_result(&result);
        result = ots_polyseed_decode_with_language(phrase, &languages[i], OTS_NETWORK_MAIN, "password", "");
        ck_assert(ots_result_is_handle(result));
        ots_handle_t* decoded_seed = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_seed_fingerprint(decoded_seed);
        ck_assert(ots_result_is_string(result));
        ck_assert_str_eq(ots_result_string(result), fingerprint);
        ots_free_result(&result);
        ots_free_handle(&decoded_seed);
    }
}
END_TEST

START_TEST(test_seed_polyseed_decode_indices)
{
    ots_result_t* result = ots_polyseed_generate(OTS_NETWORK_MAIN, 0, "");
    ck_assert(ots_result_is_handle(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_fingerprint(seed);
    ck_assert(ots_result_is_string(result));
    const char* fingerprint = ots_result_string_copy(result);
    ots_free_result(&result);
    result = ots_seed_indices(seed, "password");
    ck_assert(ots_result_is_seed_indices(result));
    ots_handle_t* indices = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_polyseed_decode_indices(indices, OTS_NETWORK_MAIN, "password", "");
    ck_assert(ots_result_is_handle(result));
    ots_handle_t* decoded_seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_fingerprint(decoded_seed);
    ck_assert(ots_result_is_string(result));
    ck_assert_str_eq(ots_result_string(result), fingerprint);
    ots_free_result(&result);
    ots_free_handle(&decoded_seed);
    ots_free_handle(&seed);
    ots_free_handle(&indices);
    ots_free_string((char**)&fingerprint);
}
END_TEST

Suite* seed_polyseed_suite(void)
{
    Suite* s = suite_create("Polyseed");
    // Core functionality
    TCase* tc_core = tcase_create("Core");
    tcase_add_test(tc_core, test_seed_polyseed_create);
    tcase_add_test(tc_core, test_seed_polyseed_generate);
    tcase_add_test(tc_core, test_seed_polyseed_decode);
    tcase_add_test(tc_core, test_seed_polyseed_decode_with_language);
    tcase_add_test(tc_core, test_seed_polyseed_decode_with_language_code);
    tcase_add_test(tc_core, test_seed_polyseed_decode_indices);
    suite_add_tcase(s, tc_core);
    return s;
}

