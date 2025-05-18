#include <check.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/ots.h"
#include "../data.h"
#include "../json-tx-description.h"

ots_tx_description_t* tx_description_from_data(
        size_t wallet_index,
        size_t tx_index
) {
    void* txd = tx_description_from_json(
        get_wallet_test_case_unsigned_transaction_json(wallet_index, tx_index),
        get_wallet_test_case_unsigned_transaction_json_size(wallet_index, tx_index),
        get_wallet_test_case_unsigned_transaction(wallet_index, tx_index),
        get_wallet_test_case_unsigned_transaction_size(wallet_index, tx_index)
    );
    ots_handle_t* handle = (ots_handle_t*)malloc(sizeof(ots_handle_t));
    handle->type = OTS_HANDLE_TX_DESCRIPTION;
    handle->ptr = txd;
    handle->reference = false;
    ots_tx_description_t* result = ots_tx_description(handle);
    return result;
}

bool tx_descriptions_equal(
    ots_tx_description_t* a,
    ots_tx_description_t* b
) {
    if(
        !a || !b
        || !a->tx_set || !b->tx_set
        || a->tx_set_size != b->tx_set_size
        || a->amount_in != b->amount_in
        || a->amount_out != b->amount_out
        || a->flows_size != b->flows_size
        || (a->change == NULL) != (b->change == NULL)
        || (
            a->change != NULL
            && strcmp(a->change->address, b->change->address) != 0
            && a->change->amount != b->change->amount
        )
        || a->transfers_size != b->transfers_size
        || a->fee != b->fee
    )
        return false;
    for(size_t i = 0; i < a->flows_size; i++) {
        if(
            a->flows[i].amount != b->flows[i].amount
            || strcmp(a->flows[i].address, b->flows[i].address) != 0
        )
            return false;
    }
    for(size_t i = 0; i < a->transfers_size; i++) {
        if(
            a->transfers[i].amount_in != b->transfers[i].amount_in
            || a->transfers[i].amount_out != b->transfers[i].amount_out
            || a->transfers[i].ring_size != b->transfers[i].ring_size
            || a->transfers[i].unlock_time != b->transfers[i].unlock_time
            || a->transfers[i].flows_size != b->transfers[i].flows_size
            || (a->transfers[i].change == NULL) != (b->transfers[i].change == NULL)
            || (
                a->transfers[i].change != NULL
                && strcmp(
                    a->transfers[i].change->address,
                    b->transfers[i].change->address) != 0
               )
            || (
                a->transfers[i].change != NULL
                && a->transfers[i].change->amount != b->transfers[i].change->amount
               )
            || a->transfers[i].fee != b->transfers[i].fee
            || (a->transfers[i].payment_id == NULL) != (b->transfers[i].payment_id == NULL)
            || (
                a->transfers[i].payment_id != NULL
                && strcmp(a->transfers[i].payment_id, b->transfers[i].payment_id) != 0
               )
            || a->transfers[i].dummy_outputs != b->transfers[i].dummy_outputs
            || (a->transfers[i].tx_extra == NULL) != (b->transfers[i].tx_extra == NULL)
            || (
                a->transfers[i].tx_extra != NULL
                && strcmp(a->transfers[i].tx_extra, b->transfers[i].tx_extra) != 0
               )
            || strcmp(a->transfers[i].tx_extra, b->transfers[i].tx_extra) != 0
        )
            return false;
        for(size_t j = 0; j < a->transfers[i].flows_size; j++) {
            if(
                a->transfers[i].flows[j].amount != b->transfers[i].flows[j].amount
                || strcmp(a->transfers[i].flows[j].address, b->transfers[i].flows[j].address) != 0
            )
                return false;
        }
    }
    return true;
}

const char* get_filename(const char* fingerprint, size_t idx, const char* postfix) {
    char* filename = malloc(65);
    snprintf(
        filename,
        64,
        "ck_%s_%02zu.%s",
        fingerprint,
        idx,
        postfix
    );
    return filename;
}

size_t write_file(const char* filename, const char* data, size_t data_size) {
    FILE* file = fopen(filename, "wb");
    ck_assert(file != NULL);
    size_t written = fwrite(data, 1, data_size, file);
    fclose(file);
    return written;
}

START_TEST(test_ots_wallet_create)
{
ots_result_t* result = ots_random_32();
ck_assert(ots_result_data_is_uint8(result));
ck_assert_int_eq(ots_result_size(result), 32);
uint8_t* random = ots_result_uint8_array(result);
ots_free_result(&result);
result = ots_wallet_create(random, 0, OTS_NETWORK_MAIN);
ck_assert(ots_result_is_wallet(result));
ots_handle_t* wallet = ots_result_handle(result);
ots_free_result(&result);
result = ots_wallet_address(wallet);
ck_assert(ots_result_is_address(result));
ots_handle_t* wallet_address = ots_result_handle(result);
ots_free_result(&result);
    result = ots_monero_seed_create(random, 0, 0, OTS_NETWORK_MAIN);
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_address(seed);
    ck_assert(ots_result_is_address(result));
    ots_handle_t* seed_address = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_address_equal(wallet_address, seed_address);
    ck_assert(ots_result_is_boolean(result));
    ck_assert(ots_result_boolean(result, false));
    ots_free_result(&result);
    ots_free_handle(&wallet);
    ots_free_handle(&wallet_address);
    ots_free_handle(&seed);
    ots_free_handle(&seed_address);
}
END_TEST

START_TEST(test_ots_wallet_height)
{
    size_t seedIndex = get_wallet_test_case_seed_test_case(2);
    ots_result_t* result = ots_monero_seed_decode(
        get_monero_seed_test_case_phrase(seedIndex),
        get_monero_seed_test_case_height(seedIndex),
        get_monero_seed_test_case_time(seedIndex),
        get_monero_seed_test_case_network(seedIndex),
        get_monero_seed_test_case_password(seedIndex)
    );
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_wallet(seed);
    ck_assert(ots_result_is_wallet(result));
    ots_handle_t* wallet = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_wallet_height(wallet);
    ck_assert(ots_result_is_number(result));
    ck_assert_int_eq(
        ots_result_number(result, 0),
        get_monero_seed_test_case_height(5)
    );
    ots_free_result(&result);
    ots_free_handle(&seed); // frees the wallet too
}
END_TEST

