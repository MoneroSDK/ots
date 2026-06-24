#include <check.h>
#include "../include/ots.h"
#include "../data.h"

START_TEST(test_seed_phrase)
{
    ots_result_t* result = ots_monero_seed_generate(0, 0, OTS_NETWORK_MAIN);
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_fingerprint(seed);
    ck_assert(ots_result_is_string(result));
    const char* fingerprint = ots_result_string_copy(result);
    result = ots_seed_language_from_code("en");
    ck_assert(ots_result_is_seed_language(result));
    ots_handle_t* language = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_phrase(seed, language, "");
    ck_assert(ots_result_is_wipeable_string(result));
    const char* phrase = ots_result_string_copy(result);
    ck_assert_str_ne(phrase, "");
    ots_free_result(&result);
    result = ots_monero_seed_decode(phrase, 0, 0, OTS_NETWORK_MAIN, "");
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* decoded_seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_fingerprint(decoded_seed);
    ck_assert(ots_result_is_string(result));
    ck_assert_str_eq(ots_result_string(result), fingerprint);
    ots_free_result(&result);
    ots_free_handle(&decoded_seed);
    ots_free_handle(&seed);
    ots_free_handle(&language);
    ots_free_string((char**)&fingerprint);
    ots_free_string((char**)&phrase);
}
END_TEST

START_TEST(test_seed_phrase_for_language_code)
{
    ots_result_t* result = ots_monero_seed_generate(0, 0, OTS_NETWORK_MAIN);
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_fingerprint(seed);
    ck_assert(ots_result_is_string(result));
    const char* fingerprint = ots_result_string_copy(result);
    result = ots_seed_phrase_for_language_code(seed, "en", "");
    ck_assert(ots_result_is_wipeable_string(result));
    const char* phrase = ots_result_string_copy(result);
    ck_assert_str_ne(phrase, "");
    ots_free_result(&result);
    result = ots_monero_seed_decode(phrase, 0, 0, OTS_NETWORK_MAIN, "");
    ck_assert(ots_result_is_seed(result));
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
}
END_TEST

