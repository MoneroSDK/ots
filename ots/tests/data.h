#pragma once

#include <stdint.h> // for uint64_t
#include <stddef.h>
#include <stdbool.h> // For C only, C++ handles bool natively
#include <ots.h>

#ifdef __cplusplus
extern "C" {
#endif

    size_t get_address_test_cases_count();
    const char* get_address_test_case_name(size_t index);
    const char* get_address_test_case_address(size_t index);
    const char* get_address_test_case_fingerprint(size_t index);
    const char* get_address_test_case_payment_id(size_t index);
    const char* get_address_test_case_base_address(size_t index);
    int get_address_test_case_network(size_t index);
    int get_address_test_case_type(size_t index);
    bool get_address_test_case_valid(size_t index);

    size_t get_seed_language_test_cases_count();
    const char* get_seed_language_test_case_name(size_t index);
    const char* get_seed_language_test_case_english_name(size_t index);
    const char* get_seed_language_test_case_code(size_t index);
    int get_seed_language_test_case_monero_index(size_t index);
    int get_seed_language_test_case_polyseed_index(size_t index);
    bool get_seed_language_test_case_monero_supported(size_t index);
    bool get_seed_language_test_case_polyseed_supported(size_t index);
    bool get_seed_language_test_case_valid(size_t index);

    const uint64_t get_highest_block();
    const uint64_t get_highest_timestamp();
    const uint64_t get_timestamp_by_height(const uint64_t height);
    const uint64_t get_height_by_timestamp(const uint64_t timestamp);

    size_t get_legacy_seed_test_cases_count();
    const char* get_legacy_seed_test_case_name(size_t index);
    const char* get_legacy_seed_test_case_phrase(size_t index);
    const uint64_t get_legacy_seed_test_case_height(size_t index);
    const uint64_t get_legacy_seed_test_case_time(size_t index);
    const uint64_t get_legacy_seed_test_case_expected_height(size_t index);
    const uint64_t get_legacy_seed_test_case_expected_time(size_t index);
    const int get_legacy_seed_test_case_network(size_t index);
    const char* get_legacy_seed_test_case_address(size_t index);
    const char* get_legacy_seed_test_case_fingerprint(size_t index);
    const bool get_legacy_seed_test_case_valid(size_t index);
    const char* get_legacy_seed_test_case_secret_spend_key(size_t index);
    const char* get_legacy_seed_test_case_public_spend_key(size_t index);
    const char* get_legacy_seed_test_case_secret_view_key(size_t index);
    const char* get_legacy_seed_test_case_public_view_key(size_t index);
    const char* get_legacy_seed_test_case_lang_code(size_t index);

    size_t get_monero_seed_test_cases_count();
    const char* get_monero_seed_test_case_name(size_t index);
    const char* get_monero_seed_test_case_phrase(size_t index);
    const uint64_t get_monero_seed_test_case_height(size_t index);
    const uint64_t get_monero_seed_test_case_time(size_t index);
    const uint64_t get_monero_seed_test_case_expected_height(size_t index);
    const uint64_t get_monero_seed_test_case_expected_time(size_t index);
    const int get_monero_seed_test_case_network(size_t index);
    const char* get_monero_seed_test_case_password(size_t index);
    const bool get_monero_seed_test_case_encrypted(size_t index);
    const char* get_monero_seed_test_case_address(size_t index);
    const char* get_monero_seed_test_case_fingerprint(size_t index);
    const bool get_monero_seed_test_case_valid(size_t index);
    const char* get_monero_seed_test_case_secret_spend_key(size_t index);
    const char* get_monero_seed_test_case_public_spend_key(size_t index);
    const char* get_monero_seed_test_case_secret_view_key(size_t index);
    const char* get_monero_seed_test_case_public_view_key(size_t index);
    const char* get_monero_seed_test_case_lang_code(size_t index);

    size_t get_polyseed_test_cases_count();
    const char* get_polyseed_test_case_name(size_t index);
    const char* get_polyseed_test_case_phrase(size_t index);
    const uint64_t get_polyseed_test_case_height(size_t index);
    const uint64_t get_polyseed_test_case_time(size_t index);
    const uint64_t get_polyseed_test_case_expected_height(size_t index);
    const uint64_t get_polyseed_test_case_expected_time(size_t index);
    const int get_polyseed_test_case_network(size_t index);
    const char* get_polyseed_test_case_password(size_t index);
    const bool get_polyseed_test_case_encrypted(size_t index);
    const char* get_polyseed_test_case_passphrase(size_t index);
    const bool get_polyseed_test_case_offset(size_t index);
    const char* get_polyseed_test_case_address(size_t index);
    const char* get_polyseed_test_case_fingerprint(size_t index);
    const bool get_polyseed_test_case_valid(size_t index);
    const char* get_polyseed_test_case_secret_spend_key(size_t index);
    const char* get_polyseed_test_case_public_spend_key(size_t index);
    const char* get_polyseed_test_case_secret_view_key(size_t index);
    const char* get_polyseed_test_case_public_view_key(size_t index);
    const char* get_polyseed_test_case_lang_code(size_t index);


    size_t get_wallet_test_cases_count();
    const char* get_wallet_test_case_name(size_t index);
    const int get_wallet_test_case_seed_type(size_t index);
    const char* get_wallet_test_case_seed_type_name(size_t index);
    const size_t get_wallet_test_case_seed_test_case(size_t index);
    const size_t get_wallet_test_case_outputs_list_count(size_t index);
    const char* get_wallet_test_case_outputs(
        size_t caseIndex,
        size_t outputIndex
    );
    const uint64_t get_wallet_test_case_outputs_count(
        size_t caseIndex,
        size_t outputIndex
    );
    const bool get_wallet_test_case_valid(size_t index);
    const size_t get_wallet_test_case_unsigned_transactions_count(size_t index);
    const char* get_wallet_test_case_unsigned_transaction(
        size_t index,
        size_t transactionIndex
    );
    const char* get_wallet_test_case_unsigned_transaction_json(
        size_t index,
        size_t transactionIndex
    );
    const size_t get_wallet_test_case_unsigned_transaction_for_outputs(
        size_t index,
        size_t transactionIndex
    );
    const uint64_t get_wallet_test_case_unsigned_transaction_amount(
        size_t index,
        size_t transactionIndex
    );
    const bool get_wallet_test_case_unsigned_transaction_has_change(
        size_t index,
        size_t transactionIndex
    );

    size_t get_wallet_verify_signed_message_test_cases_count();
    const char* get_wallet_verify_signed_message_test_case_name(size_t index);
    const char* get_wallet_verify_signed_message_test_case_message(size_t index);
    const char* get_wallet_verify_signed_message_test_case_signature(size_t index);
    const char* get_wallet_verify_signed_message_test_case_address(size_t index);
    const bool get_wallet_verify_signed_message_test_case_valid(size_t index);
    const bool get_wallet_verify_signed_message_test_case_throws(size_t index);
    const size_t get_wallet_verify_signed_message_test_case_version(size_t index);

    size_t get_wallet_subaddress_test_case_count(size_t walletIndex);
    const uint32_t get_wallet_subaddress_test_case_account(
        size_t walletIndex,
        size_t index
    );
    const uint32_t get_wallet_subaddress_test_case_index(
        size_t walletIndex,
        size_t index
    );
    const char* get_wallet_subaddress_test_case_address(
        size_t walletIndex,
        size_t index
    );

#ifdef __cplusplus
}
#endif
