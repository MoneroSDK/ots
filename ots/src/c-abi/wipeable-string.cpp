#include "ots-internal.h"

using namespace ots::internal;

extern "C" {
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
}
