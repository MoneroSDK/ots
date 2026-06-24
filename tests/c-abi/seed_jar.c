#include <check.h>
#include "../include/ots.h"
#include "../data.h"

START_TEST(test_ots_seed_jar_add_seed)
{
    ots_result_t* result = ots_legacy_seed_decode(
        get_legacy_seed_test_case_phrase(2),
        get_legacy_seed_test_case_height(2),
        get_legacy_seed_test_case_time(2),
        (OTS_NETWORK)get_legacy_seed_test_case_network(2)
    );
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_jar_add_seed(seed, "legacy");
    ck_assert(ots_result_is_seed(result));
    ck_assert(ots_result_handle_is_reference(result));
    ots_free_result(&result);
    ots_free_handle(&seed);
    result = ots_monero_seed_decode(
        get_monero_seed_test_case_phrase(5),
        get_monero_seed_test_case_height(5),
        get_monero_seed_test_case_time(5),
        (OTS_NETWORK)get_monero_seed_test_case_network(5),
        get_monero_seed_test_case_password(5)
    );
    seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_jar_add_seed(seed, "monero");
    ck_assert(ots_result_is_seed(result));
    ck_assert(ots_result_handle_is_reference(result));
    ots_free_result(&result);
    ots_free_handle(&seed);
    result = ots_polyseed_decode(
        get_polyseed_test_case_phrase(0),
        (OTS_NETWORK)get_polyseed_test_case_network(0),
        get_polyseed_test_case_password(0),
        get_polyseed_test_case_passphrase(0)
    );
    seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_jar_add_seed(seed, "polyseed");
    ck_assert(ots_result_is_seed(result));
    ck_assert(ots_result_handle_is_reference(result));
    ots_free_result(&result);
    ots_free_handle(&seed);
    result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 0), 3);
    ots_free_result(&result);
}
END_TEST

START_TEST(test_ots_seed_jar_remove_seed)
{
    ots_result_t* result = ots_legacy_seed_decode(
        get_legacy_seed_test_case_phrase(2),
        get_legacy_seed_test_case_height(2),
        get_legacy_seed_test_case_time(2),
        (OTS_NETWORK)get_legacy_seed_test_case_network(2)
    );
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* legacy_seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_jar_add_seed(legacy_seed, "legacy");
    ck_assert(ots_result_is_seed(result));
    ck_assert(ots_result_handle_is_reference(result));
    ots_free_result(&result);
    result = ots_monero_seed_decode(
        get_monero_seed_test_case_phrase(5),
        get_monero_seed_test_case_height(5),
        get_monero_seed_test_case_time(5),
        (OTS_NETWORK)get_monero_seed_test_case_network(5),
        get_monero_seed_test_case_password(5)
    );
    ots_handle_t* monero_seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_jar_add_seed(monero_seed, "monero");
    ck_assert(ots_result_is_seed(result));
    ck_assert(ots_result_handle_is_reference(result));
    ots_free_result(&result);
    result = ots_polyseed_decode(
        get_polyseed_test_case_phrase(0),
        (OTS_NETWORK)get_polyseed_test_case_network(0),
        get_polyseed_test_case_password(0),
        get_polyseed_test_case_passphrase(0)
    );
    ots_handle_t* polyseed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_jar_add_seed(polyseed, "polyseed");
    ck_assert(ots_result_is_seed(result));
    ck_assert(ots_result_handle_is_reference(result));
    ots_free_result(&result);
    result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 0), 3);
    ots_free_result(&result);
    result = ots_seed_jar_remove_seed(&legacy_seed);
    ck_assert(ots_result_is_boolean(result));
    ck_assert(ots_result_boolean(result, false));
    ots_free_result(&result);
    ck_assert_ptr_null(legacy_seed);
    result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 0), 2);
    ots_free_result(&result);
    result = ots_seed_jar_remove_seed(&monero_seed);
    ck_assert(ots_result_is_boolean(result));
    ck_assert(ots_result_boolean(result, false));
    ots_free_result(&result);
    ck_assert_ptr_null(monero_seed);
    result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 0), 1);
    ots_free_result(&result);
    result = ots_seed_jar_remove_seed(&polyseed);
    ck_assert(ots_result_is_boolean(result));
    ck_assert(ots_result_boolean(result, false));
    ots_free_result(&result);
    ck_assert_ptr_null(polyseed);
    result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 66), 0);
    ots_free_result(&result);
}
END_TEST

START_TEST(test_ots_seed_jar_purge_seed_for_index)
{
    ots_result_t* result = ots_legacy_seed_decode(
        get_legacy_seed_test_case_phrase(2),
        get_legacy_seed_test_case_height(2),
        get_legacy_seed_test_case_time(2),
        (OTS_NETWORK)get_legacy_seed_test_case_network(2)
    );
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* legacy_seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_jar_add_seed(legacy_seed, "legacy");
    ck_assert(ots_result_is_seed(result));
    ck_assert(ots_result_handle_is_reference(result));
    ots_free_result(&result);
    ots_free_handle(&legacy_seed);
    result = ots_monero_seed_decode(
        get_monero_seed_test_case_phrase(5),
        get_monero_seed_test_case_height(5),
        get_monero_seed_test_case_time(5),
        (OTS_NETWORK)get_monero_seed_test_case_network(5),
        get_monero_seed_test_case_password(5)
    );
    ots_handle_t* monero_seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_jar_add_seed(monero_seed, "monero");
    ck_assert(ots_result_is_seed(result));
    ck_assert(ots_result_handle_is_reference(result));
    ots_free_result(&result);
    ots_free_handle(&monero_seed);
    result = ots_polyseed_decode(
        get_polyseed_test_case_phrase(0),
        (OTS_NETWORK)get_polyseed_test_case_network(0),
        get_polyseed_test_case_password(0),
        get_polyseed_test_case_passphrase(0)
    );
    ots_handle_t* polyseed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_jar_add_seed(polyseed, "polyseed");
    ck_assert(ots_result_is_seed(result));
    ck_assert(ots_result_handle_is_reference(result));
    ots_free_handle(&polyseed);
    ots_free_result(&result);
    result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 0), 3);
    ots_free_result(&result);
    result = ots_seed_jar_purge_seed_for_index(0);
    ck_assert(ots_result_is_boolean(result));
    ck_assert(ots_result_boolean(result, false));
    ots_free_result(&result);
    result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 0), 2);
    ots_free_result(&result);
    result = ots_seed_jar_purge_seed_for_index(0);
    ck_assert(ots_result_is_boolean(result));
    ck_assert(ots_result_boolean(result, false));
    ots_free_result(&result);
    result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 0), 1);
    ots_free_result(&result);
    result = ots_seed_jar_purge_seed_for_index(0);
    ck_assert(ots_result_is_boolean(result));
    ck_assert(ots_result_boolean(result, false));
    ots_free_result(&result);
    result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 66), 0);
    ots_free_result(&result);
}
END_TEST