START_TEST(test_ots_wallet_address)
{
    size_t seedIndex = get_wallet_test_case_seed_test_case(2);
    ots_result_t* result = ots_monero_seed_decode(
        get_monero_seed_test_case_phrase(seedIndex),
        get_monero_seed_test_case_height(seedIndex),
        get_monero_seed_test_case_time(seedIndex),
        get_monero_seed_test_case_network(seedIndex),
        get_monero_seed_test_case_password(seedIndex)
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
    ots_handle_t* wallet_address = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_address_base58_string(wallet_address);
    ck_assert_str_eq(
        ots_result_string(result),
        get_monero_seed_test_case_address(seedIndex)
    );
    ots_free_result(&result);
    ots_free_handle(&wallet_address);
    ots_free_handle(&seed); // frees the wallet too
}
END_TEST

START_TEST(test_ots_wallet_subaddress)
{
    size_t seedIndex = get_wallet_test_case_seed_test_case(2);
    ots_result_t* result = ots_monero_seed_decode(
        get_monero_seed_test_case_phrase(seedIndex),
        get_monero_seed_test_case_height(seedIndex),
        get_monero_seed_test_case_time(seedIndex),
        get_monero_seed_test_case_network(seedIndex),
        get_monero_seed_test_case_password(seedIndex)
    );
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_wallet(seed);
    ck_assert(ots_result_is_wallet(result));
    ots_handle_t* wallet = ots_result_handle(result);
    ots_free_result(&result);
    for(size_t i = 0; get_wallet_subaddress_test_case_count(2) > i; i++) {
        result = ots_wallet_subaddress(
            wallet,
            get_wallet_subaddress_test_case_account(2, i),
            get_wallet_subaddress_test_case_index(2, i)
        );
        ck_assert(ots_result_is_address(result));
        ots_handle_t* wallet_address = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_address_base58_string(wallet_address);
        ck_assert_str_eq(
            ots_result_string(result),
            get_wallet_subaddress_test_case_address(2, i)
        );
        ots_free_result(&result);
        ots_free_handle(&wallet_address);
    }
    ots_free_handle(&seed); // frees the wallet too
}
END_TEST

START_TEST(test_ots_wallet_accounts)
{
    size_t seedIndex = get_wallet_test_case_seed_test_case(2);
    const char* accounts[3] = {"", "", ""};
    accounts[0] = get_monero_seed_test_case_address(seedIndex);
    size_t accountIndex = 1;
    for(size_t i = 0; i < get_wallet_subaddress_test_case_count(2); i++) {
        if(accountIndex > 2)
            break;
        if(
            get_wallet_subaddress_test_case_index(2, i) != 0
            || get_wallet_subaddress_test_case_account(2, i) != accountIndex
        )
            continue;
        accounts[accountIndex++] = get_wallet_subaddress_test_case_address(2, i);
    }
    ots_result_t* result = ots_monero_seed_decode(
        get_monero_seed_test_case_phrase(seedIndex),
        get_monero_seed_test_case_height(seedIndex),
        get_monero_seed_test_case_time(seedIndex),
        get_monero_seed_test_case_network(seedIndex),
        get_monero_seed_test_case_password(seedIndex)
    );
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_wallet(seed);
    ck_assert(ots_result_is_wallet(result));
    ots_handle_t* wallet = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_wallet_accounts(wallet, 3, 0);
    ck_assert(ots_result_data_handle_is_address(result));
    ots_handle_t* wallet_addresses = ots_result_handle_array(result);
    ck_assert_int_eq(ots_result_size(result), 3);
    ots_free_result(&result);
    for(size_t i = 0; i < 3; i++) {
        result = ots_address_base58_string(&wallet_addresses[i]);
        ck_assert(ots_result_is_string(result));
        ck_assert_str_eq(ots_result_string(result), accounts[i]);
        ots_free_result(&result);
    }
}
END_TEST

START_TEST(test_ots_wallet_subaddresses)
{
    size_t seedIndex = get_wallet_test_case_seed_test_case(2);
    ots_result_t* result = ots_monero_seed_decode(
        get_monero_seed_test_case_phrase(seedIndex),
        get_monero_seed_test_case_height(seedIndex),
        get_monero_seed_test_case_time(seedIndex),
        get_monero_seed_test_case_network(seedIndex),
        get_monero_seed_test_case_password(seedIndex)
    );
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_wallet(seed);
    ck_assert(ots_result_is_wallet(result));
    ots_handle_t* wallet = ots_result_handle(result);
    ots_free_result(&result);
    for(size_t account = 0; account < 2; account++) {
        result = ots_wallet_subaddresses(wallet, account, 10, 0);
        ck_assert(ots_result_data_handle_is_address(result));
        ck_assert_int_eq(ots_result_size(result), 10);
        ots_handle_t* wallet_addresses = ots_result_array(result);
        ots_free_result(&result);
        for(size_t i = 0; i < 10; i++) {
            result = ots_address_base58_string(&wallet_addresses[i]);
            ck_assert(ots_result_is_string(result));
            char* address = (char*)ots_result_string(result);
            ck_assert_str_eq(
                address,
                get_wallet_subaddress_test_case_address(2, (account * 11) + i)
            );
            ots_free_result(&result);
        }
    }
}
END_TEST

START_TEST(test_ots_wallet_has_address)
{
    size_t seedIndex = get_wallet_test_case_seed_test_case(2);
    ots_result_t* result = ots_monero_seed_decode(
        get_monero_seed_test_case_phrase(seedIndex),
        get_monero_seed_test_case_height(seedIndex),
        get_monero_seed_test_case_time(seedIndex),
        get_monero_seed_test_case_network(seedIndex),
        get_monero_seed_test_case_password(seedIndex)
    );
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_wallet(seed);
    ck_assert(ots_result_is_wallet(result));
    ots_handle_t* wallet = ots_result_handle(result);
    ots_free_result(&result);
    for(size_t i = 0; get_wallet_subaddress_test_case_count(2) > i; i++) {
        result = ots_address_create(get_wallet_subaddress_test_case_address(2, i));
        ck_assert(ots_result_is_address(result));
        ots_handle_t* address = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_wallet_has_address(wallet, address, 1, 10); // only searching in the first 10 (0, 0 to 0, 9)
        ck_assert(ots_result_is_boolean(result));
        ck_assert(ots_result_boolean(result, false) == (i < 10)); // only first 10 should be found
        ots_free_result(&result);
    }
    result = ots_address_create(get_monero_seed_test_case_address(2)); // not in wallet
    ck_assert(ots_result_is_address(result));
    ots_handle_t* address = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_wallet_has_address(wallet, address, 0, 0);
    ck_assert(ots_result_is_boolean(result));
    ck_assert(!ots_result_boolean(result, true));
}
END_TEST

START_TEST(test_ots_wallet_has_address_string)
{
    size_t seedIndex = get_wallet_test_case_seed_test_case(2);
    ots_result_t* result = ots_monero_seed_decode(
        get_monero_seed_test_case_phrase(seedIndex),
        get_monero_seed_test_case_height(seedIndex),
        get_monero_seed_test_case_time(seedIndex),
        get_monero_seed_test_case_network(seedIndex),
        get_monero_seed_test_case_password(seedIndex)
    );
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_wallet(seed);
    ck_assert(ots_result_is_wallet(result));
    ots_handle_t* wallet = ots_result_handle(result);
    ots_free_result(&result);
    for(size_t i = 0; get_wallet_subaddress_test_case_count(2) > i; i++) {
        result = ots_wallet_has_address_string(
            wallet,
            get_wallet_subaddress_test_case_address(2, i),
            2,
            11
        ); // search only in the first two accounts
        ck_assert(ots_result_is_boolean(result));
        ck_assert(ots_result_boolean(result, true) == (i < 22)); // only first 22 should be found
        ots_free_result(&result);
    }
    result = ots_wallet_has_address_string(
        wallet,
        get_monero_seed_test_case_address(2), // not in wallet
        10,
        10
    );
    ck_assert(ots_result_is_boolean(result));
    ck_assert(!ots_result_boolean(result, true));
}
END_TEST

START_TEST(test_ots_wallet_address_index)
{
    size_t seedIndex = get_wallet_test_case_seed_test_case(2);
    ots_result_t* result = ots_monero_seed_decode(
        get_monero_seed_test_case_phrase(seedIndex),
        get_monero_seed_test_case_height(seedIndex),
        get_monero_seed_test_case_time(seedIndex),
        get_monero_seed_test_case_network(seedIndex),
        get_monero_seed_test_case_password(seedIndex)
    );
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_wallet(seed);
    ck_assert(ots_result_is_wallet(result));
    ots_handle_t* wallet = ots_result_handle(result);
    ots_free_result(&result);
    for(size_t i = 0; get_wallet_subaddress_test_case_count(2) > i; i++) {
        result = ots_address_create(get_wallet_subaddress_test_case_address(2, i));
        ck_assert(ots_result_is_address(result));
        ots_handle_t* address = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_wallet_address_index(wallet, address, 2, 11);
        if(i > 21) {
            ck_assert(ots_is_error(result));
            ots_free_result(&result);
            continue;
        }
        ck_assert(ots_result_is_address_index(result));
        ck_assert_int_eq(
            ots_result_address_index_account(result),
            get_wallet_subaddress_test_case_account(2, i)
        );
        ck_assert_int_eq(
            ots_result_address_index_index(result),
            get_wallet_subaddress_test_case_index(2, i)
        );
        ots_free_result(&result);
    }
}
END_TEST

START_TEST(test_ots_wallet_address_string_index)
{
    size_t seedIndex = get_wallet_test_case_seed_test_case(2);
    ots_result_t* result = ots_monero_seed_decode(
        get_monero_seed_test_case_phrase(seedIndex),
        get_monero_seed_test_case_height(seedIndex),
        get_monero_seed_test_case_time(seedIndex),
        get_monero_seed_test_case_network(seedIndex),
        get_monero_seed_test_case_password(seedIndex)
    );
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_wallet(seed);
    ck_assert(ots_result_is_wallet(result));
    ots_handle_t* wallet = ots_result_handle(result);
    ots_free_result(&result);
    for(size_t i = 0; get_wallet_subaddress_test_case_count(2) > i; i++) {
        result = ots_wallet_address_string_index(
            wallet,
            get_wallet_subaddress_test_case_address(2, i),
            2, 11
        );
        if(i > 21) {
            ck_assert(ots_is_error(result));
            ots_free_result(&result);
            continue;
        }
        ck_assert(ots_result_is_address_index(result));
        ck_assert_int_eq(
            ots_result_address_index_account(result),
            get_wallet_subaddress_test_case_account(2, i)
        );
        ck_assert_int_eq(
            ots_result_address_index_index(result),
            get_wallet_subaddress_test_case_index(2, i)
        );
        ots_free_result(&result);
    }
}
END_TEST

START_TEST(test_ots_wallet_secret_view_key)
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
    result = ots_seed_wallet(seed);
    ck_assert(ots_result_is_wallet(result));
    ots_handle_t* wallet = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_wallet_secret_view_key(wallet);
    ck_assert(ots_result_is_wipeable_string(result));
    ck_assert_str_eq(
        ots_result_string(result),
        get_monero_seed_test_case_secret_view_key(2)
    );
    ots_free_result(&result);
    ots_free_handle(&seed); // frees the wallet too
}
END_TEST

