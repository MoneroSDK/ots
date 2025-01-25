#include "ots.h"
#include "ots.hpp"
#include "ots-exceptions.hpp"
#include <cstring>
#include <memory>
#include <vector>
#include <stdexcept>

/**
 * @brief Internal helper functions and utilities
 */
namespace {
    /**
     * @brief Translates C++ exceptions to C error structure
     */
    void translate_exception(ots_error_t* error, const std::exception& e) {
        if (!error) return;

        error->code = -1; // Generic error code
        strncpy(error->message, e.what(), OTS_MAX_ERROR_MESSAGE - 1);
        error->message[OTS_MAX_ERROR_MESSAGE - 1] = '\0';
        error->location[0] = '\0';
    }

    /**
     * @brief Sets success state in result
     */
    void set_success(ots_result_t* result) {
        if (!result) return;
        result->error.code = 0;
        result->error.message[0] = '\0';
        result->error.location[0] = '\0';
    }

    /**
     * @brief Sets error state in result
     */
    void set_error(ots_result_t* result, const std::exception& e) {
        if (!result) return;
        translate_exception(&result->error, e);
        result->result.ptr = nullptr;
    }

    /**
     * @brief Creates a string copy for return values
     */
    char* create_string_copy(const std::string& str) {
        char* copy = static_cast<char*>(malloc(str.length() + 1));
        if (copy) {
            strcpy(copy, str.c_str());
        }
        return copy;
    }

    /**
     * @brief Converts C network enum to C++ network enum
     */
    ots::Network to_cpp_network(OTS_NETWORK network) {
        switch (network) {
            case OTS_NETWORK_TEST: return ots::Network::TEST;
            case OTS_NETWORK_STAGE: return ots::Network::STAGE;
            default: return ots::Network::MAIN;
        }
    }

    /**
     * @brief Converts C++ network enum to C network enum
     */
    OTS_NETWORK to_c_network(ots::Network network) {
        switch (network) {
            case ots::Network::TEST: return OTS_NETWORK_TEST;
            case ots::Network::STAGE: return OTS_NETWORK_STAGE;
            default: return OTS_NETWORK_MAIN;
        }
    }

    /**
     * @brief Converts C++ AddressType to C address type
     */
    OTS_ADDRESS_TYPE to_c_address_type(ots::AddressType type) {
        switch (type) {
            case ots::AddressType::SubAddress: return OTS_ADDRESS_TYPE_SUBADDRESS;
            case ots::AddressType::Integrated: return OTS_ADDRESS_TYPE_INTEGRATED;
            default: return OTS_ADDRESS_TYPE_STANDARD;
        }
    }

    /**
     * @brief Creates a wipeable string from C++ WipeableString
     */
    ots_wipeable_string_t create_wipeable_string(const ots::WipeableString& str) {
        ots_wipeable_string_t result = {};
        result.length = str.size();
        result.data = static_cast<char*>(malloc(result.length + 1));
        if (result.data) {
            memcpy(result.data, str.c_str(), result.length);
            result.data[result.length] = '\0';
        }
        return result;
    }

    /**
     * @brief Converts seed indices to C structure
     */
    ots_seed_indices_t create_seed_indices(const ots::SeedIndices& indices) {
        ots_seed_indices_t result = {};
        result.count = indices.size();
        result.indices = static_cast<uint16_t*>(malloc(result.count * sizeof(uint16_t)));
        if (result.indices) {
            memcpy(result.indices, static_cast<const uint8_t*>(indices), result.count * sizeof(uint16_t));
        }
        return result;
    }

    /**
     * @brief Safely retrieves wallet pointer from handle
     */
    ots::Wallet* get_wallet(ots_handle_t handle) {
        auto* wallet = reinterpret_cast<ots::Wallet*>(handle);
        if (!wallet) throw std::runtime_error("Invalid wallet handle");
        return wallet;
    }

    /**
     * @brief Creates transaction description structure
     */
    ots_tx_description_t* create_tx_description(const ots::TxDescription& desc) {
        auto* tx_desc = new ots_tx_description_t();
        tx_desc->description = create_string_copy("Transaction Description");
        tx_desc->amount = 0;
        tx_desc->fee = 0;
        tx_desc->destination = create_string_copy("Destination Address");
        tx_desc->unlock_time = 0;
        return tx_desc;
    }

