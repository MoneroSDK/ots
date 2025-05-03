#include <check.h>
#include "../include/ots.h"
#include "../data.h"
#include <stdio.h>

START_TEST(test_address)
{
    // check correct invalid address handling
    const char* invalid_address = get_address_test_case_address(1);
    ots_result_t* error = ots_address_create(invalid_address);
    ck_assert(ots_is_error(error));
    ots_free_result(&error);
    ck_assert_ptr_null(error);

    // check correct valid address handling
    const char* valid_address = get_address_test_case_address(2);
    ots_result_t* result = ots_address_create(valid_address);
    ck_assert(ots_is_result(result));
    ck_assert(ots_result_is_type(result, OTS_RESULT_HANDLE));
    ck_assert(ots_result_handle_is_type(result, OTS_HANDLE_ADDRESS));
    ots_handle_t* address = ots_result_handle(result);
    ots_free_result(&result);
    ck_assert_ptr_null(result);
    ck_assert_ptr_nonnull(address);

    // check address comparison
    result = ots_address_equal_string(address, valid_address);
    ck_assert(ots_is_result(result));
    ck_assert(ots_result_is_type(result, OTS_RESULT_BOOLEAN));
    ck_assert(ots_result_is_equal(result));
    ots_free_result(&result);
    ck_assert_ptr_null(result);

    // check address base58 string
    result = ots_address_base58_string(address);
    ck_assert(ots_is_result(result));
    ck_assert(ots_result_is_type(result, OTS_RESULT_STRING));
    const char* address_string = ots_result_string(result);
    ots_free_result(&result);
    ck_assert_ptr_null(result);
    ck_assert_ptr_nonnull(address_string);
    ck_assert_str_eq(address_string, valid_address);
    ots_free_string((char**)&address_string);
    ck_assert_ptr_null(address_string);

    // check address type
    result = ots_address_type(address);
    ck_assert(ots_is_result(result));
    ck_assert(ots_result_is_type(result, OTS_RESULT_ADDRESS_TYPE));
    ck_assert(ots_result_address_type_is_type(result, OTS_ADDRESS_TYPE_STANDARD));
    ots_free_result(&result);
    ck_assert_ptr_null(result);

    // check address network
    result = ots_address_network(address);
    ck_assert(ots_is_result(result));
    ck_assert(ots_result_is_type(result, OTS_RESULT_NETWORK));
    ck_assert(ots_result_network_is_type(result, OTS_NETWORK_MAIN));
    ots_free_result(&result);
    ck_assert_ptr_null(result);

    // check address fingerprint
    result = ots_address_fingerprint(address);
    ck_assert(ots_is_result(result));
    ck_assert(ots_result_is_type(result, OTS_RESULT_STRING));
    const char* fingerprint = ots_result_string(result);
    ots_free_result(&result);
    ck_assert_ptr_null(result);
    ck_assert_ptr_nonnull(fingerprint);
    ck_assert_str_eq(fingerprint, get_address_test_case_fingerprint(2));
    ots_free_string((char**)&fingerprint);
    ck_assert_ptr_null(fingerprint);

    // check address is integrated
    result = ots_address_is_integrated(address);
    ck_assert(ots_is_result(result));
    ck_assert(ots_result_is_type(result, OTS_RESULT_BOOLEAN));
    ck_assert_int_eq(ots_result_boolean(result, true), false);
    ots_free_result(&result);
    ck_assert_ptr_null(result);
    result = ots_address_create(get_address_test_case_address(4));
    ck_assert(ots_is_result(result));
    ck_assert(ots_result_is_type(result, OTS_RESULT_HANDLE));
    ots_handle_t* integrated_address = ots_result_handle(result);
    ots_free_result(&result);
    ck_assert_ptr_null(result);
    result = ots_address_is_integrated(integrated_address);
    ck_assert(ots_is_result(result));
    ck_assert(ots_result_is_type(result, OTS_RESULT_BOOLEAN));
    ck_assert_int_eq(ots_result_boolean(result, false), true);
    ots_free_result(&result);
    ck_assert_ptr_null(result);
    // check payment ID
    result = ots_address_payment_id(integrated_address);
    ck_assert(ots_is_result(result));
    ck_assert(ots_result_is_type(result, OTS_RESULT_STRING));
    ck_assert_str_eq(ots_result_string(result), get_address_test_case_payment_id(4));
    ots_free_result(&result);
    result = ots_address_payment_id(address);
    ck_assert(ots_is_result(result));
    ck_assert(ots_result_is_type(result, OTS_RESULT_STRING));
    ck_assert_str_eq(ots_result_string(result), "");
    ots_free_result(&result);
    ck_assert_ptr_null(result);
    // check base address from integrated address
    result = ots_address_from_integrated(integrated_address);
    ck_assert(ots_is_result(result));
    ck_assert(ots_result_is_type(result, OTS_RESULT_HANDLE));
    ots_handle_t* base_address = ots_result_handle(result);
    ots_free_result(&result);
    ck_assert_ptr_null(result);
    result = ots_address_equal_string(base_address, get_address_test_case_base_address(4));
    ck_assert(ots_is_result(result));
    ck_assert(ots_result_is_type(result, OTS_RESULT_BOOLEAN));
    ck_assert_int_eq(ots_result_boolean(result, false), true);
    ots_free_result(&result);
    ck_assert_ptr_null(result);
    ots_free_handle(&base_address);
    ck_assert_ptr_null(base_address);

    // check base address from none integrated address
    result = ots_address_from_integrated(address);
    ck_assert(ots_is_error(result));
    ots_free_result(&result);
    ck_assert_ptr_null(result);
    ots_free_handle(&base_address);
    ck_assert_ptr_null(base_address);
    // check address length
    result = ots_address_length(address);
    ck_assert(ots_is_result(result));
    ck_assert(ots_result_is_type(result, OTS_RESULT_NUMBER));
    ck_assert_int_eq(ots_result_number(result, 0), strlen(valid_address));
    ots_free_result(&result);
    ck_assert_ptr_null(result);
    // check address equal
    result = ots_address_create(valid_address);
    ck_assert(ots_is_result(result));
    ck_assert(ots_result_is_type(result, OTS_RESULT_HANDLE));
    ots_handle_t* address2 = ots_result_handle(result);
    ots_free_result(&result);
    ck_assert_ptr_null(result);
    result = ots_address_equal(address, address2);
    ck_assert(ots_is_result(result));
    ck_assert(ots_result_is_type(result, OTS_RESULT_BOOLEAN));
    ck_assert_int_eq(ots_result_boolean(result, true), true);
    ots_free_result(&result);
    ck_assert_ptr_null(result);
    ots_free_handle(&address2);
    ck_assert_ptr_null(address2);
    // check address not equal
    result = ots_address_create(get_address_test_case_address(3));
    ck_assert(ots_is_result(result));
    ck_assert(ots_result_is_type(result, OTS_RESULT_HANDLE));
    address2 = ots_result_handle(result);
    ots_free_result(&result);
    ck_assert_ptr_null(result);
    result = ots_address_equal(address, address2);
    ck_assert(ots_is_result(result));
    ck_assert(ots_result_is_type(result, OTS_RESULT_BOOLEAN));
    ck_assert_int_eq(ots_result_boolean(result, false), false);
    ots_free_result(&result);
    ck_assert_ptr_null(result);
    ots_free_handle(&address2);
    ck_assert_ptr_null(address2);
    // check address equal string
    result = ots_address_equal_string(address, valid_address);
    ck_assert(ots_is_result(result));
    ck_assert(ots_result_is_type(result, OTS_RESULT_BOOLEAN));
    ck_assert_int_eq(ots_result_boolean(result, false), true);
    ots_free_result(&result);
    ck_assert_ptr_null(result);
    // check address not equal string
    result = ots_address_equal_string(address, get_address_test_case_address(3));
    ck_assert(ots_is_result(result));
    ck_assert(ots_result_is_type(result, OTS_RESULT_BOOLEAN));
    ck_assert_int_eq(ots_result_boolean(result, true), false);
    ots_free_result(&result);
    ck_assert_ptr_null(result);
    // check free address
    ots_free_handle(&address);
    ck_assert_ptr_null(address);
}
END_TEST