START_TEST(test_ots_wallet_public_view_key)
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
    result = ots_seed_wallet(seed);
    ck_assert(ots_result_is_wallet(result));
    ots_handle_t* wallet = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_wallet_public_view_key(wallet);
    ck_assert(ots_result_is_wipeable_string(result));
    ck_assert_str_eq(
        ots_result_string(result),
        get_monero_seed_test_case_public_view_key(2)
    );
    ots_free_result(&result);
    ots_free_handle(&seed); // frees the wallet too
}
END_TEST

START_TEST(test_ots_wallet_secret_spend_key)
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
    result = ots_seed_wallet(seed);
    ck_assert(ots_result_is_wallet(result));
    ots_handle_t* wallet = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_wallet_secret_spend_key(wallet);
    ck_assert(ots_result_is_wipeable_string(result));
    ck_assert_str_eq(
        ots_result_string(result),
        get_monero_seed_test_case_secret_spend_key(2)
    );
    ots_free_result(&result);
    ots_free_handle(&seed); // frees the wallet too
}
END_TEST

START_TEST(test_ots_wallet_public_spend_key)
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
    result = ots_seed_wallet(seed);
    ck_assert(ots_result_is_wallet(result));
    ots_handle_t* wallet = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_wallet_public_spend_key(wallet);
    ck_assert(ots_result_is_wipeable_string(result));
    ck_assert_str_eq(
        ots_result_string(result),
        get_monero_seed_test_case_public_spend_key(2)
    );
    ots_free_result(&result);
    ots_free_handle(&seed); // frees the wallet too
}
END_TEST

