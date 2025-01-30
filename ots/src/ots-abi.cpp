#include "ots.h"
#include "ots.hpp"
#include "ots-exceptions.hpp"
#include <cstring>
#include <memory>

// Minimal implementation of the C API LLM generated so it compiles and we can run `make test` without any explotions.
// TODO: throw away and implement right
namespace {
    void translate_exception(ots_error_t* error, const std::exception& e) {
        if (!error) return;
        error->code = -1;
        strncpy(error->message, e.what(), OTS_MAX_ERROR_MESSAGE - 1);
        error->message[OTS_MAX_ERROR_MESSAGE - 1] = '\0';
        error->location[0] = '\0';
    }

    void set_success(ots_result_t* result) {
        if (!result) return;
        result->error.code = 0;
        result->error.message[0] = '\0';
        result->error.location[0] = '\0';
    }

    void set_error(ots_result_t* result, const std::exception& e) {
        if (!result) return;
        translate_exception(&result->error, e);
        result->result.ptr = nullptr;
    }

    char* create_string_copy(const std::string& str) {
        char* copy = static_cast<char*>(malloc(str.length() + 1));
        if (copy) {
            strcpy(copy, str.c_str());
        }
        return copy;
    }
}

extern "C" {
    void ots_error_init(ots_error_t* error) {
        if (!error) return;
        error->code = 0;
        error->message[0] = '\0';
        error->location[0] = '\0';
    }

    bool ots_is_error(const ots_result_t* result) {
        return result && result->error.code != 0;
    }

    bool ots_has_error(const ots_result_t* result) {
        return ots_is_error(result);
    }

    ots_result_t ots_get_last_error(void) {
        ots_result_t result = {};
        result.result.ptr = create_string_copy("Not implemented");
        return result;
    }

    ots_result_t ots_get_error_message(int32_t error_code) {
        ots_result_t result = {};
        result.result.ptr = create_string_copy("Not implemented");
        return result;
    }

    void ots_clear_error(void) {
        // Nothing to do yet
    }

    void ots_free_string(char* str) {
        free(str);
    }

    void ots_free_array(void* arr) {
        free(arr);
    }

    void ots_free_handle(ots_handle_t handle) {
        if (handle.ptr) {
            free(handle.ptr);
        }
    }

    void ots_wipeable_string_free(ots_wipeable_string_t* str) {
        if (!str) return;
        if (str->data) {
            memset(str->data, 0, str->length);
            free(str->data);
        }
    }

    void ots_seed_indices_free(ots_seed_indices_t* indices) {
        if (!indices) return;
        if (indices->indices) {
            free(indices->indices);
        }
    }

    void ots_free_tx_description(ots_tx_description_t* desc) {
        if (!desc) return;
        free(desc);
    }

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
            if (!arr) throw std::bad_alloc();
            
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
}
