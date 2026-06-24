#include "ots-internal.h"
#include <cstring>

using namespace ots::internal;

extern "C" {
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
}
