#include <check.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/ots.h"
#include "../include/ots-version.h"
#include "../data.h"

START_TEST(test_ots_version)
{
    ots_result_t* result = ots_version();
    ck_assert(ots_result_is_string(result));
    ck_assert_str_eq(ots_result_string(result), OTS_VERSION_STRING);
    ots_free_result(&result);
}

START_TEST(test_ots_version_components)
{
    ots_result_t* result = ots_version_components();
    ck_assert(ots_result_is_array(result));
    ck_assert(ots_result_data_is_int(result));
    ck_assert_int_eq(ots_result_size(result), 3);
    int* components = (int*)ots_result_array(result);
    ck_assert_int_eq(components[0], OTS_VERSION_MAJOR);
    ck_assert_int_eq(components[1], OTS_VERSION_MINOR);
    ck_assert_int_eq(components[2], OTS_VERSION_PATCH);
    ots_free_array((void*)&components, sizeof(int), 3);
    ots_free_result(&result);
}
END_TEST

START_TEST(test_ots_height_from_timestamp)
{
    uint64_t acceptable_difference = 1296000; // 15 days
    for(uint64_t height = 1; height < get_highest_block(); height += 3600) {
        uint64_t timestamp = get_timestamp_by_height(height);
        ots_result_t* result = ots_height_from_timestamp(timestamp, OTS_NETWORK_MAIN);
        ck_assert(ots_result_is_number(result));
        uint64_t estimated_block = ots_result_number(result, 0);
        ots_free_result(&result);
        ck_assert_uint_lt(estimated_block, height);
        result = ots_timestamp_from_height(estimated_block, OTS_NETWORK_MAIN);
        ck_assert(ots_result_is_number(result));
        uint64_t estimated_timestamp = ots_result_number(result, 0);
        ots_free_result(&result);
        ck_assert_uint_lt(timestamp - estimated_timestamp, acceptable_difference);
    }
}

START_TEST(test_ots_timestamp_from_height)
{
    long long int acceptable_difference = 432000; // 5.5 days
    for(uint64_t height = 1; height < get_highest_block(); height += 3600) {
        uint64_t timestamp = get_timestamp_by_height(height);
        if(timestamp == 0)
            continue;
        ots_result_t* result = ots_timestamp_from_height(height, OTS_NETWORK_MAIN);
        ck_assert(ots_result_is_number(result));
        ck_assert_uint_lt(llabs((long long int)get_timestamp_by_height(height) - (long long int)ots_result_number(result, 0)), acceptable_difference);
        ots_free_result(&result);
    }
}

START_TEST(test_ots_random_bytes)
{
    size_t size = 32;
    for(size_t i = 0; i < 1000; i++) {
        ots_result_t* result = ots_random_bytes(size);
        ck_assert(ots_result_is_array(result));
        ck_assert(ots_result_data_is_uint8(result));
        ck_assert_uint_eq(ots_result_size(result), size);
        uint8_t* random_bytes = ots_result_uint8_array(result);
        ots_free_result(&result);
        result = ots_check_low_entropy(random_bytes, size, 0.35);
        ck_assert(ots_result_is_boolean(result));
        ck_assert(ots_result_boolean(result, true)==false);
        ots_free_result(&result);
        ots_free_array((void**)&random_bytes, sizeof(uint8_t), size);
    }
}

START_TEST(test_ots_random_32_bytes)
{
    for(size_t i = 0; i < 1000; i++) {
        ots_result_t* result = ots_random_32();
        ck_assert(ots_result_is_array(result));
        ck_assert(ots_result_data_is_uint8(result));
        ck_assert_uint_eq(ots_result_size(result), 32);
        uint8_t* random_bytes = (uint8_t*)ots_result_array(result);
        ots_free_result(&result);
        result = ots_check_low_entropy(random_bytes, 32, 0.35);
        ck_assert(ots_result_is_boolean(result));
        ck_assert(ots_result_boolean(result, true)==false);
        ots_free_result(&result);
        ots_free_array((void**)&random_bytes, sizeof(uint8_t), 32);
    }
}

