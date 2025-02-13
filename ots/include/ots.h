#ifndef OTS_H
#define OTS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "ots-errors.h"

/**
 * @file ots.h
 * @brief Header for the C ABI (Application Binary Interface) library
 *
 * This header provides a pure C-compatible interface to the C++ implementation.
 * All functions that return pointers or handles require explicit memory management
 * using the corresponding free functions to prevent memory leaks.
 *
 * all implementations are in src/c-abi/
 *       - internal: src/c-abi/internal.cpp
 *       - general: src/c-abi/ots.cpp
 *       - wallet: src/c-abi/wallet.cpp
 *       - address: src/c-abi/address.cpp
 *       - seed: src/c-abi/seed.cpp
 *       - legacy seed: src/c-abi/seed-legacy.cpp
 *       - monero seed: src/c-abi/seed-monero.cpp
 *       - polyseed: src/c-abi/seed-polyseed.cpp
 *       - wipeable string: src/c-abi/wipeable-string.cpp
 *
 * @note All functions returning an ots_result_t must be checked for errors using
 *       ots_is_error() before accessing the result.
 */

/*******************************************************************************
 * Constants and Defines
 ******************************************************************************/

/** @brief Maximum length for error messages */
#define OTS_MAX_ERROR_MESSAGE 256
/** @brief Maximum length for error class strings */
#define OTS_MAX_ERROR_CLASS 64
/** @brief Maximum length for version strings */
#define OTS_MAX_VERSION_STRING 32

