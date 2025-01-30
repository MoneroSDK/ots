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
 * @note All functions returning an ots_result_t must be checked for errors using
 *       ots_is_error() before accessing the result.
 */

/*******************************************************************************
 * Constants and Defines
 ******************************************************************************/

/** @brief Maximum length for error messages */
#define OTS_MAX_ERROR_MESSAGE 256
/** @brief Maximum length for error location strings */
#define OTS_MAX_ERROR_LOCATION 64
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
        OTS_HANDLE_SEED,
        OTS_HANDLE_WALLET,
        OTS_HANDLE_TX
    } ots_handle_type;

    /**
     * @brief Handle structure for C objects
     */
    typedef struct {
        ots_handle_type type;
        uint32_t version;
        void* ptr;
    } ots_handle_t;

    /** @brief Error structure for exception handling */
    typedef struct {
        int32_t code;                                 /**< Error code, 0 for success */
        char message[OTS_MAX_ERROR_MESSAGE];         /**< Error message */
        char location[OTS_MAX_ERROR_LOCATION];       /**< Error location */
    } ots_error_t;

    /** @brief Result structure combining handle and error */
    typedef struct {
        union {
            ots_handle_t handle;                     /**< Handle for created objects */
            void* ptr;                               /**< Pointer for returned data */
            bool boolean;                            /**< Boolean result */
            int64_t number;                          /**< Numeric result */
        } result;
        ots_error_t error;                          /**< Error information */
    } ots_result_t;

    /** @brief Wipeable string for sensitive data */
    typedef struct {
        char* data;                                  /**< String data */
        size_t length;                              /**< String length */
    } ots_wipeable_string_t;

    /** @brief Seed indices container */
    typedef struct {
        uint16_t* indices;                          /**< Array of indices */
        size_t count;                               /**< Number of indices */
    } ots_seed_indices_t;

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
    static inline bool ots_handle_valid(ots_handle_t h, ots_handle_type expected) {
        return h.type == expected && h.ptr != NULL;
    }

    /**
     * @brief Initialize error structure
     * @param[out] error Error structure to initialize
     */
    void ots_error_init(ots_error_t* error);

    /**
     * @brief Check if result contains an error
     * @param[in] result Result to check
     * @return true if result contains an error
     */
    bool ots_is_error(const ots_result_t* result);

    /**
     * @brief Get error message for last error
     * @return Result containing error message string
     */
    ots_result_t ots_get_last_error(void);

    /**
     * @brief Get error message for specific error code
     * @param[in] error_code Error code to get message for
     * @return Result containing error message string
     */
    ots_result_t ots_get_error_message(int32_t error_code);

    /**
     * @brief Clear last error state
     */
    void ots_clear_error(void);

    /**
     * @brief Check if result contains an error
     * @param[in] result Result to check
     * @return true if result contains an error
     */
    bool ots_has_error(const ots_result_t* result);

    /*******************************************************************************
     * Memory Management Functions
     ******************************************************************************/

    /**
     * @brief Free a wipeable string, securely wiping memory
     * @param[in,out] str String to free
     */
    void ots_wipeable_string_free(ots_wipeable_string_t* str);

    /**
     * @brief Free seed indices, securely wiping memory
     * @param[in,out] indices Indices to free
     */
    void ots_seed_indices_free(ots_seed_indices_t* indices);

    /**
     * @brief Free a string allocated by the library
     * @param[in] str String to free
     */
    void ots_free_string(char* str);

    /**
     * @brief Free an array allocated by the library
     * @param[in] arr Array to free
     */
    void ots_free_array(void* arr);

    /**
     * @brief Free a handle
     * @param[in] handle Handle to free
     */
    void ots_free_handle(ots_handle_t handle);

    /**
     * @brief Free transaction description
     * @param[in] desc Transaction description to free
     */
    void ots_free_tx_description(ots_tx_description_t* desc);

    /**
     * @brief Securely wipe and free a buffer
     * @param[in,out] buffer Buffer to wipe and free
     * @param[in] size Size of buffer
     */
    void ots_secure_free(void* buffer, size_t size);

    /**
     * @brief Create a new wipeable string
     * @param[in] str Initial string content
     * @return Result containing wipeable string
     */
    ots_result_t ots_wipeable_string_create(const char* str);

    /**
     * @brief Compare two wipeable strings
     * @param[in] str1 First string
     * @param[in] str2 Second string
     * @return Result containing comparison result
     */
    ots_result_t ots_wipeable_string_compare(
            const ots_wipeable_string_t* str1,
            const ots_wipeable_string_t* str2
            );

    /**
     * @brief Clear wipeable string content
     * @param[in,out] str String to clear
     */
    void ots_wipeable_string_clear(ots_wipeable_string_t* str);

    /**
     * @brief Create seed indices container
     * @param[in] size Number of indices
     * @return Result containing seed indices container
     */
    ots_result_t ots_seed_indices_create(size_t size);

    /**
     * @brief Clear seed indices content
     * @param[in,out] indices Indices to clear
     */
    void ots_seed_indices_clear(ots_seed_indices_t* indices);

    /**
     * @brief Get library version string
     * @return Version string, must be freed with ots_free_string()
     */
    ots_result_t ots_version(void);

    /**
     * @brief Get version components [major, minor, patch]
     * @return Result containing version components array
     */
    ots_result_t ots_version_components(void);

    /*******************************************************************************
     * Seed Management Functions
     ******************************************************************************/

    /**
     * @brief Get seed phrase in specified language
     * @param[in] handle Seed handle
     * @param[in] language_code Language code
     * @param[in] password Optional password for encrypted seeds (empty string for none)
     * @return Result containing wipeable string
     */
    ots_result_t ots_seed_phrase(
            ots_handle_t handle,
            const char* language_code,
            const char* password
            );

    /**
     * @brief Get supported languages for seed type
     * @param[in] type Seed type
     * @return Result containing array of language codes
     */
    ots_result_t ots_seed_languages(OTS_SEED_TYPE type);

    /**
     * @brief Get seed indices
     * @param[in] handle Seed handle
     * @param[in] password Optional password for encrypted seeds
     * @return Result containing seed indices
     */
    ots_result_t ots_seed_indices(
            ots_handle_t handle,
            const char* password
            );

    /**
     * @brief Get seed fingerprint
     * @param[in] handle Seed handle
     * @return Result containing fingerprint string
     */
    ots_result_t ots_seed_fingerprint(ots_handle_t handle);

    /**
     * @brief Get seed address
     * @param[in] handle Seed handle
     * @return Result containing address handle
     */
    ots_result_t ots_seed_address(ots_handle_t handle);

    /**
     * @brief Get seed creation timestamp
     * @param[in] handle Seed handle
     * @return Result containing timestamp
     */
    ots_result_t ots_seed_timestamp(ots_handle_t handle);

    /**
     * @brief Get seed blockchain height
     * @param[in] handle Seed handle
     * @return Result containing height
     */
    ots_result_t ots_seed_height(ots_handle_t handle);

    /**
     * @brief Get seed network type
     * @param[in] handle Seed handle
     * @return Result containing network type
     */
    ots_result_t ots_seed_network(ots_handle_t handle);

    /**
     * @brief Get wallet from seed
     * @param[in] handle Seed handle
     * @return Result containing wallet handle
     */
    ots_result_t ots_seed_wallet(ots_handle_t handle);

    /**
     * @brief Merge two sets of seed values
     * @param[in] values1 First set of values
     * @param[in] values2 Second set of values
     * @return Result containing merged indices
     * @throws OTS_ERROR_LENGTH_MISMATCH if value sets have different sizes
     */
    ots_result_t ots_seed_merge_values(
            const ots_seed_indices_t* values1,
            const ots_seed_indices_t* values2
            );

    /**
     * @brief Merge seed values with password
     * @param[in] password Password to merge with
     * @param[in] values Seed values to merge
     * @return Result containing merged indices
     * @throws OTS_ERROR_MERGE_FAILED if merge operation fails
     */
    ots_result_t ots_seed_merge_with_password(
            const char* password,
            const ots_seed_indices_t* values
            );

    /**
     * @brief Merge multiple sets of seed values
     * @param[in] values Array of value sets to merge
     * @param[in] count Number of value sets
     * @return Result containing merged indices
     * @throws OTS_ERROR_LENGTH_MISMATCH if value sets have different sizes
     * @throws OTS_ERROR_TOO_FEW_VALUES if less than two value sets provided
     */
    ots_result_t ots_seed_merge_multiple_values(
            const ots_seed_indices_t* values[],
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
    ots_result_t ots_seed_merge_values_and_zero(
            ots_seed_indices_t* values1,
            ots_seed_indices_t* values2,
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
    ots_result_t ots_seed_merge_values_with_password_and_zero(
            char* password,
            ots_seed_indices_t* values,
            bool delete_after
            );

    /**
     * @brief Merge multiple sets of seed values and zero
     * @param[in,out] values Array of value sets to merge (will be zeroed)
     * @param[in] count Number of value sets
     * @param[in] delete_after Delete values after merging
     * @return Result containing merged indices
     * @throws OTS_ERROR_LENGTH_MISMATCH if value sets have different sizes
     * @throws OTS_ERROR_TOO_FEW_VALUES if less than two value sets provided
     */
    ots_result_t ots_seed_merge_multiple_values_and_zero(
            ots_seed_indices_t* values[],
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
    ots_result_t ots_legacy_seed_decode(
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
    ots_result_t ots_legacy_seed_decode_indices(
            const ots_seed_indices_t* indices,
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
     * @param[in] network Network type (default: MAIN)
     * @return Result containing seed handle
     * @throws OTS_ERROR_INVALID_INPUT if random data is invalid
     */
    ots_result_t ots_monero_seed_create(
            const uint8_t random[32],
            uint64_t height,
            uint64_t time,
            OTS_NETWORK network
            );

    /**
     * @brief Generate a new Monero seed
     * @param[in] height Optional blockchain height (0 for none)
     * @param[in] time Optional timestamp (0 for none)
     * @param[in] network Network type (default: MAIN)
     * @return Result containing seed handle
     */
    ots_result_t ots_monero_seed_generate(
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
    ots_result_t ots_monero_seed_decode(
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
    ots_result_t ots_monero_seed_decode_indices(
            const ots_seed_indices_t* indices,
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
    ots_result_t ots_polyseed_create(
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
    ots_result_t ots_polyseed_generate(
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
    ots_result_t ots_polyseed_decode(
            const char* phrase,
            OTS_NETWORK network,
            const char* password,
            const char* passphrase
            );

    /**
     * @brief Decode a Polyseed from indices
     * @param[in] indices Array of seed word indices
     * @param[in] network Network type (default: MAIN)
     * @param[in] password Optional decryption password (empty string for none)
     * @param[in] passphrase Optional passphrase for seed offset (empty string for none)
     * @return Result containing seed handle
     * @throws OTS_ERROR_INVALID_SEED if decoding fails
     */
    ots_result_t ots_polyseed_decode_indices(
            const ots_seed_indices_t* indices,
            OTS_NETWORK network,
            const char* password,
            const char* passphrase
            );

    /**
     * @brief Decode a Polyseed from phrase with specific language
     * @param[in] phrase The seed phrase
     * @param[in] language_code Language code for the phrase
     * @param[in] network Network type (default: MAIN)
     * @param[in] password Optional decryption password (empty string for none)
     * @param[in] passphrase Optional passphrase for seed offset (empty string for none)
     * @return Result containing seed handle
     * @throws OTS_ERROR_INVALID_SEED if decoding fails
     */
    ots_result_t ots_polyseed_decode_with_language(
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
     * @brief Get address type
     * @param[in] address Address to check
     * @return Result containing address type
     */
    ots_result_t ots_address_type(ots_handle_t address);

    /**
     * @brief Get network type for an address
     * @param[in] address The address to check
     * @return Result containing network type
     * @throws OTS_ERROR_INVALID_ADDRESS if address is invalid
     */
    ots_result_t ots_address_network(ots_handle_t address);

    /**
     * @brief Generate fingerprint for an address
     * @param[in] address The address to generate fingerprint for
     * @return Result containing fingerprint string
     * @throws OTS_ERROR_INVALID_ADDRESS if address is invalid
     * @note Fingerprint is the last 6 digits of sha256(address) as uppercase hex
     */
    ots_result_t ots_address_fingerprint(ots_handle_t address);

    /**
     * @brief Check if address is an integrated address
     * @param[in] address The address to check
     * @return Result containing boolean status
     * @throws OTS_ERROR_INVALID_ADDRESS if address is invalid
     */
    ots_result_t ots_address_is_integrated(ots_handle_t address);

    /**
     * @brief Extract payment ID from integrated address
     * @param[in] address The integrated address
     * @return Result containing payment ID string
     * @throws OTS_ERROR_INVALID_ADDRESS if address is invalid
     * @throws OTS_ERROR_NOT_INTEGRATED if address is not an integrated address
     */
    ots_result_t ots_address_payment_id(ots_handle_t address);

    /**
     * @brief Get base address from integrated address
     * @param[in] address The integrated address
     * @return Result containing base address string
     * @throws OTS_ERROR_INVALID_ADDRESS if address is invalid
     * @throws OTS_ERROR_NOT_INTEGRATED if address is not an integrated address
     */
    ots_result_t ots_address_from_integrated(ots_handle_t address);

    /**
     * @brief Get address length
     * @param[in] address The address to check
     * @return Result containing address length
     * @throws OTS_ERROR_INVALID_ADDRESS if address is invalid
     */
    ots_result_t ots_address_length(ots_handle_t address);

    /**
     * @brief Get base58 string representation of address
     * @param[in] address_handle Address handle
     * @return Result containing address string
     */
    ots_result_t ots_address_base58_string(ots_handle_t address_handle);

    /**
     * @brief Compare two addresses for equality
     * @param[in] address1 First address
     * @param[in] address2 Second address
     * @return Result containing boolean equality status
     * @throws OTS_ERROR_INVALID_ADDRESS if either address is invalid
     */
    ots_result_t ots_address_equal(
            ots_handle_t address1,
            ots_handle_t address2
            );

    /**
     * @brief Compare address handle with string
     * @param[in] address_handle Address handle
     * @param[in] address_string Address string to compare
     * @return Result containing boolean equality status
     */
    ots_result_t ots_address_equal_string(
            ots_handle_t address_handle,
            const char* address_string
            );

    /**
     * @brief Create address object from string
     * @param[in] address Address string
     * @return Result containing address handle
     * @throws OTS_ERROR_INVALID_ADDRESS if address is invalid
     * @note This creates a managed address object that can be used with other functions
     */
    ots_result_t ots_address_create(const char* address);

    /**
     * @brief Free address handle
     * @param[in] handle Address handle to free
     */
    void ots_address_free(ots_handle_t handle);

    /**
     * @brief Validate a Monero address
     * @param[in] address Address to validate
     * @param[in] network Network to validate against
     * @return Result containing validation status
     */
    ots_result_t ots_address_string_valid(const char* address, OTS_NETWORK network);

    /**
     * @brief Get network type for an address string
     * @param[in] address The address string to check
     * @return Result containing network type
     * @throws OTS_ERROR_INVALID_ADDRESS if address is invalid
     */
    ots_result_t ots_address_string_network(const char* address);

    /**
     * @brief Get type for an address string
     * @param[in] address The address string to check
     * @return Result containing address type
     * @throws OTS_ERROR_INVALID_ADDRESS if address is invalid
     */
    ots_result_t ots_address_string_type(const char* address);

    /**
     * @brief Generate fingerprint for an address string
     * @param[in] address The address string
     * @return Result containing fingerprint string
     * @throws OTS_ERROR_INVALID_ADDRESS if address is invalid
     */
    ots_result_t ots_address_string_fingerprint(const char* address);

    /**
     * @brief Check if address string is integrated
     * @param[in] address The address string to check
     * @return Result containing boolean status
     * @throws OTS_ERROR_INVALID_ADDRESS if address is invalid
     */
    ots_result_t ots_address_string_is_integrated(const char* address);

    /**
     * @brief Extract payment ID from integrated address string
     * @param[in] address The integrated address string
     * @return Result containing payment ID string
     * @throws OTS_ERROR_INVALID_ADDRESS if address is invalid
     * @throws OTS_ERROR_NOT_INTEGRATED if address is not integrated
     */
    ots_result_t ots_address_string_payment_id(const char* address);

    /**
     * @brief Get base address from integrated address string
     * @param[in] address The integrated address string
     * @return Result containing base address string
     * @throws OTS_ERROR_INVALID_ADDRESS if address is invalid
     * @throws OTS_ERROR_NOT_INTEGRATED if address is not integrated
     */
    ots_result_t ots_address_string_integrated(const char* address);


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
    ots_result_t ots_wallet_create(
            const uint8_t key[32],
            uint64_t height,
            OTS_NETWORK network
            );

    /**
     * @brief Get wallet restore height
     * @param[in] wallet_handle Wallet handle
     * @return Result containing height
     */
    ots_result_t ots_wallet_height(ots_handle_t wallet_handle);

    /**
     * @brief Generate address for wallet
     * @param[in] wallet_handle Wallet handle
     * @param[in] account Account index
     * @param[in] index Address index
     * @return Result containing address string
     */
    ots_result_t ots_wallet_address(ots_handle_t wallet_handle, uint32_t account, uint32_t index);

    /**
     * @brief Get list of accounts in wallet
     * @param[in] wallet_handle Wallet handle
     * @param[in] max Maximum number of accounts to return
     * @param[in] offset Starting account index
     * @return Result containing array of address strings
     */
    ots_result_t ots_wallet_accounts(
            ots_handle_t wallet_handle,
            uint32_t max,
            uint32_t offset
            );

    /**
     * @brief Get list of subaddresses for an account
     * @param[in] wallet_handle Wallet handle
     * @param[in] account Account index
     * @param[in] max Maximum number of addresses to return
     * @param[in] offset Starting subaddress index
     * @return Result containing array of address strings
     */
    ots_result_t ots_wallet_subaddresses(
            ots_handle_t wallet_handle,
            uint32_t account,
            uint32_t max,
            uint32_t offset
            );

    /**
     * @brief Check if address belongs to wallet
     * @param[in] wallet_handle Wallet handle
     * @param[in] address Address to check
     * @param[in] max_account_depth Maximum account depth to search
     * @param[in] max_index_depth Maximum index depth to search
     * @return Result containing boolean status
     */
    ots_result_t ots_wallet_has_address(
            ots_handle_t wallet_handle,
            const char* address,
            uint32_t max_account_depth,
            uint32_t max_index_depth
            );

    /**
     * @brief Check if address belongs to wallet using Address handle
     * @param[in] wallet_handle Wallet handle
     * @param[in] address_handle Address handle to check
     * @param[in] max_account_depth Maximum account depth to search
     * @param[in] max_index_depth Maximum index depth to search
     * @return Result containing boolean status
     */
    ots_result_t ots_wallet_has_address_handle(
            ots_handle_t wallet_handle,
            ots_handle_t address_handle,
            uint32_t max_account_depth,
            uint32_t max_index_depth
            );

    /**
     * @brief Get account and index for address in wallet
     * @param[in] wallet_handle Wallet handle
     * @param[in] address Address to look up
     * @param[in] max_account_depth Maximum account depth to search
     * @param[in] max_index_depth Maximum index depth to search
     * @return Result containing account/index pair
     * @throws OTS_ERROR_ADDRESS_NOT_FOUND if address not found in wallet
     */
    ots_result_t ots_wallet_address_index(
            ots_handle_t wallet_handle,
            const char* address,
            uint32_t max_account_depth,
            uint32_t max_index_depth
            );

    /**
     * @brief Get account and index for address handle in wallet
     * @param[in] wallet_handle Wallet handle
     * @param[in] address_handle Address handle to look up
     * @param[in] max_account_depth Maximum account depth to search
     * @param[in] max_index_depth Maximum index depth to search
     * @return Result containing account/index pair
     * @throws OTS_ERROR_ADDRESS_NOT_FOUND if address not found in wallet
     */
    ots_result_t ots_wallet_address_index_handle(
            ots_handle_t wallet_handle,
            ots_handle_t address_handle,
            uint32_t max_account_depth,
            uint32_t max_index_depth
            );

    /**
     * @brief Get secret view key
     * @param[in] wallet_handle Wallet handle
     * @return Result containing wipeable string with key
     */
    ots_result_t ots_wallet_secret_view_key(ots_handle_t wallet_handle);

    /**
     * @brief Get public view key
     * @param[in] wallet_handle Wallet handle
     * @return Result containing wipeable string with key
     */
    ots_result_t ots_wallet_public_view_key(ots_handle_t wallet_handle);

    /**
     * @brief Get secret spend key
     * @param[in] wallet_handle Wallet handle
     * @return Result containing wipeable string with key
     */
    ots_result_t ots_wallet_secret_spend_key(ots_handle_t wallet_handle);

    /**
     * @brief Get public spend key
     * @param[in] wallet_handle Wallet handle
     * @return Result containing wipeable string with key
     */
    ots_result_t ots_wallet_public_spend_key(ots_handle_t wallet_handle);

    /**
     * @brief Import outputs from string
     * @param[in] wallet_handle Wallet handle
     * @param[in] outputs Outputs string from view wallet
     * @return Result containing number of imported outputs
     * @throws OTS_ERROR_INVALID_OUTPUTS if outputs data is invalid
     */
    ots_result_t ots_wallet_import_outputs(
            ots_handle_t wallet_handle,
            const char* outputs
            );

    /**
     * @brief Export key images
     * @param[in] wallet_handle Wallet handle
     * @return Result containing wipeable string with key images
     * @throws OTS_ERROR_NO_KEY_IMAGES if no outputs were imported
     */
    ots_result_t ots_wallet_export_key_images(ots_handle_t wallet_handle);

    /**
     * @brief Describe unsigned transaction
     * @param[in] wallet_handle Wallet handle
     * @param[in] unsigned_tx Unsigned transaction data
     * @return Result containing transaction description
     */
    ots_result_t ots_wallet_describe_tx(ots_handle_t wallet_handle, const char* unsigned_tx);

    /**
     * @brief Check transaction for warnings
     * @param[in] wallet_handle Wallet handle
     * @param[in] unsigned_tx_handle unsigned transaction handle
     * @return Result containing array of warnings
     */
    ots_result_t ots_wallet_check_tx(ots_handle_t wallet_handle, ots_handle_t unsigned_tx_handle);

    /**
     * @brief Check transaction string for warnings
     * @param[in] wallet_handle Wallet handle
     * @param[in] unsigned_tx Unsigned transaction string
     * @return Result containing array of warnings
     */
    ots_result_t ots_wallet_check_tx_string(
            ots_handle_t wallet_handle,
            const char* unsigned_tx
            );

    /**
     * @brief Sign unsigned transaction
     * @param[in] wallet_handle Wallet handle
     * @param[in] unsigned_tx Unsigned transaction string
     * @return Result containing signed transaction string
     * @throws OTS_ERROR_INVALID_TRANSACTION if transaction is invalid
     */
    ots_result_t ots_wallet_sign_transaction(
            ots_handle_t wallet_handle,
            const char* unsigned_tx
            );

    /**
     * @brief Sign arbitrary data
     * @param[in] wallet_handle Wallet handle
     * @param[in] data Data to sign
     * @return Result containing signature
     */
    ots_result_t ots_wallet_sign_data(ots_handle_t wallet_handle, const char* data);

    /**
     * @brief Sign data with specific subaddress
     * @param[in] wallet_handle Wallet handle
     * @param[in] data Data to sign
     * @param[in] account Account index
     * @param[in] subaddr Subaddress index
     * @return Result containing signature string
     */
    ots_result_t ots_wallet_sign_data_with_index(
            ots_handle_t wallet_handle,
            const char* data,
            uint32_t account,
            uint32_t subaddr
            );

    /**
     * @brief Sign data with specific address
     * @param[in] wallet_handle Wallet handle
     * @param[in] data Data to sign
     * @param[in] address_handle Address to sign with
     * @return Result containing signature string
     * @throws OTS_ERROR_ADDRESS_NOT_FOUND if address not found in wallet
     */
    ots_result_t ots_wallet_sign_data_with_address(
            ots_handle_t wallet_handle,
            const char* data,
            ots_handle_t address_handle
            );

    /**
     * @brief Verify signed data for the wallet address with legacy support
     * @param[in] data Original data
     * @param[in] signature Signature to verify
     * @param[in] legacy_fallback Try legacy verification if modern fails
     * @return Result containing verification status
     */
    ots_result_t ots_wallet_verify_data(
            const char* data,
            const char* signature,
            bool legacy_fallback
            );

    /**
     * @brief Verify signed data with specific subaddress
     * @param[in] wallet_handle Wallet handle
     * @param[in] data Original data
     * @param[in] account Account index  
     * @param[in] subaddr Subaddress index
     * @param[in] signature Signature to verify
     * @param[in] legacy_fallback Try legacy verification if modern fails
     * @return Result containing verification status
     */
    ots_result_t ots_wallet_verify_data_with_index(
            ots_handle_t wallet_handle,
            const char* data,
            uint32_t account,
            uint32_t subaddr,
            const char* signature,
            bool legacy_fallback
            );

    /**
     * @brief Verify signed data with specific address
     * @param[in] wallet_handle Wallet handle
     * @param[in] data Original data
     * @param[in] address_handle Address to verify with
     * @param[in] signature Signature to verify
     * @param[in] legacy_fallback Try legacy verification if modern fails
     * @return Result containing verification status
     * @throws OTS_ERROR_ADDRESS_NOT_FOUND if address not found in wallet
     */
    ots_result_t ots_wallet_verify_data_with_address(
            ots_handle_t wallet_handle,
            const char* data,
            ots_handle_t address_handle,
            const char* signature,
            bool legacy_fallback
            );

    /*******************************************************************************
     * OTS Utility Functions
     ******************************************************************************/

    /**
     * @brief Convert timestamp to estimated block height
     * @param[in] timestamp Unix timestamp
     * @param[in] network Network type
     * @return Result containing estimated block height
     */
    ots_result_t ots_height_from_timestamp(
            uint64_t timestamp,
            OTS_NETWORK network
            );

    /**
     * @brief Convert block height to estimated timestamp
     * @param[in] height Block height
     * @param[in] network Network type
     * @return Result containing estimated timestamp
     */
    ots_result_t ots_timestamp_from_height(
            uint64_t height,
            OTS_NETWORK network
            );

    /**
     * @brief Generate random bytes
     * @param[out] buffer Buffer to receive random bytes
     * @param[in] size Number of bytes to generate
     * @return Result containing status
     * @warning Entropy quality depends on system random number generator
     */
    ots_result_t ots_random_bytes(
            uint8_t* buffer,
            size_t size
            );

    /**
     * @brief Generate 32 random bytes
     * @return Result containing 32-byte array
     * @warning Entropy quality depends on system random number generator
     */
    ots_result_t ots_random_32(void);

    /**
     * @brief Check data entropy level
     * @param[in] data Data to check
     * @param[in] size Size of data
     * @param[in] min_entropy Minimum required entropy
     * @return Result containing boolean (true if entropy is sufficient)
     */
    ots_result_t ots_check_entropy(
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
    ots_result_t ots_verify_data(const char* data, const char* address, const char* signature);

#ifdef __cplusplus
}
#endif

#endif // OTS_H
