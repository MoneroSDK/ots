#include "ots.h"
#include "ots.hpp"
#include "ots-exceptions.hpp"
#include <cstring>
#include <memory>
#include <tuple>

namespace {
    /**
     * @brief Translate C++ exception to C error
     * @param[out] error Error to fill
     * @param[in] e Exception to translate
     * @internal
     */
    void translate_exception(ots_error_t* error, const ots::exception::Exception& e) {
        if(!error)
            return;
        error->code = e.code();
        strncpy(error->message, e.what(), OTS_MAX_ERROR_MESSAGE - 1);
        error->message[OTS_MAX_ERROR_MESSAGE - 1] = '\0';
        error->cls[0] = e.cls()[0];
    }

    /**
     * @brief Set result to success
     * @param[out] result Result to set
     * @internal
     */
    void set_success(ots_result_t* result) {
        if(!result)
            return;
        result->type = OTS_RESULT_NONE;
        result->result.data.ptr = nullptr;
        result->result.data.size = 0;
        result->result.data.type = OTS_DATA_INVALID;
        result->error.code = 0;
        result->error.message[0] = '\0';
        result->error.cls[0] = '\0';
    }

    /**
     * @brief Set result type
     * @param[out] result Result to set
     * @param[in] type Type to set
     * @internal
     */
    void set_result_type(ots_result_t* result, ots_result_type type) {
        if(result == nullptr)
            return;
        set_success(result);
        result->type = type;
    }

    /**
     * @brief Create handle
     * @param[in] type Handle type
     * @param[in] ptr Pointer to object
     * @return Handle
     * @internal
     */
    ots_handle_t create_handle(ots_handle_type type, void* ptr) {
        return ots_handle_t {
            .type = type,
            .ptr = ptr,
            .reference = false
        };
    }

    /**
     * @brief Create handle reference (no ownership)
     * @param[in] type Handle type
     * @param[in] ptr Pointer to object
     * @return Handle
     * @internal
     */
    ots_handle_t create_handle_reference(ots_handle_type type, void* ptr) {
        return ots_handle_t {
            .type = type,
            .ptr = ptr,
            .reference = true
        };
    }

    /**
     * @brief Create a copy of a string
     * @param[in] str String to copy
     * @return Copy of the string
     * @internal
     */
    char* create_string_copy(const std::string& str) {
        char* copy = new char[str.length() + 1];
        std::strncpy(copy, str.c_str(), str.length());
        copy[str.length()] = '\0';
        return copy;
    }

    /**
     * @brief Set handle to result
     * @param[out] result Result to set
     * @param[in] handle_type Handle type
     * @param[in] handle Handle to set
     * @internal
     */
    void set_handle(ots_result_t* result, ots_handle_type handle_type, void* handle) {
        if(!result)
            return;
        set_result_type(result, OTS_RESULT_HANDLE);
        result->result.handle = create_handle(handle_type, handle);
    }

    /**
     * @brief Set handle reference to result (no ownership)
     * @param[out] result Result to set
     * @param[in] handle_type Handle type
     * @param[in] handle Handle to set
     * @internal
     */
    void set_handle_reference(ots_result_t* result, ots_handle_type handle_type, void* handle) {
        if(!result)
            return;
        set_result_type(result, OTS_RESULT_HANDLE);
        result->result.handle = create_handle_reference(handle_type, handle);
    }

    /**
     * @brief Set string to result
     * @param[out] result Result to set
     * @param[in] str String to set
     * @internal
     */
    void set_string(ots_result_t* result, const std::string& str) {
        if(result == nullptr)
            return;
        set_result_type(result, OTS_RESULT_STRING);
        result->result.data.ptr = create_string_copy(str);
        result->result.data.size = str.length();
        result->result.data.type = OTS_DATA_CHAR;
        result->result.data.reference = false;
    }

    /**
     * @brief Set boolean to result
     * @param[out] result Result to set
     * @param[in] value Boolean value
     * @internal
     */
    void set_boolean(ots_result_t* result, bool value) {
        if(!result)
            return;
        set_result_type(result, OTS_RESULT_BOOLEAN);
        result->result.boolean = value;
    }

    /**
     * @brief Set number to result
     * @param[out] result Result to set
     * @param[in] value Number value
     * @internal
     */
    void set_number(ots_result_t* result, int64_t value) {
        if(!result)
            return;
        set_result_type(result, OTS_RESULT_NUMBER);
        result->result.number = value;
    }

    /**
     * @brief Set comparison result to result
     * @param[out] result Result to set
     * @param[in] value Comparison result
     * @internal
     */
    void set_comparison(ots_result_t* result, int64_t value) {
        if(!result)
            return;
        set_result_type(result, OTS_RESULT_COMPARISON);
        result->result.number = value;
    }

    /**
     * @brief Set array to result
     * @param[out] result Result to set
     * @param[in] arr Array to set
     * @param[in] size Size of array
     * @param[in] data_type Data type of array
     * @param[in] reference If true, the handle does not own the object, don't free
     * @internal
     */
    void set_array(ots_result_t* result, void* arr, size_t size, ots_data_type data_type, bool reference) {
        if(!result)
            return;
        set_result_type(result, OTS_RESULT_ARRAY);
        result->result.data.ptr = arr;
        result->result.data.size = size;
        result->result.data.type = data_type;
        result->result.data.reference = reference;
    }

    void set_address_index(ots_result_t* result, const uint32_t* index) {
        if(!result)
            return;
        set_result_type(result, OTS_RESULT_ADDRESS_INDEX);
        result->result.data.ptr = (void*)index;
        result->result.data.size = 2;
        result->result.data.type = OTS_DATA_UINT32;
        result->result.data.reference = false;
    }

    /**
     * @brief Set address type to result
     * @param[out] result Result to set
     * @param[in] type Address type
     * @internal
     */
    void set_address_type(ots_result_t* result, ots::AddressType type) {
        if(!result)
            return;
        set_result_type(result, OTS_RESULT_ADDRESS_TYPE);
        result->result.number = static_cast<int64_t>(type);
    }

    /**
     * @brief Set network to result
     * @param[out] result Result to set
     * @param[in] network Network
     * @internal
     */
    void set_network(ots_result_t* result, ots::Network network) {
        if(!result)
            return;
        set_result_type(result, OTS_RESULT_NETWORK);
        result->result.number = static_cast<int64_t>(network);
    }

    /**
     * @brief Set seed type to result
     * @param[out] result Result to set
     * @param[in] type Seed type
     * @internal
     */
    void set_seed_type(ots_result_t* result, ots::SeedType type) {
        if(!result)
            return;
        set_result_type(result, OTS_RESULT_SEED_TYPE);
        result->result.number = static_cast<int64_t>(type);
    }