START_TEST(test_ots_seed_jar_purge_seed_for_name)
{
    ots_result_t* result = ots_legacy_seed_decode(
        get_legacy_seed_test_case_phrase(2),
        get_legacy_seed_test_case_height(2),
        get_legacy_seed_test_case_time(2),
        (OTS_NETWORK)get_legacy_seed_test_case_network(2)
    );
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* legacy_seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_jar_add_seed(legacy_seed, "legacy");
    ck_assert(ots_result_is_seed(result));
    ck_assert(ots_result_handle_is_reference(result));
    ots_free_result(&result);
    ots_free_handle(&legacy_seed);
    result = ots_monero_seed_decode(
        get_monero_seed_test_case_phrase(5),
        get_monero_seed_test_case_height(5),
        get_monero_seed_test_case_time(5),
        (OTS_NETWORK)get_monero_seed_test_case_network(5),
        get_monero_seed_test_case_password(5)
    );
    ots_handle_t* monero_seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_jar_add_seed(monero_seed, "monero");
    ck_assert(ots_result_is_seed(result));
    ck_assert(ots_result_handle_is_reference(result));
    ots_free_result(&result);
    ots_free_handle(&monero_seed);
    result = ots_polyseed_decode(
        get_polyseed_test_case_phrase(0),
        (OTS_NETWORK)get_polyseed_test_case_network(0),
        get_polyseed_test_case_password(0),
        get_polyseed_test_case_passphrase(0)
    );
    ots_handle_t* polyseed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_jar_add_seed(polyseed, "polyseed");
    ck_assert(ots_result_is_seed(result));
    ck_assert(ots_result_handle_is_reference(result));
    ots_free_handle(&polyseed);
    ots_free_result(&result);
    result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 0), 3);
    ots_free_result(&result);
    result = ots_seed_jar_purge_seed_for_name("legacy");
    ck_assert(ots_result_is_boolean(result));
    ck_assert(ots_result_boolean(result, false));
    ots_free_result(&result);
    result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 0), 2);
    ots_free_result(&result);
    result = ots_seed_jar_purge_seed_for_name("monero");
    ck_assert(ots_result_is_boolean(result));
    ck_assert(ots_result_boolean(result, false));
    ots_free_result(&result);
    result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 0), 1);
    ots_free_result(&result);
    result = ots_seed_jar_purge_seed_for_name("polyseed");
    ck_assert(ots_result_is_boolean(result));
    ck_assert(ots_result_boolean(result, false));
    ots_free_result(&result);
    result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 66), 0);
    ots_free_result(&result);
}
END_TEST

START_TEST(test_ots_seed_jar_purge_seed_for_fingerprint)
{
    ots_result_t* result = ots_legacy_seed_decode(
        get_legacy_seed_test_case_phrase(2),
        get_legacy_seed_test_case_height(2),
        get_legacy_seed_test_case_time(2),
        (OTS_NETWORK)get_legacy_seed_test_case_network(2)
    );
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* legacy_seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_jar_add_seed(legacy_seed, "legacy");
    ck_assert(ots_result_is_seed(result));
    ck_assert(ots_result_handle_is_reference(result));
    ots_free_result(&result);
    ots_free_handle(&legacy_seed);
    result = ots_monero_seed_decode(
        get_monero_seed_test_case_phrase(5),
        get_monero_seed_test_case_height(5),
        get_monero_seed_test_case_time(5),
        (OTS_NETWORK)get_monero_seed_test_case_network(5),
        get_monero_seed_test_case_password(5)
    );
    ots_handle_t* monero_seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_jar_add_seed(monero_seed, "monero");
    ck_assert(ots_result_is_seed(result));
    ck_assert(ots_result_handle_is_reference(result));
    ots_free_result(&result);
    ots_free_handle(&monero_seed);
    result = ots_polyseed_decode(
        get_polyseed_test_case_phrase(0),
        (OTS_NETWORK)get_polyseed_test_case_network(0),
        get_polyseed_test_case_password(0),
        get_polyseed_test_case_passphrase(0)
    );
    ots_handle_t* polyseed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_jar_add_seed(polyseed, "polyseed");
    ck_assert(ots_result_is_seed(result));
    ck_assert(ots_result_handle_is_reference(result));
    ots_free_handle(&polyseed);
    ots_free_result(&result);
    result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 0), 3);
    ots_free_result(&result);
    result = ots_seed_jar_purge_seed_for_fingerprint(
        get_legacy_seed_test_case_fingerprint(2)
    );
    ck_assert(ots_result_is_boolean(result));
    ck_assert(ots_result_boolean(result, false));
    ots_free_result(&result);
    result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 0), 2);
    ots_free_result(&result);
    result = ots_seed_jar_purge_seed_for_fingerprint(
        get_monero_seed_test_case_fingerprint(5)
    );
    ck_assert(ots_result_is_boolean(result));
    ck_assert(ots_result_boolean(result, false));
    ots_free_result(&result);
    result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 0), 1);
    ots_free_result(&result);
    result = ots_seed_jar_purge_seed_for_fingerprint(
        get_polyseed_test_case_fingerprint(0)
    );
    ck_assert(ots_result_is_boolean(result));
    ck_assert(ots_result_boolean(result, false));
    ots_free_result(&result);
    result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 66), 0);
    ots_free_result(&result);
}
END_TEST

START_TEST(test_ots_seed_jar_purge_seed_for_address)
{
    ots_result_t* result = ots_legacy_seed_decode(
        get_legacy_seed_test_case_phrase(2),
        get_legacy_seed_test_case_height(2),
        get_legacy_seed_test_case_time(2),
        (OTS_NETWORK)get_legacy_seed_test_case_network(2)
    );
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* legacy_seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_jar_add_seed(legacy_seed, "legacy");
    ck_assert(ots_result_is_seed(result));
    ck_assert(ots_result_handle_is_reference(result));
    ots_free_result(&result);
    ots_free_handle(&legacy_seed);
    result = ots_monero_seed_decode(
        get_monero_seed_test_case_phrase(5),
        get_monero_seed_test_case_height(5),
        get_monero_seed_test_case_time(5),
        (OTS_NETWORK)get_monero_seed_test_case_network(5),
        get_monero_seed_test_case_password(5)
    );
    ots_handle_t* monero_seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_jar_add_seed(monero_seed, "monero");
    ck_assert(ots_result_is_seed(result));
    ck_assert(ots_result_handle_is_reference(result));
    ots_free_result(&result);
    ots_free_handle(&monero_seed);
    result = ots_polyseed_decode(
        get_polyseed_test_case_phrase(0),
        (OTS_NETWORK)get_polyseed_test_case_network(0),
        get_polyseed_test_case_password(0),
        get_polyseed_test_case_passphrase(0)
    );
    ots_handle_t* polyseed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_jar_add_seed(polyseed, "polyseed");
    ck_assert(ots_result_is_seed(result));
    ck_assert(ots_result_handle_is_reference(result));
    ots_free_handle(&polyseed);
    ots_free_result(&result);
    result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 0), 3);
    ots_free_result(&result);
    result = ots_seed_jar_purge_seed_for_address(
        get_legacy_seed_test_case_address(2)
    );
    ck_assert(ots_result_is_boolean(result));
    ck_assert(ots_result_boolean(result, false));
    ots_free_result(&result);
    result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 0), 2);
    ots_free_result(&result);
    result = ots_seed_jar_purge_seed_for_address(
        get_monero_seed_test_case_address(5)
    );
    ck_assert(ots_result_is_boolean(result));
    ck_assert(ots_result_boolean(result, false));
    ots_free_result(&result);
    result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 0), 1);
    ots_free_result(&result);
    result = ots_seed_jar_purge_seed_for_address(
        get_polyseed_test_case_address(0)
    );
    ck_assert(ots_result_is_boolean(result));
    ck_assert(ots_result_boolean(result, false));
    ots_free_result(&result);
    result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 66), 0);
    ots_free_result(&result);
}
END_TEST