START_TEST(test_ots_low_entropy)
{
    for(size_t i=0; i < 1000; i++) {
        ots_result_t* result = ots_random_32();
        ck_assert(ots_result_data_is_uint8(result));
        uint8_t* data = (uint8_t*)ots_result_array(result);
        ots_free_result(&result);
        result = ots_check_low_entropy(data, 32, 0.35);
        ck_assert(ots_result_is_boolean(result));
        ck_assert(ots_result_boolean(result, true)==false);
        ots_free_result(&result);
        ots_free_array((void**)&data, sizeof(uint8_t), 32);
    }
}

START_TEST(test_ots_enforce_entropy_level)
{
    double min_entropy = 4.75;
    size_t test_set_size = 100000;
    ots_set_enforce_entropy(false);
    size_t low_entropy_results = 0;
    for(size_t i = 0; i < test_set_size; ++i) {
        ots_result_t* result = ots_random_32();
        ck_assert(ots_is_result(result));
        uint8_t* data = (uint8_t*)ots_result_array(result);
        ots_free_result(&result);
        result = ots_check_low_entropy(data, 32, min_entropy);
        ck_assert(ots_result_is_boolean(result));
        if(ots_result_boolean(result, true))
            low_entropy_results++;
        ots_free_result(&result);
    }
    ck_assert_int_lt(low_entropy_results, test_set_size);
    ots_set_enforce_entropy_level(min_entropy);
    low_entropy_results = 0;
    size_t no_random_results = 0;
    for(size_t i = 0; i < test_set_size; ++i) {
        ots_result_t* result = ots_random_32();
        if(!ots_result_data_is_uint8(result)) {
            no_random_results++;
            continue;
        }
        uint8_t* data = ots_result_uint8_array(result);
        ots_free_result(&result);
        result = ots_check_low_entropy(data, 32, min_entropy);
        ck_assert(ots_result_is_boolean(result));
        if(ots_result_boolean(result, true))
            low_entropy_results++;
        ots_free_result(&result);
        ots_free_array((void**)&data, sizeof(uint8_t), 32);
    }
    ck_assert_int_eq(low_entropy_results, 0);
    ck_assert_int_lt(no_random_results, test_set_size);
}

START_TEST(test_ots_max_depth)
{
    uint32_t max_account_depth = ots_get_max_account_depth(0);
    uint32_t max_index_depth = ots_get_max_index_depth(0);
    ck_assert_int_ne(max_account_depth, 0);
    ck_assert_int_ne(max_index_depth, 0);
    ots_set_max_depth(5, 5);
    ck_assert_int_eq(ots_get_max_account_depth(0), 5);
    ck_assert_int_eq(ots_get_max_index_depth(0), 5);
    ots_set_max_depth(10, 10);
    ck_assert_int_eq(ots_get_max_account_depth(0), 10);
    ck_assert_int_eq(ots_get_max_index_depth(0), 10);
    ots_reset_max_depth();
    ck_assert_int_eq(ots_get_max_account_depth(0), max_account_depth);
    ck_assert_int_eq(ots_get_max_index_depth(0), max_index_depth);
}

Suite* ots_suite(void)
{
    Suite* s = suite_create("OTS");
    
    // Core functionality
    TCase* tc_core = tcase_create("Core");
    tcase_add_test(tc_core, test_ots_version);
    tcase_add_test(tc_core, test_ots_version_components);
    tcase_add_test(tc_core, test_ots_height_from_timestamp);
    tcase_add_test(tc_core, test_ots_timestamp_from_height);
    tcase_add_test(tc_core, test_ots_random_bytes);
    tcase_add_test(tc_core, test_ots_random_32_bytes);
    tcase_add_test(tc_core, test_ots_low_entropy);
    tcase_add_test(tc_core, test_ots_enforce_entropy_level);
    tcase_add_test(tc_core, test_ots_max_depth);
    suite_add_tcase(s, tc_core);
    
    return s;
}