    /**
     * @brief Set error to result
     * @param[out] result Result to set
     * @param[in] e Exception to set
     * @internal
     */
    void set_error(ots_result_t* result, const ots::exception::Exception& e) {
        if(!result)
            return;
        translate_exception(&result->error, e);
        result->result.data.ptr = nullptr;
    }

    /**
     * @brief Convert C network to C++ network
     * @param[in] network Network to convert
     * @return C++ network
     * @internal
     */
    ots::Network to_cpp_network(OTS_NETWORK network) {
        switch(network) {
            case OTS_NETWORK_TEST:
                return ots::Network::TEST;
            case OTS_NETWORK_STAGE:
                return ots::Network::STAGE;
            default:
                return ots::Network::MAIN;
        }
    }
}

extern "C" {
    bool ots_is_error(const ots_result_t* result) {
        return result && result->error.code != 0;
    }

    char* ots_get_error_message(const ots_result_t* result) {
        if(!result)
            return nullptr;
        return create_string_copy(result->error.message);
    }

    char* ots_get_error_class(const ots_result_t* result) {
        if(!result)
            return nullptr;
        return create_string_copy(result->error.cls);
    }

    int32_t ots_get_error_code(const ots_result_t* result) {
        if(!result)
            return 0;
        return result->error.code;
    }

    bool ots_is_result(const ots_result_t* result) {
        return result && result->error.code == 0;
    }

    bool ots_result_is_type(const ots_result_t* result, ots_result_type type) {
        if(result == nullptr)
            return false;
        return result->type == type;
    }

    bool ots_result_data_is_type(const ots_result_t* result, ots_data_type type) {
        if(!result)
            return false;
        switch(result->type) {
            case OTS_RESULT_ARRAY:
            case OTS_RESULT_STRING:
                return result->result.data.type == type;
            default:
                return false;
        }
    }

    bool ots_result_data_is_reference(const ots_result_t* result) {
        if(!result)
            return false;
        switch(result->type) {
            case OTS_RESULT_HANDLE:
            case OTS_RESULT_ARRAY:
            case OTS_RESULT_STRING:
                return result->result.data.reference;
            default:
                return false;
        }
    }

    const char* ots_result_string(const ots_result_t* result) {
        if(!result)
            return nullptr;
        if(ots_result_is_type(result, OTS_RESULT_STRING))
            return static_cast<char*>(result->result.data.ptr);
        if(
            ots_result_is_type(result, OTS_RESULT_HANDLE) ||
            result->result.handle.type == OTS_HANDLE_WIPEABLE_STRING
            ) {
            try {
                return static_cast<ots::WipeableString*>(result->result.handle.ptr)->c_str();
            } catch(const ots::exception::Exception& e) {}
        }
        return nullptr;
    }

    size_t ots_result_string_size(const ots_result_t* result) {
        if(!result)
            return 0;
        if(ots_result_is_type(result, OTS_RESULT_STRING))
            return result->result.data.size;
        if(
            ots_result_is_type(result, OTS_RESULT_HANDLE) ||
            result->result.handle.type == OTS_HANDLE_WIPEABLE_STRING
            ) {
            try {
                return static_cast<ots::WipeableString*>(result->result.handle.ptr)->size();
            } catch(const ots::exception::Exception& e) {}
        }
        return 0;
    }

    char* ots_result_string_copy(const ots_result_t* result) {
        const char* out = ots_result_string(result);
        if(!out)
            return nullptr;
        return create_string_copy(out);
    }

    bool ots_result_boolean(const ots_result_t* result, bool default_value) {
        if(!result || !ots_result_is_type(result, OTS_RESULT_BOOLEAN))
            return default_value;
        return result->result.boolean;
    }

    int64_t ots_result_number(const ots_result_t* result, int64_t default_value) {
        if(!result || !ots_result_is_type(result, OTS_RESULT_NUMBER))
            return default_value;
        return result->result.number;
    }

    void* ots_result_array(const ots_result_t* result) {
        if(!result || !ots_result_is_type(result, OTS_RESULT_ARRAY))
            return nullptr;
        return result->result.data.ptr;
    }

    bool ots_result_is_comparison(const ots_result_t* result) {
        return ots_result_is_type(result, OTS_RESULT_COMPARISON);
    }

    int64_t ots_result_comparison(const ots_result_t* result) {
        if(!result || !ots_result_is_type(result, OTS_RESULT_COMPARISON))
            return 0;
        return result->result.number;
    }

    bool ots_result_is_equal(const ots_result_t* result) {
        return ots_result_comparison(result) == 0;
    }

    size_t ots_result_size(const ots_result_t* result) {
        if(!result)
            return 0;
        return result->result.data.size;
    }

    bool ots_result_is_address_type(const ots_result_t* result) {
        return ots_result_is_type(result, OTS_RESULT_ADDRESS_TYPE);
    }

    bool ots_result_is_address_index(const ots_result_t* result) {
        return ots_result_is_type(result, OTS_RESULT_ADDRESS_INDEX) &&
            ots_result_data_is_type(result, OTS_DATA_UINT32) &&
            ots_result_size(result) == 2;
    }

    uint32_t ots_result_address_index_account(const ots_result_t* result) {
        if(!ots_result_is_address_index(result))
            return 0;
        return static_cast<uint32_t*>(result->result.data.ptr)[0];
    }

    uint32_t ots_result_address_index_index(const ots_result_t* result) {
        if(!ots_result_is_address_index(result))
            return 0;
        return static_cast<uint32_t*>(result->result.data.ptr)[1];
    }

    bool ots_result_address_type_is_type(const ots_result_t* result, OTS_ADDRESS_TYPE type) {
        if(!result || !ots_result_is_type(result, OTS_RESULT_ADDRESS_TYPE))
            return false;
        return result->result.number == static_cast<int64_t>(type);
    }

    bool ots_result_is_network(const ots_result_t* result) {
        return ots_result_is_type(result, OTS_RESULT_NETWORK);
    }

    bool ots_result_network_is_type(const ots_result_t* result, OTS_NETWORK network) {
        if(!result || !ots_result_is_type(result, OTS_RESULT_NETWORK))
            return false;
        return result->result.number == static_cast<int64_t>(network);
    }

    bool ots_result_is_seed_type(const ots_result_t* result) {
        return ots_result_is_type(result, OTS_RESULT_SEED_TYPE);
    }

    bool ots_result_seed_type_is_type(const ots_result_t* result, OTS_SEED_TYPE type) {
        if(!result || !ots_result_is_type(result, OTS_RESULT_SEED_TYPE))
            return false;
        return result->result.number == static_cast<int64_t>(type);
    }

    ots_handle_t* ots_result_handle(const ots_result_t* result) {
        if(!result || !ots_result_is_type(result, OTS_RESULT_HANDLE))
            return nullptr;
        return new ots_handle_t{result->result.handle};
    }

    bool ots_result_handle_is_type(const ots_result_t* result, ots_handle_type type) {
        if(!result || !ots_result_is_type(result, OTS_RESULT_HANDLE))
            return false;
        return result->result.handle.type == type;
    }

    bool ots_result_handle_is_reference(const ots_result_t* result) {
        if(!result || !ots_result_is_type(result, OTS_RESULT_HANDLE))
            return false;
        return result->result.handle.reference;
    }

    void ots_free_string(char** str) {
        if(!*str)
            return;
        memset(*str, 0, strlen(*str)); // Securely wipe memory always, cost is low, see no reason not to do it
        delete[] *str;
        *str = nullptr;
    }

    void ots_free_array(void** arr, size_t elem_size, size_t count) {
        if (!arr || !*arr)
            return;
        size_t total_size = elem_size * count;
        if (total_size > 0)
            memset(*arr, 0, total_size);
        delete[] static_cast<unsigned char*>(*arr); 
        *arr = nullptr;
    }

    void ots_free_result(ots_result_t** result) {
        if(!result || !*result)
            return;
        switch((*result)->type) {
            case OTS_RESULT_STRING:
                memset(&(*result)->result.data, 0, (*result)->result.data.size);
                break;
            case OTS_RESULT_ARRAY: {
                if((*result)->result.data.reference)
                    break;
                size_t size = (*result)->result.data.size;
                switch((*result)->result.data.type) {
                    case OTS_DATA_UINT16:
                        size *= sizeof(uint16_t);
                        break;
                    default:
                        break;
                }
                void** data = &(*result)->result.data.ptr;
                ots_free_array(data, size, (*result)->result.data.size);
                break;
            }
            case OTS_RESULT_HANDLE: {
                if((*result)->result.handle.reference)
                    break;
                switch((*result)->result.handle.type) {
                    OTS_HANDLE_WIPEABLE_STRING:
                        delete static_cast<ots::WipeableString*>((*result)->result.handle.ptr);
                        break;
                    case OTS_HANDLE_SEED_INDICES:
                        delete static_cast<ots::SeedIndices*>((*result)->result.handle.ptr);
                        break;
                    case OTS_HANDLE_SEED:
                        delete static_cast<ots::Seed*>((*result)->result.handle.ptr);
                        break;
                    case OTS_HANDLE_WALLET:
                        delete static_cast<ots::Wallet*>((*result)->result.handle.ptr);
                        break;
                    case OTS_HANDLE_TX:
                        delete static_cast<ots::TxDescription*>((*result)->result.handle.ptr);
                        break;
                    default:
                        break;
                }
                break;
            }
            default:
                break;
        }
        delete *result;
        *result = nullptr;
    }

    void ots_free_handle(ots_handle_t** handle) {
        if(!*handle)
            return;
        if(!(*handle)->reference)
            switch((*handle)->type) {
                case OTS_HANDLE_WIPEABLE_STRING:
                    delete static_cast<ots::WipeableString*>((*handle)->ptr);
                    break;
                case OTS_HANDLE_SEED_INDICES:
                    delete static_cast<ots::SeedIndices*>((*handle)->ptr);
                    break;
                case OTS_HANDLE_SEED:
                    delete static_cast<ots::Seed*>((*handle)->ptr);
                    break;
                case OTS_HANDLE_WALLET:
                    delete static_cast<ots::Wallet*>((*handle)->ptr);
                    break;
                case OTS_HANDLE_TX:
                    delete static_cast<ots::TxDescription*>((*handle)->ptr);
                    break;
                case OTS_HANDLE_SEED_LANGUAGE: // is always a reference
                default:
                    break;
            }
        delete *handle;
        *handle = nullptr;
    }

    void ots_secure_free(void** buffer, size_t size) {
        if(!buffer || !*buffer || size == 0)
            return;
        memset(*buffer, 0, size);
        delete[] static_cast<unsigned char*>(*buffer);
        *buffer = nullptr;
    }

    ots_result_t* ots_wipeable_string_create(const char* str) {
        ots_result_t* result = new ots_result_t();
        try {
            set_handle(result, OTS_HANDLE_WIPEABLE_STRING, new ots::WipeableString(str));
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wipeable_string_compare(
            const ots_handle_t* str1,
            const ots_handle_t* str2
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(!str1 || !str2 || str1->type != OTS_HANDLE_WIPEABLE_STRING || str2->type != OTS_HANDLE_WIPEABLE_STRING)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_comparison(
                result,
                static_cast<ots::WipeableString*>(str1->ptr)->compare(
                    *static_cast<ots::WipeableString*>(str2->ptr)
                )
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    const char* ots_wipeable_string_c_str(const ots_handle_t* str) {
        if(!str || str->type != OTS_HANDLE_WIPEABLE_STRING)
            return nullptr;
        try {
            return static_cast<ots::WipeableString*>(str->ptr)->c_str();
        } catch(const ots::exception::Exception& e) {
            return nullptr;
        }
    }

    ots_result_t* ots_seed_indices_create(uint16_t* indices, size_t size) {
        ots_result_t* result = new ots_result_t();
        try {
            set_handle(result, OTS_HANDLE_SEED_INDICES, new ots::SeedIndices(indices, size));
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_indices_create_from_string(const char* str, const char* separator) {
        ots_result_t* result = new ots_result_t();
        try {
            set_handle(
                result,
                OTS_HANDLE_SEED_INDICES,
                new ots::SeedIndices(ots::SeedIndices::fromNumeric(str, separator))
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_indices_create_from_hex(const char* hex, const char* separator) {
        ots_result_t* result = new ots_result_t();
        try {
            set_handle(
                result,
                OTS_HANDLE_SEED_INDICES,
                new ots::SeedIndices(ots::SeedIndices::fromHex(hex, separator))
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    const uint16_t* ots_seed_indices_values(const ots_handle_t* handle) {
        if(handle->type != OTS_HANDLE_SEED_INDICES)
            return nullptr;
        return *static_cast<ots::SeedIndices*>(handle->ptr);
    }

    size_t ots_seed_indices_count(const ots_handle_t* handle) {
        if(handle->type != OTS_HANDLE_SEED_INDICES)
            return 0;
        return static_cast<ots::SeedIndices*>(handle->ptr)->size();
    }

    void ots_seed_indices_clear(const ots_handle_t* handle) {
        if(handle->type != OTS_HANDLE_SEED_INDICES)
            return;
        static_cast<ots::SeedIndices*>(handle->ptr)->clear();
    }

    void ots_seed_indices_append(const ots_handle_t* handle, uint16_t index) {
        if(handle->type != OTS_HANDLE_SEED_INDICES)
            return;
        static_cast<ots::SeedIndices*>(handle->ptr)->emplace_back(index);
    }

    char* ots_seed_indices_numeric(const ots_handle_t* handle, const char* separator) {
        if(handle->type != OTS_HANDLE_SEED_INDICES)
            return nullptr;
        return create_string_copy(static_cast<ots::SeedIndices*>(handle->ptr)->numeric(separator));
    }

    char* ots_seed_indices_hex(const ots_handle_t* handle, const char* separator) {
        if(handle->type != OTS_HANDLE_SEED_INDICES)
            return nullptr;
        return create_string_copy(static_cast<ots::SeedIndices*>(handle->ptr)->hex(separator));
    }

    ots_result_t* ots_seed_languages(void) {
        ots_result_t* result = new ots_result_t();
        try {
            auto languages = ots::SeedLanguage::list();
            ots_handle_t* out = new ots_handle_t[languages.size()];
            for (size_t i = 0; i < languages.size(); ++i)
                out[i] = create_handle_reference(OTS_HANDLE_SEED_LANGUAGE, (void *)&(languages[i].get())); // references
            set_array(result, out, languages.size(), OTS_DATA_HANDLE, false); // handles itself are no references
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_languages_for_type(OTS_SEED_TYPE type) {
        ots_result_t* result = new ots_result_t();
        try {
            auto languages = ots::SeedLanguage::listFor(static_cast<ots::SeedType>(type));
            ots_handle_t* out = new ots_handle_t[languages.size()];
            for (size_t i = 0; i < languages.size(); ++i)
                out[i] = create_handle_reference(OTS_HANDLE_SEED_LANGUAGE, (void *)&(languages[i].get())); // references
            set_array(result, out, languages.size(), OTS_DATA_HANDLE, false); // handles itself are no references
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_language_for_code(const char* code) {
        ots_result_t* result = new ots_result_t();
        try {
            auto language = ots::SeedLanguage::fromCode(code);
            set_handle_reference(result, OTS_HANDLE_SEED_LANGUAGE, (void *)&language);
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_language_for_name(const char* name) {
        ots_result_t* result = new ots_result_t();
        try {
            auto language = ots::SeedLanguage::fromName(name);
            set_handle_reference(result, OTS_HANDLE_SEED_LANGUAGE, (void *)&language);
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_language_for_english_name(const char* name) {
        ots_result_t* result = new ots_result_t();
        try {
            auto language = ots::SeedLanguage::fromEnglishName(name);
            set_handle_reference(result, OTS_HANDLE_SEED_LANGUAGE, (void *)&language);
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_language_default(OTS_SEED_TYPE type) {
        ots_result_t* result = new ots_result_t();
        try {
            auto language = ots::SeedLanguage::defaultLanguage(static_cast<ots::SeedType>(type));
            set_handle_reference(result, OTS_HANDLE_SEED_LANGUAGE, (void *)&language);
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_language_set_default(OTS_SEED_TYPE type, const ots_handle_t* language) {
        ots_result_t* result = new ots_result_t();
        try {
            if(language->type != OTS_HANDLE_SEED_LANGUAGE)
                throw ots::exception::InvalidArgument("Invalid handle type");
            ots::SeedLanguage::setDefaultLanguage(static_cast<ots::SeedType>(type), *static_cast<const ots::SeedLanguage*>(language->ptr));
            set_handle_reference(result, OTS_HANDLE_SEED_LANGUAGE, (void *)&(ots::SeedLanguage::defaultLanguage(static_cast<ots::SeedType>(type))));
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_language_from_code(const char* code) {
        ots_result_t* result = new ots_result_t();
        try {
            auto language = ots::SeedLanguage::fromCode(code);
            set_handle_reference(result, OTS_HANDLE_SEED_LANGUAGE, (void *)&language);
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_language_from_name(const char* name) {
        ots_result_t* result = new ots_result_t();
        try {
            auto language = ots::SeedLanguage::fromName(name);
            set_handle_reference(result, OTS_HANDLE_SEED_LANGUAGE, (void *)&language);
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_language_from_english_name(const char* name) {
        ots_result_t* result = new ots_result_t();
        try {
            auto language = ots::SeedLanguage::fromEnglishName(name);
            set_handle_reference(result, OTS_HANDLE_SEED_LANGUAGE, (void *)&language);
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_language_code(const ots_handle_t* language) {
        ots_result_t* result = new ots_result_t();
        try {
            if(language->type != OTS_HANDLE_SEED_LANGUAGE)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_string(result, static_cast<const ots::SeedLanguage*>(language->ptr)->code());
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_language_name(const ots_handle_t* language) {
        ots_result_t* result = new ots_result_t();
        try {
            if(language->type != OTS_HANDLE_SEED_LANGUAGE)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_string(result, static_cast<const ots::SeedLanguage*>(language->ptr)->name());
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_language_english_name(const ots_handle_t* language) {
        ots_result_t* result = new ots_result_t();
        try {
            if(language->type != OTS_HANDLE_SEED_LANGUAGE)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_string(result, static_cast<const ots::SeedLanguage*>(language->ptr)->englishName());
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_language_supported(const ots_handle_t* language, OTS_SEED_TYPE type) {
        ots_result_t* result = new ots_result_t();
        try {
            if(language->type != OTS_HANDLE_SEED_LANGUAGE)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_boolean(
                result,
                static_cast<const ots::SeedLanguage*>(language->ptr)->supported(static_cast<ots::SeedType>(type))
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_language_is_default(const ots_handle_t* language, OTS_SEED_TYPE type) {
        ots_result_t* result = new ots_result_t();
        try {
            if(language->type != OTS_HANDLE_SEED_LANGUAGE)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_boolean(
                result,
                static_cast<const ots::SeedLanguage*>(language->ptr)->isDefault(static_cast<ots::SeedType>(type))
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_language_equals(const ots_handle_t* language1, const ots_handle_t* language2) {
        ots_result_t* result = new ots_result_t();
        try {
            if(language1->type != OTS_HANDLE_SEED_LANGUAGE || language2->type != OTS_HANDLE_SEED_LANGUAGE)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_boolean(
                result,
                *static_cast<const ots::SeedLanguage*>(language1->ptr) == *static_cast<const ots::SeedLanguage*>(language2->ptr)
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_language_equals_code(const ots_handle_t* language, const char* code) {
        ots_result_t* result = new ots_result_t();
        try {
            if(language->type != OTS_HANDLE_SEED_LANGUAGE)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_boolean(
                result,
                *static_cast<const ots::SeedLanguage*>(language->ptr) == code
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_phrase(
            const ots_handle_t* seed,
            const ots_handle_t* language,
            const char* password
            ){
        ots_result_t* result = new ots_result_t();
        try {
            if(seed->type != OTS_HANDLE_SEED)
                throw ots::exception::InvalidArgument("Invalid handle type");
            if(language->type != OTS_HANDLE_SEED_LANGUAGE)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_handle(
                    result,
                    OTS_HANDLE_WIPEABLE_STRING,
                    new ots::WipeableString(static_cast<ots::Seed*>(seed->ptr)->phrase(*static_cast<ots::SeedLanguage*>(language->ptr), password))
                    );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_phrase_for_language_code(
            const ots_handle_t* seed,
            const char* language_code,
            const char* password
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(seed->type != OTS_HANDLE_SEED)
                throw ots::exception::InvalidArgument("Invalid handle type");
            auto language = ots::SeedLanguage::fromCode(language_code);
            set_handle(
                    result,
                    OTS_HANDLE_WIPEABLE_STRING,
                    new ots::WipeableString(static_cast<ots::Seed*>(seed->ptr)->phrase(language, password))
                    );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_indices(
            const ots_handle_t* handle,
            const char* password
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(handle->type != OTS_HANDLE_SEED)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_handle(
                    result,
                    OTS_HANDLE_SEED_INDICES,
                    new ots::SeedIndices(static_cast<ots::Seed*>(handle->ptr)->indices(password))
                    );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_fingerprint(const ots_handle_t* handle) {
        ots_result_t* result = new ots_result_t();
        try {
            if(handle->type != OTS_HANDLE_SEED)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_string(result, static_cast<ots::Seed*>(handle->ptr)->fingerprint());
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_address(const ots_handle_t* handle) {
        ots_result_t* result = new ots_result_t();
        try {
            if(handle->type != OTS_HANDLE_SEED)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_handle_reference(
                    result,
                    OTS_HANDLE_ADDRESS,
                    (void*)&static_cast<ots::Seed*>(handle->ptr)->address()
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_timestamp(const ots_handle_t* handle) {
        ots_result_t* result = new ots_result_t();
        try {
            if(handle->type != OTS_HANDLE_SEED)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_number(result, static_cast<ots::Seed*>(handle->ptr)->timestamp());
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_height(const ots_handle_t* handle) {
        ots_result_t* result = new ots_result_t();
        try {
            if(handle->type != OTS_HANDLE_SEED)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_number(result, static_cast<ots::Seed*>(handle->ptr)->height());
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_network(const ots_handle_t* handle) {
        ots_result_t* result = new ots_result_t();
        try {
            if(handle->type != OTS_HANDLE_SEED)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_number(result, static_cast<int64_t>(static_cast<ots::Seed*>(handle->ptr)->network()));
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_wallet(const ots_handle_t* handle) {
        ots_result_t* result = new ots_result_t();
        try {
            if(handle->type != OTS_HANDLE_SEED)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_handle_reference(
                result,
                OTS_HANDLE_WALLET,
                static_cast<ots::Seed*>(handle->ptr)->wallet().get()
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_indices_merge_values(
            const ots_handle_t* seed_indices1,
            const ots_handle_t* seed_indices2
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(
                seed_indices1->type != OTS_HANDLE_SEED ||
                seed_indices2->type != OTS_HANDLE_SEED_INDICES
            )
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_handle(
                result,
                OTS_HANDLE_SEED_INDICES,
                new ots::SeedIndices(
                    ots::Seed::mergeValues(
                        static_cast<const std::vector<uint16_t>>(
                            *static_cast<ots::SeedIndices*>(seed_indices1->ptr)),
                        static_cast<const std::vector<uint16_t>>(
                            *static_cast<ots::SeedIndices*>(seed_indices2->ptr))
                    )
                )
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_indices_merge_with_password(
            const char* password,
            const ots_handle_t* seed_indices
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(
                seed_indices->type != OTS_HANDLE_SEED_INDICES
            )
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_handle(
                result,
                OTS_HANDLE_SEED_INDICES,
                new ots::SeedIndices(
                    ots::Seed::mergeWithPassword(
                        password,
                        static_cast<const std::vector<uint16_t>>(
                            *static_cast<ots::SeedIndices*>(seed_indices->ptr))
                    )
                )
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_indices_merge_multiple_values(
            const ots_handle_t* seed_indices[],
            size_t elements,
            size_t count
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            std::vector<std::vector<uint16_t>> indices;
            for(size_t i = 0; i < count; ++i) {
                if(seed_indices[i]->type != OTS_HANDLE_SEED_INDICES)
                    throw ots::exception::InvalidArgument("Invalid handle type");
                indices.emplace_back(static_cast<const std::vector<uint16_t>>(
                            *static_cast<ots::SeedIndices*>(seed_indices[i]->ptr)));
            }
            set_handle(
                result,
                OTS_HANDLE_SEED_INDICES,
                new ots::SeedIndices(ots::Seed::mergeValues(indices))
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_indices_merge_values_and_zero(
            const ots_handle_t* seed_indices1,
            const ots_handle_t* seed_indices2,
            bool delete_after
            ) {
        ots_result_t* result = ots_seed_indices_merge_values(seed_indices1, seed_indices2);
        if(ots_is_error(result))
            return result;
        try {
            if(delete_after) {
                ots_free_handle(const_cast<ots_handle_t**>(&seed_indices1));
                ots_free_handle(const_cast<ots_handle_t**>(&seed_indices2));
            }
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_indices_merge_with_password_and_zero(
            const char* password,
            const ots_handle_t* seed_indices,
            bool delete_after
            ) {
        ots_result_t* result = ots_seed_indices_merge_with_password(password, seed_indices);
        if(ots_is_error(result))
            return result;
        try {
            if(delete_after)
                ots_free_handle(const_cast<ots_handle_t**>(&seed_indices));
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_indices_merge_multiple_values_and_zero(
            const ots_handle_t* seed_indices[],
            size_t elements,
            size_t count,
            bool delete_after
            ) {
        ots_result_t* result = ots_seed_indices_merge_multiple_values(seed_indices, elements, count);
        if(ots_is_error(result))
            return result;
        try {
            if(delete_after)
                for(size_t i = 0; i < count; ++i)
                    ots_free_handle(const_cast<ots_handle_t**>(&seed_indices[i]));
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_legacy_seed_decode(
            const char* phrase,
            uint64_t height,
            uint64_t timestamp,
            OTS_NETWORK network
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            set_handle(
                result,
                OTS_HANDLE_SEED,
                new ots::LegacySeed(ots::LegacySeed::decode(phrase, height, timestamp, to_cpp_network(network)))
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_legacy_seed_decode_indices(
            const ots_handle_t* indices,
            uint64_t height,
            uint64_t timestamp,
            OTS_NETWORK network
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(indices->type != OTS_HANDLE_SEED_INDICES)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_handle(
                result,
                OTS_HANDLE_SEED,
                new ots::LegacySeed(ots::LegacySeed::decode(
                    static_cast<const std::vector<uint16_t>>(
                        *static_cast<ots::SeedIndices*>(indices->ptr)
                    ),
                    height,
                    timestamp,
                    to_cpp_network(network)
                ))
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_monero_seed_create(
            const uint8_t random[32],
            uint64_t height,
            uint64_t timestamp,
            OTS_NETWORK network
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            std::array<uint8_t, 32> random_array;
            memcpy(random_array.data(), random, 32);
            set_handle(
                result,
                OTS_HANDLE_SEED,
                new ots::MoneroSeed(ots::MoneroSeed::create(random_array, height, timestamp, to_cpp_network(network)))
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_monero_seed_generate(
            uint64_t height,
            uint64_t timestamp,
            OTS_NETWORK network
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            set_handle(
                result,
                OTS_HANDLE_SEED,
                new ots::MoneroSeed(ots::MoneroSeed::generate(height, timestamp, to_cpp_network(network)))
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_monero_seed_decode(
            const char* phrase,
            uint64_t height,
            uint64_t timestamp,
            OTS_NETWORK network,
            const char* password
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            set_handle(
                result,
                OTS_HANDLE_SEED,
                new ots::MoneroSeed(ots::MoneroSeed::decode(phrase, height, timestamp, to_cpp_network(network), password))
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_monero_seed_decode_indices(
            const ots_handle_t* indices,
            uint64_t height,
            uint64_t timestamp,
            OTS_NETWORK network,
            const char* password
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(indices->type != OTS_HANDLE_SEED_INDICES)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_handle(
                result,
                OTS_HANDLE_SEED,
                new ots::MoneroSeed(ots::MoneroSeed::decode(
                    *static_cast<ots::SeedIndices*>(indices->ptr),
                    height,
                    timestamp,
                    to_cpp_network(network),
                    password
                ))
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_polyseed_create(
            const uint8_t random[19],
            OTS_NETWORK network,
            uint64_t timestamp,
            const char* passphrase
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            std::array<uint8_t, 19> random_array;
            memcpy(random_array.data(), random, 19);
            set_handle(
                result,
                OTS_HANDLE_SEED,
                new ots::Polyseed(
                    ots::Polyseed::create(
                        random_array,
                        to_cpp_network(network),
                        timestamp,
                        passphrase
                    )
                )
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_polyseed_generate(
            OTS_NETWORK network,
            uint64_t timestamp,
            const char* passphrase
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            set_handle(
                result,
                OTS_HANDLE_SEED,
                new ots::Polyseed(
                    ots::Polyseed::generate(
                        to_cpp_network(network),
                        timestamp,
                        passphrase
                    )
                )
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_polyseed_decode(
            const char* phrase,
            OTS_NETWORK network,
            const char* password,
            const char* passphrase
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            set_handle(
                result,
                OTS_HANDLE_SEED,
                new ots::Polyseed(
                    ots::Polyseed::decode(
                        phrase,
                        to_cpp_network(network),
                        password,
                        passphrase
                    )
                )
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_polyseed_decode_indices(
            const ots_handle_t* indices,
            OTS_NETWORK network,
            const char* password,
            const char* passphrase
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(indices->type != OTS_HANDLE_SEED_INDICES)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_handle(
                result,
                OTS_HANDLE_SEED,
                new ots::Polyseed(
                    ots::Polyseed::decode(
                        *static_cast<ots::SeedIndices*>(indices->ptr),
                        to_cpp_network(network),
                        password,
                        passphrase
                    )
                )
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_polyseed_decode_with_language(
            const char* phrase,
            const ots_handle_t* language,
            OTS_NETWORK network,
            const char* password,
            const char* passphrase
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(language->type != OTS_HANDLE_SEED_LANGUAGE)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_handle(
                result,
                OTS_HANDLE_SEED,
                new ots::Polyseed(
                    ots::Polyseed::decode(
                        phrase,
                        *static_cast<const ots::SeedLanguage*>(language->ptr),
                        to_cpp_network(network),
                        password,
                        passphrase
                    )
                )
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_polyseed_decode_with_language_code(
            const char* phrase,
            const char* language_code,
            OTS_NETWORK network,
            const char* password,
            const char* passphrase
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            set_handle(
                result,
                OTS_HANDLE_SEED,
                new ots::Polyseed(
                    ots::Polyseed::decode(
                        phrase,
                        ots::SeedLanguage::fromCode(language_code),
                        to_cpp_network(network),
                        password,
                        passphrase
                    )
                )
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_address_create(
            const char* address
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            set_handle(
                result,
                OTS_HANDLE_ADDRESS,
                new ots::Address(address)
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_address_type(const ots_handle_t* handle) {
        ots_result_t* result = new ots_result_t();
        try {
            if(handle->type != OTS_HANDLE_ADDRESS)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_address_type(
                result,
                static_cast<ots::Address*>(handle->ptr)->type()
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_address_network(const ots_handle_t* handle) {
        ots_result_t* result = new ots_result_t();
        try {
            if(handle->type != OTS_HANDLE_ADDRESS)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_network(
                result,
                static_cast<ots::Address*>(handle->ptr)->network()
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_address_fingerprint(const ots_handle_t* handle) {
        ots_result_t* result = new ots_result_t();
        try {
            if(handle->type != OTS_HANDLE_ADDRESS)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_string(
                result,
                static_cast<ots::Address*>(handle->ptr)->fingerprint()
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_address_is_integrated(const ots_handle_t* handle) {
        ots_result_t* result = new ots_result_t();
        try {
            if(handle->type != OTS_HANDLE_ADDRESS)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_boolean(
                result,
                static_cast<ots::Address*>(handle->ptr)->isIntegrated()
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_address_payment_id(const ots_handle_t* handle) {
        ots_result_t* result = new ots_result_t();
        try {
            if(handle->type != OTS_HANDLE_ADDRESS)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_string(
                result,
                static_cast<ots::Address*>(handle->ptr)->paymentID()
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_address_from_integrated(
            const ots_handle_t* handle
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(handle->type != OTS_HANDLE_ADDRESS)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_handle(
                result,
                OTS_HANDLE_ADDRESS,
                new ots::Address(static_cast<ots::Address*>(handle->ptr)->integratedAddress())
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_address_length(const ots_handle_t* handle) {
        ots_result_t* result = new ots_result_t();
        try {
            if(handle->type != OTS_HANDLE_ADDRESS)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_number(
                result,
                static_cast<int64_t>(static_cast<ots::Address*>(handle->ptr)->length())
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_address_base58_string(const ots_handle_t* handle) {
        ots_result_t* result = new ots_result_t();
        try {
            if(handle->type != OTS_HANDLE_ADDRESS)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_string(
                result,
                *static_cast<ots::Address*>(handle->ptr)
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_address_equal(
            const ots_handle_t* address1,
            const ots_handle_t* address2
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(address1->type != OTS_HANDLE_ADDRESS || address2->type != OTS_HANDLE_ADDRESS)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_boolean(
                result,
                *static_cast<ots::Address*>(address1->ptr) == *static_cast<ots::Address*>(address2->ptr)
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_address_equal_string(
            const ots_handle_t* address,
            const char* string
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(address->type != OTS_HANDLE_ADDRESS)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_boolean(
                result,
                *static_cast<ots::Address*>(address->ptr) == string
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_address_string_valid(
            const char* address,
            OTS_NETWORK network
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            set_boolean(
                result,
                ots::Address::isValid(address, to_cpp_network(network))
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_address_string_network(
            const char* address
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            set_network(
                result,
                ots::Address::network(address)
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_address_string_type(
            const char* address
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            set_address_type(
                result,
                ots::Address::type(address)
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_address_string_fingerprint(
            const char* address
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            set_string(
                result,
                ots::Address::fingerprint(address)
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_address_string_is_integrated(
            const char* address
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            set_boolean(
                result,
                ots::Address::isIntegrated(address)
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_address_string_payment_id(
            const char* address
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            set_string(
                result,
                ots::Address::paymentID(address)
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_address_string_integrated(
            const char* address
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            set_string(
                result,
                ots::Address::integratedAddress(address)
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_create(
            const uint8_t key[32],
            uint64_t height,
            OTS_NETWORK network
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            std::array<uint8_t, 32> key_array;
            memcpy(key_array.data(), key, 32);
            set_handle(
                result,
                OTS_HANDLE_WALLET,
                new ots::Wallet(key_array, height, to_cpp_network(network))
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }
    
    ots_result_t* ots_wallet_height(const ots_handle_t* wallet) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_number(
                result,
                static_cast<int64_t>(static_cast<ots::Wallet*>(wallet->ptr)->height())
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_address(const ots_handle_t* wallet) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_handle(
                result,
                OTS_HANDLE_ADDRESS,
                new ots::Address(static_cast<ots::Wallet*>(wallet->ptr)->address())
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_subaddress(
            const ots_handle_t* wallet,
            uint32_t account,
            uint32_t index
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_handle(
                result,
                OTS_HANDLE_ADDRESS,
                new ots::Address(static_cast<ots::Wallet*>(wallet->ptr)->address(account, index))
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_accounts(
            const ots_handle_t* wallet,
            uint32_t max,
            uint32_t offset
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            std::vector<ots::Address> addresses = static_cast<ots::Wallet*>(wallet->ptr)->accounts(max, offset);
            ots_handle_t* handles = new ots_handle_t[addresses.size()];
            for(size_t i = 0; i < addresses.size(); i++) {
                ots::Address* address = new ots::Address(addresses[i]);
                handles[i] = create_handle(OTS_HANDLE_ADDRESS, address);
            }
            set_array(
                result,
                handles,
                addresses.size(),
                OTS_DATA_HANDLE,
                true
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_subaddresses(
            const ots_handle_t* wallet,
            uint32_t account,
            uint32_t max,
            uint32_t offset
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            std::vector<ots::Address> addresses = static_cast<ots::Wallet*>(wallet->ptr)->subAddresses(account, max, offset);
            ots_handle_t* handles = new ots_handle_t[addresses.size()];
            for(size_t i = 0; i < addresses.size(); i++) {
                ots::Address* address = new ots::Address(addresses[i]);
                handles[i] = create_handle(OTS_HANDLE_ADDRESS, address);
            }
            set_array(
                result,
                handles,
                addresses.size(),
                OTS_DATA_HANDLE,
                true
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_has_address(
            const ots_handle_t* wallet,
            const ots_handle_t* address,
            uint32_t max_account_depth,
            uint32_t max_index_depth
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET || address->type != OTS_HANDLE_ADDRESS)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_boolean(
                result,
                static_cast<ots::Wallet*>(wallet->ptr)->hasAddress(
                    *static_cast<ots::Address*>(address->ptr),
                    max_account_depth,
                    max_index_depth
                )
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_has_address_string(
            const ots_handle_t* wallet,
            const char* address,
            uint32_t max_account_depth,
            uint32_t max_index_depth
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_boolean(
                result,
                static_cast<ots::Wallet*>(wallet->ptr)->hasAddress(
                    address,
                    max_account_depth,
                    max_index_depth
                )
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_address_index(
            const ots_handle_t* wallet,
            const ots_handle_t* address,
            uint32_t max_account_depth,
            uint32_t max_index_depth
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET || address->type != OTS_HANDLE_ADDRESS)
                throw ots::exception::InvalidArgument("Invalid handle type");
            std::pair<uint32_t, uint32_t> idx = static_cast<ots::Wallet*>(wallet->ptr)->addressIndex(
                        *static_cast<ots::Address*>(address->ptr),
                        max_account_depth,
                        max_index_depth
                    );
            
            uint32_t* arr = new uint32_t[2];
            arr[0] = idx.first;
            arr[1] = idx.second;
            set_address_index(result, arr);
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_address_string_index(
            const ots_handle_t* wallet,
            const char* address,
            uint32_t max_account_depth,
            uint32_t max_index_depth
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            std::pair<uint32_t, uint32_t> idx = static_cast<ots::Wallet*>(wallet->ptr)->addressIndex(
                        address,
                        max_account_depth,
                        max_index_depth
                    );
            
            uint32_t* arr = new uint32_t[2];
            arr[0] = idx.first;
            arr[1] = idx.second;
            set_address_index(result, arr);
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_secret_view_key(const ots_handle_t* wallet) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_string(
                result,
                static_cast<ots::Wallet*>(wallet->ptr)->secretViewKey()
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_public_view_key(const ots_handle_t* wallet) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_string(
                result,
                static_cast<ots::Wallet*>(wallet->ptr)->publicViewKey()
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_secret_spend_key(const ots_handle_t* wallet) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_string(
                result,
                static_cast<ots::Wallet*>(wallet->ptr)->secretSpendKey()
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_public_spend_key(const ots_handle_t* wallet) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_string(
                result,
                static_cast<ots::Wallet*>(wallet->ptr)->publicSpendKey()
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_import_outputs(
            const ots_handle_t* wallet,
            const char* outputs
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            uint64_t imported = static_cast<ots::Wallet*>(wallet->ptr)->importOutputs(outputs);
            if(imported & 0x8000000000000000) // int64_t max would be 9,223,372,036,854,775,807 (should never happen IMO)
                throw ots::exception::RangeError("Imported outputs count is too large to convert to int64_t: " + std::to_string(imported));
            set_number(result, static_cast<int64_t>(imported));
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_export_key_images(const ots_handle_t* wallet) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_string(
                result,
                static_cast<ots::Wallet*>(wallet->ptr)->exportKeyImages()
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_describe_tx(const ots_handle_t* wallet, const char* tx) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_handle(
                result,
                OTS_HANDLE_TX_DESCRIPTION,
                new ots::TxDescription(static_cast<ots::Wallet*>(wallet->ptr)->describeTransaction(tx))
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_check_tx(const ots_handle_t* wallet, const ots_handle_t* tx) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET || tx->type != OTS_HANDLE_TX)
                throw ots::exception::InvalidArgument("Invalid handle type");
            std::vector<ots::TxWarning> warnings = static_cast<ots::Wallet*>(wallet->ptr)->checkTransaction(*static_cast<ots::TxDescription*>(tx->ptr));
            ots_handle_t* handles = new ots_handle_t[warnings.size()];
            for(size_t i = 0; i < warnings.size(); i++)
                handles[i] = create_handle(OTS_HANDLE_TX_WARNING, new ots::TxWarning(warnings[i]));
            set_array(result, handles, warnings.size(), OTS_DATA_HANDLE, false);
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_check_tx_string(const ots_handle_t* wallet, const char* tx) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            std::vector<ots::TxWarning> warnings = static_cast<ots::Wallet*>(wallet->ptr)->checkTransaction(tx);
            ots_handle_t* handles = new ots_handle_t[warnings.size()];
            for(size_t i = 0; i < warnings.size(); i++)
                handles[i] = create_handle(OTS_HANDLE_TX_WARNING, new ots::TxWarning(warnings[i]));
            set_array(result, handles, warnings.size(), OTS_DATA_HANDLE, false);
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_sign_transaction(
            const ots_handle_t* wallet,
            const char* tx
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_string(
                result,
                static_cast<ots::Wallet*>(wallet->ptr)->signTransaction(tx)
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_sign_data(
            const ots_handle_t* wallet,
            const char* data
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_string(
                result,
                static_cast<ots::Wallet*>(wallet->ptr)->signData(data)
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_sign_data_with_index(
            const ots_handle_t* wallet,
            const char* data,
            uint32_t account,
            uint32_t index
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_string(
                result,
                static_cast<ots::Wallet*>(wallet->ptr)->signData(data, std::make_pair(account, index))
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_sign_data_with_address(
            const ots_handle_t* wallet,
            const char* data,
            const ots_handle_t* address
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET || address->type != OTS_HANDLE_ADDRESS)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_string(
                result,
                static_cast<ots::Wallet*>(wallet->ptr)->signData(data, *static_cast<ots::Address*>(address->ptr))
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_sign_data_with_address_string(
            const ots_handle_t* wallet,
            const char* data,
            const char* address
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_string(
                result,
                static_cast<ots::Wallet*>(wallet->ptr)->signData(data, address)
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_verify_data(
            const ots_handle_t* wallet,
            const char* data,
            const char* signature,
            bool legacy_fallback
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_boolean(
                result,
                static_cast<ots::Wallet*>(wallet->ptr)->verifyData(data, signature, legacy_fallback)
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_verify_data_with_index(
            const ots_handle_t* wallet,
            const char* data,
            uint32_t account,
            uint32_t index,
            const char* signature,
            bool legacy_fallback
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_boolean(
                result,
                static_cast<ots::Wallet*>(wallet->ptr)->verifyData(data, std::make_pair(account, index), signature, legacy_fallback)
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_verify_data_with_address(
            const ots_handle_t* wallet,
            const char* data,
            const ots_handle_t* address,
            const char* signature,
            bool legacy_fallback
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET || address->type != OTS_HANDLE_ADDRESS)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_boolean(
                result,
                ots::Wallet::verifyData(data, signature, *static_cast<ots::Address*>(address->ptr), legacy_fallback)
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_verify_data_with_address_string(
            const ots_handle_t* wallet,
            const char* data,
            const char* address,
            const char* signature,
            bool legacy_fallback
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_boolean(
                result,
                ots::Wallet::verifyData(data, signature, address, legacy_fallback)
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }


    ots_result_t* ots_version(void) {
        ots_result_t* result = new ots_result_t();
        try {
            set_string(result, ots::OTS::version());
        } catch (const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_version_components(void) {
        ots_result_t* result = new ots_result_t();
        try {
            auto components = ots::OTS::versionComponents();
            size_t size = components.size();
            int* arr = new int[size];
            memcpy(arr, components.data(), size * sizeof(int));
            set_array(result, arr, size, OTS_DATA_INT, false);
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_height_from_timestamp(
            uint64_t timestamp,
            OTS_NETWORK network
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            set_number(
                result,
                ots::OTS::heightFromTimestamp(timestamp, to_cpp_network(network))
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_timestamp_from_height(
            uint64_t height,
            OTS_NETWORK network
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            set_number(
                result,
                ots::OTS::timestampFromHeight(height, to_cpp_network(network))
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_random_bytes(size_t size) {
        ots_result_t* result = new ots_result_t();
        try {
            uint8_t* arr = new uint8_t[size];
            ots::OTS::random(size, arr);
            set_array(result, arr, size, OTS_DATA_UINT8, false);
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_random_32(void) {
        return ots_random_bytes(32);
    }

    ots_result_t* ots_check_low_entropy(
            const uint8_t* data,
            size_t size,
            double min_entropy
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            set_boolean(
                result,
                ots::OTS::lowEntropy(size, data, min_entropy)
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    void ots_set_enforce_checksums(bool enforce) {
        ots::OTS::enforceEntropy(enforce);
    }

    void ots_set_max_account_depth(uint32_t depth) {
        ots::OTS::setMaxAccountDepth(depth);
    }

    void ots_set_max_index_depth(uint32_t depth) {
        ots::OTS::setMaxIndexDepth(depth);
    }

    void ots_set_max_depth(uint32_t account_depth, uint32_t index_depth) {
        ots::OTS::setMaxDepth(account_depth, index_depth);
    }

    void ots_reset_max_depth(void) {
        ots::OTS::resetMaxDepth();
    }

    uint32_t ots_get_max_account_depth(uint32_t default_max_account_depth) {
        return ots::OTS::maxAccountDepth(default_max_account_depth);
    }

    uint32_t ots_get_max_index_depth(uint32_t default_max_index_depth) {
        return ots::OTS::maxIndexDepth(default_max_index_depth);
    }

    ots_result_t* ots_verify_data(
            const char* data,
            const char* address,
            const char* signature
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            set_boolean(
                result,
                // TODO: should probably be a static method of ots::OTS, but not sure yet, what to do.
                ots::Wallet::verifyData(data, address, signature, false)
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }
}