START_TEST(test_ots_seed_jar_transfer_seed_in)
{
    ots_result_t* result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 666), 0);
    ots_free_result(&result);
    result = ots_legacy_seed_decode(
        get_legacy_seed_test_case_phrase(2),
        get_legacy_seed_test_case_height(2),
        get_legacy_seed_test_case_time(2),
        (OTS_NETWORK)get_legacy_seed_test_case_network(2)
    );
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* legacy_seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_jar_transfer_seed_in(
        &legacy_seed,
        "legacy"
    );
    ck_assert(ots_result_is_seed(result));
    ck_assert(ots_result_handle_is_reference(result));
    ots_free_result(&result);
    ck_assert_ptr_null(legacy_seed);
    result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 0), 1);
    ots_free_result(&result);
}
END_TEST

START_TEST(test_ots_seed_jar_transfer_seed_out)
{
    ots_result_t* result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 666), 0);
    ots_free_result(&result);
    result = ots_legacy_seed_decode(
        get_legacy_seed_test_case_phrase(2),
        get_legacy_seed_test_case_height(2),
        get_legacy_seed_test_case_time(2),
        (OTS_NETWORK)get_legacy_seed_test_case_network(2)
    );
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* legacy_seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_jar_add_seed(
        legacy_seed,
        "legacy"
    );
    ck_assert(ots_result_is_seed(result));
    ck_assert(ots_result_handle_is_reference(result));
    ots_free_result(&result);
    ck_assert_ptr_nonnull(legacy_seed);
    ck_assert(legacy_seed->reference);
    result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 0), 1);
    ots_free_result(&result);
    result = ots_seed_jar_transfer_seed_out(&legacy_seed);
    ck_assert(ots_result_is_seed(result));
    ck_assert_ptr_null(legacy_seed);
    legacy_seed = ots_result_handle(result);
    ck_assert_ptr_nonnull(legacy_seed);
    ck_assert(!legacy_seed->reference);
    ots_free_result(&result);
    result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 666), 0);
    ots_free_result(&result);
    ots_free_handle(&legacy_seed);
}
END_TEST

START_TEST(test_ots_seed_jar_transfer_seed_out_for_index)
{
    ots_result_t* result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 666), 0);
    ots_free_result(&result);
    result = ots_legacy_seed_decode(
        get_legacy_seed_test_case_phrase(2),
        get_legacy_seed_test_case_height(2),
        get_legacy_seed_test_case_time(2),
        (OTS_NETWORK)get_legacy_seed_test_case_network(2)
    );
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* legacy_seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_jar_transfer_seed_in(
        &legacy_seed,
        "legacy"
    );
    ck_assert(ots_result_is_seed(result));
    ck_assert(ots_result_handle_is_reference(result));
    ots_free_result(&result);
    ck_assert_ptr_null(legacy_seed);
    result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 0), 1);
    ots_free_result(&result);
    result = ots_seed_jar_transfer_seed_out_for_index(0);
    ck_assert(ots_result_is_seed(result));
    ck_assert_ptr_null(legacy_seed);
    legacy_seed = ots_result_handle(result);
    ck_assert_ptr_nonnull(legacy_seed);
    ck_assert(!legacy_seed->reference);
    ots_free_result(&result);
    result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 666), 0);
    ots_free_result(&result);
    ots_free_handle(&legacy_seed);
}
END_TEST

START_TEST(test_ots_seed_jar_transfer_seed_out_for_name)
{
    ots_result_t* result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 666), 0);
    ots_free_result(&result);
    result = ots_legacy_seed_decode(
        get_legacy_seed_test_case_phrase(2),
        get_legacy_seed_test_case_height(2),
        get_legacy_seed_test_case_time(2),
        (OTS_NETWORK)get_legacy_seed_test_case_network(2)
    );
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* legacy_seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_jar_transfer_seed_in(
        &legacy_seed,
        "legacy"
    );
    ck_assert(ots_result_is_seed(result));
    ck_assert(ots_result_handle_is_reference(result));
    ots_free_result(&result);
    ck_assert_ptr_null(legacy_seed);
    result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 0), 1);
    ots_free_result(&result);
    result = ots_seed_jar_transfer_seed_out_for_name("legacy");
    ck_assert(ots_result_is_seed(result));
    ck_assert_ptr_null(legacy_seed);
    legacy_seed = ots_result_handle(result);
    ck_assert_ptr_nonnull(legacy_seed);
    ck_assert(!legacy_seed->reference);
    ots_free_result(&result);
    result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 666), 0);
    ots_free_result(&result);
    ots_free_handle(&legacy_seed);
}
END_TEST

START_TEST(test_ots_seed_jar_transfer_seed_out_for_fingerprint)
{
    ots_result_t* result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 666), 0);
    ots_free_result(&result);
    result = ots_legacy_seed_decode(
        get_legacy_seed_test_case_phrase(2),
        get_legacy_seed_test_case_height(2),
        get_legacy_seed_test_case_time(2),
        (OTS_NETWORK)get_legacy_seed_test_case_network(2)
    );
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* legacy_seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_jar_transfer_seed_in(
        &legacy_seed,
        "legacy"
    );
    ck_assert(ots_result_is_seed(result));
    ck_assert(ots_result_handle_is_reference(result));
    ots_free_result(&result);
    ck_assert_ptr_null(legacy_seed);
    result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 0), 1);
    ots_free_result(&result);
    result = ots_seed_jar_transfer_seed_out_for_fingerprint(
        get_legacy_seed_test_case_fingerprint(2)
    );
    ck_assert(ots_result_is_seed(result));
    ck_assert_ptr_null(legacy_seed);
    legacy_seed = ots_result_handle(result);
    ck_assert_ptr_nonnull(legacy_seed);
    ck_assert(!legacy_seed->reference);
    ots_free_result(&result);
    result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 666), 0);
    ots_free_result(&result);
    ots_free_handle(&legacy_seed);
}
END_TEST

