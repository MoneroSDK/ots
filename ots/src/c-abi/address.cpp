#include "ots-internal.h"

using namespace ots::internal;

extern "C" {
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
}
