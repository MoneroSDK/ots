#include "data.h"
#include "data-address.hpp"
#include "data-seed-language.hpp"
#include <cstring>

/**
 * @file data.cpp
 * @brief This file provides a C interface to the data defined in data-address.hpp and data-seed-language.hpp
 * @todo TODO: expand also for the following data for testcases:
 * - data-seed-legacy.hpp
 * - data-seed-monero.hpp
 * - data-seed-polyseed.hpp
 * - data-wallet-sign.hpp
 * - data-blocktime.hpp
 *
 * @todo TODO: add more documentation
 */

extern "C" {
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
}