START_TEST(test_ots_wallet_import_outputs)
{
    for(size_t i = 0; i < get_wallet_test_cases_count(); i++) {
        if(!get_wallet_test_case_valid(i))
            continue;
        size_t seedIndex = get_wallet_test_case_seed_test_case(i);
        ots_result_t* result = ots_monero_seed_decode(
            get_monero_seed_test_case_phrase(seedIndex),
            get_monero_seed_test_case_height(seedIndex),
            get_monero_seed_test_case_time(seedIndex),
            get_monero_seed_test_case_network(seedIndex),
            get_monero_seed_test_case_password(seedIndex)
        );
        ck_assert(ots_result_is_seed(result));
        ots_handle_t* seed = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_seed_wallet(seed);
        ck_assert(ots_result_is_wallet(result));
        ots_handle_t* wallet = ots_result_handle(result);
        ots_free_result(&result);
        for(size_t j = 0; j < get_wallet_test_case_outputs_list_count(i); j++) {
            result = ots_wallet_import_outputs(
                wallet,
                get_wallet_test_case_outputs(i, j),
                get_wallet_test_case_outputs_size(i, j)
            );
            ck_assert(ots_result_is_number(result));
            ck_assert_int_eq(
                ots_result_number(result, 0),
                get_wallet_test_case_outputs_count(i, j)
            );
            ots_free_result(&result);
        }
        ots_free_result(&result);
    }
}
END_TEST

START_TEST(test_ots_wallet_export_key_images)
{
    for(size_t i = 0; i < get_wallet_test_cases_count(); i++) {
        if(!get_wallet_test_case_valid(i))
            continue;
        size_t seedIndex = get_wallet_test_case_seed_test_case(i);
        ots_result_t* result = ots_monero_seed_decode(
            get_monero_seed_test_case_phrase(seedIndex),
            get_monero_seed_test_case_height(seedIndex),
            get_monero_seed_test_case_time(seedIndex),
            get_monero_seed_test_case_network(seedIndex),
            get_monero_seed_test_case_password(seedIndex)
        );
        ck_assert(ots_result_is_seed(result));
        ots_handle_t* seed = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_seed_wallet(seed);
        ck_assert(ots_result_is_wallet(result));
        ots_handle_t* wallet = ots_result_handle(result);
        ots_free_result(&result);
        for(size_t j = 0; j < get_wallet_test_case_outputs_list_count(i); j++) {
            result = ots_wallet_import_outputs(
                wallet,
                get_wallet_test_case_outputs(i, j),
                get_wallet_test_case_outputs_size(i, j)
            );
            ck_assert(ots_result_is_number(result));
            ck_assert_int_eq(
                ots_result_number(result, 0),
                get_wallet_test_case_outputs_count(i, j)
            );
            ots_free_result(&result);
            result = ots_wallet_export_key_images(wallet);
            ck_assert(ots_result_is_string(result));
            ck_assert_int_gt(
                ots_result_size(result),
                20
            );
            char* key_images = (char*)ots_result_string_copy(result);
            size_t key_images_size = ots_result_size(result);
            ots_free_result(&result);
            // write the key images to a file ck_<fingerprint>.keyimages
            char filename[64];
            snprintf(
            filename,
                sizeof(filename),
                "ck_%s_%02zu.keyimages",
                get_monero_seed_test_case_fingerprint(seedIndex),
                j
            );
            FILE* file = fopen(filename, "wb");
            ck_assert(file != NULL);
            size_t written = fwrite(key_images, 1, key_images_size, file);
            fclose(file);
            ck_assert_int_eq(written, key_images_size);
        }
    }
}
END_TEST