    /**
     * @brief Creates array of transaction warnings
     */
    ots_tx_warning_t* create_tx_warnings(const std::vector<ots::TxWarning>& warnings, size_t* count) {
        if (count) *count = warnings.size();
        if (warnings.empty()) return nullptr;

        auto* tx_warnings = static_cast<ots_tx_warning_t*>(
                malloc(warnings.size() * sizeof(ots_tx_warning_t)));

        if (!tx_warnings) throw std::bad_alloc();

        for (size_t i = 0; i < warnings.size(); ++i) {
            tx_warnings[i].message = create_string_copy("Warning Message");
            tx_warnings[i].severity = 1;
        }

        return tx_warnings;
    }
}

extern "C" {

    // Basic functions and error handling
    void ots_error_init(ots_error_t* error) {
        if (!error) return;
        error->code = 0;
        error->message[0] = '\0';
        error->location[0] = '\0';
    }

    bool ots_is_error(const ots_result_t* result) {
        return result && result->error.code != 0;
    }

    void ots_free_string(char* str) {
        if (str) {
            free(str);
        }
    }

    void ots_free_array(void* arr) {
        if (arr) {
            free(arr);
        }
    }

    // Version functions
    ots_result_t ots_version(void) {
        ots_result_t result = {};
        try {
            std::string version = ots::OTS::version();
            result.result.ptr = create_string_copy(version);
            set_success(&result);
        } catch (const std::exception& e) {
            set_error(&result, e);
        }
        return result;
    }

    ots_result_t ots_version_components(void) {
        ots_result_t result = {};
        try {
            auto components = ots::OTS::versionComponents();
            int* arr = static_cast<int*>(malloc(3 * sizeof(int)));
            if (!arr) {
                throw std::bad_alloc();
            }
            arr[0] = components[0];
            arr[1] = components[1];
            arr[2] = components[2];
            result.result.ptr = arr;
            set_success(&result);
        } catch (const std::exception& e) {
            set_error(&result, e);
        }
        return result;
    }

    // Memory management functions
    void ots_wipeable_string_free(ots_wipeable_string_t* str) {
        if (!str) return;
        if (str->data) {
            memset(str->data, 0, str->length); // Secure wiping
            free(str->data);
        }
        str->data = nullptr;
        str->length = 0;
    }

    void ots_seed_indices_free(ots_seed_indices_t* indices) {
        if (!indices) return;
        if (indices->indices) {
            memset(indices->indices, 0, indices->count * sizeof(uint16_t)); // Secure wiping
            free(indices->indices);
        }
        indices->indices = nullptr;
        indices->count = 0;
    }

    // Address functions
    ots_result_t ots_address_valid(const char* address, OTS_NETWORK network) {
        ots_result_t result = {};
        if (!address) {
            result.error.code = -1;
            strncpy(result.error.message, "Invalid address pointer", OTS_MAX_ERROR_MESSAGE - 1);
            return result;
        }

        try {
            result.result.boolean = ots::Address::isValid(address, to_cpp_network(network));
            set_success(&result);
        } catch (const std::exception& e) {
            set_error(&result, e);
        }
        return result;
    }

    ots_result_t ots_address_type(const char* address) {
        ots_result_t result = {};
        if (!address) {
            result.error.code = -1;
            strncpy(result.error.message, "Invalid address pointer", OTS_MAX_ERROR_MESSAGE - 1);
            return result;
        }

        try {
            auto type = ots::Address::type(address);
            result.result.number = static_cast<int64_t>(to_c_address_type(type));
            set_success(&result);
        } catch (const std::exception& e) {
            set_error(&result, e);
        }
        return result;
    }

    ots_result_t ots_address_payment_id(const char* address) {
        ots_result_t result = {};
        if (!address) {
            result.error.code = -1;
            strncpy(result.error.message, "Invalid address pointer", OTS_MAX_ERROR_MESSAGE - 1);
            return result;
        }

        try {
            std::string payment_id = ots::Address::paymentID(address);
            result.result.ptr = create_string_copy(payment_id);
            set_success(&result);
        } catch (const std::exception& e) {
            set_error(&result, e);
        }
        return result;
    }

    // Seed management functions
    ots_result_t ots_seed_create(OTS_SEED_TYPE type, OTS_NETWORK network) {
        ots_result_t result = {};
        try {
            ots::Seed* seed = nullptr;
            switch (type) {
                case OTS_SEED_TYPE_MONERO:
                    seed = new ots::MoneroSeed(ots::MoneroSeed::generate(0, 0, to_cpp_network(network)));
                    break;
                case OTS_SEED_TYPE_POLYSEED:
                    seed = new ots::Polyseed(ots::Polyseed::generate(to_cpp_network(network)));
                    break;
            }
            result.result.handle = reinterpret_cast<ots_handle_t>(seed);
            set_success(&result);
        } catch (const std::exception& e) {
            set_error(&result, e);
        }
        return result;
    }

    ots_result_t ots_seed_decode(const char* phrase, OTS_SEED_TYPE type, 
            OTS_NETWORK network, const char* passphrase) {
        ots_result_t result = {};
        if (!phrase) {
            result.error.code = -1;
            strncpy(result.error.message, "Invalid phrase pointer", OTS_MAX_ERROR_MESSAGE - 1);
            return result;
        }

        try {
            ots::Seed* seed = nullptr;
            std::string pass = passphrase ? passphrase : "";

            switch (type) {
                case OTS_SEED_TYPE_MONERO:
                    seed = new ots::MoneroSeed(ots::MoneroSeed::decode(
                                phrase, 0, 0, to_cpp_network(network), pass));
                    break;
                case OTS_SEED_TYPE_POLYSEED:
                    seed = new ots::Polyseed(ots::Polyseed::decode(
                                phrase, to_cpp_network(network), "", pass));
                    break;
            }
            result.result.handle = reinterpret_cast<ots_handle_t>(seed);
            set_success(&result);
        } catch (const std::exception& e) {
            set_error(&result, e);
        }
        return result;
    }

    ots_result_t ots_seed_phrase(ots_handle_t handle, const char* language_code) {
        ots_result_t result = {};
        if (!language_code) {
            result.error.code = -1;
            strncpy(result.error.message, "Invalid language code pointer", OTS_MAX_ERROR_MESSAGE - 1);
            return result;
        }

        try {
            auto* seed = reinterpret_cast<ots::Seed*>(handle);
            if (!seed) throw std::runtime_error("Invalid seed handle");

            const auto& lang = ots::SeedLanguage::fromCode(language_code);
            auto phrase = seed->phrase(lang);

            ots_wipeable_string_t* wstr = new ots_wipeable_string_t(create_wipeable_string(phrase));
            result.result.ptr = wstr;
            set_success(&result);
        } catch (const std::exception& e) {
            set_error(&result, e);
        }
        return result;
    }

    ots_result_t ots_seed_languages(OTS_SEED_TYPE type) {
        ots_result_t result = {};
        try {
            auto langs = ots::SeedLanguage::listFor(
                    type == OTS_SEED_TYPE_MONERO ? ots::SeedType::Monero : ots::SeedType::Polyseed);

            char** languages = static_cast<char**>(malloc(langs.size() * sizeof(char*)));
            if (!languages) throw std::bad_alloc();

            for (size_t i = 0; i < langs.size(); ++i) {
                languages[i] = create_string_copy(langs[i].get().code());
            }

            result.result.ptr = languages;
            set_success(&result);
        } catch (const std::exception& e) {
            set_error(&result, e);
        }
        return result;
    }

    // Wallet operations
    ots_result_t ots_wallet_create(ots_handle_t seed_handle) {
        ots_result_t result = {};
        try {
            auto* seed = reinterpret_cast<ots::Seed*>(seed_handle);
            if (!seed) throw std::runtime_error("Invalid seed handle");

            auto wallet = seed->wallet();
            auto* wallet_ptr = new std::shared_ptr<ots::Wallet>(wallet);
            result.result.handle = reinterpret_cast<ots_handle_t>(wallet_ptr);
            set_success(&result);
        } catch (const std::exception& e) {
            set_error(&result, e);
        }
        return result;
    }

    ots_result_t ots_wallet_address(ots_handle_t wallet_handle, uint32_t account, uint32_t index) {
        ots_result_t result = {};
        try {
            auto* wallet_ptr = reinterpret_cast<std::shared_ptr<ots::Wallet>*>(wallet_handle);
            if (!wallet_ptr || !*wallet_ptr) throw std::runtime_error("Invalid wallet handle");

            auto address = (*wallet_ptr)->address(account, index);
            result.result.ptr = create_string_copy(address);
            set_success(&result);
        } catch (const std::exception& e) {
            set_error(&result, e);
        }
        return result;
    }

    ots_result_t ots_wallet_import_outputs(ots_handle_t wallet_handle, const char* outputs) {
        ots_result_t result = {};
        if (!outputs) {
            result.error.code = -1;
            strncpy(result.error.message, "Invalid outputs pointer", OTS_MAX_ERROR_MESSAGE - 1);
            return result;
        }

        try {
            auto* wallet_ptr = reinterpret_cast<std::shared_ptr<ots::Wallet>*>(wallet_handle);
            if (!wallet_ptr || !*wallet_ptr) throw std::runtime_error("Invalid wallet handle");

            uint64_t count = (*wallet_ptr)->importOutputs(outputs);
            result.result.number = static_cast<int64_t>(count);
            set_success(&result);
        } catch (const std::exception& e) {
            set_error(&result, e);
        }
        return result;
    }

    ots_result_t ots_wallet_export_key_images(ots_handle_t wallet_handle) {
        ots_result_t result = {};
        try {
            auto* wallet_ptr = reinterpret_cast<std::shared_ptr<ots::Wallet>*>(wallet_handle);
            if (!wallet_ptr || !*wallet_ptr) throw std::runtime_error("Invalid wallet handle");

            auto key_images = (*wallet_ptr)->exportKeyImages();
            ots_wipeable_string_t* wstr = new ots_wipeable_string_t(create_wipeable_string(key_images));
            result.result.ptr = wstr;
            set_success(&result);
        } catch (const std::exception& e) {
            set_error(&result, e);
        }
        return result;
    }

    ots_result_t ots_wallet_describe_tx(ots_handle_t wallet_handle, const char* unsigned_tx) {
        ots_result_t result = {};
        if (!unsigned_tx) {
            result.error.code = -1;
            strncpy(result.error.message, "Invalid transaction pointer", OTS_MAX_ERROR_MESSAGE - 1);
            return result;
        }

        try {
            auto* wallet_ptr = reinterpret_cast<std::shared_ptr<ots::Wallet>*>(wallet_handle);
            if (!wallet_ptr || !*wallet_ptr) throw std::runtime_error("Invalid wallet handle");

            auto desc = (*wallet_ptr)->describeTransaction(unsigned_tx);
            result.result.ptr = create_tx_description(desc);
            set_success(&result);
        } catch (const std::exception& e) {
            set_error(&result, e);
        }
        return result;
    }

    ots_result_t ots_wallet_check_tx(ots_handle_t wallet_handle, const char* unsigned_tx) {
        ots_result_t result = {};
        if (!unsigned_tx) {
            result.error.code = -1;
            strncpy(result.error.message, "Invalid transaction pointer", OTS_MAX_ERROR_MESSAGE - 1);
            return result;
        }

        try {
            auto* wallet_ptr = reinterpret_cast<std::shared_ptr<ots::Wallet>*>(wallet_handle);
            if (!wallet_ptr || !*wallet_ptr) throw std::runtime_error("Invalid wallet handle");

            auto warnings = (*wallet_ptr)->checkTransaction(unsigned_tx);
            size_t warning_count = 0;
            result.result.ptr = create_tx_warnings(warnings, &warning_count);
            result.error.code = static_cast<int32_t>(warning_count);
            set_success(&result);
        } catch (const std::exception& e) {
            set_error(&result, e);
        }
        return result;
    }

    ots_result_t ots_wallet_sign_tx(ots_handle_t wallet_handle, const char* unsigned_tx) {
        ots_result_t result = {};
        if (!unsigned_tx) {
            result.error.code = -1;
            strncpy(result.error.message, "Invalid transaction pointer", OTS_MAX_ERROR_MESSAGE - 1);
            return result;
        }

        try {
            auto* wallet_ptr = reinterpret_cast<std::shared_ptr<ots::Wallet>*>(wallet_handle);
            if (!wallet_ptr || !*wallet_ptr) throw std::runtime_error("Invalid wallet handle");

            std::string signed_tx = (*wallet_ptr)->signTransaction(unsigned_tx);
            result.result.ptr = create_string_copy(signed_tx);
            set_success(&result);
        } catch (const std::exception& e) {
            set_error(&result, e);
        }
        return result;
    }

    ots_result_t ots_wallet_sign_data(ots_handle_t wallet_handle, const char* data) {
        ots_result_t result = {};
        if (!data) {
            result.error.code = -1;
            strncpy(result.error.message, "Invalid data pointer", OTS_MAX_ERROR_MESSAGE - 1);
            return result;
        }

        try {
            auto* wallet_ptr = reinterpret_cast<std::shared_ptr<ots::Wallet>*>(wallet_handle);
            if (!wallet_ptr || !*wallet_ptr) throw std::runtime_error("Invalid wallet handle");

            std::string signature = (*wallet_ptr)->signData(data);
            result.result.ptr = create_string_copy(signature);
            set_success(&result);
        } catch (const std::exception& e) {
            set_error(&result, e);
        }
        return result;
    }

    ots_result_t ots_verify_signature(const char* data, const char* address, const char* signature) {
        ots_result_t result = {};
        if (!data || !address || !signature) {
            result.error.code = -1;
            strncpy(result.error.message, "Invalid parameter pointer", OTS_MAX_ERROR_MESSAGE - 1);
            return result;
        }

        try {
            auto* dummy_wallet = new ots::Wallet(std::array<unsigned char, 32>(), 0, ots::Network::MAIN);
            bool verified = dummy_wallet->verifyData(data, address, signature);
            delete dummy_wallet;

            result.result.boolean = verified;
            set_success(&result);

        } catch (const std::exception& e) {
            set_error(&result, e);
        }
        return result;
    }

    // Cleanup functions
    void ots_free_handle(ots_handle_t handle) {
        if (handle) {
            // Try to determine the type and free appropriately
            try {
                // First try as Seed
                auto* seed = reinterpret_cast<ots::Seed*>(handle);
                delete seed;
            } catch (...) {
                try {
                    // Then try as Wallet
                    auto* wallet_ptr = reinterpret_cast<std::shared_ptr<ots::Wallet>*>(handle);
                    delete wallet_ptr;
                } catch (...) {
                    // If both fail, ignore - nothing we can do
                }
            }
        }
    }

    void ots_free_tx_description(ots_tx_description_t* desc) {
        if (!desc) return;
        if (desc->description) {
            free(desc->description);
        }
        if (desc->destination) {
            free(desc->destination);
        }
        free(desc);
    }

    void ots_free_tx_warnings(ots_tx_warning_t* warnings, size_t count) {
        if (!warnings) return;
        for (size_t i = 0; i < count; ++i) {
            if (warnings[i].message) {
                free(warnings[i].message);
            }
        }
        free(warnings);
    }

    // Additional helper functions for language management
    const char* ots_seed_language_default(OTS_SEED_TYPE type, ots_error_t* error) {
        try {
            const auto& lang = ots::SeedLanguage::defaultLanguage(
                    type == OTS_SEED_TYPE_MONERO ? ots::SeedType::Monero : ots::SeedType::Polyseed);
            return create_string_copy(lang.code());
        } catch (const std::exception& e) {
            if (error) {
                translate_exception(error, e);
            }
            return nullptr;
        }
    }

    void ots_free_language_list(char** languages, size_t count) {
        if (!languages) return;
        for (size_t i = 0; i < count; ++i) {
            if (languages[i]) {
                free(languages[i]);
            }
        }
        free(languages);
    }

} // extern "C"