START_TEST(test_address_string)
{
    const char* invalid_address = get_address_test_case_address(1);
    const char* valid_address = get_address_test_case_address(2);
    // check address string valid
    ots_result_t* result = ots_address_string_valid(invalid_address, OTS_NETWORK_MAIN);
    ck_assert_int_eq(ots_result_boolean(result, false), false);
    ots_free_result(&result);
    ck_assert_ptr_null(result);
    result = ots_address_string_valid(valid_address, OTS_NETWORK_MAIN);
    ck_assert(ots_is_result(result));
    ck_assert(ots_result_is_type(result, OTS_RESULT_BOOLEAN));
    ck_assert_int_eq(ots_result_boolean(result, true), true);
    ots_free_result(&result);
    // check address string network
    result = ots_address_string_network(invalid_address);
    ck_assert(ots_is_error(result));
    ots_free_result(&result);
    ck_assert_ptr_null(result);
    result = ots_address_string_network(valid_address);
    ck_assert(ots_is_result(result));
    ck_assert(ots_result_is_type(result, OTS_RESULT_NETWORK));
    ck_assert(ots_result_network_is_type(result, get_address_test_case_network(2)));
    ots_free_result(&result);
    ck_assert_ptr_null(result);
    // check address string type
    result = ots_address_string_type(invalid_address);
    ck_assert(ots_is_error(result));
    ots_free_result(&result);
    ck_assert_ptr_null(result);
    result = ots_address_string_type(valid_address);
    ck_assert(ots_is_result(result));
    ck_assert(ots_result_is_type(result, OTS_RESULT_ADDRESS_TYPE));
    ck_assert(ots_result_address_type_is_type(result, get_address_test_case_type(2)));
    ots_free_result(&result);
    ck_assert_ptr_null(result);
    // check address string fingerprint
    result = ots_address_string_fingerprint(invalid_address);
    ck_assert(ots_is_error(result));
    ots_free_result(&result);
    result = ots_address_string_fingerprint(valid_address);
    ck_assert(ots_is_result(result));
    ck_assert(ots_result_is_type(result, OTS_RESULT_STRING));
    ck_assert_str_eq(ots_result_string(result), get_address_test_case_fingerprint(2));
    ots_free_result(&result);
    ck_assert_ptr_null(result);
    // check address string is integrated
    result = ots_address_string_is_integrated(get_address_test_case_address(4));
    ck_assert(ots_is_result(result));
    ck_assert(ots_result_is_type(result, OTS_RESULT_BOOLEAN));
    ck_assert(ots_result_boolean(result, false));
    ots_free_result(&result);
    ck_assert_ptr_null(result);
    // check address string payment ID
    result = ots_address_string_payment_id(get_address_test_case_address(4));
    ck_assert(ots_is_result(result));
    ck_assert(ots_result_is_type(result, OTS_RESULT_STRING));
    ck_assert_str_eq(ots_result_string(result), get_address_test_case_payment_id(4));
    ots_free_result(&result);
    ck_assert_ptr_null(result);
    // check address string base address from integrated address
    result = ots_address_string_integrated(get_address_test_case_address(4));
    ck_assert(ots_is_result(result));
    ck_assert(ots_result_is_string(result));
    ck_assert_str_eq(ots_result_string(result), get_address_test_case_base_address(4));
    ots_free_result(&result);
    ck_assert_ptr_null(result);
    // check address string base address from none integrated address
    result = ots_address_string_integrated(get_address_test_case_address(2));
    ck_assert(ots_is_error(result));
    ots_free_result(&result);
    ck_assert_ptr_null(result);
}
END_TEST

Suite* address_suite(void)
{
    Suite* s = suite_create("Address");
    // Core functionality
    TCase* tc_core = tcase_create("Core");
    tcase_add_test(tc_core, test_address);
    tcase_add_test(tc_core, test_address_string);
    suite_add_tcase(s, tc_core);
    return s;
}