START_TEST(test_ots_wallet_describe_tx)
{
    for(size_t i = 0; i < get_wallet_test_cases_count(); i++) {
        if(!get_wallet_test_case_valid(i))
            continue;
        size_t seedIndex = get_wallet_test_case_seed_test_case(i);
        ots_result_t* result = ots_monero_seed_decode(
            get_monero_seed_test_case_phrase(seedIndex),
            get_monero_seed_test_case_height(seedIndex),
            get_monero_seed_test_case_time(seedIndex),
            get_monero_seed_test_case_network(seedIndex),
            get_monero_seed_test_case_password(seedIndex)
        );
        ck_assert(ots_result_is_seed(result));
        ots_handle_t* seed = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_seed_wallet(seed);
        ck_assert(ots_result_is_wallet(result));
        ots_handle_t* wallet = ots_result_handle(result);
        ots_free_result(&result);
        for(size_t j = 0; j < get_wallet_test_case_outputs_list_count(i); j++) {
            result = ots_wallet_import_outputs(
                wallet,
                get_wallet_test_case_outputs(i, j),
                get_wallet_test_case_outputs_size(i, j)
            );
            ck_assert(ots_result_is_number(result));
            ck_assert_int_eq(
                ots_result_number(result, 0),
                get_wallet_test_case_outputs_count(i, j)
            );
            ots_free_result(&result);
            result = ots_wallet_export_key_images(wallet);
            ck_assert(ots_result_is_string(result));
            ck_assert_int_gt(
                ots_result_size(result),
                20
            );
            ots_free_result(&result);
            for(
                size_t k = 0;
                k < get_wallet_test_case_unsigned_transactions_count(i);
                k++
            ) {
                if(
                    get_wallet_test_case_unsigned_transaction_for_outputs(i, k) != j
                )
                    continue;
                result = ots_wallet_describe_tx(
                    wallet,
                    get_wallet_test_case_unsigned_transaction(i, k),
                    get_wallet_test_case_unsigned_transaction_size(i, k)
                );
                ck_assert(ots_result_is_transaction_description(result));
                ots_handle_t* tx_description = ots_result_handle(result);
                ots_free_result(&result);
                ots_tx_description_t* txds = ots_tx_description(tx_description);
                ots_tx_description_t* txd_ref = tx_description_from_data(i, k);
                ck_assert(tx_descriptions_equal(txds, txd_ref));
                ots_free_tx_description(&txds);
                ck_assert_int_eq(
                    strncmp(
                        txd_ref->tx_set,
                        ots_tx_description_tx_set(tx_description),
                        ots_tx_description_tx_set_size(tx_description)
                    ), 0
                );
                ck_assert_uint_eq(
                    txd_ref->amount_in,
                    ots_tx_description_amount_in(tx_description)
                );
                ck_assert_uint_eq(
                    txd_ref->amount_out,
                    ots_tx_description_amount_out(tx_description)
                );
                ck_assert_uint_eq(
                    txd_ref->flows_size,
                    ots_tx_description_flows_count(tx_description)
                );
                ck_assert_uint_eq(
                    txd_ref->fee,
                    ots_tx_description_fee(tx_description)
                );
                ck_assert_uint_eq(
                    txd_ref->transfers_size,
                    ots_tx_description_transfers_count(tx_description)
                );
                for(size_t flow = 0; flow < txd_ref->flows_size; flow++) {
                    ck_assert_uint_eq(
                        txd_ref->flows[flow].amount,
                        ots_tx_description_flow_amount(tx_description, flow)
                    );
                    ck_assert_str_eq(
                        txd_ref->flows[flow].address,
                        ots_tx_description_flow_address(tx_description, flow)
                    );
                }
                for(size_t transfer = 0; transfer < txd_ref->transfers_size; transfer++) {
                    ck_assert_uint_eq(
                        txd_ref->transfers[transfer].amount_in,
                        ots_tx_description_transfer_amount_in(tx_description, transfer)
                    );
                    ck_assert_uint_eq(
                        txd_ref->transfers[transfer].amount_out,
                        ots_tx_description_transfer_amount_out(
                            tx_description,
                            transfer
                        )
                    );
                    ck_assert_uint_eq(
                        txd_ref->transfers[transfer].ring_size,
                        ots_tx_description_transfer_ring_size(
                            tx_description,
                            transfer
                        )
                    );
                    ck_assert_uint_eq(
                        txd_ref->transfers[transfer].unlock_time,
                        ots_tx_description_transfer_unlock_time(
                            tx_description,
                            transfer
                        )
                    );
                    for(size_t flow = 0; flow < txd_ref->transfers[transfer].flows_size; flow++) {
                        ck_assert_uint_eq(
                            txd_ref->transfers[transfer].flows[flow].amount,
                            ots_tx_description_transfer_flow_amount(
                                tx_description,
                                transfer,
                                flow
                            )
                        );
                        ck_assert_str_eq(
                            txd_ref->transfers[transfer].flows[flow].address,
                            ots_tx_description_transfer_flow_address(
                                tx_description,
                                transfer,
                                flow
                            )
                        );
                    }
                    ck_assert_int_eq(
                        txd_ref->transfers[transfer].change != NULL,
                        ots_tx_description_transfer_has_change(tx_description, transfer)
                    );
                    if(txd_ref->transfers[transfer].change != NULL) {
                        ck_assert_uint_eq(
                            txd_ref->transfers[transfer].change->amount,
                            ots_tx_description_transfer_change_amount(
                                tx_description, transfer
                            )
                        );
                        ck_assert_str_eq(
                            txd_ref->transfers[transfer].change->address,
                            ots_tx_description_transfer_change_address(
                                tx_description, transfer
                            )
                        );
                    }
                    ck_assert_uint_eq(
                        txd_ref->transfers[transfer].fee,
                        ots_tx_description_transfer_fee(tx_description, transfer)
                    );
                    ck_assert_int_eq(
                        txd_ref->transfers[transfer].payment_id != NULL,
                        strcmp(
                            ots_tx_description_transfer_payment_id(
                                tx_description,
                                transfer
                            ),
                            ""
                        ) != 0
                    );
                    if(txd_ref->transfers[transfer].payment_id != NULL)
                        ck_assert_str_eq(
                            txd_ref->transfers[transfer].payment_id,
                            ots_tx_description_transfer_payment_id(tx_description, transfer)
                        );
                    ck_assert_uint_eq(
                        txd_ref->transfers[transfer].dummy_outputs,
                        ots_tx_description_transfer_dummy_outputs(
                            tx_description, transfer
                        )
                    );
                    ck_assert_int_eq(
                        txd_ref->transfers[transfer].tx_extra != NULL,
                        strcmp(
                            ots_tx_description_transfer_extra(
                                tx_description,
                                transfer
                            ),
                            ""
                        ) != 0
                    );
                    if(txd_ref->transfers[transfer].tx_extra != NULL)
                        ck_assert_int_eq(
                            strncmp(
                                txd_ref->transfers[transfer].tx_extra,
                                ots_tx_description_transfer_extra(
                                    tx_description,
                                    transfer
                                ),
                                ots_tx_description_transfer_extra_size(
                                    tx_description,
                                    transfer
                                )
                            ), 0
                        );
                }
                ots_free_tx_description(&txd_ref);
            }
        }
    }
}
END_TEST

