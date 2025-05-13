#include "ots-internal.h"
#include <cstring>

using namespace ots::internal;

extern "C" {
    bool ots_is_error(const ots_result_t* result) {
        return result && result->error.code != 0;
    }

    char* ots_error_message(const ots_result_t* result) {
        if(!ots_is_error(result))
            return nullptr;
        return create_string_copy(result->error.message);
    }

    char* ots_error_class(const ots_result_t* result) {
        if(!ots_is_error(result))
            return nullptr;
        return create_string_copy(result->error.cls);
    }

    int32_t ots_error_code(const ots_result_t* result) {
        if(!ots_is_error(result))
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
            case OTS_RESULT_ARRAY:
            case OTS_RESULT_STRING:
                return result->result.data.reference;
            default:
                return false;
        }
    }

    bool ots_result_data_is_int(const ots_result_t* result) {
        return ots_result_data_is_type(result, OTS_DATA_INT);
    }

    bool ots_result_data_is_char(const ots_result_t* result) {
        return ots_result_data_is_type(result, OTS_DATA_CHAR);
    }

    bool ots_result_data_is_uint8(const ots_result_t* result) {
        return ots_result_data_is_type(result, OTS_DATA_UINT8);
    }

    bool ots_result_data_is_uint16(const ots_result_t* result) {
        return ots_result_data_is_type(result, OTS_DATA_UINT16);
    }

    bool ots_result_data_is_uint32(const ots_result_t* result) {
        return ots_result_data_is_type(result, OTS_DATA_UINT32);
    }

    bool ots_result_data_is_uint64(const ots_result_t* result) {
        return ots_result_data_is_type(result, OTS_DATA_UINT64);
    }

    bool ots_result_data_is_handle(const ots_result_t* result) {
        return ots_result_data_is_type(result, OTS_DATA_HANDLE);
    }

    bool ots_result_data_handle_is_type(const ots_result_t* result, ots_handle_type type) {
        if(!ots_result_data_is_handle(result))
            return false;
        return ((ots_handle_t*)result->result.data.ptr)->type == type;
    }

    bool ots_result_data_handle_is_reference(const ots_result_t* result) {
        if(!ots_result_data_is_handle(result))
            return false;
        return ((ots_handle_t*)result->result.data.ptr)->reference;
    }

    bool ots_result_data_handle_is_wipeable_string(const ots_result_t* result) {
        if(!ots_result_data_is_handle(result))
            return false;
        return ots_result_data_handle_is_type(result, OTS_HANDLE_WIPEABLE_STRING);
    }

    bool ots_result_data_handle_is_seed_indices(const ots_result_t* result) {
        if(!ots_result_data_is_handle(result))
            return false;
        return ots_result_data_handle_is_type(result, OTS_HANDLE_SEED_INDICES);
    }

    bool ots_result_data_handle_is_seed_language(const ots_result_t* result) {
        if(!ots_result_data_is_handle(result))
            return false;
        return ots_result_data_handle_is_type(result, OTS_HANDLE_SEED_LANGUAGE);
    }

    bool ots_result_data_handle_is_address(const ots_result_t* result) {
        if(!ots_result_data_is_handle(result))
            return false;
        return ots_result_data_handle_is_type(result, OTS_HANDLE_ADDRESS);
    }

    bool ots_result_data_handle_is_seed(const ots_result_t* result) {
        if(!ots_result_data_is_handle(result))
            return false;
        return ots_result_data_handle_is_type(result, OTS_HANDLE_SEED);
    }

    bool ots_result_data_handle_is_wallet(const ots_result_t* result) {
        if(!ots_result_data_is_handle(result))
            return false;
        return ots_result_data_handle_is_type(result, OTS_HANDLE_WALLET);
    }

    bool ots_result_data_handle_is_transaction(const ots_result_t* result) {
        if(!ots_result_data_is_handle(result))
            return false;
        return ots_result_data_handle_is_type(result, OTS_HANDLE_TX);
    }

    bool ots_result_data_handle_is_transaction_description(const ots_result_t* result) {
        if(!ots_result_data_is_handle(result))
            return false;
        return ots_result_data_handle_is_type(result, OTS_HANDLE_TX_DESCRIPTION);
    }

    bool ots_result_data_handle_is_transaction_warning(const ots_result_t* result) {
        if(!ots_result_data_is_handle(result))
            return false;
        return ots_result_data_handle_is_type(result, OTS_HANDLE_TX_WARNING);
    }

    const char* ots_result_string(const ots_result_t* result) {
        if(!result)
            return nullptr;
        if(ots_result_is_string(result))
            return static_cast<char*>(result->result.data.ptr);
        if(ots_result_is_wipeable_string(result))
            try {
                return static_cast<ots::WipeableString*>(result->result.handle.ptr)->c_str();
            } catch(const ots::exception::Exception& e) {}
        return nullptr;
    }

    char* ots_result_string_copy(const ots_result_t* result) {
        const char* out = ots_result_string(result);
        if(!out)
            return nullptr;
        return create_string_copy(out);
    }

    bool ots_result_boolean(const ots_result_t* result, bool default_value) {
        if(!ots_result_is_boolean(result))
            return default_value;
        return result->result.boolean;
    }

    int64_t ots_result_number(const ots_result_t* result, int64_t default_value) {
        if(!ots_result_is_number(result))
            return default_value;
        return result->result.number;
    }

    void* ots_result_array(const ots_result_t* result) {
        if(!ots_result_is_array(result))
            return nullptr;
        switch(result->result.data.type) {
            case OTS_DATA_INT:
                return ots_result_int_array(result);
            case OTS_DATA_CHAR:
                return ots_result_char_array(result);
            case OTS_DATA_UINT8:
                return ots_result_uint8_array(result);
            case OTS_DATA_UINT16:
                return ots_result_uint16_array(result);
            case OTS_DATA_UINT32:
                return ots_result_uint32_array(result);
            case OTS_DATA_UINT64:
                return ots_result_uint64_array(result);
            case OTS_DATA_HANDLE:
                return ots_result_handle_array(result);
            default:
                return nullptr;
        }
    }

    void* ots_result_array_reference(const ots_result_t* result) {
        if(!ots_result_is_array(result))
            return nullptr;
        return result->result.data.ptr;
    }

    void* ots_result_array_get(const ots_result_t* result, size_t index) {
        if(!ots_result_is_array(result) || index >= ots_result_size(result))
            return nullptr;
        return static_cast<uint8_t*>(result->result.data.ptr) + index * result->result.data.type;
    }

    ots_handle_t* ots_result_array_get_handle(const ots_result_t* result, size_t index) {
        if(
            !ots_result_data_is_handle(result)
            || index >= ots_result_size(result)
            || result->result.data.ptr == nullptr
        )
            return nullptr;
        return new ots_handle_t(reinterpret_cast<ots_handle_t*>(result->result.data.ptr)[index]);
    }

    int ots_result_array_get_int(const ots_result_t* result, size_t index) {
        if(!ots_result_data_is_int(result) || index >= ots_result_size(result))
            return 0;
        return static_cast<int*>(result->result.data.ptr)[index];
    }

    char ots_result_array_get_char(const ots_result_t* result, size_t index) {
        if(!ots_result_data_is_char(result) || index >= ots_result_size(result))
            return 0;
        return static_cast<char*>(result->result.data.ptr)[index];
    }

    uint8_t ots_result_array_get_uint8(const ots_result_t* result, size_t index) {
        if(!ots_result_data_is_uint8(result) || index >= ots_result_size(result))
            return 0;
        return static_cast<uint8_t*>(result->result.data.ptr)[index];
    }

    uint16_t ots_result_array_get_uint16(const ots_result_t* result, size_t index) {
        if(!ots_result_data_is_uint16(result) || index >= ots_result_size(result))
            return 0;
        return static_cast<uint16_t*>(result->result.data.ptr)[index];
    }

    uint32_t ots_result_array_get_uint32(const ots_result_t* result, size_t index) {
        if(!ots_result_data_is_uint32(result) || index >= ots_result_size(result))
            return 0;
        return static_cast<uint32_t*>(result->result.data.ptr)[index];
    }

    uint64_t ots_result_array_get_uint64(const ots_result_t* result, size_t index) {
        if(!ots_result_data_is_uint64(result) || index >= ots_result_size(result))
            return 0;
        return static_cast<uint64_t*>(result->result.data.ptr)[index];
    }

    ots_handle_t* ots_result_handle_array(const ots_result_t* result) {
        if(!ots_result_data_is_handle(result))
            return nullptr;
        ots_handle_t* arr = new ots_handle_t[result->result.data.size];
        memcpy(arr, result->result.data.ptr, result->result.data.size * sizeof(ots_handle_t));
        return arr;
    }

    int* ots_result_int_array(const ots_result_t* result) {
        if(!ots_result_data_is_int(result))
            return nullptr;
        int* arr = new int[result->result.data.size];
        memcpy(arr, result->result.data.ptr, result->result.data.size * sizeof(int));
        return arr;
    }

    char* ots_result_char_array(const ots_result_t* result) {
        if(!ots_result_data_is_char(result))
            return nullptr;
        char* arr = new char[result->result.data.size];
        memcpy(arr, result->result.data.ptr, result->result.data.size * sizeof(char));
        return arr;
    }

    uint8_t* ots_result_uint8_array(const ots_result_t* result) {
        if(!ots_result_data_is_uint8(result))
            return nullptr;
        uint8_t* arr = new uint8_t[result->result.data.size];
        memcpy(arr, result->result.data.ptr, result->result.data.size * sizeof(uint8_t));
        return arr;
    }

    uint16_t* ots_result_uint16_array(const ots_result_t* result) {
        if(!ots_result_data_is_uint16(result))
            return nullptr;
        uint16_t* arr = new uint16_t[result->result.data.size];
        memcpy(arr, result->result.data.ptr, result->result.data.size * sizeof(uint16_t));
        return arr;
    }

    uint32_t* ots_result_uint32_array(const ots_result_t* result) {
        if(!ots_result_data_is_uint32(result))
            return nullptr;
        uint32_t* arr = new uint32_t[result->result.data.size];
        memcpy(arr, result->result.data.ptr, result->result.data.size * sizeof(uint32_t));
        return arr;
    }

    uint64_t* ots_result_uint64_array(const ots_result_t* result) {
        if(!ots_result_data_is_uint64(result))
            return nullptr;
        uint64_t* arr = new uint64_t[result->result.data.size];
        memcpy(arr, result->result.data.ptr, result->result.data.size * sizeof(uint64_t));
        return arr;
    }

    ots_handle_t* ots_result_handle_array_reference(const ots_result_t* result) {
        if(!ots_result_data_is_handle(result))
            return nullptr;
        return static_cast<ots_handle_t*>(result->result.data.ptr);
    }

    int* ots_result_int_array_reference(const ots_result_t* result) {
        if(!ots_result_data_is_int(result))
            return nullptr;
        return static_cast<int*>(result->result.data.ptr);
    }

    char* ots_result_char_array_reference(const ots_result_t* result) {
        if(!ots_result_data_is_char(result))
            return nullptr;
        return static_cast<char*>(result->result.data.ptr);
    }

    uint8_t* ots_result_uint8_array_reference(const ots_result_t* result) {
        if(!ots_result_data_is_uint8(result))
            return nullptr;
        return static_cast<uint8_t*>(result->result.data.ptr);
    }

    uint16_t* ots_result_uint16_array_reference(const ots_result_t* result) {
        if(!ots_result_data_is_uint16(result))
            return nullptr;
        return static_cast<uint16_t*>(result->result.data.ptr);
    }

    uint32_t* ots_result_uint32_array_reference(const ots_result_t* result) {
        if(!ots_result_data_is_uint32(result))
            return nullptr;
        return static_cast<uint32_t*>(result->result.data.ptr);
    }

    uint64_t* ots_result_uint64_array_reference(const ots_result_t* result) {
        if(!ots_result_data_is_uint64(result))
            return nullptr;
        return static_cast<uint64_t*>(result->result.data.ptr);
    }

    bool ots_result_is_array(const ots_result_t* result) {
        return ots_result_is_type(result, OTS_RESULT_ARRAY);
    }

    bool ots_result_is_comparison(const ots_result_t* result) {
        return ots_result_is_type(result, OTS_RESULT_COMPARISON);
    }

    int64_t ots_result_comparison(const ots_result_t* result) {
        if(!ots_result_is_comparison(result))
            return 0;
        return result->result.number;
    }

    bool ots_result_is_equal(const ots_result_t* result) {
        return ots_result_comparison(result) == 0;
    }

    size_t ots_result_size(const ots_result_t* result) {
        if(ots_result_is_array(result))
            return result->result.data.size;
        if(ots_result_is_string(result))
            return result->result.data.size;
        if(ots_result_is_wipeable_string(result))
            try {
                return static_cast<ots::WipeableString*>(result->result.handle.ptr)->size();
            } catch(const ots::exception::Exception& e) {}
        return 0;
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
        if(!ots_result_is_address_type(result))
            return false;
        return result->result.number == static_cast<int64_t>(type);
    }

    bool ots_result_is_handle(const ots_result_t* result) {
        return result && ots_result_is_type(result, OTS_RESULT_HANDLE);
    }

    bool ots_result_is_wipeable_string(const ots_result_t* result) {
        return ots_result_handle_is_type(result, OTS_HANDLE_WIPEABLE_STRING);
    }

    bool ots_result_is_seed_indices(const ots_result_t* result) {
        return ots_result_handle_is_type(result, OTS_HANDLE_SEED_INDICES);
    }

    bool ots_result_is_seed_language(const ots_result_t* result) {
        return ots_result_handle_is_type(result, OTS_HANDLE_SEED_LANGUAGE);
    }

    bool ots_result_is_address(const ots_result_t* result) {
        return ots_result_handle_is_type(result, OTS_HANDLE_ADDRESS);
    }

    bool ots_result_is_seed(const ots_result_t* result) {
        return ots_result_handle_is_type(result, OTS_HANDLE_SEED);
    }

    bool ots_result_is_wallet(const ots_result_t* result) {
        return ots_result_handle_is_type(result, OTS_HANDLE_WALLET);
    }

    bool ots_result_is_transaction(const ots_result_t* result) {
        return ots_result_handle_is_type(result, OTS_HANDLE_TX);
    }

    bool ots_result_is_transaction_description(const ots_result_t* result) {
        return ots_result_handle_is_type(result, OTS_HANDLE_TX_DESCRIPTION);
    }

    bool ots_result_is_transaction_warning(const ots_result_t* result) {
        return ots_result_handle_is_type(result, OTS_HANDLE_TX_WARNING);
    }

    bool ots_result_is_string(const ots_result_t* result) {
        return result && ots_result_is_type(result, OTS_RESULT_STRING);
    }

    bool ots_result_is_boolean(const ots_result_t* result) {
        return result && ots_result_is_type(result, OTS_RESULT_BOOLEAN);
    }

    bool ots_result_is_number(const ots_result_t* result) {
        return result && ots_result_is_type(result, OTS_RESULT_NUMBER);
    }

    bool ots_result_is_network(const ots_result_t* result) {
        return result && ots_result_is_type(result, OTS_RESULT_NETWORK);
    }

    bool ots_result_network_is_type(const ots_result_t* result, OTS_NETWORK network) {
        if(!result || !ots_result_is_type(result, OTS_RESULT_NETWORK))
            return false;
        return result->result.number == static_cast<int64_t>(network);
    }

    bool ots_result_is_seed_type(const ots_result_t* result) {
        return result && ots_result_is_type(result, OTS_RESULT_SEED_TYPE);
    }

    OTS_SEED_TYPE ots_result_seed_type(const ots_result_t* result) {
        if(!result || !ots_result_is_seed_type(result))
            return OTS_SEED_TYPE_MONERO;
        return static_cast<OTS_SEED_TYPE>(result->result.number);
    }

    bool ots_result_seed_type_is_type(const ots_result_t* result, OTS_SEED_TYPE type) {
        if(!ots_result_is_seed_type(result))
            return false;
        return result->result.number == static_cast<int64_t>(type);
    }

    ots_handle_t* ots_result_handle(ots_result_t* result) {
        if(!ots_result_is_handle(result))
            return nullptr;
        result->result.handle.reference = true; // Don't free the handle anymore
        return new ots_handle_t{
            result->result.handle.type,
                result->result.handle.ptr,
                false
        };
    }

    bool ots_result_handle_is_type(const ots_result_t* result, ots_handle_type type) {
        if(!result || !ots_result_is_handle(result))
            return false;
        return result->result.handle.type == type;
    }

    bool ots_result_handle_is_reference(const ots_result_t* result) {
        if(!result || !ots_result_is_handle(result))
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
                    case OTS_DATA_INT:
                        size = sizeof(int);
                        break;
                    case OTS_DATA_CHAR:
                        size = sizeof(char);
                        break;
                    case OTS_DATA_UINT8:
                        size = sizeof(uint8_t);
                        break;
                    case OTS_DATA_UINT16:
                        size = sizeof(uint16_t);
                        break;
                    case OTS_DATA_UINT32:
                        size = sizeof(uint32_t);
                        break;
                    case OTS_DATA_HANDLE:
                        // Free the objects of the handles in the array first
                        for(size_t i = 0; i < (*result)->result.data.size; ++i) {
                            ots_handle_t* handle = &static_cast<ots_handle_t*>((*result)->result.data.ptr)[i];
                            ots_free_handle_object(handle);
                        }
                        size = sizeof(ots_handle_t);
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
                ots_free_handle_object(&(*result)->result.handle);
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
            ots_free_handle_object(*handle);
        delete *handle;
        *handle = nullptr;
    }

    void ots_free_handle_object(ots_handle_t* handle) {
        if(!handle)
            return;
        switch(handle->type) {
            case OTS_HANDLE_WIPEABLE_STRING:
                delete static_cast<ots::WipeableString*>(handle->ptr);
                break;
            case OTS_HANDLE_SEED_INDICES:
                delete static_cast<ots::SeedIndices*>(handle->ptr);
                break;
            case OTS_HANDLE_SEED:
                delete static_cast<ots::Seed*>(handle->ptr);
                break;
            case OTS_HANDLE_WALLET:
                delete static_cast<ots::Wallet*>(handle->ptr);
                break;
            case OTS_HANDLE_TX:
                delete static_cast<ots::TxDescription*>(handle->ptr);
                break;
            case OTS_HANDLE_SEED_LANGUAGE: // is always a reference
            default:
                break;
        }
    }

    void ots_secure_free(void** buffer, size_t size) {
        if(!buffer || !*buffer || size == 0)
            return;
        memset(*buffer, 0, size);
        delete[] static_cast<unsigned char*>(*buffer);
        *buffer = nullptr;
    }
}
