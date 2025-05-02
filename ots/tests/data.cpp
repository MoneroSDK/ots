#include "data.h"
#include "data-address.hpp"
#include "data-seed-language.hpp"
#include "data-blocktime.hpp"
#include "data-seed-legacy.hpp"
#include "data-seed-monero.hpp"
#include "data-seed-polyseed.hpp"
#include "data-wallet.hpp"
#include "data-wallet-sign.hpp"
#include <cstring>

/**
 * @file data.cpp
 * @brief This file provides a C interface to the data defined in:
 *        - data-address.hpp
 *        - data-seed-language.hpp
 *        - data-seed-legacy.hpp
 *        - data-seed-monero.hpp
 *        - data-seed-polyseed.hpp
 *        - data-wallet.hpp
 *        - data-wallet-sign.hpp
 *        - data-blocktime.hpp
 *
 * @todo TODO: add more documentation
 */

extern "C" {

    /** address test cases */

    size_t get_address_test_cases_count() {
        return address_test_cases.size();
    }

    const char* get_address_test_case_name(size_t index) {
        if(index >= address_test_cases.size())
            return nullptr;
        return address_test_cases[index].name.c_str();
    }

    const char* get_address_test_case_address(size_t index) {
        if(index >= address_test_cases.size())
            return nullptr;
        return address_test_cases[index].address.c_str();
    }

    const char* get_address_test_case_fingerprint(size_t index) {
        if(index >= address_test_cases.size())
            return nullptr;
        return address_test_cases[index].fingerprint.c_str();
    }

    const char* get_address_test_case_payment_id(size_t index) {
        if(index >= address_test_cases.size())
            return nullptr;
        return address_test_cases[index].payment_id.c_str();
    }

    const char* get_address_test_case_base_address(size_t index) {
        if(index >= address_test_cases.size())
            return nullptr;
        return address_test_cases[index].base_address.c_str();
    }

    int get_address_test_case_network(size_t index) {
        if(index >= address_test_cases.size())
            return -1;
        return static_cast<int>(address_test_cases[index].network);
    }

    int get_address_test_case_type(size_t index) {
        if(index >= address_test_cases.size())
            return -1;
        return static_cast<int>(address_test_cases[index].type);
    }

    bool get_address_test_case_valid(size_t index) {
        if(index >= address_test_cases.size())
            return false;
        return address_test_cases[index].valid;
    }

    /** seed language test cases */

    size_t get_seed_language_test_cases_count() {
        return seed_language_test_cases.size();
    }

    const char* get_seed_language_test_case_name(size_t index) {
        if(index >= seed_language_test_cases.size())
            return nullptr;
        return seed_language_test_cases[index].name.c_str();
    }

    const char* get_seed_language_test_case_english_name(size_t index) {
        if(index >= seed_language_test_cases.size())
            return nullptr;
        return seed_language_test_cases[index].englishName.c_str();
    }

    const char* get_seed_language_test_case_code(size_t index) {
        if(index >= seed_language_test_cases.size())
            return nullptr;
        return seed_language_test_cases[index].code.c_str();
    }

    int get_seed_language_test_case_monero_index(size_t index) {
        if(index >= seed_language_test_cases.size())
            return -1;
        return seed_language_test_cases[index].index.monero;
    }

    int get_seed_language_test_case_polyseed_index(size_t index) {
        if(index >= seed_language_test_cases.size())
            return -1;
        return seed_language_test_cases[index].index.polyseed;
    }

    bool get_seed_language_test_case_monero_supported(size_t index) {
        if(index >= seed_language_test_cases.size())
            return false;
        return seed_language_test_cases[index].supported.monero;
    }

    bool get_seed_language_test_case_polyseed_supported(size_t index) {
        if(index >= seed_language_test_cases.size())
            return false;
        return seed_language_test_cases[index].supported.polyseed;
    }

    bool get_seed_language_test_case_valid(size_t index) {
        if(index >= seed_language_test_cases.size())
            return false;
        return seed_language_test_cases[index].valid;
    }

    /** block time test cases */

    const uint64_t get_highest_block() {
        return ots::blocktime::main::highestBlock();
    }

    const uint64_t get_highest_timestamp() {
        return ots::blocktime::main::highestTimestamp();
    }

    const uint64_t get_timestamp_by_height(const uint64_t height) {
        return ots::blocktime::main::timestampByHeight(height);
    }

    const uint64_t get_height_by_timestamp(const uint64_t timestamp) {
        return ots::blocktime::main::heightByTimestamp(timestamp);
    }

    /** lecacy seed test cases */

    size_t get_legacy_seed_test_cases_count() {
        return legacy_seed_test_cases.size();
    }

    const char* get_legacy_seed_test_case_name(size_t index) {
        if(index >= legacy_seed_test_cases.size())
            return nullptr;
        return legacy_seed_test_cases[index].name.c_str();
    }

    const char* get_legacy_seed_test_case_phrase(size_t index) {
        if(index >= legacy_seed_test_cases.size())
            return nullptr;
        return legacy_seed_test_cases[index].phrase.c_str();
    }

    const uint64_t get_legacy_seed_test_case_height(size_t index) {
        if(index >= legacy_seed_test_cases.size())
            return 0;
        return legacy_seed_test_cases[index].height;
    }

    const uint64_t get_legacy_seed_test_case_time(size_t index) {
        if(index >= legacy_seed_test_cases.size())
            return 0;
        return legacy_seed_test_cases[index].time;
    }

    const uint64_t get_legacy_seed_test_case_expected_height(size_t index) {
        if(index >= legacy_seed_test_cases.size())
            return 0;
        return legacy_seed_test_cases[index].expected_height;
    }

    const uint64_t get_legacy_seed_test_case_expected_time(size_t index) {
        if(index >= legacy_seed_test_cases.size())
            return 0;
        return legacy_seed_test_cases[index].expected_time;
    }

    const int get_legacy_seed_test_case_network(size_t index) {
        if(index >= legacy_seed_test_cases.size())
            return -1;
        return static_cast<int>(legacy_seed_test_cases[index].network);
    }

    const char* get_legacy_seed_test_case_address(size_t index) {
        if(index >= legacy_seed_test_cases.size())
            return nullptr;
        return legacy_seed_test_cases[index].address.c_str();
    }

    const char* get_legacy_seed_test_case_fingerprint(size_t index) {
        if(index >= legacy_seed_test_cases.size())
            return nullptr;
        return legacy_seed_test_cases[index].fingerprint.c_str();
    }

    const bool get_legacy_seed_test_case_valid(size_t index) {
        if(index >= legacy_seed_test_cases.size())
            return false;
        return legacy_seed_test_cases[index].valid;
    }

    const char* get_legacy_seed_test_case_secret_spend_key(size_t index) {
        if(index >= legacy_seed_test_cases.size())
            return nullptr;
        return legacy_seed_test_cases[index].secret_spend_key.c_str();
    }

    const char* get_legacy_seed_test_case_public_spend_key(size_t index) {
        if(index >= legacy_seed_test_cases.size())
            return nullptr;
        return legacy_seed_test_cases[index].public_spend_key.c_str();
    }

    const char* get_legacy_seed_test_case_secret_view_key(size_t index) {
        if(index >= legacy_seed_test_cases.size())
            return nullptr;
        return legacy_seed_test_cases[index].secret_view_key.c_str();
    }

    const char* get_legacy_seed_test_case_public_view_key(size_t index) {
        if(index >= legacy_seed_test_cases.size())
            return nullptr;
        return legacy_seed_test_cases[index].public_view_key.c_str();
    }

    const char* get_legacy_seed_test_case_lang_code(size_t index) {
        if(index >= legacy_seed_test_cases.size())
            return nullptr;
        return legacy_seed_test_cases[index].lang_code.c_str();
    }

    /** monero seed test cases */

    size_t get_monero_seed_test_cases_count() {
        return monero_seed_test_cases.size();
    }

    const char* get_monero_seed_test_case_name(size_t index) {
        if(index >= monero_seed_test_cases.size())
            return nullptr;
        return monero_seed_test_cases[index].name.c_str();
    }

    const char* get_monero_seed_test_case_phrase(size_t index) {
        if(index >= monero_seed_test_cases.size())
            return nullptr;
        return monero_seed_test_cases[index].phrase.c_str();
    }

    const uint64_t get_monero_seed_test_case_height(size_t index) {
        if(index >= monero_seed_test_cases.size())
            return 0;
        return monero_seed_test_cases[index].height;
    }

    const uint64_t get_monero_seed_test_case_time(size_t index) {
        if(index >= monero_seed_test_cases.size())
            return 0;
        return monero_seed_test_cases[index].time;
    }

    const uint64_t get_monero_seed_test_case_expected_height(size_t index) {
        if(index >= monero_seed_test_cases.size())
            return 0;
        return monero_seed_test_cases[index].expected_height;
    }

    const uint64_t get_monero_seed_test_case_expected_time(size_t index) {
        if(index >= monero_seed_test_cases.size())
            return 0;
        return monero_seed_test_cases[index].expected_time;
    }

    const int get_monero_seed_test_case_network(size_t index) {
        if(index >= monero_seed_test_cases.size())
            return -1;
        return static_cast<int>(monero_seed_test_cases[index].network);
    }

    const char* get_monero_seed_test_case_password(size_t index) {
        if(index >= monero_seed_test_cases.size())
            return nullptr;
        return monero_seed_test_cases[index].password.c_str();
    }

    const bool get_monero_seed_test_case_encrypted(size_t index) {
        if(index >= monero_seed_test_cases.size())
            return false;
        return monero_seed_test_cases[index].encrypted;
    }

    const char* get_monero_seed_test_case_address(size_t index) {
        if(index >= monero_seed_test_cases.size())
            return nullptr;
        return monero_seed_test_cases[index].address.c_str();
    }

    const char* get_monero_seed_test_case_fingerprint(size_t index) {
        if(index >= monero_seed_test_cases.size())
            return nullptr;
        return monero_seed_test_cases[index].fingerprint.c_str();
    }

    const bool get_monero_seed_test_case_valid(size_t index) {
        if(index >= monero_seed_test_cases.size())
            return false;
        return monero_seed_test_cases[index].valid;
    }

    const char* get_monero_seed_test_case_secret_spend_key(size_t index) {
        if(index >= monero_seed_test_cases.size())
            return nullptr;
        return monero_seed_test_cases[index].secret_spend_key.c_str();
    }

    const char* get_monero_seed_test_case_public_spend_key(size_t index) {
        if(index >= monero_seed_test_cases.size())
            return nullptr;
        return monero_seed_test_cases[index].public_spend_key.c_str();
    }

    const char* get_monero_seed_test_case_secret_view_key(size_t index) {
        if(index >= monero_seed_test_cases.size())
            return nullptr;
        return monero_seed_test_cases[index].secret_view_key.c_str();
    }

    const char* get_monero_seed_test_case_public_view_key(size_t index) {
        if(index >= monero_seed_test_cases.size())
            return nullptr;
        return monero_seed_test_cases[index].public_view_key.c_str();
    }

    const char* get_monero_seed_test_case_lang_code(size_t index) {
        if(index >= monero_seed_test_cases.size())
            return nullptr;
        return monero_seed_test_cases[index].lang_code.c_str();
    }

    /** polyseed test cases */
    size_t get_polyseed_test_cases_count() {
        return polyseed_test_cases.size();
    }

    const char* get_polyseed_test_case_name(size_t index) {
        if(index >= polyseed_test_cases.size())
            return nullptr;
        return polyseed_test_cases[index].name.c_str();
    }

    const char* get_polyseed_test_case_phrase(size_t index) {
        if(index >= polyseed_test_cases.size())
            return nullptr;
        return polyseed_test_cases[index].phrase.c_str();
    }

    const uint64_t get_polyseed_test_case_height(size_t index) {
        if(index >= polyseed_test_cases.size())
            return 0;
        return polyseed_test_cases[index].height;
    }

    const uint64_t get_polyseed_test_case_time(size_t index) {
        if(index >= polyseed_test_cases.size())
            return 0;
        return polyseed_test_cases[index].time;
    }

    const uint64_t get_polyseed_test_case_expected_height(size_t index) {
        if(index >= polyseed_test_cases.size())
            return 0;
        return polyseed_test_cases[index].expected_height;
    }

    const uint64_t get_polyseed_test_case_expected_time(size_t index) {
        if(index >= polyseed_test_cases.size())
            return 0;
        return polyseed_test_cases[index].expected_time;
    }

    const int get_polyseed_test_case_network(size_t index) {
        if(index >= polyseed_test_cases.size())
            return -1;
        return static_cast<int>(polyseed_test_cases[index].network);
    }

    const char* get_polyseed_test_case_password(size_t index) {
        if(index >= polyseed_test_cases.size())
            return nullptr;
        return polyseed_test_cases[index].password.c_str();
    }

    const bool get_polyseed_test_case_encrypted(size_t index) {
        if(index >= polyseed_test_cases.size())
            return false;
        return polyseed_test_cases[index].encrypted;
    }

    const char* get_polyseed_test_case_passphrase(size_t index) {
        if(index >= polyseed_test_cases.size())
            return nullptr;
        return polyseed_test_cases[index].passphrase.c_str();
    }

    const bool get_polyseed_test_case_offset(size_t index) {
        if(index >= polyseed_test_cases.size())
            return false;
        return polyseed_test_cases[index].offset;
    }

    const char* get_polyseed_test_case_address(size_t index) {
        if(index >= polyseed_test_cases.size())
            return nullptr;
        return polyseed_test_cases[index].address.c_str();
    }

    const char* get_polyseed_test_case_fingerprint(size_t index) {
        if(index >= polyseed_test_cases.size())
            return nullptr;
        return polyseed_test_cases[index].fingerprint.c_str();
    }

    const bool get_polyseed_test_case_valid(size_t index) {
        if(index >= polyseed_test_cases.size())
            return false;
        return polyseed_test_cases[index].valid;
    }

    const char* get_polyseed_test_case_secret_spend_key(size_t index) {
        if(index >= polyseed_test_cases.size())
            return nullptr;
        return polyseed_test_cases[index].secret_spend_key.c_str();
    }

    const char* get_polyseed_test_case_public_spend_key(size_t index) {
        if(index >= polyseed_test_cases.size())
            return nullptr;
        return polyseed_test_cases[index].public_spend_key.c_str();
    }

    const char* get_polyseed_test_case_secret_view_key(size_t index) {
        if(index >= polyseed_test_cases.size())
            return nullptr;
        return polyseed_test_cases[index].secret_view_key.c_str();
    }

    const char* get_polyseed_test_case_public_view_key(size_t index) {
        if(index >= polyseed_test_cases.size())
            return nullptr;
        return polyseed_test_cases[index].public_view_key.c_str();
    }

    const char* get_polyseed_test_case_lang_code(size_t index) {
        if(index >= polyseed_test_cases.size())
            return nullptr;
        return polyseed_test_cases[index].lang_code.c_str();
    }




    size_t get_wallet_test_cases_count() {
        return wallet_test_cases.size();
    }

    const char* get_wallet_test_case_name(size_t index) {
        if(index >= wallet_test_cases.size())
            return nullptr;
        return wallet_test_cases[index].name.c_str();
    }

    const int get_wallet_test_case_seed_type(size_t index) {
        if(index >= wallet_test_cases.size())
            return -1;
        return static_cast<int>(wallet_test_cases[index].seed_type);
    }

    const char* get_wallet_test_case_seed_type_name(size_t index) {
        switch(get_wallet_test_case_seed_type(index)) {
            case OTS_SEED_TYPE_MONERO:
                return "Monero";
            case OTS_SEED_TYPE_POLYSEED:
                return "Polyseed";
            default:
                return "Unknown";
        }
    }

    const size_t get_wallet_test_case_seed_test_case(size_t index) {
        if(index >= wallet_test_cases.size())
            return 0;
        return wallet_test_cases[index].seed_test_case;
    }

    const size_t get_wallet_test_case_outputs_list_count(size_t index) {
        return wallet_test_cases[index].outputs.size();
    }

    const char* get_wallet_test_case_outputs(size_t caseIndex, size_t outputIndex) {
        if(
            caseIndex >= wallet_test_cases.size()
            || outputIndex >= wallet_test_cases[caseIndex].outputs.size()
        )
            return nullptr;
        return wallet_test_cases[caseIndex].outputs[outputIndex].outputs.c_str();
    }

    const uint64_t get_wallet_test_case_outputs_count(size_t caseIndex, size_t outputIndex) {
        if(
            caseIndex >= wallet_test_cases.size()
            || outputIndex >= wallet_test_cases[caseIndex].outputs.size()
        )
            return 0;
        return wallet_test_cases[caseIndex].outputs[outputIndex].count;
    }

    const bool get_wallet_test_case_valid(size_t index) {
        if(index >= wallet_test_cases.size())
            return false;
        return wallet_test_cases[index].valid;
    }

    const size_t get_wallet_test_case_unsigned_transactions_count(size_t index) {
        if(index >= wallet_test_cases.size())
            return 0;
        return wallet_test_cases[index].unsigned_transactions.size();
    }

    const char* get_wallet_test_case_unsigned_transaction(
        size_t index,
        size_t transactionIndex
    ) {
        if(index >= wallet_test_cases.size())
            return nullptr;
        return wallet_test_cases[index].unsigned_transactions[transactionIndex].tx.c_str();
    }

    const char* get_wallet_test_case_unsigned_transaction_json(
        size_t index,
        size_t transactionIndex
    ) {
        if(index >= wallet_test_cases.size())
            return nullptr;
        return wallet_test_cases[index].unsigned_transactions[transactionIndex].json.c_str();
    }

    const size_t get_wallet_test_case_unsigned_transaction_for_outputs(
        size_t index,
        size_t transactionIndex
    ) {
        if(index >= wallet_test_cases.size())
            return 0;
        return wallet_test_cases[index].unsigned_transactions[transactionIndex].forOutputs;
    }

    const uint64_t get_wallet_test_case_unsigned_transaction_amount(
        size_t index,
        size_t transactionIndex
    ) {
        if(index >= wallet_test_cases.size())
            return 0;
        return wallet_test_cases[index].unsigned_transactions[transactionIndex].amount;
    }

    const bool get_wallet_test_case_unsigned_transaction_has_change(
        size_t index,
        size_t transactionIndex
    ) {
        if(index >= wallet_test_cases.size())
            return false;
        return wallet_test_cases[index].unsigned_transactions[transactionIndex].hasChange;
    }

    /** wallet verify signed message test cases */

    size_t get_wallet_verify_signed_message_test_cases_count() {
        return wallet_verify_signed_message_test_cases.size();
    }

    const char* get_wallet_verify_signed_message_test_case_name(size_t index) {
        if(index >= wallet_verify_signed_message_test_cases.size())
            return nullptr;
        return wallet_verify_signed_message_test_cases[index].name.c_str();
    }

    const char* get_wallet_verify_signed_message_test_case_message(size_t index) {
        if(index >= wallet_verify_signed_message_test_cases.size())
            return nullptr;
        return wallet_verify_signed_message_test_cases[index].message.c_str();
    }

    const char* get_wallet_verify_signed_message_test_case_signature(size_t index) {
        if(index >= wallet_verify_signed_message_test_cases.size())
            return nullptr;
        return wallet_verify_signed_message_test_cases[index].signature.c_str();
    }

    const char* get_wallet_verify_signed_message_test_case_address(size_t index) {
        if(index >= wallet_verify_signed_message_test_cases.size())
            return nullptr;
        return wallet_verify_signed_message_test_cases[index].address.c_str();
    }

    const bool get_wallet_verify_signed_message_test_case_valid(size_t index) {
        if(index >= wallet_verify_signed_message_test_cases.size())
            return false;
        return wallet_verify_signed_message_test_cases[index].valid;
    }

    const bool get_wallet_verify_signed_message_test_case_throws(size_t index) {
        if(index >= wallet_verify_signed_message_test_cases.size())
            return false;
        return wallet_verify_signed_message_test_cases[index].throws;
    }

    const size_t get_wallet_verify_signed_message_test_case_version(size_t index) {
        if(index >= wallet_verify_signed_message_test_cases.size())
            return 0;
        return wallet_verify_signed_message_test_cases[index].version;
    }
}
