#include "ots-internal.h"
#include <cstring>

using namespace ots::internal;

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
        if(!str || !*str)
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
}