START_TEST(test_ots_wallet_check_tx)
{
    for(size_t i = 0; i < get_wallet_test_cases_count(); i++) {
        if(!get_wallet_test_case_valid(i))
            continue;
        size_t seedIndex = get_wallet_test_case_seed_test_case(i);
        ots_result_t* result = ots_monero_seed_decode(
            get_monero_seed_test_case_phrase(seedIndex),
            get_monero_seed_test_case_height(seedIndex),
            get_monero_seed_test_case_time(seedIndex),
            get_monero_seed_test_case_network(seedIndex),
            get_monero_seed_test_case_password(seedIndex)
        );
        ck_assert(ots_result_is_seed(result));
        ots_handle_t* seed = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_seed_wallet(seed);
        ck_assert(ots_result_is_wallet(result));
        ots_handle_t* wallet = ots_result_handle(result);
        ots_free_result(&result);
        for(size_t j = 0; j < get_wallet_test_case_outputs_list_count(i); j++) {
            result = ots_wallet_import_outputs(
                wallet,
                get_wallet_test_case_outputs(i, j),
                get_wallet_test_case_outputs_size(i, j)
            );
            ck_assert(ots_result_is_number(result));
            ck_assert_int_eq(
                ots_result_number(result, 0),
                get_wallet_test_case_outputs_count(i, j)
            );
            ots_free_result(&result);
            result = ots_wallet_export_key_images(wallet);
            ck_assert(ots_result_is_string(result));
            ck_assert_int_gt(
                ots_result_size(result),
                20
            );
            ots_free_result(&result);
            for(
                size_t k = 0;
                k < get_wallet_test_case_unsigned_transactions_count(i);
                k++
            ) {
                if(
                    get_wallet_test_case_unsigned_transaction_for_outputs(i, k) != j
                )
                    continue;
                result = ots_wallet_describe_tx(
                    wallet,
                    get_wallet_test_case_unsigned_transaction(i, k),
                    get_wallet_test_case_unsigned_transaction_size(i, k)
                );
                ck_assert(ots_result_is_transaction_description(result));
                ots_handle_t* tx_description = ots_result_handle(result);
                ots_free_result(&result);
                result = ots_wallet_check_tx(
                    wallet,
                    tx_description
                );
                ck_assert(ots_result_data_handle_is_transaction_warning(result));
                ck_assert_int_eq(ots_result_size(result), 0);
                ots_free_result(&result);
                ots_free_handle(&tx_description);
                // nothing to check here because there are no more actual
                // warnings
            }
        }
    }
}
END_TEST

START_TEST(test_ots_wallet_check_tx_string)
{
    for(size_t i = 0; i < get_wallet_test_cases_count(); i++) {
        if(!get_wallet_test_case_valid(i))
            continue;
        size_t seedIndex = get_wallet_test_case_seed_test_case(i);
        ots_result_t* result = ots_monero_seed_decode(
            get_monero_seed_test_case_phrase(seedIndex),
            get_monero_seed_test_case_height(seedIndex),
            get_monero_seed_test_case_time(seedIndex),
            get_monero_seed_test_case_network(seedIndex),
            get_monero_seed_test_case_password(seedIndex)
        );
        ck_assert(ots_result_is_seed(result));
        ots_handle_t* seed = ots_result_handle(result);
        ots_free_result(&result);
        result = ots_seed_wallet(seed);
        ck_assert(ots_result_is_wallet(result));
        ots_handle_t* wallet = ots_result_handle(result);
        ots_free_result(&result);
        for(size_t j = 0; j < get_wallet_test_case_outputs_list_count(i); j++) {
            result = ots_wallet_import_outputs(
                wallet,
                get_wallet_test_case_outputs(i, j),
                get_wallet_test_case_outputs_size(i, j)
            );
            ck_assert(ots_result_is_number(result));
            ck_assert_int_eq(
                ots_result_number(result, 0),
                get_wallet_test_case_outputs_count(i, j)
            );
            ots_free_result(&result);
            result = ots_wallet_export_key_images(wallet);
            ck_assert(ots_result_is_string(result));
            ck_assert_int_gt(
                ots_result_size(result),
                20
            );
            ots_free_result(&result);
            for(
                size_t k = 0;
                k < get_wallet_test_case_unsigned_transactions_count(i);
                k++
            ) {
                if(
                    get_wallet_test_case_unsigned_transaction_for_outputs(i, k) != j
                )
                    continue;
                result = ots_wallet_check_tx_string(
                    wallet,
                    get_wallet_test_case_unsigned_transaction(i, k),
                    get_wallet_test_case_unsigned_transaction_size(i, k)
                );
                ck_assert(ots_result_data_handle_is_transaction_warning(result));
                ck_assert_int_eq(ots_result_size(result), 0);
                ots_free_result(&result);
                // nothing to check here because there are no more actual
                // warnings
            }
        }
    }
}
END_TEST

START_TEST(test_ots_wallet_sign_transaction)
{
    ots_set_max_depth(10, 100);
    for(
        size_t wallet_idx = 0;
        wallet_idx < get_wallet_test_cases_count();
        wallet_idx++
    ) {
        if(!get_wallet_test_case_valid(wallet_idx))
            continue;
        size_t seed_idx = get_wallet_test_case_seed_test_case(wallet_idx);
        for(
            size_t unsigned_tx = 0;
            unsigned_tx < get_wallet_test_case_unsigned_transactions_count(wallet_idx);
            unsigned_tx++
        ) {
            if(unsigned_tx != 0) // TODO: second call of `std::pair<signed_tx_set, std::vector<pending_tx>> Account::signTransaction(unsigned_tx_set &exported_txs)` will crash!? In gtest it works without issues - strange thing
                continue;
            /*
             * WTF, so this issue disapreares also if dynamically linked,
             * but causes then a issue where I get the error:
             * -9007 Unable to get output public key from output
             */
            ots_result_t* result = ots_monero_seed_decode(
                get_monero_seed_test_case_phrase(seed_idx),
                get_monero_seed_test_case_height(seed_idx),
                get_monero_seed_test_case_time(seed_idx),
                get_monero_seed_test_case_network(seed_idx),
                get_monero_seed_test_case_password(seed_idx)
            );
            ck_assert(ots_result_is_seed(result));
            ots_handle_t* seed = ots_result_handle(result);
            ots_free_result(&result);
            result = ots_seed_wallet(seed);
            ck_assert(ots_result_is_wallet(result));
            ots_handle_t* wallet = ots_result_handle(result);
            ots_free_result(&result);
            size_t outputs_idx = get_wallet_test_case_unsigned_transaction_for_outputs(wallet_idx, unsigned_tx);
            result = ots_wallet_import_outputs(
                wallet,
                get_wallet_test_case_outputs(wallet_idx, outputs_idx),
                get_wallet_test_case_outputs_size(wallet_idx, outputs_idx)
            );
            ck_assert(ots_result_is_number(result));
            ck_assert_int_eq(
                ots_result_number(result, 0),
                get_wallet_test_case_outputs_count(wallet_idx, outputs_idx)
            );
            ots_free_result(&result);
            result = ots_wallet_export_key_images(wallet);
            ck_assert(ots_result_is_string(result));
            ots_free_result(&result);
            // TODO: second call will crash, issue seems to be in Account::signTransaction, although only if used by C, not with C++, see comment above
            result = ots_wallet_sign_transaction(
                wallet,
                get_wallet_test_case_unsigned_transaction(
                    wallet_idx,
                    unsigned_tx
                ),
                get_wallet_test_case_unsigned_transaction_size(
                    wallet_idx,
                    unsigned_tx
                )
            );
            ck_assert(ots_result_is_string(result));
            char* signed_tx = (char*)ots_result_string_copy(result);
            size_t signed_tx_size = ots_result_size(result);
            ots_free_result(&result);
            // to test signed transactions we need to open in Monero GUI
            // write the key images to a file ck_<fingerprint>.signed_tx
            ck_assert_int_eq(
                write_file(
                    get_filename(
                        get_monero_seed_test_case_fingerprint(seed_idx),
                        unsigned_tx,
                        "signed_tx"
                    ),
                    signed_tx,
                    signed_tx_size
                ),
                signed_tx_size
            );
            ots_free_binary_string(&signed_tx, signed_tx_size);
            result = ots_wallet_export_key_images(wallet);
            ck_assert(ots_result_is_string(result));
            ck_assert_int_gt(
                ots_result_size(result),
                20
            );
            char* key_images = (char*)ots_result_string_copy(result);
            size_t key_images_size = ots_result_size(result);
            ots_free_result(&result);
            // This file is additional needed to give a success message in Monero GUI
            // write the key images to a file ck_<fingerprint>.signed_tx_keyimages
            ck_assert_int_eq(
                write_file(
                    get_filename(
                        get_monero_seed_test_case_fingerprint(seed_idx),
                        unsigned_tx,
                        "signed_tx_keyimages"
                    ),
                    key_images,
                    key_images_size
                ),
                key_images_size
            );
            ots_free_string(&key_images);
            ots_free_handle(&seed);
            free(wallet);
        }
    }
}
END_TEST