START_TEST(test_ots_seed_jar_transfer_seed_out_for_address)
{
    ots_result_t* result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 666), 0);
    ots_free_result(&result);
    result = ots_legacy_seed_decode(
        get_legacy_seed_test_case_phrase(2),
        get_legacy_seed_test_case_height(2),
        get_legacy_seed_test_case_time(2),
        (OTS_NETWORK)get_legacy_seed_test_case_network(2)
    );
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* legacy_seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_jar_transfer_seed_in(
        &legacy_seed,
        "legacy"
    );
    ck_assert(ots_result_is_seed(result));
    ck_assert(ots_result_handle_is_reference(result));
    ots_free_result(&result);
    ck_assert_ptr_null(legacy_seed);
    result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 0), 1);
    ots_free_result(&result);
    result = ots_seed_jar_transfer_seed_out_for_address(
        get_legacy_seed_test_case_address(2)
    );
    ck_assert(ots_result_is_seed(result));
    ck_assert_ptr_null(legacy_seed);
    legacy_seed = ots_result_handle(result);
    ck_assert_ptr_nonnull(legacy_seed);
    ck_assert(!legacy_seed->reference);
    ots_free_result(&result);
    result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 666), 0);
    ots_free_result(&result);
    ots_free_handle(&legacy_seed);
}
END_TEST

START_TEST(test_ots_seed_jar_clear)
{
    ots_result_t* result = ots_legacy_seed_decode(
        get_legacy_seed_test_case_phrase(2),
        get_legacy_seed_test_case_height(2),
        get_legacy_seed_test_case_time(2),
        (OTS_NETWORK)get_legacy_seed_test_case_network(2)
    );
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* legacy_seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_jar_transfer_seed_in(
        &legacy_seed,
        "legacy"
    );
    ck_assert(ots_result_is_seed(result));
    ots_free_result(&result);
    result = ots_monero_seed_decode(
        get_monero_seed_test_case_phrase(5),
        get_monero_seed_test_case_height(5),
        get_monero_seed_test_case_time(5),
        (OTS_NETWORK)get_monero_seed_test_case_network(5),
        get_monero_seed_test_case_password(5)
    );
    ots_handle_t* monero_seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_jar_transfer_seed_in(
        &monero_seed,
        "monero"
    );
    ck_assert(ots_result_is_seed(result));
    ots_free_result(&result);
    result = ots_polyseed_decode(
        get_polyseed_test_case_phrase(0),
        (OTS_NETWORK)get_polyseed_test_case_network(0),
        get_polyseed_test_case_password(0),
        get_polyseed_test_case_passphrase(0)
    );
    ots_handle_t* polyseed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_jar_transfer_seed_in(
        &polyseed,
        "polyseed"
    );
    ck_assert(ots_result_is_seed(result));
    ots_free_result(&result);
    result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 0), 3);
    ots_free_result(&result);
    result = ots_seed_jar_clear();
    ck_assert(ots_result_is_boolean(result));
    ck_assert(ots_result_boolean(result, false));
    ots_free_result(&result);
    result = ots_seed_jar_seed_count();
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(ots_result_number(result, 666), 0);
    ots_free_result(&result);
}
END_TEST

START_TEST(test_ots_seed_jar_seeds)
{
    char* fingerprints[10];
    for(int i = 0; i < 11; i++) {
        ots_result_t* result = ots_seed_jar_seed_count();
        ck_assert(ots_result_is_number(result));
        ck_assert_int_eq(ots_result_number(result, 666), i);
        ots_free_result(&result);
        if(i==10)
            break;
        result = ots_monero_seed_generate(0, 0, OTS_NETWORK_MAIN);
        ck_assert(ots_result_is_seed(result));
        ots_handle_t* seed = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_seed_fingerprint(seed);
        ck_assert(ots_result_is_string(result));
        fingerprints[i] = ots_result_string_copy(result);
        ots_free_result(&result);
        result = ots_seed_jar_transfer_seed_in(&seed, fingerprints[i]);
        ots_free_result(&result);
    }
    ots_result_t* result = ots_seed_jar_seeds();
    ck_assert(ots_result_data_handle_is_seed(result));
    ots_handle_t* seeds = ots_result_handle_array(result);
    ck_assert_ptr_nonnull(seeds);
    for(size_t i = 0; i < 10; i++) {
        ck_assert_ptr_nonnull(&seeds[i]);
        result = ots_seed_fingerprint((const ots_handle_t*)&seeds[i]);
        ck_assert(ots_result_is_string(result));
        ck_assert_str_eq(fingerprints[i], ots_result_string(result));
        ots_free_result(&result);
    }
}
END_TEST

START_TEST(test_ots_seed_jar_seed_count)
{
    for(int i = 0; i < 11; i++) {
        ots_result_t* result = ots_seed_jar_seed_count();
        ck_assert(ots_result_is_number(result));
        ck_assert_int_eq(ots_result_number(result, 666), i);
        ots_free_result(&result);
        if(i==10)
            break;
        result = ots_monero_seed_generate(0, 0, OTS_NETWORK_MAIN);
        ck_assert(ots_result_is_seed(result));
        ots_handle_t* seed = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_seed_fingerprint(seed);
        ck_assert(ots_result_is_string(result));
        char* fingerprint = ots_result_string_copy(result);
        ots_free_result(&result);
        result = ots_seed_jar_transfer_seed_in(&seed, fingerprint);
        ots_free_string(&fingerprint);
    }
}
END_TEST

START_TEST(test_ots_seed_jar_seed_for_index)
{
    char* fingerprints[10];
    for(int i = 0; i < 11; i++) {
        ots_result_t* result = ots_seed_jar_seed_count();
        ck_assert(ots_result_is_number(result));
        ck_assert_int_eq(ots_result_number(result, 666), i);
        ots_free_result(&result);
        if(i==10)
            break;
        result = ots_monero_seed_generate(0, 0, OTS_NETWORK_MAIN);
        ck_assert(ots_result_is_seed(result));
        ots_handle_t* seed = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_seed_fingerprint(seed);
        ck_assert(ots_result_is_string(result));
        fingerprints[i] = ots_result_string_copy(result);
        ots_free_result(&result);
        result = ots_seed_jar_transfer_seed_in(&seed, fingerprints[i]);
        ots_free_result(&result);
    }
    for(size_t i = 0; i < 10; i++) {
        ots_result_t* result = ots_seed_jar_seed_for_index(i);
        ck_assert(ots_result_is_seed(result));
        ots_handle_t* seed = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_seed_fingerprint(seed);
        ck_assert(ots_result_is_string(result));
        ck_assert_str_eq(fingerprints[i], ots_result_string(result));
        ots_free_result(&result);
    }
}