START_TEST(test_seed_indices)
{
    ots_result_t* result = ots_monero_seed_generate(0, 0, OTS_NETWORK_MAIN);
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_fingerprint(seed);
    ck_assert(ots_result_is_string(result));
    const char* fingerprint = ots_result_string_copy(result);
    result = ots_seed_indices(seed, "");
    ck_assert(ots_result_is_seed_indices(result));
    ots_handle_t* indices = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_monero_seed_decode_indices(indices, 0, 0, OTS_NETWORK_MAIN, "");
    ck_assert(ots_result_is_seed(result));
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

START_TEST(test_seed_fingerprint)
{
    ots_result_t* result = ots_monero_seed_decode(
        get_monero_seed_test_case_phrase(2),
        get_monero_seed_test_case_height(2),
        get_monero_seed_test_case_time(2),
        get_monero_seed_test_case_network(2),
        get_monero_seed_test_case_password(2)
    );
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_fingerprint(seed);
    ck_assert(ots_result_is_string(result));
    ck_assert_str_eq(ots_result_string(result), get_monero_seed_test_case_fingerprint(2));
    ots_free_result(&result);
    ots_free_handle(&seed);
}
END_TEST

START_TEST(test_seed_is_legacy)
{
    ots_result_t* result = ots_monero_seed_decode(
        get_monero_seed_test_case_phrase(5),
        get_monero_seed_test_case_height(5),
        get_monero_seed_test_case_time(5),
        get_monero_seed_test_case_network(5),
        get_monero_seed_test_case_password(5)
    );
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_is_legacy(seed);
    ck_assert(ots_result_is_boolean(result));
    ck_assert(!ots_result_boolean(result, true));
    ots_free_result(&result);
    ots_free_handle(&seed);
    result = ots_legacy_seed_decode(
        get_legacy_seed_test_case_phrase(2),
        get_legacy_seed_test_case_height(2),
        get_legacy_seed_test_case_time(2),
        get_legacy_seed_test_case_network(2)
    );
    ck_assert(ots_result_is_seed(result));
    seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_is_legacy(seed);
    ck_assert(ots_result_is_boolean(result));
    ck_assert(ots_result_boolean(result, false));
    ots_free_result(&result);
    ots_free_handle(&seed);
    result = ots_polyseed_decode(
        get_polyseed_test_case_phrase(0),
        get_polyseed_test_case_network(0),
        get_polyseed_test_case_password(0),
        get_polyseed_test_case_passphrase(0)
    );
    ck_assert(ots_result_is_seed(result));
    seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_is_legacy(seed);
    ck_assert(ots_result_is_boolean(result));
    ck_assert(!ots_result_boolean(result, true));
    ots_free_result(&result);
    ots_free_handle(&seed);
}
END_TEST

START_TEST(test_seed_type)
{
    ots_result_t* result = ots_monero_seed_decode(
        get_monero_seed_test_case_phrase(5),
        get_monero_seed_test_case_height(5),
        get_monero_seed_test_case_time(5),
        get_monero_seed_test_case_network(5),
        get_monero_seed_test_case_password(5)
    );
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_type(seed);
    ck_assert(ots_result_is_seed_type(result));
    ck_assert(ots_result_seed_type_is_type(result, OTS_SEED_TYPE_MONERO));
    ck_assert(ots_result_seed_type(result) == OTS_SEED_TYPE_MONERO);
    ots_free_result(&result);
    ots_free_handle(&seed);

    result = ots_legacy_seed_decode(
        get_legacy_seed_test_case_phrase(2),
        get_legacy_seed_test_case_height(2),
        get_legacy_seed_test_case_time(2),
        get_legacy_seed_test_case_network(2)
    );
    ck_assert(ots_result_is_seed(result));
    seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_type(seed);
    ck_assert(ots_result_is_seed_type(result));
    ck_assert(ots_result_seed_type_is_type(result, OTS_SEED_TYPE_MONERO));
    ck_assert(ots_result_seed_type(result) == OTS_SEED_TYPE_MONERO);
    ots_free_result(&result);
    ots_free_handle(&seed);

    result = ots_polyseed_decode(
        get_polyseed_test_case_phrase(0),
        get_polyseed_test_case_network(0),
        get_polyseed_test_case_password(0),
        get_polyseed_test_case_passphrase(0)
    );
    ck_assert(ots_result_is_seed(result));
    seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_type(seed);
    ck_assert(ots_result_is_seed_type(result));
    ck_assert(ots_result_seed_type_is_type(result, OTS_SEED_TYPE_POLYSEED));
    ck_assert(ots_result_seed_type(result) == OTS_SEED_TYPE_POLYSEED);
    ots_free_result(&result);
    ots_free_handle(&seed);
}

START_TEST(test_seed_address)
{
    ots_result_t* result = ots_monero_seed_decode(
        get_monero_seed_test_case_phrase(2),
        get_monero_seed_test_case_height(2),
        get_monero_seed_test_case_time(2),
        get_monero_seed_test_case_network(2),
        get_monero_seed_test_case_password(2)
    );
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_address(seed);
    ck_assert(ots_result_is_address(result));
    ots_handle_t* address = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_address_base58_string(address);
    ck_assert(ots_result_is_string(result));
    ck_assert_str_eq(ots_result_string(result), get_monero_seed_test_case_address(2));
    ots_free_result(&result);
    ots_free_handle(&address);
    ots_free_handle(&seed);
}
END_TEST

START_TEST(test_seed_seed_timestamp)
{
    ots_result_t* result = ots_monero_seed_decode(
        get_monero_seed_test_case_phrase(5),
        get_monero_seed_test_case_height(5),
        get_monero_seed_test_case_time(5),
        get_monero_seed_test_case_network(5),
        get_monero_seed_test_case_password(5)
    );
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_timestamp(seed);
    ck_assert(ots_result_is_number(result));
    ck_assert_uint_lt(ots_result_number(result, 0), get_monero_seed_test_case_expected_time(5));
    ck_assert_uint_lt(get_monero_seed_test_case_expected_time(5) - ots_result_number(result, 0), 3600 * 24 * 11);
    ots_free_result(&result);
    ots_free_handle(&seed);
}
END_TEST

START_TEST(test_seed_seed_height)
{
    ots_result_t* result = ots_monero_seed_decode(
        get_monero_seed_test_case_phrase(5),
        get_monero_seed_test_case_height(5),
        get_monero_seed_test_case_time(5),
        get_monero_seed_test_case_network(5),
        get_monero_seed_test_case_password(5)
    );
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_height(seed);
    ck_assert(ots_result_is_number(result));
    ck_assert_uint_eq(ots_result_number(result, 0), get_monero_seed_test_case_expected_height(5));
    ots_free_result(&result);
    ots_free_handle(&seed);
}
END_TEST

START_TEST(test_seed_seed_network)
{
    OTS_NETWORK networks[3] = {
        OTS_NETWORK_MAIN,
        OTS_NETWORK_TEST,
        OTS_NETWORK_STAGE
    };
    for(size_t i = 0; i < 3; i++) {
        ots_result_t* result = ots_monero_seed_decode(
            get_monero_seed_test_case_phrase(5),
            get_monero_seed_test_case_height(5),
            get_monero_seed_test_case_time(5),
            networks[i],
            get_monero_seed_test_case_password(5)
        );
        ck_assert(ots_result_is_seed(result));
        ots_handle_t* seed = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_seed_network(seed);
        ck_assert(ots_result_is_network(result));
        ck_assert(ots_result_network_is_type(result, networks[i]));
        ck_assert(!ots_result_network_is_type(result, networks[(i+1)%2]));
        ots_free_result(&result);
        ots_free_handle(&seed);
    }
}
END_TEST

START_TEST(test_seed_seed_wallet)
{
    ots_result_t* result = ots_monero_seed_decode(
        get_monero_seed_test_case_phrase(5),
        get_monero_seed_test_case_height(5),
        get_monero_seed_test_case_time(5),
        get_monero_seed_test_case_network(5),
        get_monero_seed_test_case_password(5)
    );
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_wallet(seed);
    ck_assert(ots_result_is_wallet(result));
    ots_handle_t* wallet = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_wallet_address(wallet);
    ck_assert(ots_result_is_address(result));
    ots_handle_t* address = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_address_base58_string(address);
    ck_assert(ots_result_is_string(result));
    ck_assert_str_eq(ots_result_string(result), get_monero_seed_test_case_address(5));
    ots_free_result(&result);
    ots_free_handle(&seed);
}
END_TEST

Suite* seed_suite(void)
{
    Suite* s = suite_create("Seed");
    // Core functionality
    TCase* tc_core = tcase_create("Core");
    tcase_add_test(tc_core, test_seed_phrase);
    tcase_add_test(tc_core, test_seed_phrase_for_language_code);
    tcase_add_test(tc_core, test_seed_indices);
    tcase_add_test(tc_core, test_seed_fingerprint);
    tcase_add_test(tc_core, test_seed_is_legacy);
    tcase_add_test(tc_core, test_seed_type);
    tcase_add_test(tc_core, test_seed_address);
    tcase_add_test(tc_core, test_seed_seed_timestamp);
    tcase_add_test(tc_core, test_seed_seed_height);
    tcase_add_test(tc_core, test_seed_seed_network);
    tcase_add_test(tc_core, test_seed_seed_wallet);
    suite_add_tcase(s, tc_core);
    return s;
}