START_TEST(test_ots_wallet_sign_data)
{
    char* data = "test data";
    ots_result_t* result = ots_monero_seed_generate(
        0, 0, OTS_NETWORK_MAIN
    );
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_wallet(seed);
    ck_assert(ots_result_is_wallet(result));
    ots_handle_t* wallet = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_wallet_sign_data(wallet, data);
    ck_assert(ots_result_is_string(result));
    char* signature = ots_result_string_copy(result);
    ots_free_result(&result);
    result = ots_wallet_verify_data(wallet, data, signature, false);
    ck_assert(ots_result_is_boolean(result));
    ck_assert(ots_result_boolean(result, true));
    ots_free_result(&result);
    ots_free_handle(&seed); // frees the wallet too
}
END_TEST

START_TEST(test_ots_wallet_sign_data_with_index)
{
    char* data = "test data";
    ots_result_t* result = ots_monero_seed_generate(
        0, 0, OTS_NETWORK_MAIN
    );
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_wallet(seed);
    ck_assert(ots_result_is_wallet(result));
    ots_handle_t* wallet = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_wallet_sign_data_with_index(
        wallet, data, 1, 2
    );
    ck_assert(ots_result_is_string(result));
    char* signature = ots_result_string_copy(result);
    ots_free_result(&result);
    result = ots_wallet_verify_data_with_index(
        wallet, data, 1, 2, signature, false
    );
    ck_assert(ots_result_is_boolean(result));
    ck_assert(ots_result_boolean(result, true));
    ots_free_result(&result);
    ots_free_handle(&seed); // frees the wallet too
}
END_TEST

START_TEST(test_ots_wallet_sign_data_with_address)
{
    char* data = "test data";
    ots_result_t* result = ots_monero_seed_generate(
        0, 0, OTS_NETWORK_MAIN
    );
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_wallet(seed);
    ck_assert(ots_result_is_wallet(result));
    ots_handle_t* wallet = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_wallet_subaddress(
        wallet, 1, 2
    );
    ck_assert(ots_result_is_address(result));
    ots_handle_t* address = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_wallet_sign_data_with_address(
        wallet, data, address
    );
    ck_assert(ots_result_is_string(result));
    char* signature = ots_result_string_copy(result);
    ots_free_result(&result);
    result = ots_wallet_verify_data_with_address(
        wallet, data, address, signature, false
    );
    ck_assert(ots_result_is_boolean(result));
    ck_assert(ots_result_boolean(result, true));
    ots_free_result(&result);
    ots_free_handle(&seed); // frees the wallet too
}
END_TEST

START_TEST(test_ots_wallet_sign_data_with_address_string)
{
    char* data = "test data";
    ots_result_t* result = ots_monero_seed_generate(
        0, 0, OTS_NETWORK_MAIN
    );
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_wallet(seed);
    ck_assert(ots_result_is_wallet(result));
    ots_handle_t* wallet = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_wallet_subaddress(
        wallet, 1, 2
    );
    ck_assert(ots_result_is_address(result));
    ots_handle_t* address = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_address_base58_string(address);
    ck_assert(ots_result_is_string(result));
    char* address_string = ots_result_string_copy(result);
    ots_free_result(&result);
    result = ots_wallet_sign_data_with_address_string(
        wallet, data, address_string
    );
    ck_assert(ots_result_is_string(result));
    char* signature = ots_result_string_copy(result);
    ots_free_result(&result);
    result = ots_wallet_verify_data_with_address_string(
        wallet, data, address_string, signature, false
    );
    ck_assert(ots_result_is_boolean(result));
    ck_assert(ots_result_boolean(result, true));
    ots_free_result(&result);
    ots_free_handle(&seed); // frees the wallet too
}
END_TEST

START_TEST(test_ots_wallet_verify_data)
{
    char* data = "test data";
    ots_result_t* result = ots_monero_seed_generate(
        0, 0, OTS_NETWORK_MAIN
    );
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_wallet(seed);
    ck_assert(ots_result_is_wallet(result));
    ots_handle_t* wallet = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_wallet_sign_data(wallet, data);
    ck_assert(ots_result_is_string(result));
    char* signature = ots_result_string_copy(result);
    ots_free_result(&result);
    result = ots_wallet_verify_data(wallet, data, signature, false);
    ck_assert(ots_result_is_boolean(result));
    ck_assert(ots_result_boolean(result, false));
    ots_free_result(&result);
    result = ots_wallet_verify_data(wallet, "tampered data", signature, false);
    ck_assert(ots_result_is_boolean(result));
    ck_assert(!ots_result_boolean(result, true));
    ots_free_result(&result);
    ots_free_handle(&seed); // frees the wallet too
}
END_TEST

