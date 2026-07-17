#include "ots-internal.h"
#include <cstring>
#include <cstdint>

namespace ots::internal {
    void translate_exception(ots_error_t* error, const ots::exception::Exception& e) {
        if(!error)
            return;
        error->code = e.code();
        strncpy(error->message, e.what(), OTS_MAX_ERROR_MESSAGE - 1);
        error->message[OTS_MAX_ERROR_MESSAGE - 1] = '\0';
        memset(error->cls, 0, OTS_MAX_ERROR_CLASS);
        strncpy(error->cls, e.cls(), OTS_MAX_ERROR_CLASS - 1);
        error->cls[OTS_MAX_ERROR_CLASS - 1] = '\0';
    }

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

    void set_result_type(ots_result_t* result, ots_result_type type) {
        if(result == nullptr)
            return;
        set_success(result);
        result->type = type;
    }

    ots_handle_t create_handle(ots_handle_type type, void* ptr) {
        return ots_handle_t {
            .type = type,
            .ptr = ptr,
            .reference = false
        };
    }

    ots_handle_t create_handle_reference(ots_handle_type type, void* ptr) {
        return ots_handle_t {
            .type = type,
            .ptr = ptr,
            .reference = true
        };
    }

    char* create_string_copy(const std::string& str, size_t size) {
        char* copy = new char[size + 1];
        std::strncpy(copy, str.c_str(), size);
        copy[size] = '\0';
        return copy;
    }

    char* create_string_binary_copy(const std::string& str, size_t size) {
        char* copy = new char[size + 1];
        std::memcpy(copy, str.data(), size);
        copy[size] = '\0';
        return copy;
    }

    void set_handle(ots_result_t* result, ots_handle_type handle_type, void* handle) {
        if(!result)
            return;
        set_result_type(result, OTS_RESULT_HANDLE);
        result->result.handle = create_handle(handle_type, handle);
    }

    void set_handle_reference(ots_result_t* result, ots_handle_type handle_type, void* handle) {
        if(!result)
            return;
        set_result_type(result, OTS_RESULT_HANDLE);
        result->result.handle = create_handle_reference(handle_type, handle);
    }

    void set_string(ots_result_t* result, const std::string& str) {
        if(result == nullptr)
            return;
        set_result_type(result, OTS_RESULT_STRING);
        result->result.data.ptr = create_string_copy(str, str.length());
        result->result.data.size = str.length();
        result->result.data.type = OTS_DATA_CHAR;
        result->result.data.reference = false;
    }

    void set_binary_string(
        ots_result_t* result,
        const std::string& data,
        size_t size
    ) {
        if(result == nullptr)
            return;
        set_result_type(result, OTS_RESULT_STRING);
        result->result.data.ptr = create_string_binary_copy(data, size);
        result->result.data.size = size;
        result->result.data.type = OTS_DATA_CHAR;
        result->result.data.reference = false;
    }

    void set_boolean(ots_result_t* result, bool value) {
        if(!result)
            return;
        set_result_type(result, OTS_RESULT_BOOLEAN);
        result->result.boolean = value;
    }

    void set_number(ots_result_t* result, int64_t value) {
        if(!result)
            return;
        set_result_type(result, OTS_RESULT_NUMBER);
        result->result.number = value;
    }

    void set_comparison(ots_result_t* result, int64_t value) {
        if(!result)
            return;
        set_result_type(result, OTS_RESULT_COMPARISON);
        result->result.number = value;
    }

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

    void set_address_type(ots_result_t* result, ots::AddressType type) {
        if(!result)
            return;
        set_result_type(result, OTS_RESULT_ADDRESS_TYPE);
        result->result.number = static_cast<int64_t>(type);
    }

    void set_network(ots_result_t* result, ots::Network network) {
        if(!result)
            return;
        set_result_type(result, OTS_RESULT_NETWORK);
        result->result.number = static_cast<int64_t>(network);
    }

    void set_seed_type(ots_result_t* result, ots::SeedType type) {
        if(!result)
            return;
        set_result_type(result, OTS_RESULT_SEED_TYPE);
        result->result.number = static_cast<int64_t>(type);
    }

    void set_error(ots_result_t* result, const ots::exception::Exception& e) {
        if(!result)
            return;
        translate_exception(&result->error, e);
        result->result.data.ptr = nullptr;
    }

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
