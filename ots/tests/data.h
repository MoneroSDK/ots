#pragma once

#include <stddef.h>
#include <stdbool.h> // For C only, C++ handles bool natively

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

#ifdef __cplusplus
}
#endif
