#ifndef OTS_INTERNAL_H
#define OTS_INTERNAL_H

#include "ots.h"
#include "ots.hpp"

/**
 * @file ots-internal.h
 * @brief Internal header for the C ABI (Application Binary Interface) library
 * @internal
 *
 * This header provides internal functions for the C ABI implementation.
 * These functions are implemented in @see src/c-abi/internal.cpp
 */

/**
 * @namespace ots::internal
 * @brief Internal functions for the C ABI implementation
 */
namespace ots::internal {
    /**
     * @brief Translate C++ exception to C error
     * @param[out] error Error to fill
     * @param[in] e Exception to translate
     * @internal
     */
    void translate_exception(ots_error_t* error, const ots::exception::Exception& e);

    /**
     * @brief Set result to success
     * @param[out] result Result to set
     * @internal
     */
    void set_success(ots_result_t* result);

    /**
     * @brief Set result type
     * @param[out] result Result to set
     * @param[in] type Type to set
     * @internal
     */
    void set_result_type(ots_result_t* result, ots_result_type type);

    /**
     * @brief Create handle
     * @param[in] type Handle type
     * @param[in] ptr Pointer to object
     * @return Handle
     * @internal
     */
    ots_handle_t create_handle(ots_handle_type type, void* ptr);

    /**
     * @brief Create handle reference (no ownership)
     * @param[in] type Handle type
     * @param[in] ptr Pointer to object
     * @return Handle
     * @internal
     */
    ots_handle_t create_handle_reference(ots_handle_type type, void* ptr);

    /**
     * @brief Create a copy of a string
     * @param[in] str String to copy
     * @return Copy of the string
     * @internal
     */
    char* create_string_copy(const std::string& str);

    /**
     * @brief Set handle to result
     * @param[out] result Result to set
     * @param[in] handle_type Handle type
     * @param[in] handle Handle to set
     * @internal
     */
    void set_handle(ots_result_t* result, ots_handle_type handle_type, void* handle);

    /**
     * @brief Set handle reference to result (no ownership)
     * @param[out] result Result to set
     * @param[in] handle_type Handle type
     * @param[in] handle Handle to set
     * @internal
     */
    void set_handle_reference(ots_result_t* result, ots_handle_type handle_type, void* handle);

    /**
     * @brief Set string to result
     * @param[out] result Result to set
     * @param[in] str String to set
     * @internal
     */
    void set_string(ots_result_t* result, const std::string& str);

    /**
     * @brief Set boolean to result
     * @param[out] result Result to set
     * @param[in] value Boolean value
     * @internal
     */
    void set_boolean(ots_result_t* result, bool value);

    /**
     * @brief Set number to result
     * @param[out] result Result to set
     * @param[in] value Number value
     * @internal
     */
    void set_number(ots_result_t* result, int64_t value);

    /**
     * @brief Set comparison result to result
     * @param[out] result Result to set
     * @param[in] value Comparison result
     * @internal
     */
    void set_comparison(ots_result_t* result, int64_t value);

    /**
     * @brief Set array to result
     * @param[out] result Result to set
     * @param[in] arr Array to set
     * @param[in] size Size of array
     * @param[in] data_type Data type of array
     * @param[in] reference If true, the handle does not own the object, don't free
     * @internal
     */
    void set_array(ots_result_t* result, void* arr, size_t size, ots_data_type data_type, bool reference);

    /**
     * @brief Set address index to result
     * @param[out] result Result to set
     * @param[in] index Address index
     * @internal
     */
    void set_address_index(ots_result_t* result, const uint32_t* index);

    /**
     * @brief Set address type to result
     * @param[out] result Result to set
     * @param[in] type Address type
     * @internal
     */
    void set_address_type(ots_result_t* result, ots::AddressType type);

    /**
     * @brief Set network to result
     * @param[out] result Result to set
     * @param[in] network Network
     * @internal
     */
    void set_network(ots_result_t* result, ots::Network network);

    /**
     * @brief Set seed type to result
     * @param[out] result Result to set
     * @param[in] type Seed type
     * @internal
     */
    void set_seed_type(ots_result_t* result, ots::SeedType type);

    /**
     * @brief Set error to result
     * @param[out] result Result to set
     * @param[in] e Exception to set
     * @internal
     */
    void set_error(ots_result_t* result, const ots::exception::Exception& e);

    /**
     * @brief Convert C network to C++ network
     * @param[in] network Network to convert
     * @return C++ network
     * @internal
     */
    ots::Network to_cpp_network(OTS_NETWORK network);
} // namespace
#endif // OTS_INTERNAL_H
