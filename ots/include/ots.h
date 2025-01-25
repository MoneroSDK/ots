#ifndef OTS_H
#define OTS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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

/** @brief Maximum length for error messages */
#define OTS_MAX_ERROR_MESSAGE 256
/** @brief Maximum length for error location strings */
#define OTS_MAX_ERROR_LOCATION 64
/** @brief Maximum length for version strings */
#define OTS_MAX_VERSION_STRING 32

/** @brief Standard seed phrase lengths */
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

    /** @brief Opaque handles for C objects */
    typedef size_t ots_handle_t;

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
     * @brief Get library version string
     * @return Version string, must be freed with ots_free_string()
     */
    ots_result_t ots_version(void);

    /**
     * @brief Get version components [major, minor, patch]
     * @return Result containing version components array
     */
    ots_result_t ots_version_components(void);

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
     * @brief Validate a Monero address
     * @param[in] address Address to validate
     * @param[in] network Network to validate against
     * @return Result containing validation status
     */
    ots_result_t ots_address_valid(const char* address, OTS_NETWORK network);

    /**
     * @brief Get address type
     * @param[in] address Address to check
     * @return Result containing address type
     */
    ots_result_t ots_address_type(const char* address);

    /**
     * @brief Get payment ID from integrated address
     * @param[in] address Integrated address
     * @return Result containing payment ID string
     */
    ots_result_t ots_address_payment_id(const char* address);

    /**
     * @brief Create a seed from random data
     * @param[in] type Seed type to create
     * @param[in] network Network for the seed
     * @return Result containing seed handle
     */
    ots_result_t ots_seed_create(OTS_SEED_TYPE type, OTS_NETWORK network);

    /**
     * @brief Decode a seed phrase
     * @param[in] phrase Seed phrase
     * @param[in] type Seed type
     * @param[in] network Network
     * @param[in] passphrase Optional passphrase (can be NULL)
     * @return Result containing seed handle
     */
    ots_result_t ots_seed_decode(const char* phrase, OTS_SEED_TYPE type, 
                                OTS_NETWORK network, const char* passphrase);

    /**
     * @brief Get seed phrase in specified language
     * @param[in] handle Seed handle
     * @param[in] language_code Language code
     * @return Result containing wipeable string
     */
    ots_result_t ots_seed_phrase(ots_handle_t handle, const char* language_code);

    /**
     * @brief Get supported languages for seed type
     * @param[in] type Seed type
     * @return Result containing array of language codes
     */
    ots_result_t ots_seed_languages(OTS_SEED_TYPE type);

    /**
     * @brief Create wallet from seed
     * @param[in] seed_handle Seed handle
     * @return Result containing wallet handle
     */
    ots_result_t ots_wallet_create(ots_handle_t seed_handle);

    /**
     * @brief Generate address for wallet
     * @param[in] wallet_handle Wallet handle
     * @param[in] account Account index
     * @param[in] index Address index
     * @return Result containing address string
     */
    ots_result_t ots_wallet_address(ots_handle_t wallet_handle, 
                                   uint32_t account, uint32_t index);

    /**
     * @brief Import outputs to wallet
     * @param[in] wallet_handle Wallet handle
     * @param[in] outputs Outputs data
     * @return Result containing number of imported outputs
     */
    ots_result_t ots_wallet_import_outputs(ots_handle_t wallet_handle, 
                                          const char* outputs);

    /**
     * @brief Export key images from wallet
     * @param[in] wallet_handle Wallet handle
     * @return Result containing key images data
     */
    ots_result_t ots_wallet_export_key_images(ots_handle_t wallet_handle);

    /**
     * @brief Describe unsigned transaction
     * @param[in] wallet_handle Wallet handle
     * @param[in] unsigned_tx Unsigned transaction data
     * @return Result containing transaction description
     */
    ots_result_t ots_wallet_describe_tx(ots_handle_t wallet_handle, 
                                       const char* unsigned_tx);



    /**
     * @brief Free transaction description
     * @param[in] desc Transaction description to free
     */
    void ots_free_tx_description(ots_tx_description_t *desc);

    /**
     * @brief Check transaction for warnings
     * @param[in] wallet_handle Wallet handle
     * @param[in] unsigned_tx Unsigned transaction data
     * @return Result containing array of warnings
     */
    ots_result_t ots_wallet_check_tx(ots_handle_t wallet_handle, 
                                    const char* unsigned_tx);

    /**
     * @brief Sign transaction
     * @param[in] wallet_handle Wallet handle
     * @param[in] unsigned_tx Unsigned transaction data
     * @return Result containing signed transaction
     */
    ots_result_t ots_wallet_sign_tx(ots_handle_t wallet_handle, 
                                   const char* unsigned_tx);

    /**
     * @brief Sign arbitrary data
     * @param[in] wallet_handle Wallet handle
     * @param[in] data Data to sign
     * @return Result containing signature
     */
    ots_result_t ots_wallet_sign_data(ots_handle_t wallet_handle, 
                                     const char* data);

    /**
     * @brief Verify signed data
     * @param[in] data Original data
     * @param[in] address Signer's address
     * @param[in] signature Signature to verify
     * @return Result containing verification status
     */
    ots_result_t ots_verify_signature(const char* data, const char* address, 
                                     const char* signature);

    /**
     * @brief Free a handle
     * @param[in] handle Handle to free
     */
    void ots_free_handle(ots_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif // OTS_H