START_TEST(test_ots_seed_jar_seed_for_fingerprint)
{
    char* fingerprints[10];
    for(int i = 0; i < 11; i++) {
        ots_result_t* result = ots_seed_jar_seed_count();
        ck_assert(ots_result_is_number(result));
        ck_assert_int_eq(ots_result_number(result, 666), i);
        ots_free_result(&result);
        if(i==10)
            break;
        result = ots_monero_seed_generate(0, 0, OTS_NETWORK_MAIN);
        ck_assert(ots_result_is_seed(result));
        ots_handle_t* seed = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_seed_fingerprint(seed);
        ck_assert(ots_result_is_string(result));
        fingerprints[i] = ots_result_string_copy(result);
        ots_free_result(&result);
        result = ots_seed_jar_transfer_seed_in(&seed, fingerprints[i]);
        ots_free_result(&result);
    }
    for(size_t i = 0; i < 10; i++) {
        ots_result_t* result = ots_seed_jar_seed_for_fingerprint(fingerprints[i]);
        ck_assert(ots_result_is_seed(result));
        ots_handle_t* seed = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_seed_fingerprint(seed);
        ck_assert(ots_result_is_string(result));
        ck_assert_str_eq(fingerprints[i], ots_result_string(result));
        ots_free_result(&result);
    }
}
END_TEST

START_TEST(test_ots_seed_jar_seed_for_address)
{
    char* fingerprints[10];
    char* addresses[10];
    for(int i = 0; i < 11; i++) {
        ots_result_t* result = ots_seed_jar_seed_count();
        ck_assert(ots_result_is_number(result));
        ck_assert_int_eq(ots_result_number(result, 666), i);
        ots_free_result(&result);
        if(i==10)
            break;
        result = ots_monero_seed_generate(0, 0, OTS_NETWORK_MAIN);
        ck_assert(ots_result_is_seed(result));
        ots_handle_t* seed = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_seed_fingerprint(seed);
        ck_assert(ots_result_is_string(result));
        fingerprints[i] = ots_result_string_copy(result);
        ots_free_result(&result);
        result = ots_seed_address(seed);
        ck_assert(ots_result_is_address(result));
        ots_handle_t* address = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_address_base58_string(address);
        addresses[i] = ots_result_string_copy(result);
        ots_free_result(&result);
        ots_free_handle(&address);
        ots_free_result(&result);
        result = ots_seed_jar_transfer_seed_in(&seed, fingerprints[i]);
        ots_free_result(&result);
    }
    for(size_t i = 0; i < 10; i++) {
        ots_result_t* result = ots_seed_jar_seed_for_address(addresses[i]);
        ck_assert(ots_result_is_seed(result));
        ots_handle_t* seed = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_seed_fingerprint(seed);
        ck_assert(ots_result_is_string(result));
        ck_assert_str_eq(fingerprints[i], ots_result_string(result));
        ots_free_result(&result);
    }
}
END_TEST

START_TEST(test_ots_seed_jar_seed_for_name)
{
    char* fingerprints[10];
    for(int i = 0; i < 11; i++) {
        ots_result_t* result = ots_seed_jar_seed_count();
        ck_assert(ots_result_is_number(result));
        ck_assert_int_eq(ots_result_number(result, 666), i);
        ots_free_result(&result);
        if(i==10)
            break;
        result = ots_monero_seed_generate(0, 0, OTS_NETWORK_MAIN);
        ck_assert(ots_result_is_seed(result));
        ots_handle_t* seed = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_seed_fingerprint(seed);
        ck_assert(ots_result_is_string(result));
        fingerprints[i] = ots_result_string_copy(result);
        ots_free_result(&result);
        result = ots_seed_jar_transfer_seed_in(&seed, fingerprints[i]);
        ots_free_result(&result);
    }
    for(size_t i = 0; i < 10; i++) {
        ots_result_t* result = ots_seed_jar_seed_for_name(fingerprints[i]);
        ck_assert(ots_result_is_seed(result));
        ots_handle_t* seed = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_seed_fingerprint(seed);
        ck_assert(ots_result_is_string(result));
        ck_assert_str_eq(fingerprints[i], ots_result_string(result));
        ots_free_result(&result);
    }
}
END_TEST

START_TEST(test_ots_seed_jar_seed_name)
{
    ots_result_t* result = ots_monero_seed_generate(0, 0, OTS_NETWORK_MAIN);
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_jar_add_seed(
        seed,
        "monero"
    );
    ck_assert(ots_result_is_seed(result));
    ck_assert(ots_result_handle_is_reference(result));
    ots_free_result(&result);
    ck_assert_ptr_nonnull(seed);
    result = ots_seed_jar_seed_name(seed);
    ck_assert(ots_result_is_string(result));
    ck_assert_str_eq(ots_result_string(result), "monero");
    ots_free_result(&result);

}
END_TEST

START_TEST(test_ots_seed_jar_seed_rename)
{
    ots_result_t* result = ots_monero_seed_generate(0, 0, OTS_NETWORK_MAIN);
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_jar_add_seed(
        seed,
        "monero"
    );
    ck_assert(ots_result_is_seed(result));
    ck_assert(ots_result_handle_is_reference(result));
    ots_free_result(&result);
    ck_assert_ptr_nonnull(seed);
    result = ots_seed_jar_seed_name(seed);
    ck_assert(ots_result_is_string(result));
    ck_assert_str_eq(ots_result_string(result), "monero");
    ots_free_result(&result);
    result = ots_seed_jar_seed_rename(
        seed,
        "monero2"
    );
    ck_assert(ots_result_is_boolean(result));
    ck_assert(ots_result_boolean(result, false));
    ots_free_result(&result);
    result = ots_seed_jar_seed_name(seed);
    ck_assert(ots_result_is_string(result));
    ck_assert_str_eq(ots_result_string(result), "monero2");
    ots_free_result(&result);

}
END_TEST


START_TEST(test_ots_seed_jar_item_name)
{
    char* fingerprints[10];
    for(int i = 0; i < 11; i++) {
        ots_result_t* result = ots_seed_jar_seed_count();
        ck_assert(ots_result_is_number(result));
        ck_assert_int_eq(ots_result_number(result, 666), i);
        ots_free_result(&result);
        if(i==10)
            break;
        result = ots_monero_seed_generate(0, 0, OTS_NETWORK_MAIN);
        ck_assert(ots_result_is_seed(result));
        ots_handle_t* seed = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_seed_fingerprint(seed);
        ck_assert(ots_result_is_string(result));
        fingerprints[i] = ots_result_string_copy(result);
        ots_free_result(&result);
        result = ots_seed_jar_transfer_seed_in(&seed, fingerprints[i]);
        ots_free_result(&result);
    }
    for(size_t i = 0; i < 10; i++) {
        ots_result_t* result = ots_seed_jar_item_name(i);
        ck_assert(ots_result_is_string(result));
        ck_assert_str_eq(fingerprints[i], ots_result_string(result));
        ots_free_result(&result);
    }
}
END_TEST