/** @brief Seed phrase lengths */
#define OTS_MONERO_SEED_WORDS 25
#define OTS_POLYSEED_WORDS 16
#define OTS_LEGACY_SEED_WORDS 13

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * @brief Network types matching ots::Network
     */
    typedef enum {
        OTS_NETWORK_MAIN,    /**< Main production network */
        OTS_NETWORK_TEST,    /**< Test development network */
        OTS_NETWORK_STAGE    /**< Staging pre-production network */
    } OTS_NETWORK;

    /**
     * @brief Address types matching ots::AddressType
     */
    typedef enum {
        OTS_ADDRESS_TYPE_STANDARD,    /**< Standard Monero address */
        OTS_ADDRESS_TYPE_SUBADDRESS,  /**< Sub-address */
        OTS_ADDRESS_TYPE_INTEGRATED   /**< Integrated address with payment ID */
    } OTS_ADDRESS_TYPE;

    /**
     * @brief Seed types matching ots::SeedType
     */
    typedef enum {
        OTS_SEED_TYPE_MONERO,    /**< Monero 25-word seed */
        OTS_SEED_TYPE_POLYSEED   /**< Polyseed 16-word seed */
    } OTS_SEED_TYPE;

    /**
     * @brief Types of handles for type safety
     */
    typedef enum {
        OTS_HANDLE_INVALID = 0,
        OTS_HANDLE_WIPEABLE_STRING,
        OTS_HANDLE_SEED_INDICES,
        OTS_HANDLE_SEED_LANGUAGE,
        OTS_HANDLE_ADDRESS,
        OTS_HANDLE_SEED,
        OTS_HANDLE_WALLET,
        OTS_HANDLE_TX,
        OTS_HANDLE_TX_DESCRIPTION,
        OTS_HANDLE_TX_WARNING
    } ots_handle_type;

    typedef enum {
        OTS_RESULT_NONE          =   0,
        OTS_RESULT_HANDLE        =   1,
        OTS_RESULT_STRING        =   2,
        OTS_RESULT_BOOLEAN       =   4,
        OTS_RESULT_NUMBER        =   8,
        OTS_RESULT_COMPARISON    =  16,
        OTS_RESULT_ARRAY         =  32,
        OTS_RESULT_ADDRESS_TYPE  =  64,
        OTS_RESULT_NETWORK       = 128,
        OTS_RESULT_SEED_TYPE     = 256,
        OTS_RESULT_ADDRESS_INDEX = 512
    } ots_result_type;

    typedef enum {
        OTS_DATA_INVALID = 0,
        OTS_DATA_INT,
        OTS_DATA_CHAR,
        OTS_DATA_UINT8,
        OTS_DATA_UINT16,
        OTS_DATA_UINT32,
        OTS_DATA_HANDLE
    } ots_data_type;

    /**
     * @brief Handle structure for C objects
     */
    typedef struct {
        ots_handle_type type; /**< Type of handle */
        void* ptr;            /**< Pointer to object */
        bool reference;       /**< If true, the handle does not own the object, don't free */
    } ots_handle_t;

    /** @brief Error structure for exception handling */
    typedef struct {
        int32_t code;                                /**< Error code, 0 for success */
        char message[OTS_MAX_ERROR_MESSAGE];         /**< Error message */
        char cls[OTS_MAX_ERROR_CLASS];               /**< Error class */
    } ots_error_t;

    /** @brief Result structure combining handle and error */
    typedef struct {
        union {
            /* DO NOT CHANGE to pointer, the purpose is,
             * to make sure to not copy pointer and then
             * cut the branch your sitting on.
             * Thought through now 5 times, STOP!
             * Simply let it be, relax, the handle itself
             * uses a pointer so there is no much overhead
             * but it is much safer this way.
             */
            ots_handle_t handle;                     /**< Handle for created objects */
            struct {
                void* ptr;                           /**< Pointer to data */
                size_t size;                         /**< Size of data */
                ots_data_type type;                  /**< Type of data */
                bool reference;                   /**< If true, the handle does not own the object, don't free */
            } data;
            bool boolean;                            /**< Boolean result */
            int64_t number;                          /**< Numeric result */
        } result;
        ots_result_type type;                        /**< Type of result, can be more then one */
        ots_error_t error;                           /**< Error information */
    } ots_result_t;

    /** @brief Transaction description */
    typedef struct {
        char* description;                          /**< Transaction description */
        uint64_t amount;                           /**< Transaction amount */
        uint64_t fee;                              /**< Transaction fee */
        char* destination;                         /**< Destination address */
        uint32_t unlock_time;                      /**< Unlock time */
    } ots_tx_description_t;

    /** @brief Transaction warning */
    typedef struct {
        char* message;                             /**< Warning message */
        uint32_t severity;                         /**< Warning severity */
    } ots_tx_warning_t;

    /**
     * @brief Validate handle type
     */
    static inline bool ots_handle_valid(const ots_handle_t* h, ots_handle_type expected) {
        return h->type == expected && h->ptr != NULL;
    }

    /**
     * @brief Check if result contains an error
     * @param[in] result Result to check
     * @return true if result contains an error
     */
    bool ots_is_error(const ots_result_t* result);

    /**
     * @brief Get error message for result
     * @param[in] result handle to get error message for
     * @return Result containing error message string or NULL
     */
    char* ots_get_error_message(const ots_result_t* result);

    /**
     * @brief Get error class for result
     * @param[in] result handle to get error class for
     * @return Result containing error class string or NULL
     */
    char* ots_get_error_class(const ots_result_t* result);

    /**
     * @brief Get error code for result
     * @param[in] result handle to get error code for
     * @return Error code
     */
    int32_t ots_get_error_code(const ots_result_t* result);

    /**
     * @brief Check if result is a result and not an error
     * @param[in] result Result to check
     * @return true if result, false if error
     * @see ots_is_error(), it is the opposite
     */
    bool ots_is_result(const ots_result_t* result);

    /**
     * @brief Check if result has a specific type
     * @param[in] result Result to check
     * @param[in] type Type to check for
     * @return true if result is the specified type
     */
    bool ots_result_is_type(const ots_result_t* result, ots_result_type type);

    /**
     * @brief Check if result has a specific data type
     * @param[in] result Result to check
     * @param[in] type Data type to check for
     * @return true if result is the specified data type
     */
    bool ots_result_data_is_type(const ots_result_t* result, ots_data_type type);

    /**
     * @brief Check if result data is a reference
     * @param[in] result Result to check
     * @return true if result data is a reference
     */
    bool ots_result_data_is_reference(const ots_result_t* result);

    /**
     * @brief Get handle from result if result type OTS_RESULT_HANDLE is available
     * @param[in] result Result to get handle from
     * @return Handle or NULL
     */
    ots_handle_t* ots_result_handle(const ots_result_t* result);

    /**
     * @brief Check if handle is of a specific type
     * @param[in] result with a handle to check
     * @param[in] type Type to check for
     * @return true if handle is the specified type
     */
    bool ots_result_handle_is_type(const ots_result_t* result, ots_handle_type type);

    /**
     * @brief Check if handle is a reference
     * @param[in] result with a handle to check
     * @return true if handle is a reference
     */
    bool ots_result_handle_is_reference(const ots_result_t* result);

    /**
     * @brief Get string from result if result type is
     *        OTS_RESULT_STRING or
     *        OTS_RESULT_HANDLE with ots_data_type OTS_HANDLE_WIPEABLE_STRING
     * @param[in] result Result to get string from
     * @return String or NULL
     * @warning DO NOT free the string with ots_free_string()
     *          content will be freed with the result.
     *          Use simply like:
     *          ```c
     *          const char* str = ots_result_string(result);
     *          // use string
     *          // or simply use it directly without assignment to a variable
     *          printf("String: %s\n", ots_result_string(result));
     *          ots_free_result(&result); // content of the string will be freed
     *          ```
     */
    const char* ots_result_string(const ots_result_t* result);

    /**
     * @brief Get string as copy from result if result type is
     *        OTS_RESULT_STRING or
     *        OTS_RESULT_HANDLE with ots_data_type OTS_HANDLE_WIPEABLE_STRING
     * @param[in] result Result to get string from
     * @return string or NULL
     * @note use ots_free_string() to free the string, how this is a separate copy,
     *       you can free the result without loosing the string content.
     */
    char* ots_result_string_copy(const ots_result_t* result);

    /**
     * @brief Get size of string from result if result type OTS_RESULT_STRING is available
     * @param[in] result Result to get string size from
     * @return Size of string or 0
     */
    size_t ots_result_string_size(const ots_result_t* result);

    /**
     * @brief Get boolean from result if result type OTS_RESULT_BOOLEAN is available
     * @param[in] result Result to get boolean from
     * @param[in] default_value Default value if result doesn't contain a boolean type
     * @return Boolean value
     */
    bool ots_result_boolean(const ots_result_t* result, bool default_value);

    /**
     * @brief Get number from result if result type OTS_RESULT_NUMBER is available
     * @param[in] result Result to get number from
     * @param[in] default_value Default value if result doesn't contain a number type
     * @return Number value
     */
    int64_t ots_result_number(const ots_result_t* result, int64_t default_value);

    /**
     * @brief Get array from result if result type OTS_RESULT_ARRAY is available
     * @param[in] result Result to get array from
     * @return Array pointer or NULL
     */
    void* ots_result_array(const ots_result_t* result);

    /**
     * @brief Check if result is a comparison result
     * @param[in] result Result to check
     * @return true if result is a comparison result
     */
    bool ots_result_is_comparison(const ots_result_t* result);

    /**
     * @brief Get comparison result from result
     * @param[in] result Result to get comparison from
     * @return Comparison result or 0
     * @warning Use ots_result_is_comparison() to check if result is a comparison result before
     */
    int64_t ots_result_comparison(const ots_result_t* result);

    /**
     * @brief Check if comparison result is equals
     * @param[in] result Result to check
     * @return true if comparison result is 0
     * @warning Use ots_result_is_comparison() to check if result is a comparison result before
     */
    bool ots_result_is_equal(const ots_result_t* result);

    /**
     * @brief returns the size of the result if it is an array or string
     * @param[in] result Result to get size from
     * @return Size of result or 0 if it is not a supported result type
     */
    size_t ots_result_size(const ots_result_t* result);

    /**
     * @brief Check if result is an address type
     * @param[in] result Result to check
     * @return true if result is an address type
     */
    bool ots_result_is_address_type(const ots_result_t* result);

    /**
     * @brief Check if result is a specific address type
     * @param[in] result Result to check
     * @param[in] type Address type to check for
     * @return true if result is the specified address type
     */
    bool ots_result_address_type_is_type(const ots_result_t* result, OTS_ADDRESS_TYPE type);

    /**
     * @brief Check if result is an address index
     * @param[in] result to check
     */
    bool ots_result_is_address_index(const ots_result_t* result);

    /**
     * @brief Get the account part of the address index
     * @param[in] result to check
     * @note Use ots_result_is_address_index(const ots_result_t*) first!
     */
    uint32_t ots_result_address_index_account(const ots_result_t* result);

    /**
     * @brief Get the account part of the address index
     * @param[in] result to check
     * @note Use ots_result_is_address_index(const ots_result_t*) first!
     */
    uint32_t ots_result_address_index_index(const ots_result_t* result);

    /**
     * @brief Check if result is a network type
     * @param[in] result Result to check
     * @return true if result is a network type
     */
    bool ots_result_is_network(const ots_result_t* result);

    /**
     * @brief Check if result is a specific network type
     * @param[in] result Result to check
     * @param[in] network Network type to check for
     * @return true if result is the specified network type
     */
    bool ots_result_network_is_type(const ots_result_t* result, OTS_NETWORK network);

    /**
     * @brief Check if result is a seed type
     * @param[in] result Result to check
     * @return true if result is a seed type
     */
    bool ots_result_is_seed_type(const ots_result_t* result);

    /**
     * @brief Check if result is a specific seed type
     * @param[in] result Result to check
     * @param[in] type Seed type to check for
     * @return true if result is the specified seed type
     */
    bool ots_result_seed_type_is_type(const ots_result_t* result, OTS_SEED_TYPE type);

    /**
     * @brief Check if a pointer is NULL
     * @param[in] ptr Pointer to check
     * @return true if pointer is NULL
     */
    bool ots_is_null(const void* ptr);

    /**
     * @brief Get array from result if result type OTS_RESULT_ARRAY is available
     * @param[in] result Result to get array from
     * @return Array or NULL
     */
    void* ots_result_array(const ots_result_t* result);

    /**
     * @brief Check if handle is of a specific type
     * @param[in] handle Handle to check
     * @param[in] type Type to check for
     * @return true if handle is the specified type
     */
    bool ots_handle_is_type(const ots_handle_t* handle, ots_handle_type type);

    /**
     * @brief Check if handle is a reference
     * @param[in] handle Handle to check
     * @return true if handle is a reference
     */
    bool ots_handle_is_reference(const ots_handle_t* handle);

    /*******************************************************************************
     * Memory Management Functions
     ******************************************************************************/

    /**
     * @brief Free a string allocated by the library
     * @param[in] str String to free
     */
    void ots_free_string(char** str);

    /**
     * @brief Free an array allocated by the library
     * @param[in] arr Array to free
     * @param[in] elem_size Size of array elements
     * @param[in] count Number of elements
     */
    void ots_free_array(void** arr, size_t elem_size, size_t count);

    /**
     * @brief Free a result allocated by the library
     * @param[in] result Result to free
     */
    void ots_free_result(ots_result_t** result);

    /**
     * @brief Free a handle
     * @param[in] handle Handle to free
     */
    void ots_free_handle(ots_handle_t** handle);

    /**
     * @brief Securely wipe and free a buffer
     * @param[in,out] buffer Buffer to wipe and free
     * @param[in] size Size of buffer
     */
    void ots_secure_free(void** buffer, size_t size);

    /**
     * @brief Create a new wipeable string
     * @param[in] str Initial string content
     * @return Result containing wipeable string
     */
    ots_result_t* ots_wipeable_string_create(const char* str);

    /**
     * @brief Compare two wipeable strings
     * @param[in] str1 First string
     * @param[in] str2 Second string
     * @return Result containing comparison result, bool true if equal, number comparison result
     */
    ots_result_t* ots_wipeable_string_compare(
            const ots_handle_t* str1,
            const ots_handle_t* str2
            );

    /**
     * @brief Get string content of wipeable string
     * @param[in] str Wipeable string handle
     * @return C-string content of wipeable string or NULL
     */
    const char* ots_wipeable_string_c_str(const ots_handle_t* str);

    /**
     * @brief Create seed indices container
     * @param[in] size Number of indices
     * @return Result containing seed indices handle
     */
    ots_result_t* ots_seed_indices_create(uint16_t* indices, size_t size);

    /**
     * @brief Create seed indices container from string
     * @param[in] str String containing indices, 4 digits per 2 bytes with leading zero
     * @param[in] separator separator between indices, set "" for default behavior
     * @return Result containing seed indices handle
     * @see ots::SeedIndices::fromNumeric()
     */
    ots_result_t* ots_seed_indices_create_from_string(const char* str, const char* separator);

    /**
     * @brief Create seed indices container from hex string
     * @param[in] hex Hex string containing indices, 2 digits per 2 bytes with leading zero
     * @param[in] separator separator between indices, set "" for default behavior
     * @return Result containing seed indices handle
     * @see ots::SeedIndices::fromHex()
     */
    ots_result_t* ots_seed_indices_create_from_hex(const char* hex, const char* separator);

    /**
     * @brief Get seed indices values
     * @param[in] handle Seed indices handle
     * @return Pointer to indices array or NULL
     */
    const uint16_t* ots_seed_indices_values(const ots_handle_t* handle);

    /**
     * @brief Get seed indices count
     * @param[in] handle Seed indices handle
     * @return Number of indices
     */
    size_t ots_seed_indices_count(const ots_handle_t* handle);

    /**
     * @brief Clear seed indices
     * @param[in] handle Seed indices handle
     */
    void ots_seed_indices_clear(const ots_handle_t* handle);

    /**
     * @brief Append seed indices
     * @param[in] handle Seed indices handle
     * @param[in] value Index value to append
     */
    void ots_seed_indices_append(const ots_handle_t* handle, uint16_t value);

    /**
     * @brief Get numeric seed indices
     * @param[in] handle Seed indices handle
     * @param[in] separator separator between indices, set "" for default behavior
     * @return Numeric representation of seed indices or NULL
     */
    char* ots_seed_indices_numeric(const ots_handle_t* handle, const char* separator);

    /**
     * @brief Get hex seed indices
     * @param[in] handle Seed indices handle
     * @param[in] separator separator between indices, set "" for default behavior
     * @return Hex representation of seed indices or NULL
     */
    char* ots_seed_indices_hex(const ots_handle_t* handle, const char* separator);

    /*******************************************************************************
     * Seed Management Functions
     ******************************************************************************/

    /**
     * @brief Get all languages available
     * @return Result containing array of language handles
     */
    ots_result_t* ots_seed_languages(void);

    /**
     * @brief Get supported languages for seed type
     * @param[in] type Seed type
     * @return Result containing array of language codes
     */
    ots_result_t* ots_seed_languages_for_type(OTS_SEED_TYPE type);

    /**
     * @brief Get language from language code
     * @param[in] code Language code
     * @return Result containing language handle
     */
    ots_result_t* ots_seed_language_for_code(const char* code);

    /**
     * @brief Get language from language name
     * @param[in] name Language name
     * @return Result containing language handle
     */
    ots_result_t* ots_seed_language_for_name(const char* name);

    /**
     * @brief Get language from English language name
     * @param[in] name English language name
     * @return Result containing language handle
     */
    ots_result_t* ots_seed_language_for_english_name(const char* name);

    /**
     * @brief Get default language for seed type
     * @param[in] type Seed type
     * @return Result containing language handle
     */
    ots_result_t* ots_seed_language_default(OTS_SEED_TYPE type);

    /**
     * @brief Set default language for seed type
     * @param[in] type Seed type
     * @param[in] language Language handle
     * @return Result containing language handle for now current default
     * @note Only need to `check ots_is_result(handle)` or `!ots_is_error(handle)`
     */
    ots_result_t* ots_seed_language_set_default(
            OTS_SEED_TYPE type,
            const ots_handle_t* language
            );

    /**
     * @brief Seed language code
     * @param[in] language Language handle
     * @return Result containing language code
     */
    ots_result_t* ots_seed_language_from_code(const char* code);

    /**
     * @brief Seed language name
     * @param[in] language Language handle
     * @return Result containing language name
     */
    ots_result_t* ots_seed_language_from_name(const char* name);

    /**
     * @brief Seed language English name
     * @param[in] language Language handle
     * @return Result containing English language name
     */
    ots_result_t* ots_seed_language_from_english_name(const char* name);

    /**
     * @brief Seed language code
     * @param[in] language Language handle
     * @return Result containing language code
     */
    ots_result_t* ots_seed_language_code(const ots_handle_t* language);

    /**
     * @brief Seed language name
     * @param[in] language Language handle
     * @return Result containing language name
     */
    ots_result_t* ots_seed_language_name(const ots_handle_t* language);

    /**
     * @brief Seed language English name
     * @param[in] language Language handle
     * @return Result containing English language name
     */
    ots_result_t* ots_seed_language_english_name(const ots_handle_t* language);

    /**
     * @brief Seed language supported for seed type
     * @param[in] language Language handle
     * @param[in] type Seed type
     * @return true if language supports seed type
     */
    ots_result_t* ots_seed_language_supported(
            const ots_handle_t* language,
            OTS_SEED_TYPE type
            );

    /**
     * @brief Seed language is default for seed type
     * @param[in] language Language handle
     * @param[in] type Seed type
     * @return true if language is default for seed type
     */
    ots_result_t* ots_seed_language_is_default(
            const ots_handle_t* language,
            OTS_SEED_TYPE type
            );

    /**
     * @brief Seed language equals
     * @param[in] language1 First language handle
     * @param[in] language2 Second language handle
     * @return true if languages are equal
     */
    ots_result_t* ots_seed_language_equals(
            const ots_handle_t* language1,
            const ots_handle_t* language2
            );

    /**
     * @brief Seed language equals code
     * @param[in] language Language handle
     * @param[in] code Language code
     * @return true if language code matches
     */
    ots_result_t* ots_seed_language_equals_code(
            const ots_handle_t* language,
            const char* code
            );

    /**
     * @brief Get seed phrase in specified language
     * @param[in] handle Seed handle
     * @param[in] language Language handle
     * @param[in] password Optional password for encrypted seeds (empty string for none)
     * @return Result containing wipeable string
     */
    ots_result_t* ots_seed_phrase(
            const ots_handle_t* seed,
            const ots_handle_t* language,
            const char* password
            );

    /**
     * @brief Get seed phrase in specified language
     * @param[in] handle Seed handle
     * @param[in] language_code Language code
     * @param[in] password Optional password for encrypted seeds (empty string for none)
     * @return Result containing wipeable string
     */
    ots_result_t* ots_seed_phrase_for_language_code(
            const ots_handle_t* seed,
            const char* language_code,
            const char* password
            );

    /**
     * @brief Get seed indices
     * @param[in] handle Seed handle
     * @param[in] password Optional password for encrypted seeds
     * @return Result containing seed indices
     */
    ots_result_t* ots_seed_indices(
            const ots_handle_t* handle,
            const char* password
            );

    /**
     * @brief Get seed fingerprint
     * @param[in] handle Seed handle
     * @return Result containing fingerprint string
     */
    ots_result_t* ots_seed_fingerprint(const ots_handle_t* handle);

    /**
     * @brief Get seed address
     * @param[in] handle Seed handle
     * @return Result containing address handle
     */
    ots_result_t* ots_seed_address(const ots_handle_t* handle);

    /**
     * @brief Get seed creation timestamp
     * @param[in] handle Seed handle
     * @return Result containing timestamp
     */
    ots_result_t* ots_seed_timestamp(const ots_handle_t* handle);

    /**
     * @brief Get seed blockchain height
     * @param[in] handle Seed handle
     * @return Result containing height
     */
    ots_result_t* ots_seed_height(const ots_handle_t* handle);

    /**
     * @brief Get seed network type
     * @param[in] handle Seed handle
     * @return Result containing network type
     */
    ots_result_t* ots_seed_network(const ots_handle_t* handle);

    /**
     * @brief Get wallet from seed
     * @param[in] handle Seed handle
     * @return Result containing wallet handle
     */
    ots_result_t* ots_seed_wallet(const ots_handle_t* handle);

    /**
     * @brief Merge two sets of seed values
     * @param[in] seed_indices1 handle of seed indices for first set of values
     * @param[in] seed_indices2 handle of seed indices for second set of values
     * @return Result containing merged indices
     * @throws OTS_ERROR_LENGTH_MISMATCH if value sets have different sizes
     */
    ots_result_t* ots_seed_indices_merge_values(
            const ots_handle_t* seed_indices1,
            const ots_handle_t* seed_indices2
            );

    /**
     * @brief Merge seed values with password
     * @param[in] password Password to merge with
     * @param[in] seed_indices Seed values to merge
     * @return Result containing merged indices
     * @throws OTS_ERROR_MERGE_FAILED if merge operation fails
     */
    ots_result_t* ots_seed_indices_merge_with_password(
            const char* password,
            const ots_handle_t* seed_indices
            );

    /**
     * @brief Merge multiple sets of seed values
     * @param[in] seed_indices Array of value seed indices handles with sets to merge
     * @param[in] elements Number of elements in each value set
     * @param[in] count Number of value sets
     * @return Result containing merged indices
     * @throws OTS_ERROR_LENGTH_MISMATCH if value sets have different sizes
     * @throws OTS_ERROR_TOO_FEW_VALUES if less than two value sets provided
     */
    ots_result_t* ots_seed_indices_merge_multiple_values(
            const ots_handle_t* seed_indices[],
            size_t elements,
            size_t count
            );

    /**
     * @brief Merge and zero two sets of seed values
     * @param[in,out] values1 First set of values (will be zeroed)
     * @param[in,out] values2 Second set of values (will be zeroed)
     * @param[in] delete_after Delete values after merging
     * @return Result containing merged indices
     * @throws OTS_ERROR_LENGTH_MISMATCH if value sets have different sizes
     */
    ots_result_t* ots_seed_indices_merge_values_and_zero(
            const ots_handle_t* seed_indices1,
            const ots_handle_t* seed_indices2,
            bool delete_after
            );

    /**
     * @brief Merge seed values with password and zero
     * @param[in,out] password Password to merge with (will be zeroed)
     * @param[in,out] values Seed values to merge (will be zeroed)
     * @param[in] delete_after Delete values after merging
     * @return Result containing merged indices
     * @throws OTS_ERROR_MERGE_FAILED if merge operation fails
     */
    ots_result_t* ots_seed_indices_merge_values_with_password_and_zero(
            char* password,
            const ots_handle_t* seed_indices,
            bool delete_after
            );

    /**
     * @brief Merge multiple sets of seed values and zero
     * @param[in,out] values Array of value sets to merge (will be zeroed)
     * @param[in] elements Number of elements in each value set
     * @param[in] count Number of value sets
     * @param[in] delete_after Delete values after merging
     * @return Result containing merged indices
     * @throws OTS_ERROR_LENGTH_MISMATCH if value sets have different sizes
     * @throws OTS_ERROR_TOO_FEW_VALUES if less than two value sets provided
     */
    ots_result_t* ots_seed_indices_merge_multiple_values_and_zero(
            const ots_handle_t* seed_indices[],
            size_t elements,
            size_t count,
            bool delete_after
            );

    /*******************************************************************************
     * Legacy Seed Functions
     ******************************************************************************/

    /**
     * @brief Decode a legacy (13 word) seed from phrase
     * @param[in] phrase The seed phrase
     * @param[in] height Optional blockchain height (0 for none)
     * @param[in] time Optional timestamp (0 for none)
     * @param[in] network Network type (default: MAIN)
     * @return Result containing seed handle
     * @throws OTS_ERROR_INVALID_SEED if decoding fails
     */
    ots_result_t* ots_legacy_seed_decode(
            const char* phrase,
            uint64_t height,
            uint64_t time,
            OTS_NETWORK network
            );

    /**
     * @brief Decode a legacy seed from indices
     * @param[in] indices Array of seed word indices
     * @param[in] height Optional blockchain height (0 for none)
     * @param[in] time Optional timestamp (0 for none)
     * @param[in] network Network type (default: MAIN)
     * @return Result containing seed handle
     * @throws OTS_ERROR_INVALID_SEED if decoding fails
     */
    ots_result_t* ots_legacy_seed_decode_indices(
            const ots_handle_t* indices,
            uint64_t height,
            uint64_t time,
            OTS_NETWORK network
            );

    /*******************************************************************************
     * Monero Seed Functions
     ******************************************************************************/

    /**
     * @brief Create a Monero seed from random data
     * @param[in] random 32-byte random input
     * @param[in] height Optional blockchain height (0 for none)
     * @param[in] time Optional timestamp (0 for none)
     * @param[in] network Network type
     * @return Result containing seed handle
     * @throws OTS_ERROR_INVALID_INPUT if random data is invalid
     * @note height or timestamp must be set to 0
     */
    ots_result_t* ots_monero_seed_create(
            const uint8_t random[32],
            uint64_t height,
            uint64_t time,
            OTS_NETWORK network
            );

    /**
     * @brief Generate a new Monero seed
     * @param[in] height Optional blockchain height (0 for none)
     * @param[in] time Optional timestamp (0 for none)
     * @param[in] network Network type
     * @return Result containing seed handle
     * @note height or timestamp must be set to 0
     */
    ots_result_t* ots_monero_seed_generate(
            uint64_t height,
            uint64_t time,
            OTS_NETWORK network
            );

    /**
     * @brief Decode a Monero seed from phrase
     * @param[in] phrase The seed phrase
     * @param[in] height Optional blockchain height (0 for none)
     * @param[in] time Optional timestamp (0 for none)
     * @param[in] network Network type (default: MAIN)
     * @param[in] passphrase Optional passphrase (empty string for none)
     * @return Result containing seed handle
     * @throws OTS_ERROR_INVALID_SEED if decoding fails
     */
    ots_result_t* ots_monero_seed_decode(
            const char* phrase,
            uint64_t height,
            uint64_t time,
            OTS_NETWORK network,
            const char* passphrase
            );

    /**
     * @brief Decode a Monero seed from indices
     * @param[in] indices Array of seed word indices
     * @param[in] height Optional blockchain height (0 for none)
     * @param[in] time Optional timestamp (0 for none)
     * @param[in] network Network type (default: MAIN)
     * @param[in] passphrase Optional passphrase (empty string for none)
     * @return Result containing seed handle
     * @throws OTS_ERROR_INVALID_SEED if decoding fails
     */
    ots_result_t* ots_monero_seed_decode_indices(
            const ots_handle_t* indices,
            uint64_t height,
            uint64_t time,
            OTS_NETWORK network,
            const char* passphrase
            );

    /*******************************************************************************
     * Polyseed Functions
     ******************************************************************************/

    /**
     * @brief Create a Polyseed from random data
     * @param[in] random 19-byte random input
     * @param[in] network Network type (default: MAIN)
     * @param[in] time Optional timestamp (0 for current time)
     * @param[in] passphrase Optional passphrase for seed offset (empty string for none)
     * @return Result containing seed handle
     * @throws OTS_ERROR_INVALID_INPUT if random data is invalid
     */
    ots_result_t* ots_polyseed_create(
            const uint8_t random[19],
            OTS_NETWORK network,
            uint64_t time,
            const char* passphrase
            );

    /**
     * @brief Generate a new Polyseed
     * @param[in] network Network type (default: MAIN)
     * @param[in] time Optional timestamp (0 for current time)
     * @param[in] passphrase Optional passphrase for seed offset (empty string for none)
     * @return Result containing seed handle
     */
    ots_result_t* ots_polyseed_generate(
            OTS_NETWORK network,
            uint64_t time,
            const char* passphrase
            );

    /**
     * @brief Decode a Polyseed from phrase
     * @param[in] phrase The seed phrase
     * @param[in] network Network type (default: MAIN)
     * @param[in] password Optional decryption password (empty string for none)
     * @param[in] passphrase Optional passphrase for seed offset (empty string for none)
     * @return Result containing seed handle
     * @throws OTS_ERROR_INVALID_SEED if decoding fails
     */
    ots_result_t* ots_polyseed_decode(
            const char* phrase,
            OTS_NETWORK network,
            const char* password,
            const char* passphrase
            );

    /**
     * @brief Decode a Polyseed from indices
     * @param[in] indices Array of seed word indices
     * @param[in] network Network type
     * @param[in] password Optional decryption password (empty string for none)
     * @param[in] passphrase Optional passphrase for seed offset (empty string for none)
     * @return Result containing seed handle
     * @throws OTS_ERROR_INVALID_SEED if decoding fails
     */
    ots_result_t* ots_polyseed_decode_indices(
            const ots_handle_t* indices,
            OTS_NETWORK network,
            const char* password,
            const char* passphrase
            );

    /**
     * @brief Decode a Polyseed from phrase with specific language
     * @param[in] phrase The seed phrase
     * @param[in] language_code Language code
     * @param[in] network Network type
     * @param[in] password Optional decryption password (empty string for none)
     * @param[in] passphrase Optional passphrase for seed offset (empty string for none)
     * @return Result containing seed handle
     * @throws OTS_ERROR_INVALID_SEED if decoding fails
     */
    ots_result_t* ots_polyseed_decode_with_language(
            const char* phrase,
            const ots_handle_t* language,
            OTS_NETWORK network,
            const char* password,
            const char* passphrase
            );

    /**
     * @brief Decode a Polyseed from phrase with specific language code
     * @param[in] phrase The seed phrase
     * @param[in] language_code Language code
     * @param[in] network Network type
     * @param[in] password Optional decryption password (empty string for none)
     * @param[in] passphrase Optional passphrase for seed offset (empty string for none)
     * @return Result containing seed handle
     * @throws OTS_ERROR_INVALID_SEED if decoding fails
     */
    ots_result_t* ots_polyseed_decode_with_language_code(
            const char* phrase,
            const char* language_code,
            OTS_NETWORK network,
            const char* password,
            const char* passphrase
            );

    /*******************************************************************************
     * Address Management Functions
     ******************************************************************************/

    /**
     * @brief Create address object from string
     * @param[in] address Address string
     * @return Result containing address handle
     * @throws OTS_ERROR_INVALID_ADDRESS if address is invalid
     * @note This creates a managed address object that can be used with other functions
     */
    ots_result_t* ots_address_create(const char* address);

    /**
     * @brief Get address type
     * @param[in] address Address to check
     * @return Result containing address type
     */
    ots_result_t* ots_address_type(const ots_handle_t* address);

    /**
     * @brief Get network type for an address
     * @param[in] address The address to check
     * @return Result containing network type
     */
    ots_result_t* ots_address_network(const ots_handle_t* address);

    /**
     * @brief Generate fingerprint for an address
     * @param[in] address The address to generate fingerprint for
     * @return Result containing fingerprint string
     * @note Fingerprint is the last 6 digits of sha256(address) as uppercase hex
     */
    ots_result_t* ots_address_fingerprint(const ots_handle_t* address);

    /**
     * @brief Check if address is an integrated address
     * @param[in] address The address to check
     * @return Result containing boolean status
     */
    ots_result_t* ots_address_is_integrated(const ots_handle_t* address);

    /**
     * @brief Extract payment ID from integrated address
     * @param[in] address The integrated address
     * @return Result containing payment ID string
     */
    ots_result_t* ots_address_payment_id(const ots_handle_t* address);

    /**
     * @brief Get base address from integrated address
     * @param[in] address The integrated address
     * @return Result containing base address string
     * @throws OTS_ERROR_INVALID_ADDRESS if address is invalid
     * @throws OTS_ERROR_NOT_INTEGRATED if address is not an integrated address
     */
    ots_result_t* ots_address_from_integrated(const ots_handle_t* address);

    /**
     * @brief Get address length
     * @param[in] address The address to check
     * @return Result containing address length
     * @throws OTS_ERROR_INVALID_ADDRESS if address is invalid
     */
    ots_result_t* ots_address_length(const ots_handle_t* address);

    /**
     * @brief Get base58 string representation of address
     * @param[in] address_handle Address handle
     * @return Result containing address string
     */
    ots_result_t* ots_address_base58_string(const ots_handle_t* address_handle);

    /**
     * @brief Compare two addresses for equality
     * @param[in] address1 First address
     * @param[in] address2 Second address
     * @return Result containing boolean equality status
     * @throws OTS_ERROR_INVALID_ADDRESS if either address is invalid
     */
    ots_result_t* ots_address_equal(
            const ots_handle_t* address1,
            const ots_handle_t* address2
            );

    /**
     * @brief Compare address handle with string
     * @param[in] address_handle Address handle
     * @param[in] address_string Address string to compare
     * @return Result containing boolean equality status
     */
    ots_result_t* ots_address_equal_string(
            const ots_handle_t* address_handle,
            const char* address_string
            );

    /**
     * @brief Validate a Monero address
     * @param[in] address Address to validate
     * @param[in] network Network to validate against
     * @return Result containing validation status
     */
    ots_result_t* ots_address_string_valid(const char* address, OTS_NETWORK network);

    /**
     * @brief Get network type for an address string
     * @param[in] address The address string to check
     * @return Result containing network type
     * @throws OTS_ERROR_INVALID_ADDRESS if address is invalid
     */
    ots_result_t* ots_address_string_network(const char* address);

    /**
     * @brief Get type for an address string
     * @param[in] address The address string to check
     * @return Result containing address type
     * @throws OTS_ERROR_INVALID_ADDRESS if address is invalid
     */
    ots_result_t* ots_address_string_type(const char* address);

    /**
     * @brief Generate fingerprint for an address string
     * @param[in] address The address string
     * @return Result containing fingerprint string
     * @throws OTS_ERROR_INVALID_ADDRESS if address is invalid
     */
    ots_result_t* ots_address_string_fingerprint(const char* address);

    /**
     * @brief Check if address string is integrated
     * @param[in] address The address string to check
     * @return Result containing boolean status
     * @throws OTS_ERROR_INVALID_ADDRESS if address is invalid
     */
    ots_result_t* ots_address_string_is_integrated(const char* address);

    /**
     * @brief Extract payment ID from integrated address string
     * @param[in] address The integrated address string
     * @return Result containing payment ID string
     * @throws OTS_ERROR_INVALID_ADDRESS if address is invalid
     * @throws OTS_ERROR_NOT_INTEGRATED if address is not integrated
     */
    ots_result_t* ots_address_string_payment_id(const char* address);

    /**
     * @brief Get base address from integrated address string
     * @param[in] address The integrated address string
     * @return Result containing base address string
     * @throws OTS_ERROR_INVALID_ADDRESS if address is invalid
     * @throws OTS_ERROR_NOT_INTEGRATED if address is not integrated
     */
    ots_result_t* ots_address_string_integrated(const char* address);


    /*******************************************************************************
     * Wallet Functions
     ******************************************************************************/

    /**
     * @brief Create wallet from secret key
     * @param[in] key 32-byte secret key
     * @param[in] height Restore height
     * @param[in] network Network type
     * @return Result containing wallet handle
     */
    ots_result_t* ots_wallet_create(
            const uint8_t key[32],
            uint64_t height,
            OTS_NETWORK network
            );

    /**
     * @brief Get wallet restore height
     * @param[in] wallet Wallet handle
     * @return Result containing height
     */
    ots_result_t* ots_wallet_height(const ots_handle_t* wallet);

    /**
     * @brief Generate address for wallet
     * @param[in] wallet Wallet handle
     * @return Result containing wallet address
     */
    ots_result_t* ots_wallet_address(const ots_handle_t* wallet);

    /**
     * @brief Generate subaddress for wallet
     * @param[in] wallet Wallet handle
     * @param[in] account Account index
     * @param[in] index Address index
     * @return Result containing address
     */
    ots_result_t* ots_wallet_subaddress(
            const ots_handle_t* wallet,
            uint32_t account,
            uint32_t index
            );

    /**
     * @brief Get list of accounts in wallet
     * @param[in] wallet Wallet handle
     * @param[in] max Maximum number of accounts to return
     * @param[in] offset Starting account index
     * @return Result containing array of address handles
     */
    ots_result_t* ots_wallet_accounts(
            const ots_handle_t* wallet,
            uint32_t max,
            uint32_t offset
            );

    /**
     * @brief Get list of subaddresses for an account
     * @param[in] wallet Wallet handle
     * @param[in] account Account index
     * @param[in] max Maximum number of addresses to return
     * @param[in] offset Starting subaddress index
     * @return Result containing array of address handles
     */
    ots_result_t* ots_wallet_subaddresses(
            const ots_handle_t* wallet,
            uint32_t account,
            uint32_t max,
            uint32_t offset
            );

    /**
     * @brief Check if address belongs to wallet using Address handle
     * @param[in] wallet Wallet handle
     * @param[in] address Address handle to check
     * @param[in] max_account_depth Maximum account depth to search
     * @param[in] max_index_depth Maximum index depth to search
     * @return Result containing boolean status
     */
    ots_result_t* ots_wallet_has_address(
            const ots_handle_t* wallet,
            const ots_handle_t* address,
            uint32_t max_account_depth,
            uint32_t max_index_depth
            );

    /**
     * @brief Check if address belongs to wallet
     * @param[in] wallet Wallet handle
     * @param[in] address address string to check
     * @param[in] max_account_depth Maximum account depth to search
     * @param[in] max_index_depth Maximum index depth to search
     * @return Result containing boolean status
     */
    ots_result_t* ots_wallet_has_address_string(
            const ots_handle_t* wallet_handle,
            const char* address,
            uint32_t max_account_depth,
            uint32_t max_index_depth
            );

    /**
     * @brief Get account and index for address handle in wallet
     * @param[in] wallet Wallet handle
     * @param[in] address Address handle to look up
     * @param[in] max_account_depth Maximum account depth to search
     * @param[in] max_index_depth Maximum index depth to search
     * @return Result containing account/index array[2]
     * @throws OTS_ERROR_ADDRESS_NOT_FOUND if address not found in wallet
     */
    ots_result_t* ots_wallet_address_index(
            const ots_handle_t* wallet,
            const ots_handle_t* address,
            uint32_t max_account_depth,
            uint32_t max_index_depth
            );

    /**
     * @brief Get account and index for address in wallet
     * @param[in] wallet Wallet handle
     * @param[in] address Address string to look up
     * @param[in] max_account_depth Maximum account depth to search
     * @param[in] max_index_depth Maximum index depth to search
     * @return Result containing account/index pair
     * @throws OTS_ERROR_ADDRESS_NOT_FOUND if address not found in wallet
     */
    ots_result_t* ots_wallet_address_string_index(
            const ots_handle_t* wallet_handle,
            const char* address,
            uint32_t max_account_depth,
            uint32_t max_index_depth
            );

    /**
     * @brief Get secret view key
     * @param[in] wallet Wallet handle
     * @return Result containing wipeable string with key
     */
    ots_result_t* ots_wallet_secret_view_key(const ots_handle_t* wallet);

    /**
     * @brief Get public view key
     * @param[in] wallet Wallet handle
     * @return Result containing wipeable string with key
     */
    ots_result_t* ots_wallet_public_view_key(const ots_handle_t* wallet);

    /**
     * @brief Get secret spend key
     * @param[in] wallet_handle Wallet handle
     * @return Result containing wipeable string with key
     */
    ots_result_t* ots_wallet_secret_spend_key(const ots_handle_t* wallet);

    /**
     * @brief Get public spend key
     * @param[in] wallet_handle Wallet handle
     * @return Result containing wipeable string with key
     */
    ots_result_t* ots_wallet_public_spend_key(const ots_handle_t* wallet);

    /**
     * @brief Import outputs from string
     * @param[in] wallet Wallet handle
     * @param[in] outputs Outputs string from view wallet
     * @return Result containing number of imported outputs
     * @throws OTS_ERROR_INVALID_OUTPUTS if outputs data is invalid
     * @throws OTS_ERROR_RANGE_ERROR if imported outputs are bigger then 9,223,372,036,854,775,807 (should never happen IMO), if so, contact me and kick me for this stupid decision
     */
    ots_result_t* ots_wallet_import_outputs(
            const ots_handle_t* wallet,
            const char* outputs
            );

    /**
     * @brief Export key images
     * @param[in] wallet Wallet handle
     * @return Result containing wipeable string with key images
     * @throws OTS_ERROR_NO_KEY_IMAGES if no outputs were imported
     */
    ots_result_t* ots_wallet_export_key_images(const ots_handle_t* wallet);

    /**
     * @brief Describe unsigned transaction
     * @param[in] wallet Wallet handle
     * @param[in] unsigned_tx Unsigned transaction data
     * @return Result containing transaction description
     */
    ots_result_t* ots_wallet_describe_tx(const ots_handle_t* wallet, const char* unsigned_tx);

    /**
     * @brief Check transaction for warnings
     * @param[in] wallet Wallet handle
     * @param[in] unsigned_tx unsigned transaction handle
     * @return Result containing array of warnings
     */
    ots_result_t* ots_wallet_check_tx(const ots_handle_t* wallet, const ots_handle_t* unsigned_tx);

    /**
     * @brief Check transaction string for warnings
     * @param[in] wallet Wallet handle
     * @param[in] unsigned_tx Unsigned transaction string
     * @return Result containing array of warnings
     */
    ots_result_t* ots_wallet_check_tx_string(
            const ots_handle_t* wallet,
            const char* unsigned_tx
            );

    /**
     * @brief Sign unsigned transaction
     * @param[in] wallet Wallet handle
     * @param[in] unsigned_tx Unsigned transaction string
     * @return Result containing signed transaction string
     * @throws OTS_ERROR_INVALID_TRANSACTION if transaction is invalid
     */
    ots_result_t* ots_wallet_sign_transaction(
            const ots_handle_t* wallet,
            const char* unsigned_tx
            );

    /**
     * @brief Sign arbitrary data
     * @param[in] wallet Wallet handle
     * @param[in] data Data to sign
     * @return Result containing signature
     */
    ots_result_t* ots_wallet_sign_data(
            const ots_handle_t* wallet,
            const char* data
            );

    /**
     * @brief Sign data with specific subaddress
     * @param[in] wallet Wallet handle
     * @param[in] data Data to sign
     * @param[in] account Account index
     * @param[in] subaddr Subaddress index
     * @return Result containing signature string
     */
    ots_result_t* ots_wallet_sign_data_with_index(
            const ots_handle_t* wallet,
            const char* data,
            uint32_t account,
            uint32_t subaddr
            );

    /**
     * @brief Sign data with specific address
     * @param[in] wallet Wallet handle
     * @param[in] data Data to sign
     * @param[in] address Address to sign with
     * @return Result containing signature string
     * @throws OTS_ERROR_ADDRESS_NOT_FOUND if address not found in wallet
     */
    ots_result_t* ots_wallet_sign_data_with_address(
            const ots_handle_t* wallet,
            const char* data,
            const ots_handle_t* address
            );

    /**
     * @brief Sign data with specific address
     * @param[in] wallet Wallet handle
     * @param[in] data Data to sign
     * @param[in] address Address string to sign with
     * @return Result containing signature string
     * @throws OTS_ERROR_ADDRESS_NOT_FOUND if address not found in wallet
     */
    ots_result_t* ots_wallet_sign_data_with_address_string(
            const ots_handle_t* wallet,
            const char* data,
            const char* address
            );

    /**
     * @brief Verify signed data for the wallet address with legacy support
     * @param[in] wallet Wallet handle
     * @param[in] data Original data
     * @param[in] signature Signature to verify
     * @param[in] legacy_fallback Try legacy verification if modern fails
     * @return Result containing verification status
     */
    ots_result_t* ots_wallet_verify_data(
            const ots_handle_t* wallet,
            const char* data,
            const char* signature,
            bool legacy_fallback
            );

    /**
     * @brief Verify signed data with specific subaddress
     * @param[in] wallet Wallet handle
     * @param[in] data Original data
     * @param[in] account Account index  
     * @param[in] subaddr Subaddress index
     * @param[in] signature Signature to verify
     * @param[in] legacy_fallback Try legacy verification if modern fails
     * @return Result containing verification status
     */
    ots_result_t* ots_wallet_verify_data_with_index(
            const ots_handle_t* wallet,
            const char* data,
            uint32_t account,
            uint32_t subaddr,
            const char* signature,
            bool legacy_fallback
            );

    /**
     * @brief Verify signed data with specific address
     * @param[in] wallet Wallet handle
     * @param[in] data Original data
     * @param[in] address Address handle to verify with
     * @param[in] signature Signature to verify
     * @param[in] legacy_fallback Try legacy verification if modern fails
     * @return Result containing verification status
     * @throws OTS_ERROR_ADDRESS_NOT_FOUND if address not found in wallet
     */
    ots_result_t* ots_wallet_verify_data_with_address(
            const ots_handle_t* wallet,
            const char* data,
            const ots_handle_t* address,
            const char* signature,
            bool legacy_fallback
            );

    /**
     * @brief Verify signed data with specific address
     * @param[in] wallet Wallet handle
     * @param[in] data Original data
     * @param[in] address Address string to verify with
     * @param[in] signature Signature to verify
     * @param[in] legacy_fallback Try legacy verification if modern fails
     * @return Result containing verification status
     * @throws OTS_ERROR_ADDRESS_NOT_FOUND if address not found in wallet
     */
    ots_result_t* ots_wallet_verify_data_with_address_string(
            const ots_handle_t* wallet,
            const char* data,
            const char* address,
            const char* signature,
            bool legacy_fallback
            );

    /*******************************************************************************
     * Seed Jar Functions
     ******************************************************************************/

    /**
     * @brief Add a seed to the seed jar
     * @param[in|out] seed Seed handle, will be set to reference only
     * @param[in] name Optional seed name, empty string for none
     * @return Result containing seed jar handle, esentially the same as the seed handle
     */
    ots_result_t* ots_seed_jar_add_seed(ots_handle_t* seed, const char* name);

    /**
     * @brief Remove a seed from the seed jar
     * @param[in|out] seed Seed handle, will be freed
     * @return Result true on success
     */
    ots_result_t* ots_seed_jar_remove_seed(ots_handle_t** seed);

    /**
     * @brief Move the seed from the handle to the jar
     * @param[in|out] seed Seed handle, will be freed
     * @return Result with reference handle to seed in jar
     */
    ots_result_t* ots_seed_jar_transfer_seed_in(ots_handle_t** seed, const char* name);

    /**
     * @brief Move the seed from the jar to a new handle
     * @param[in|out] seed Seed handle, will be freed
     * @return Result with handle to seed before in jar
     * @note The seed will be removed from the jar, the
     *       provided handle needs now to be freed with
     *       `ots_free_handle` if not needed anymore.
     */
    ots_result_t* ots_seed_jar_transfer_seed_out(ots_handle_t** seed);

    /**
     * @brief Clear and free all seeds from the seed jar
     * @return Result true on success
     */
    ots_result_t* ots_seed_jar_clear(void);

    /**
     * @brief Get a list of all seeds as handle array
     * @return Result containing array of seed handles
     */
    ots_result_t* ots_seed_jar_seeds(void);

    /**
     * @brief Get the number of seeds in the seed jar
     * @return Result containing seed count
     */
    ots_result_t* ots_seed_jar_seed_count(void);

    /**
     * @brief Get seed with the given fingerprint
     * @param[in] fingerprint Fingerprint to search for
     * @return Result containing seed handle
     */
    ots_result_t* ots_seed_jar_seed_for_fingerprint(const char* fingerprint);

    /**
     * @brief Get seed with the given address
     * @param[in] address Address to search for
     * @return Result containing seed handle
     */
    ots_result_t* ots_seed_jar_seed_for_address(const char* address);

    /**
     * @brief Get seed with the given name
     * @param[in] name Name to search for
     * @return Result containing seed handle
     */
    ots_result_t* ots_seed_jar_seed_for_name(const char* name);

    /**
     * @brief Get seed name
     * @param[in] seed Seed handle
     * @return Result containing seed name
     */
    ots_result_t* ots_seed_jar_seed_name(const ots_handle_t* seed);

    /**
     * @brief Rename seed in the seed jar
     * @param[in] seed Seed handle
     * @param[in] name New name
     * @return Result containing seed handle
     */
    ots_result_t* ots_seed_jar_seed_rename(const ots_handle_t* seed, const char* name);

    /*******************************************************************************
     * OTS Utility Functions
     ******************************************************************************/

    /**
     * @brief Get library version string
     * @return Version string, must be freed with ots_free_string()
     */
    ots_result_t* ots_version(void);

    /**
     * @brief Get version components [major, minor, patch]
     * @return Result containing version components array
     */
    ots_result_t* ots_version_components(void);

    /**
     * @brief Convert timestamp to estimated block height
     * @param[in] timestamp Unix timestamp
     * @param[in] network Network type
     * @return Result containing estimated block height
     */
    ots_result_t* ots_height_from_timestamp(
            uint64_t timestamp,
            OTS_NETWORK network
            );

    /**
     * @brief Convert block height to estimated timestamp
     * @param[in] height Block height
     * @param[in] network Network type
     * @return Result containing estimated timestamp
     */
    ots_result_t* ots_timestamp_from_height(
            uint64_t height,
            OTS_NETWORK network
            );

    /**
     * @brief Generate random bytes
     * @param[in] size Number of bytes to generate
     * @return Result random bytes as array
     * @warning Entropy quality depends on system random number generator
     */
    ots_result_t* ots_random_bytes(size_t size);

    /**
     * @brief Generate 32 random bytes
     * @return Result containing 32-byte array
     * @warning Entropy quality depends on system random number generator
     */
    ots_result_t* ots_random_32(void);

    /**
     * @brief Check data for low entropy level
     * @param[in] data Data to check
     * @param[in] size Size of data
     * @param[in] min_entropy Minimum required entropy
     * @return Result containing boolean (true if entropy is sufficient)
     */
    ots_result_t* ots_check_low_entropy(
            const uint8_t* data,
            size_t size,
            double min_entropy
            );

    /**
     * @brief Set entropy enforcement
     * @param[in] enforce Whether to enforce entropy checks
     */
    void ots_set_enforce_entropy(bool enforce);

    /**
     * @brief Set maximum account depth for searching
     * @param[in] depth Maximum account depth
     */
    void ots_set_max_account_depth(uint32_t depth);

    /**
     * @brief Set maximum index depth for searching
     * @param[in] depth Maximum index depth
     */
    void ots_set_max_index_depth(uint32_t depth);

    /**
     * @brief Set maximum depths for searching
     * @param[in] account_depth Maximum account depth
     * @param[in] index_depth Maximum index depth
     */
    void ots_set_max_depth(uint32_t account_depth, uint32_t index_depth);

    /**
     * @brief Reset maximum depths to defaults
     */
    void ots_reset_max_depth(void);

    /**
     * @brief Get maximum account depth
     * @param[in] depth Optional new depth (0 to just query current)
     * @return Current/new maximum account depth
     */
    uint32_t ots_max_account_depth(uint32_t depth);

    /**
     * @brief Get maximum index depth
     * @param[in] depth Optional new depth (0 to just query current)
     * @return Current/new maximum index depth
     */
    uint32_t ots_max_index_depth(uint32_t depth);

    /**
     * @brief Verify signed data
     * @param[in] data Original data
     * @param[in] address Signer's address
     * @param[in] signature Signature to verify
     * @return Result containing verification status
     */
    ots_result_t* ots_verify_data(const char* data, const char* address, const char* signature);

#ifdef __cplusplus
}
#endif

#endif // OTS_H
