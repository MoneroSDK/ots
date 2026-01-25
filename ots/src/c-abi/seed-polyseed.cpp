#include "ots-internal.h"
#include <cstring>

using namespace ots::internal;

extern "C" {
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

    ots_result_t* ots_polyseed_convert_to_monero_seed(
            const ots_handle_t* polyseed
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(polyseed->type != OTS_HANDLE_SEED)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_handle(
                result,
                OTS_HANDLE_SEED,
                new ots::MoneroSeed(
                    static_cast<const ots::Polyseed*>(polyseed->ptr)->moneroSeed()
                )
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }
}