START_TEST(test_ots_seed_jar_item_fingerprint)
{
    char* fingerprints[10];
    for(int i = 0; i < 11; i++) {
        ots_result_t* result = ots_seed_jar_seed_count();
        ck_assert(ots_result_is_number(result));
        ck_assert_int_eq(ots_result_number(result, 666), i);
        ots_free_result(&result);
        if(i==10)
            break;
        result = ots_monero_seed_generate(0, 0, OTS_NETWORK_MAIN);
        ck_assert(ots_result_is_seed(result));
        ots_handle_t* seed = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_seed_fingerprint(seed);
        ck_assert(ots_result_is_string(result));
        fingerprints[i] = ots_result_string_copy(result);
        ots_free_result(&result);
        result = ots_seed_jar_transfer_seed_in(&seed, fingerprints[i]);
        ots_free_result(&result);
    }
    for(size_t i = 0; i < 10; i++) {
        ots_result_t* result = ots_seed_jar_item_fingerprint(i);
        ck_assert(ots_result_is_string(result));
        ck_assert_str_eq(fingerprints[i], ots_result_string(result));
        ots_free_result(&result);
    }
}
END_TEST

START_TEST(test_ots_seed_jar_item_address)
{
    char* fingerprints[10];
    char* addresses[10];
    for(int i = 0; i < 11; i++) {
        ots_result_t* result = ots_seed_jar_seed_count();
        ck_assert(ots_result_is_number(result));
        ck_assert_int_eq(ots_result_number(result, 666), i);
        ots_free_result(&result);
        if(i==10)
            break;
        result = ots_monero_seed_generate(0, 0, OTS_NETWORK_MAIN);
        ck_assert(ots_result_is_seed(result));
        ots_handle_t* seed = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_seed_fingerprint(seed);
        ck_assert(ots_result_is_string(result));
        fingerprints[i] = ots_result_string_copy(result);
        ots_free_result(&result);
        result = ots_seed_address(seed);
        ck_assert(ots_result_is_address(result));
        ots_handle_t* address = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_address_base58_string(address);
        addresses[i] = ots_result_string_copy(result);
        ots_free_result(&result);
        ots_free_handle(&address);
        ots_free_result(&result);
        result = ots_seed_jar_transfer_seed_in(&seed, fingerprints[i]);
        ots_free_result(&result);
    }
    for(size_t i = 0; i < 10; i++) {
        ots_result_t* result = ots_seed_jar_item_address(i);
        ck_assert(ots_result_is_address(result));
        ots_handle_t* address = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_address_base58_string(address);
        ck_assert(ots_result_is_string(result));
        ck_assert_str_eq(addresses[i], ots_result_string(result));
        ots_free_result(&result);
    }
}
END_TEST

START_TEST(test_ots_seed_jar_item_address_string)
{
    char* fingerprints[10];
    char* addresses[10];
    for(int i = 0; i < 11; i++) {
        ots_result_t* result = ots_seed_jar_seed_count();
        ck_assert(ots_result_is_number(result));
        ck_assert_int_eq(ots_result_number(result, 666), i);
        ots_free_result(&result);
        if(i==10)
            break;
        result = ots_monero_seed_generate(0, 0, OTS_NETWORK_MAIN);
        ck_assert(ots_result_is_seed(result));
        ots_handle_t* seed = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_seed_fingerprint(seed);
        ck_assert(ots_result_is_string(result));
        fingerprints[i] = ots_result_string_copy(result);
        ots_free_result(&result);
        result = ots_seed_address(seed);
        ck_assert(ots_result_is_address(result));
        ots_handle_t* address = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_address_base58_string(address);
        addresses[i] = ots_result_string_copy(result);
        ots_free_result(&result);
        ots_free_handle(&address);
        ots_free_result(&result);
        result = ots_seed_jar_transfer_seed_in(&seed, fingerprints[i]);
        ots_free_result(&result);
    }
    for(size_t i = 0; i < 10; i++) {
        ots_result_t* result = ots_seed_jar_item_address_string(i);
        ck_assert(ots_result_is_string(result));
        ck_assert_str_eq(addresses[i], ots_result_string(result));
        ots_free_result(&result);
    }
}
END_TEST

START_TEST(test_ots_seed_jar_item_seed_type)
{
    char* fingerprints[3];
    OTS_SEED_TYPE seed_types[3];
    for(int i = 0; i < 4; i++) {
        ots_result_t* result = ots_seed_jar_seed_count();
        ck_assert(ots_result_is_number(result));
        ck_assert_int_eq(ots_result_number(result, 666), i);
        ots_free_result(&result);
        if(i==3)
            break;
        switch(i) {
            case 0:
                result = ots_legacy_seed_decode(
                    get_legacy_seed_test_case_phrase(2),
                    get_legacy_seed_test_case_height(2),
                    get_legacy_seed_test_case_time(2),
                    (OTS_NETWORK)get_legacy_seed_test_case_network(2)
                );
                seed_types[i] = OTS_SEED_TYPE_MONERO;
                break;
            case 1:
                result = ots_monero_seed_generate(0, 0, OTS_NETWORK_MAIN);
                seed_types[i] = OTS_SEED_TYPE_MONERO;
                break;
            case 2:
                result = ots_polyseed_decode(
                    get_polyseed_test_case_phrase(0),
                    (OTS_NETWORK)get_polyseed_test_case_network(0),
                    get_polyseed_test_case_password(0),
                    get_polyseed_test_case_passphrase(0)
                );
                seed_types[i] = OTS_SEED_TYPE_POLYSEED;
                break;
        }
        ck_assert(ots_result_is_seed(result));
        ots_handle_t* seed = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_seed_fingerprint(seed);
        ck_assert(ots_result_is_string(result));
        fingerprints[i] = ots_result_string_copy(result);
        ots_free_result(&result);
        result = ots_seed_jar_transfer_seed_in(&seed, fingerprints[i]);
        ots_free_result(&result);
    }
    for(size_t i = 0; i < 3; i++) {
        ots_result_t* result = ots_seed_jar_item_seed_type(i);
        ck_assert(ots_result_is_seed_type(result));
        ck_assert_uint_eq(ots_result_seed_type(result), seed_types[i]);
        ck_assert(ots_result_seed_type_is_type(result, seed_types[i]));
        ots_free_result(&result);
    }
}
END_TEST

