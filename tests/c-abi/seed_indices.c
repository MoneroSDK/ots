#include <check.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../include/ots.h"

START_TEST(test_ots_seed_indices_create)
{
    ots_result_t* result = ots_random_32();
    ck_assert(ots_result_data_is_uint8(result));
    uint16_t* indices = (uint16_t*)ots_result_uint8_array(result);
    ots_free_result(&result);
    result = ots_seed_indices_create(indices, 16);
    ck_assert(ots_result_is_seed_indices(result));
    ots_handle_t* seedIndices = ots_result_handle(result);
    ots_free_result(&result);
    const uint16_t* seed_indices = ots_seed_indices_values(seedIndices);
    ck_assert_ptr_nonnull(seed_indices);
    for(size_t i = 0; i < 16; i++)
        ck_assert_int_eq(seed_indices[i], indices[i]);
    ots_free_handle(&seedIndices);
}
END_TEST

START_TEST(test_ots_seed_indices_create_from_string)
{
    uint16_t indices[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
    char* separator[] = { ",", " ", ";", "" };
    for(size_t s = 0; s < 4; s++) {
        size_t buffer_size = 16 * 5 + (s<3?15 * strlen(separator[s]):0);
        char str[buffer_size + 1];
        str[0] = '\0';
        for(size_t i = 0; i < 16; i++) {
            if(s < 3 && i > 0)
                strncat(str, separator[s], buffer_size - strlen(str));
            char number[5];
            snprintf(number, sizeof(number), "%04d", indices[i]);
            strncat(str, number, buffer_size - strlen(str));
        }
        ots_result_t* result = ots_seed_indices_create_from_string(str, separator[s]);
        ck_assert(ots_result_is_seed_indices(result));
        ots_handle_t* seedIndices = ots_result_handle(result);
        ots_free_result(&result);
        const uint16_t* seed_indices = ots_seed_indices_values(seedIndices);
        ck_assert_ptr_nonnull(seed_indices);
        for(size_t i = 0; i < 16; i++)
            ck_assert_int_eq(seed_indices[i], indices[i]);
        ots_free_handle(&seedIndices);
    }
}
END_TEST

START_TEST(test_ots_seed_indices_create_from_hex)
{
    uint16_t indices[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
    char* separator[] = { ",", " ", ";", "" };
    for(size_t s = 0; s < 4; s++) {
        size_t buffer_size = 16 * 3 + (s<3?15 * strlen(separator[s]):0);
        char str[buffer_size + 1];
        str[0] = '\0';
        for(size_t i = 0; i < 16; i++) {
            if(s < 3 && i > 0)
                strncat(str, separator[s], buffer_size - strlen(str));
            char number[4];
            snprintf(number, sizeof(number), "%03X", indices[i]);
            strncat(str, number, buffer_size - strlen(str));
        }
        ots_result_t* result = ots_seed_indices_create_from_hex(str, separator[s]);
        ck_assert(ots_result_is_seed_indices(result));
        ots_handle_t* seedIndices = ots_result_handle(result);
        ots_free_result(&result);
        const uint16_t* seed_indices = ots_seed_indices_values(seedIndices);
        ck_assert_ptr_nonnull(seed_indices);
        for(size_t i = 0; i < 16; i++)
            ck_assert_int_eq(seed_indices[i], indices[i]);
        ots_free_handle(&seedIndices);
    }
}
END_TEST

START_TEST(test_ots_seed_indices_count)
{
    ots_result_t* result = ots_random_32();
    ck_assert(ots_result_data_is_uint8(result));
    uint16_t* indices = (uint16_t*)ots_result_uint8_array(result);
    ots_free_result(&result);
    result = ots_seed_indices_create(indices, 16);
    ck_assert(ots_result_is_seed_indices(result));
    ots_handle_t* seedIndices = ots_result_handle(result);
    ots_free_result(&result);
    ck_assert_int_eq(ots_seed_indices_count(seedIndices), 16);
    ots_free_handle(&seedIndices);
}
END_TEST

START_TEST(test_ots_seed_indices_clear)
{
    ots_result_t* result = ots_random_32();
    ck_assert(ots_result_data_is_uint8(result));
    uint16_t* indices = (uint16_t*)ots_result_uint8_array(result);
    ots_free_result(&result);
    result = ots_seed_indices_create(indices, 16);
    ck_assert(ots_result_is_seed_indices(result));
    ots_handle_t* seedIndices = ots_result_handle(result);
    ots_free_result(&result);
    ck_assert_int_eq(ots_seed_indices_count(seedIndices), 16);
    ots_seed_indices_clear(seedIndices);
    ck_assert_int_eq(ots_seed_indices_count(seedIndices), 0);
    ots_free_handle(&seedIndices);
}
END_TEST

START_TEST(test_ots_seed_indices_append)
{
    ots_result_t* result = ots_random_32();
    ck_assert(ots_result_data_is_uint8(result));
    uint16_t* indices = (uint16_t*)ots_result_uint8_array(result);
    ots_free_result(&result);
    result = ots_seed_indices_create(NULL, 0);
    ck_assert(ots_result_is_seed_indices(result));
    ots_handle_t* seedIndices = ots_result_handle(result);
    ots_free_result(&result);
    for(size_t i = 0; i < 16; i++)
        ots_seed_indices_append(seedIndices, indices[i]);
    ck_assert_int_eq(ots_seed_indices_count(seedIndices), 16);
    const uint16_t* seed_indices = ots_seed_indices_values(seedIndices);
    ck_assert_ptr_nonnull(seed_indices);
    for(size_t i = 0; i < 16; i++)
        ck_assert_int_eq(seed_indices[i], indices[i]);
    ots_free_handle(&seedIndices);
}
END_TEST

START_TEST(test_ots_seed_indices_numeric)
{
    ots_result_t* result = ots_random_32();
    ck_assert(ots_result_data_is_uint8(result));
    uint16_t* indices = (uint16_t*)ots_result_uint8_array(result);
    ots_free_result(&result);
    result = ots_seed_indices_create(indices, 16);
    ck_assert(ots_result_is_seed_indices(result));
    ots_handle_t* seedIndices = ots_result_handle(result);
    ots_free_result(&result);
    ck_assert_int_eq(ots_seed_indices_count(seedIndices), 16);
    const char* numeric = (const char*)ots_seed_indices_numeric(seedIndices, ",");
    ck_assert_ptr_nonnull(numeric);
    result = ots_seed_indices_create_from_string(numeric, ",");
    ck_assert(ots_result_is_seed_indices(result));
    ots_handle_t* seedIndices2 = ots_result_handle(result);
    ots_free_result(&result);
    ck_assert_int_eq(ots_seed_indices_count(seedIndices2), 16);
    const uint16_t* seed_indices = ots_seed_indices_values(seedIndices2);
    ck_assert_ptr_nonnull(seed_indices);
    for(size_t i = 0; i < 16; i++)
        ck_assert_int_eq(seed_indices[i], indices[i]);
    ots_free_handle(&seedIndices2);
    ots_free_handle(&seedIndices);
}
END_TEST

START_TEST(test_ots_seed_indices_hex)
{
    ots_result_t* result = ots_random_32();
    ck_assert(ots_result_data_is_uint8(result));
    uint16_t* indices = (uint16_t*)ots_result_uint8_array(result);
    ots_free_result(&result);
    result = ots_seed_indices_create(indices, 16);
    ck_assert(ots_result_is_seed_indices(result));
    ots_handle_t* seedIndices = ots_result_handle(result);
    ots_free_result(&result);
    ck_assert_int_eq(ots_seed_indices_count(seedIndices), 16);
    const char* hex = (const char*)ots_seed_indices_hex(seedIndices, ",");
    ck_assert_ptr_nonnull(hex);
    result = ots_seed_indices_create_from_hex(hex, ",");
    ck_assert(ots_result_is_seed_indices(result));
    ots_handle_t* seedIndices2 = ots_result_handle(result);
    ots_free_result(&result);
    ck_assert_int_eq(ots_seed_indices_count(seedIndices2), 16);
    const uint16_t* seed_indices = ots_seed_indices_values(seedIndices2);
    ck_assert_ptr_nonnull(seed_indices);
    for(size_t i = 0; i < 16; i++)
        ck_assert_int_eq(seed_indices[i], indices[i]);
    ots_free_handle(&seedIndices2);
    ots_free_handle(&seedIndices);
}
END_TEST

START_TEST(test_ots_seed_indices_merge_values)
{
    ots_handle_t* seedIndices[4];
    for(size_t i = 0; i < 2; i++) {
        ots_result_t* result = ots_random_32();
        ck_assert(ots_result_data_is_uint8(result));
        uint16_t* indices = (uint16_t*)ots_result_uint8_array(result);
        ots_free_result(&result);
        result = ots_seed_indices_create(indices, 16);
        ck_assert(ots_result_is_seed_indices(result));
        seedIndices[i] = ots_result_handle(result);
        ots_free_result(&result);
    }
    ots_result_t* result = ots_seed_indices_merge_values(seedIndices[0], seedIndices[1]);
    ck_assert(ots_result_is_seed_indices(result));
    seedIndices[2] = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_indices_merge_values(seedIndices[2], seedIndices[1]);
    ck_assert(ots_result_is_seed_indices(result));
    seedIndices[3] = ots_result_handle(result);
    ots_free_result(&result);
    uint16_t* indices[4];
    for(size_t i = 0; i < 4; i++) {
        indices[i] = (uint16_t*)ots_seed_indices_values(seedIndices[i]);
        ck_assert_ptr_nonnull(indices[i]);
    }
    for(size_t i = 0; i < 16; i++) {
        ck_assert_int_ne(indices[0][i], indices[2][i]);
        ck_assert_int_ne(indices[1][i], indices[2][i]);
        ck_assert_int_eq(indices[0][i], indices[3][i]);
    }
}
END_TEST

START_TEST(test_ots_seed_indices_merge_with_password)
{
    ots_handle_t* seedIndices[3];
    ots_result_t* result = ots_random_32();
    ck_assert(ots_result_data_is_uint8(result));
    uint16_t* random = (uint16_t*)ots_result_uint8_array(result);
    ots_free_result(&result);
    result = ots_seed_indices_create(random, 16);
    ck_assert(ots_result_is_seed_indices(result));
    seedIndices[0] = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_indices_merge_with_password(seedIndices[0], "password");
    ck_assert(ots_result_is_seed_indices(result));
    seedIndices[1] = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_indices_merge_with_password(seedIndices[1], "password");
    ck_assert(ots_result_is_seed_indices(result));
    seedIndices[2] = ots_result_handle(result);
    ots_free_result(&result);
    uint16_t* indices[3];
    for(size_t i = 0; i < 3; i++) {
        indices[i] = (uint16_t*)ots_seed_indices_values(seedIndices[i]);
        ck_assert_ptr_nonnull(indices[i]);
    }
    for(size_t i = 0; i < 16; i++) {
        ck_assert_int_ne(indices[0][i], indices[1][i]);
        ck_assert_int_eq(indices[0][i], indices[2][i]);
    }
}
END_TEST

START_TEST(test_ots_seed_indices_merge_multiple_values)
{
    ots_handle_t* seedIndices[6];
    for(size_t i = 0; i < 4; i++) {
        ots_result_t* result = ots_random_32();
        ck_assert(ots_result_data_is_uint8(result));
        uint16_t* indices = (uint16_t*)ots_result_uint8_array(result);
        ots_free_result(&result);
        result = ots_seed_indices_create(indices, 16);
        ck_assert(ots_result_is_seed_indices(result));
        seedIndices[i] = ots_result_handle(result);
        ots_free_result(&result);
    }
    ots_result_t* result = ots_seed_indices_merge_multiple_values(
            (const ots_handle_t**)&seedIndices,
            16,
            4
    );
    ck_assert(ots_result_is_seed_indices(result));
    seedIndices[4] = ots_result_handle(result);
    ots_free_result(&result);
    for(size_t i = 1; i < 4; i++) {
        ots_result_t* result = ots_seed_indices_merge_values(
            seedIndices[ i==1 ? 4 : 5 ],
            seedIndices[i]
        );
        ck_assert(ots_result_is_seed_indices(result));
        if(i != 1)
            ots_free_handle(&seedIndices[5]);
        seedIndices[5] = ots_result_handle(result);
        ots_free_result(&result);
    }
    uint16_t* indices[6];
    for(size_t i = 0; i < 6; i++) {
        indices[i] = (uint16_t*)ots_seed_indices_values(seedIndices[i]);
        ck_assert_ptr_nonnull(indices[i]);
    }
    for(size_t i = 0; i < 16; i++) {
        ck_assert_int_ne(indices[4][i], indices[5][i]);
        ck_assert_int_eq(indices[0][i], indices[5][i]);
    }
}
END_TEST

START_TEST(test_ots_seed_indices_merge_values_and_zero)
{
    ots_handle_t* seedIndices[4];
    for(size_t i = 0; i < 2; i++) {
        ots_result_t* result = ots_random_32();
        ck_assert(ots_result_data_is_uint8(result));
        uint16_t* indices = (uint16_t*)ots_result_uint8_array(result);
        ots_free_result(&result);
        result = ots_seed_indices_create(indices, 16);
        ck_assert(ots_result_is_seed_indices(result));
        seedIndices[i] = ots_result_handle(result);
        ots_free_result(&result);
    }
    ots_result_t* result = ots_seed_indices_merge_values(
        seedIndices[0],
        seedIndices[1]
    );
    ck_assert(ots_result_is_seed_indices(result));
    seedIndices[2] = ots_result_handle(result);
    ots_free_result(&result);
    uint16_t* indices[4];
    for(size_t i = 0; i < 3; i++) {
        indices[i] = (uint16_t*)ots_seed_indices_values(seedIndices[i]);
        ck_assert_ptr_nonnull(indices[i]);
    }
    result = ots_seed_indices_merge_values_and_zero(
        seedIndices[1],
        seedIndices[2],
        true
    );
    ck_assert(ots_result_is_seed_indices(result));
    seedIndices[3] = ots_result_handle(result);
    ots_free_result(&result);
    indices[3] = (uint16_t*)ots_seed_indices_values(seedIndices[3]);
    ck_assert_ptr_nonnull(indices[3]);
    for(size_t i = 0; i < 16; i++) {
        ck_assert_int_ne(indices[2][i], indices[3][i]);
        ck_assert_int_eq(indices[0][i], indices[3][i]);
    }
    result = ots_seed_indices_merge_values(
        seedIndices[1],
        seedIndices[2]
    );
    ck_assert(ots_is_error(result));
    ck_assert_int_eq(ots_error_code(result), OTS_ERROR_INVALID_ARGUMENT);
}
END_TEST

START_TEST(test_ots_seed_indices_merge_with_password_and_zero)
{
    char* password = "password";
    ots_handle_t* seedIndices[3];
    ots_result_t* result = ots_random_32();
    ck_assert(ots_result_data_is_uint8(result));
    uint16_t* random = (uint16_t*)ots_result_uint8_array(result);
    ots_free_result(&result);
    result = ots_seed_indices_create(random, 16);
    ck_assert(ots_result_is_seed_indices(result));
    seedIndices[0] = ots_result_handle(result);
    ots_free_result(&result);
    result = ots_seed_indices_merge_with_password(seedIndices[0], password);
    ck_assert(ots_result_is_seed_indices(result));
    seedIndices[1] = ots_result_handle(result);
    ots_free_result(&result);
    uint16_t* indices[3];
    for(size_t i = 0; i < 2; i++) {
        indices[i] = (uint16_t*)ots_seed_indices_values(seedIndices[i]);
        ck_assert_ptr_nonnull(indices[i]);
    }
    result = ots_seed_indices_merge_with_password_and_zero(
        seedIndices[1],
        password,
        true
    );
    ck_assert(ots_result_is_seed_indices(result));
    seedIndices[2] = ots_result_handle(result);
    ots_free_result(&result);
    indices[2] = (uint16_t*)ots_seed_indices_values(seedIndices[2]);
    ck_assert_ptr_nonnull(indices[2]);
    for(size_t i = 0; i < 16; i++) {
        ck_assert_int_ne(indices[0][i], indices[1][i]);
        ck_assert_int_eq(indices[0][i], indices[2][i]);
    }
}
END_TEST

START_TEST(test_ots_seed_indices_merge_multiple_values_and_zero)
{
    ots_handle_t* seedIndices[6];
    for(size_t i = 0; i < 4; i++) {
        ots_result_t* result = ots_random_32();
        ck_assert(ots_result_data_is_uint8(result));
        uint16_t* indices = (uint16_t*)ots_result_uint8_array(result);
        ots_free_result(&result);
        result = ots_seed_indices_create(indices, 16);
        ck_assert(ots_result_is_seed_indices(result));
        seedIndices[i] = ots_result_handle(result);
        ots_free_result(&result);
    }
    ots_result_t* result = ots_seed_indices_merge_multiple_values(
            (const ots_handle_t**)&seedIndices,
            16,
            4
    );
    ck_assert(ots_result_is_seed_indices(result));
    seedIndices[4] = ots_result_handle(result);
    ots_free_result(&result);
    uint16_t* indices[6];
    for(size_t i = 0; i < 5; i++) {
        indices[i] = (uint16_t*)ots_seed_indices_values(seedIndices[i]);
        ck_assert_ptr_nonnull(indices[i]);
    }
    result = ots_seed_indices_merge_multiple_values_and_zero(
            (const ots_handle_t**)&seedIndices[1],
            16,
            4,
            true
    );
    ck_assert(ots_result_is_seed_indices(result));
    seedIndices[5] = ots_result_handle(result);
    ots_free_result(&result);
    indices[5] = (uint16_t*)ots_seed_indices_values(seedIndices[5]);
    ck_assert_ptr_nonnull(indices[5]);
    for(size_t i = 0; i < 16; i++) {
        ck_assert_int_ne(indices[0][i], indices[4][i]);
        ck_assert_int_eq(indices[0][i], indices[5][i]);
    }
    for(size_t i = 1; i < 5; i++)
        ck_assert_ptr_null(seedIndices[i]);
    ck_assert_ptr_nonnull(seedIndices[0]);
    ck_assert_ptr_nonnull(seedIndices[5]);
}
END_TEST

Suite* seed_indices_suite(void)
{
    Suite* s = suite_create("Seed Indices");
    // Core functionality
    TCase* tc_core = tcase_create("Core");
    tcase_add_test(tc_core, test_ots_seed_indices_create);
    tcase_add_test(tc_core, test_ots_seed_indices_create_from_string);
    tcase_add_test(tc_core, test_ots_seed_indices_create_from_hex);
    tcase_add_test(tc_core, test_ots_seed_indices_count);
    tcase_add_test(tc_core, test_ots_seed_indices_clear);
    tcase_add_test(tc_core, test_ots_seed_indices_append);
    tcase_add_test(tc_core, test_ots_seed_indices_numeric);
    tcase_add_test(tc_core, test_ots_seed_indices_hex);
    tcase_add_test(tc_core, test_ots_seed_indices_merge_values);
    tcase_add_test(tc_core, test_ots_seed_indices_merge_with_password);
    tcase_add_test(tc_core, test_ots_seed_indices_merge_multiple_values);
    tcase_add_test(tc_core, test_ots_seed_indices_merge_values_and_zero);
    tcase_add_test(tc_core, test_ots_seed_indices_merge_with_password_and_zero);
    tcase_add_test(tc_core, test_ots_seed_indices_merge_multiple_values_and_zero);
    suite_add_tcase(s, tc_core);
    return s;
}
