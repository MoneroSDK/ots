#include <check.h>
#include "../include/ots.h"
#include "../data.h"

START_TEST(test_ots_seed_jar_add_seed)
{
    ck_abort_msg("ots_seed_jar_add_seed incomplete");
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
}
END_TEST

START_TEST(test_ots_seed_jar_remove_seed)
{
    ck_abort_msg("ots_seed_jar_remove_seed incomplete");
}
END_TEST

START_TEST(test_ots_seed_jar_transfer_seed_in)
{
    ck_abort_msg("ots_seed_jar_transfer_seed_in incomplete");
}
END_TEST

START_TEST(test_ots_seed_jar_transfer_seed_out)
{
    ck_abort_msg("ots_seed_jar_transfer_seed_out incomplete");
}
END_TEST

START_TEST(test_ots_seed_jar_clear)
{
    ck_abort_msg("ots_seed_jar_clear incomplete");
}
END_TEST

START_TEST(test_ots_seed_jar_seeds)
{
    ck_abort_msg("ots_seed_jar_seeds incomplete");
}
END_TEST

START_TEST(test_ots_seed_jar_seed_count)
{
    ck_abort_msg("ots_seed_jar_seed_count incomplete");
}
END_TEST

START_TEST(test_ots_seed_jar_seed_for_fingerprint)
{
    ck_abort_msg("ots_seed_jar_seed_for_fingerprint incomplete");
}
END_TEST

START_TEST(test_ots_seed_jar_seed_for_address)
{
    ck_abort_msg("ots_seed_jar_seed_for_address incomplete");
}
END_TEST

START_TEST(test_ots_seed_jar_seed_for_name)
{
    ck_abort_msg("ots_seed_jar_seed_for_name incomplete");
}
END_TEST

START_TEST(test_ots_seed_jar_seed_name)
{
    ck_abort_msg("ots_seed_jar_seed_name incomplete");
}
END_TEST

START_TEST(test_ots_seed_jar_seed_rename)
{
    ck_abort_msg("ots_seed_jar_seed_rename incomplete");
}
END_TEST


START_TEST(test_ots_seed_jar_item_name)
{
    ck_abort_msg("ots_seed_jar_item_name incomplete");
}
END_TEST

START_TEST(test_ots_seed_jar_item_fingerprint)
{
    ck_abort_msg("ots_seed_jar_item_fingerprint incomplete");
}
END_TEST

START_TEST(test_ots_seed_jar_item_address)
{
    ck_abort_msg("ots_seed_jar_item_address incomplete");
}
END_TEST

START_TEST(test_ots_seed_jar_item_address_string)
{
    ck_abort_msg("ots_seed_jar_item_address_string incomplete");
}
END_TEST

START_TEST(test_ots_seed_jar_item_seed_type)
{
    ck_abort_msg("ots_seed_jar_item_seed_type incomplete");
}
END_TEST

START_TEST(test_ots_seed_jar_item_seed_type_string)
{
    ck_abort_msg("ots_seed_jar_item_seed_type_string incomplete");
}
END_TEST

START_TEST(test_ots_seed_jar_item_is_legacy)
{
    ck_abort_msg("ots_seed_jar_item_is_legacy incomplete");
}
END_TEST

START_TEST(test_ots_seed_jar_item_network)
{
    ck_abort_msg("ots_seed_jar_item_network incomplete");
}
END_TEST

START_TEST(test_ots_seed_jar_item_network_string)
{
    ck_abort_msg("ots_seed_jar_item_network_string incomplete");
}
END_TEST

START_TEST(test_ots_seed_jar_item_height)
{
    ck_abort_msg("ots_seed_jar_item_height incomplete");
}
END_TEST

START_TEST(test_ots_seed_jar_item_timestamp)
{
    ck_abort_msg("ots_seed_jar_item_timestamp incomplete");
}
END_TEST

START_TEST(test_ots_seed_jar_item_wallet)
{
    ck_abort_msg("ots_seed_jar_item_wallet incomplete");
}
END_TEST

Suite* seed_jar_suite(void)
{
    Suite* s = suite_create("Seed Jar");
    // Core functionality
    TCase* tc_core = tcase_create("Core");
    tcase_add_test(tc_core, test_ots_seed_jar_add_seed);
    tcase_add_test(tc_core, test_ots_seed_jar_remove_seed);
    tcase_add_test(tc_core, test_ots_seed_jar_transfer_seed_in);
    tcase_add_test(tc_core, test_ots_seed_jar_transfer_seed_out);
    tcase_add_test(tc_core, test_ots_seed_jar_clear);
    tcase_add_test(tc_core, test_ots_seed_jar_seeds);
    tcase_add_test(tc_core, test_ots_seed_jar_seed_count);
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