START_TEST(test_ots_seed_jar_item_seed_type_string)
{
    char* fingerprints[3];
    char* seed_types[3];
    for(int i = 0; i < 4; i++) {
        ots_result_t* result = ots_seed_jar_seed_count();
        ck_assert(ots_result_is_number(result));
        ck_assert_int_eq(ots_result_number(result, 666), i);
        ots_free_result(&result);
        if(i==3)
            break;
        switch(i) {
            case 0:
                result = ots_legacy_seed_decode(
                    get_legacy_seed_test_case_phrase(2),
                    get_legacy_seed_test_case_height(2),
                    get_legacy_seed_test_case_time(2),
                    (OTS_NETWORK)get_legacy_seed_test_case_network(2)
                );
                seed_types[i] = "Monero";
                break;
            case 1:
                result = ots_monero_seed_generate(0, 0, OTS_NETWORK_MAIN);
                seed_types[i] = "Monero";
                break;
            case 2:
                result = ots_polyseed_decode(
                    get_polyseed_test_case_phrase(0),
                    (OTS_NETWORK)get_polyseed_test_case_network(0),
                    get_polyseed_test_case_password(0),
                    get_polyseed_test_case_passphrase(0)
                );
                seed_types[i] = "Polyseed";
                break;
        }
        ck_assert(ots_result_is_seed(result));
        ots_handle_t* seed = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_seed_fingerprint(seed);
        ck_assert(ots_result_is_string(result));
        fingerprints[i] = ots_result_string_copy(result);
        ots_free_result(&result);
        result = ots_seed_jar_transfer_seed_in(&seed, fingerprints[i]);
        ots_free_result(&result);
    }
    for(size_t i = 0; i < 3; i++) {
        ots_result_t* result = ots_seed_jar_item_seed_type_string(i);
        ck_assert(ots_result_is_string(result));
        ck_assert_str_eq(ots_result_string(result), seed_types[i]);
        ots_free_result(&result);
    }
}
END_TEST

START_TEST(test_ots_seed_jar_item_is_legacy)
{
    char* fingerprints[3];
    bool is_legacy[3];
    for(int i = 0; i < 4; i++) {
        ots_result_t* result = ots_seed_jar_seed_count();
        ck_assert(ots_result_is_number(result));
        ck_assert_int_eq(ots_result_number(result, 666), i);
        ots_free_result(&result);
        if(i==3)
            break;
        switch(i) {
            case 0:
                result = ots_legacy_seed_decode(
                    get_legacy_seed_test_case_phrase(2),
                    get_legacy_seed_test_case_height(2),
                    get_legacy_seed_test_case_time(2),
                    (OTS_NETWORK)get_legacy_seed_test_case_network(2)
                );
                is_legacy[i] = true;
                break;
            case 1:
                result = ots_monero_seed_generate(0, 0, OTS_NETWORK_MAIN);
                is_legacy[i] = false;
                break;
            case 2:
                result = ots_polyseed_decode(
                    get_polyseed_test_case_phrase(0),
                    (OTS_NETWORK)get_polyseed_test_case_network(0),
                    get_polyseed_test_case_password(0),
                    get_polyseed_test_case_passphrase(0)
                );
                is_legacy[i] = false;
                break;
        }
        ck_assert(ots_result_is_seed(result));
        ots_handle_t* seed = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_seed_fingerprint(seed);
        ck_assert(ots_result_is_string(result));
        fingerprints[i] = ots_result_string_copy(result);
        ots_free_result(&result);
        result = ots_seed_jar_transfer_seed_in(&seed, fingerprints[i]);
        ots_free_result(&result);
    }
    for(size_t i = 0; i < 3; i++) {
        ots_result_t* result = ots_seed_jar_item_is_legacy(i);
        ck_assert(ots_result_is_boolean(result));
        ck_assert(ots_result_boolean(result, false) == is_legacy[i]);
        ots_free_result(&result);
    }
}
END_TEST

START_TEST(test_ots_seed_jar_item_network)
{
    char* fingerprints[3];
    OTS_NETWORK networks[3];
    for(int i = 0; i < 4; i++) {
        ots_result_t* result = ots_seed_jar_seed_count();
        ck_assert(ots_result_is_number(result));
        ck_assert_int_eq(ots_result_number(result, 666), i);
        ots_free_result(&result);
        if(i==3)
            break;
        switch(i) {
            case 0:
                result = ots_legacy_seed_decode(
                    get_legacy_seed_test_case_phrase(2),
                    get_legacy_seed_test_case_height(2),
                    get_legacy_seed_test_case_time(2),
                    (OTS_NETWORK)get_legacy_seed_test_case_network(2)
                );
                networks[i] = (OTS_NETWORK)get_legacy_seed_test_case_network(2);
                break;
            case 1:
                result = ots_monero_seed_generate(0, 0, OTS_NETWORK_STAGE);
                networks[i] = OTS_NETWORK_STAGE;
                break;
            case 2:
                result = ots_polyseed_decode(
                    get_polyseed_test_case_phrase(0),
                    OTS_NETWORK_TEST,
                    get_polyseed_test_case_password(0),
                    get_polyseed_test_case_passphrase(0)
                );
                networks[i] = OTS_NETWORK_TEST;
                break;
        }
        ck_assert(ots_result_is_seed(result));
        ots_handle_t* seed = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_seed_fingerprint(seed);
        ck_assert(ots_result_is_string(result));
        fingerprints[i] = ots_result_string_copy(result);
        ots_free_result(&result);
        result = ots_seed_jar_transfer_seed_in(&seed, fingerprints[i]);
        ots_free_result(&result);
    }
    for(size_t i = 0; i < 3; i++) {
        ots_result_t* result = ots_seed_jar_item_network(i);
        ck_assert(ots_result_is_network(result));
        ck_assert(ots_result_network_is_type(result, networks[i]));
        ck_assert_int_eq(ots_result_network(result), networks[i]);
        ots_free_result(&result);
    }
}
END_TEST

START_TEST(test_ots_seed_jar_item_network_string)
{
    char* fingerprints[3];
    char* networks[3];
    for(int i = 0; i < 4; i++) {
        ots_result_t* result = ots_seed_jar_seed_count();
        ck_assert(ots_result_is_number(result));
        ck_assert_int_eq(ots_result_number(result, 666), i);
        ots_free_result(&result);
        if(i==3)
            break;
        switch(i) {
            case 0:
                result = ots_legacy_seed_decode(
                    get_legacy_seed_test_case_phrase(2),
                    get_legacy_seed_test_case_height(2),
                    get_legacy_seed_test_case_time(2),
                    (OTS_NETWORK)get_legacy_seed_test_case_network(2)
                );
                networks[i] = "Main";
                break;
            case 1:
                result = ots_monero_seed_generate(0, 0, OTS_NETWORK_TEST);
                networks[i] = "Test";
                break;
            case 2:
                result = ots_polyseed_decode(
                    get_polyseed_test_case_phrase(0),
                    OTS_NETWORK_STAGE,
                    get_polyseed_test_case_password(0),
                    get_polyseed_test_case_passphrase(0)
                );
                networks[i] = "Stage";
                break;
        }
        ck_assert(ots_result_is_seed(result));
        ots_handle_t* seed = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_seed_fingerprint(seed);
        ck_assert(ots_result_is_string(result));
        fingerprints[i] = ots_result_string_copy(result);
        ots_free_result(&result);
        result = ots_seed_jar_transfer_seed_in(&seed, fingerprints[i]);
        ots_free_result(&result);
    }
    for(size_t i = 0; i < 3; i++) {
        ots_result_t* result = ots_seed_jar_item_network_string(i);
        ck_assert(ots_result_is_string(result));
        ck_assert_str_eq(ots_result_string(result), networks[i]);
        ots_free_result(&result);
    }
}
END_TEST