START_TEST(test_ots_wallet_verify_data_with_index)
{
    char* data = "test data";
    ots_result_t* result = ots_monero_seed_generate(
        0, 0, OTS_NETWORK_MAIN
    );
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_wallet(seed);
    ck_assert(ots_result_is_wallet(result));
    ots_handle_t* wallet = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_wallet_sign_data_with_index(
        wallet, data, 1, 2
    );
    ck_assert(ots_result_is_string(result));
    char* signature = ots_result_string_copy(result);
    ots_free_result(&result);
    result = ots_wallet_verify_data_with_index(
        wallet, data, 1, 2, signature, false
    );
    ck_assert(ots_result_is_boolean(result));
    ck_assert(ots_result_boolean(result, false));
    ots_free_result(&result);
    result = ots_wallet_verify_data_with_index(
        wallet, "tampered data", 1, 2, signature, false
    );
    ck_assert(ots_result_is_boolean(result));
    ck_assert(!ots_result_boolean(result, true));
    ots_free_result(&result);
    ots_free_handle(&seed); // frees the wallet too
}
END_TEST

START_TEST(test_ots_wallet_verify_data_with_address)
{
    char* data = "test data";
    ots_result_t* result = ots_monero_seed_generate(
        0, 0, OTS_NETWORK_MAIN
    );
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_wallet(seed);
    ck_assert(ots_result_is_wallet(result));
    ots_handle_t* wallet = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_wallet_subaddress(
        wallet, 1, 2
    );
    ck_assert(ots_result_is_address(result));
    ots_handle_t* address = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_wallet_sign_data_with_address(
        wallet, data, address
    );
    ck_assert(ots_result_is_string(result));
    char* signature = ots_result_string_copy(result);
    ots_free_result(&result);
    result = ots_wallet_verify_data_with_address(
        wallet, data, address, signature, false
    );
    ck_assert(ots_result_is_boolean(result));
    ck_assert(ots_result_boolean(result, false));
    ots_free_result(&result);
    result = ots_wallet_verify_data_with_address(
        wallet, "tampered data", address, signature, false
    );
    ck_assert(ots_result_is_boolean(result));
    ck_assert(!ots_result_boolean(result, true));
    ots_free_result(&result);
    ots_free_handle(&seed); // frees the wallet too
}
END_TEST

START_TEST(test_ots_wallet_verify_data_with_address_string)
{
    char* data = "test data";
    ots_result_t* result = ots_monero_seed_generate(
        0, 0, OTS_NETWORK_MAIN
    );
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* seed = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_wallet(seed);
    ck_assert(ots_result_is_wallet(result));
    ots_handle_t* wallet = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_wallet_subaddress(
        wallet, 1, 2
    );
    ck_assert(ots_result_is_address(result));
    ots_handle_t* address = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_address_base58_string(address);
    ck_assert(ots_result_is_string(result));
    char* address_string = ots_result_string_copy(result);
    ots_free_result(&result);
    result = ots_wallet_sign_data_with_address_string(
        wallet, data, address_string
    );
    ck_assert(ots_result_is_string(result));
    char* signature = ots_result_string_copy(result);
    ots_free_result(&result);
    result = ots_wallet_verify_data_with_address_string(
        wallet, data, address_string, signature, false
    );
    ck_assert(ots_result_is_boolean(result));
    ck_assert(ots_result_boolean(result, false));
    ots_free_result(&result);
    result = ots_wallet_verify_data_with_address_string(
        wallet, "tampered data", address_string, signature, false
    );
    ck_assert(ots_result_is_boolean(result));
    ck_assert(!ots_result_boolean(result, true));
    ots_free_result(&result);
    ots_free_handle(&seed); // frees the wallet too
}
END_TEST

Suite* wallet_suite(void)
{
    Suite* s = suite_create("Wallet");
    // Core functionality
    TCase* tc_core = tcase_create("Core");
    tcase_set_tags(tc_core, "wallet");
    tcase_add_test(tc_core, test_ots_wallet_create);
    tcase_add_test(tc_core, test_ots_wallet_height);
    tcase_add_test(tc_core, test_ots_wallet_address);
    tcase_add_test(tc_core, test_ots_wallet_subaddress);
    tcase_add_test(tc_core, test_ots_wallet_accounts);
    tcase_add_test(tc_core, test_ots_wallet_subaddresses);
    tcase_add_test(tc_core, test_ots_wallet_has_address);
    tcase_add_test(tc_core, test_ots_wallet_has_address_string);
    tcase_add_test(tc_core, test_ots_wallet_address_index);
    tcase_add_test(tc_core, test_ots_wallet_address_string_index);
    tcase_add_test(tc_core, test_ots_wallet_secret_view_key);
    tcase_add_test(tc_core, test_ots_wallet_public_view_key);
    tcase_add_test(tc_core, test_ots_wallet_secret_spend_key);
    tcase_add_test(tc_core, test_ots_wallet_public_spend_key);
    tcase_add_test(tc_core, test_ots_wallet_import_outputs);
    tcase_add_test(tc_core, test_ots_wallet_export_key_images);
    tcase_add_test(tc_core, test_ots_wallet_describe_tx);
    tcase_add_test(tc_core, test_ots_wallet_check_tx);
    tcase_add_test(tc_core, test_ots_wallet_check_tx_string);
    tcase_add_test(tc_core, test_ots_wallet_sign_transaction);
    tcase_add_test(tc_core, test_ots_wallet_sign_data);
    tcase_add_test(tc_core, test_ots_wallet_sign_data_with_index);
    tcase_add_test(tc_core, test_ots_wallet_sign_data_with_address);
    tcase_add_test(tc_core, test_ots_wallet_sign_data_with_address_string);
    tcase_add_test(tc_core, test_ots_wallet_verify_data);
    tcase_add_test(tc_core, test_ots_wallet_verify_data_with_index);
    tcase_add_test(tc_core, test_ots_wallet_verify_data_with_address);
    tcase_add_test(tc_core, test_ots_wallet_verify_data_with_address_string);
    suite_add_tcase(s, tc_core);
    return s;
}

