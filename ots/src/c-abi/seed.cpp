#include "ots-internal.h"

using namespace ots::internal;

extern "C" {
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

    ots_result_t* ots_seed_is_legacy(const ots_handle_t* handle) {
        ots_result_t* result = new ots_result_t();
        try {
            if(handle->type != OTS_HANDLE_SEED)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_boolean(result, static_cast<ots::Seed*>(handle->ptr)->isLegacy());
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_type(const ots_handle_t* handle) {
        ots_result_t* result = new ots_result_t();
        try {
            if(handle->type != OTS_HANDLE_SEED)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_seed_type(result, static_cast<ots::Seed*>(handle->ptr)->type());
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
            set_network(result, static_cast<ots::Seed*>(handle->ptr)->network());
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
}