START_TEST(test_ots_seed_jar_item_height)
{
    uint64_t heights[5] = { 1234, 5678, 91011, 121314, 151617 };
    for(size_t i = 0; i < 5; i++) {
        ots_result_t* result = ots_monero_seed_generate(
            heights[i],
            0,
            OTS_NETWORK_MAIN
        );
        ck_assert(ots_result_is_seed(result));
        ots_handle_t* seed = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_seed_fingerprint(seed);
        ck_assert(ots_result_is_string(result));
        char* fingerprint = ots_result_string_copy(result);
        ots_free_result(&result);
        result = ots_seed_jar_transfer_seed_in(&seed, fingerprint);
        ck_assert(ots_result_is_seed(result));
        ots_free_result(&result);
    }
    for(size_t i = 0; i < 5; i++) {
        ots_result_t* result = ots_seed_jar_item_height(i);
        ck_assert(ots_result_is_number(result));
        ck_assert_uint_eq(ots_result_number(result, 0), heights[i]);
        ots_free_result(&result);
    }
}
END_TEST

START_TEST(test_ots_seed_jar_item_timestamp)
{
    uint64_t heights[5] = { 1234, 5678, 91011, 121314, 151617 };
    uint64_t timestamps[5];
    for(size_t i = 0; i < 5; i++)
        timestamps[i] = get_timestamp_by_height(heights[i]);
    for(size_t i = 0; i < 5; i++) {
        ots_result_t* result = ots_monero_seed_generate(
            0,
            timestamps[i],
            OTS_NETWORK_MAIN
        );
        ck_assert(ots_result_is_seed(result));
        ots_handle_t* seed = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_seed_fingerprint(seed);
        ck_assert(ots_result_is_string(result));
        char* fingerprint = ots_result_string_copy(result);
        ots_free_result(&result);
        result = ots_seed_jar_transfer_seed_in(&seed, fingerprint);
        ck_assert(ots_result_is_seed(result));
        ots_free_result(&result);
    }
    for(size_t i = 0; i < 5; i++) {
        ots_result_t* result = ots_seed_jar_item_timestamp(i);
        ck_assert(ots_result_is_number(result));
        ck_assert_uint_eq(ots_result_number(result, 0), timestamps[i]);
        ots_free_result(&result);
    }
}
END_TEST

START_TEST(test_ots_seed_jar_item_wallet)
{
    char* addresses[5];
    for(size_t i = 0; i < 5; i++) {
        ots_result_t* result = ots_monero_seed_generate(
            0,
            0,
            OTS_NETWORK_MAIN
        );
        ck_assert(ots_result_is_seed(result));
        ots_handle_t* seed = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_seed_address(seed);
        ck_assert(ots_result_is_address(result));
        ots_handle_t* address = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_address_base58_string(address);
        addresses[i] = ots_result_string_copy(result);
        ots_free_handle(&address);
        ots_free_result(&result);
        result = ots_seed_jar_transfer_seed_in(&seed, addresses[i]);
        ck_assert(ots_result_is_seed(result));
        ots_free_result(&result);
    }
    for(size_t i = 0; i < 5; i++) {
        ots_result_t* result = ots_seed_jar_item_wallet(i);
        ck_assert(ots_result_is_wallet(result));
        ots_handle_t* wallet = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_wallet_address(wallet);
        ck_assert(ots_result_is_address(result));
        ots_handle_t* address = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_address_base58_string(address);
        ck_assert(ots_result_is_string(result));
        ck_assert_str_eq(ots_result_string(result), addresses[i]);
        ots_free_result(&result);
        ots_free_handle(&address);
    }
}
END_TEST

Suite* seed_jar_suite(void)
{
    Suite* s = suite_create("Seed Jar");
    // Core functionality
    TCase* tc_core = tcase_create("Core");
    tcase_add_test(tc_core, test_ots_seed_jar_add_seed);
    tcase_add_test(tc_core, test_ots_seed_jar_remove_seed);
    tcase_add_test(tc_core, test_ots_seed_jar_purge_seed_for_index);
    tcase_add_test(tc_core, test_ots_seed_jar_purge_seed_for_name);
    tcase_add_test(tc_core, test_ots_seed_jar_purge_seed_for_fingerprint);
    tcase_add_test(tc_core, test_ots_seed_jar_purge_seed_for_address);
    tcase_add_test(tc_core, test_ots_seed_jar_transfer_seed_in);
    tcase_add_test(tc_core, test_ots_seed_jar_transfer_seed_out);
    tcase_add_test(tc_core, test_ots_seed_jar_transfer_seed_out_for_index);
    tcase_add_test(tc_core, test_ots_seed_jar_transfer_seed_out_for_name);
    tcase_add_test(tc_core, test_ots_seed_jar_transfer_seed_out_for_fingerprint);
    tcase_add_test(tc_core, test_ots_seed_jar_transfer_seed_out_for_address);
    tcase_add_test(tc_core, test_ots_seed_jar_clear);
    tcase_add_test(tc_core, test_ots_seed_jar_seeds);
    tcase_add_test(tc_core, test_ots_seed_jar_seed_count);
    tcase_add_test(tc_core, test_ots_seed_jar_seed_for_index);
    tcase_add_test(tc_core, test_ots_seed_jar_seed_for_fingerprint);
    tcase_add_test(tc_core, test_ots_seed_jar_seed_for_address);
    tcase_add_test(tc_core, test_ots_seed_jar_seed_for_name);
    tcase_add_test(tc_core, test_ots_seed_jar_seed_name);
    tcase_add_test(tc_core, test_ots_seed_jar_seed_rename);
    tcase_add_test(tc_core, test_ots_seed_jar_item_name);
    tcase_add_test(tc_core, test_ots_seed_jar_item_fingerprint);
    tcase_add_test(tc_core, test_ots_seed_jar_item_address);
    tcase_add_test(tc_core, test_ots_seed_jar_item_address_string);
    tcase_add_test(tc_core, test_ots_seed_jar_item_seed_type);
    tcase_add_test(tc_core, test_ots_seed_jar_item_seed_type_string);
    tcase_add_test(tc_core, test_ots_seed_jar_item_is_legacy);
    tcase_add_test(tc_core, test_ots_seed_jar_item_network);
    tcase_add_test(tc_core, test_ots_seed_jar_item_network_string);
    tcase_add_test(tc_core, test_ots_seed_jar_item_height);
    tcase_add_test(tc_core, test_ots_seed_jar_item_timestamp);
    tcase_add_test(tc_core, test_ots_seed_jar_item_wallet);
    suite_add_tcase(s, tc_core);
    return s;
}

