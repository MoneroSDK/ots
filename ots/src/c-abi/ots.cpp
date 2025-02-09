#include "ots.h"
#include "ots.hpp"
#include "ots-internal.h"
#include <cstring>

using namespace ots::internal;

extern "C" {
    ots_result_t* ots_version(void) {
        ots_result_t* result = new ots_result_t();
        try {
            set_string(result, ots::OTS::version());
        } catch (const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_version_components(void) {
        ots_result_t* result = new ots_result_t();
        try {
            auto components = ots::OTS::versionComponents();
            size_t size = components.size();
            int* arr = new int[size];
            memcpy(arr, components.data(), size * sizeof(int));
            set_array(result, arr, size, OTS_DATA_INT, false);
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_height_from_timestamp(
            uint64_t timestamp,
            OTS_NETWORK network
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            set_number(
                result,
                ots::OTS::heightFromTimestamp(timestamp, to_cpp_network(network))
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_timestamp_from_height(
            uint64_t height,
            OTS_NETWORK network
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            set_number(
                result,
                ots::OTS::timestampFromHeight(height, to_cpp_network(network))
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_random_bytes(size_t size) {
        ots_result_t* result = new ots_result_t();
        try {
            uint8_t* arr = new uint8_t[size];
            ots::OTS::random(size, arr);
            set_array(result, arr, size, OTS_DATA_UINT8, false);
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_random_32(void) {
        return ots_random_bytes(32);
    }

    ots_result_t* ots_check_low_entropy(
            const uint8_t* data,
            size_t size,
            double min_entropy
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            set_boolean(
                result,
                ots::OTS::lowEntropy(size, data, min_entropy)
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    void ots_set_enforce_checksums(bool enforce) {
        ots::OTS::enforceEntropy(enforce);
    }

    void ots_set_max_account_depth(uint32_t depth) {
        ots::OTS::setMaxAccountDepth(depth);
    }

    void ots_set_max_index_depth(uint32_t depth) {
        ots::OTS::setMaxIndexDepth(depth);
    }

    void ots_set_max_depth(uint32_t account_depth, uint32_t index_depth) {
        ots::OTS::setMaxDepth(account_depth, index_depth);
    }

    void ots_reset_max_depth(void) {
        ots::OTS::resetMaxDepth();
    }

    uint32_t ots_get_max_account_depth(uint32_t default_max_account_depth) {
        return ots::OTS::maxAccountDepth(default_max_account_depth);
    }

    uint32_t ots_get_max_index_depth(uint32_t default_max_index_depth) {
        return ots::OTS::maxIndexDepth(default_max_index_depth);
    }

    ots_result_t* ots_verify_data(
            const char* data,
            const char* address,
            const char* signature
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            set_boolean(
                result,
                // TODO: should probably be a static method of ots::OTS, but not sure yet, what to do.
                ots::Wallet::verifyData(data, address, signature, false)
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }
}
