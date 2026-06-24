#include "ots-internal.h"

using namespace ots::internal;

extern "C" {
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
}
