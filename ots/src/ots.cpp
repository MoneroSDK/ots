#include "ots.hpp"
#include "ots-internal.hpp"
#include "entropy.hpp"
#include "key-store.hpp"
#include "ots-version.h"
#include "crypto/crypto.h"

// needed to deactivate easy logging
#include "easylogging++.h"
INITIALIZE_EASYLOGGINGPP

namespace ots {

    OTS::OTS() {};

    const std::string OTS::version() noexcept {
        return OTS_VERSION_STRING;
    };

    std::array<int, 3> OTS::versionComponents() noexcept {
        return {OTS_VERSION_MAJOR, OTS_VERSION_MINOR, OTS_VERSION_PATCH};
    };

    /**
     * @todo TODO: unluckily, there are for 15116 blocks a hughe difference, so the user would load too many blocks, we should fix this with approximate, and if approximate fails, then estimate the block height
     */
    uint64_t OTS::heightFromTimestamp(uint64_t timestamp, Network network) {
        const ots::data::NetworkData* data = getNetworkData(network);
        if(timestamp <= data->birth) // Monero didn't exist, so return to square 0
            return 0;
        // Apply security margin (10 days in seconds, should be enough how the biggest positive difference are 8 days)
        timestamp -= 864000; // 10 * 24 * 60 * 60
        uint64_t height;
        if(timestamp <= data->birth)
            return 0;
        if(timestamp >= data->v2_time)
            height = data->v2_block + (timestamp - data->v2_time) / V2_SECONDS_PER_BLOCK;
        else
            height = (timestamp - data->birth) / V1_SECONDS_PER_BLOCK;
        if(data->rollback != 0 && height > data->rollback)
            return height - data->rollback;
        return height;
    }

    uint64_t OTS::timestampFromHeight(uint64_t height, Network network) {
        const ots::data::NetworkData* data = getNetworkData(network);
        if(data->rollback != 0 && height > data->rollback)
            height -= data->rollback;
        if(height >= data->v2_block)
            return data->v2_time + V2_SECONDS_PER_BLOCK * (height - data->v2_block);
        return data->birth + V1_SECONDS_PER_BLOCK * height;
    }

    std::array<unsigned char, 32> OTS::random() {
        std::array<unsigned char, 32> key;
        OTS::random(key.size(), key.data());
        return std::move(key);
    }

    void OTS::random(size_t size, uint8_t *bytes) {
        try {
            // if enforceEntropy is false, we don't check for low entropy
            // if enforceEntropy is true, we check for low entropy and retry 3 times
            // and after 3 tries we throw an exception
            for(size_t i = 0; i < 3; i++) {
                crypto::generate_random_bytes_thread_safe(size, bytes);
                if(!OTS::s_enforceEntropy || !OTS::lowEntropy(size, bytes))
                    return; // success
            }
            throw ots::exception::LowEntropy(); // after 3 tries still low entropy
        } catch(...) {
            throw ots::exception::SecurityError("Random generation failed!");
        }
    }

    bool OTS::lowEntropy(size_t size, const uint8_t* data, double minEntropy) noexcept {
        if(size == 0 || data == nullptr)
            return true;
        return Entropy::allBytesIdentical(data, size) ||
            Entropy::isIncrementalSequence(data, size) ||
            Entropy::hasRepeatingSubpattern(data, size, 0x00) ||
            Entropy::hasRepeatingSubpattern(data, size, 0xFF) ||
            Entropy::shannonEntropy(data, size) < minEntropy ||
            Entropy::mostlyExtremes(data, size);
    }

    void OTS::ensureEntropy(size_t size, const uint8_t* data, double minEntropy) {
        if(OTS::lowEntropy(size, data, minEntropy))
            throw ots::exception::LowEntropy();
    }

    void OTS::enforceEntropy(bool enforce) noexcept {
        OTS::s_enforceEntropy = enforce;
    }

    bool OTS::s_enforceEntropy = true;

    cryptonote::network_type cryptonoteNetwork(Network network) noexcept {
        if(static_cast<uint8_t>(network) > 2)
            return cryptonote::network_type::UNDEFINED;
        return static_cast<cryptonote::network_type>(network);
    }

    SeedIndices seedIndices(const unsigned char* bytes, size_t byte_length, size_t word_list_length, size_t bytes_per_chunk, size_t words_per_chunk) {
        if((byte_length % bytes_per_chunk) != 0)
            throw ots::exception::seed::SeedEncodingFailed("Input byte size is not a multiple of bytes_per_chunk.");
        SeedIndices indices;
        indices.reserve((size_t)(byte_length / bytes_per_chunk * words_per_chunk));
        size_t chunks = byte_length / bytes_per_chunk;
        for(size_t pos = 0; pos < chunks; ++pos) {
            uint32_t chunk = SWAP32LE(*(const uint32_t*)(bytes + (pos * bytes_per_chunk)));
            uint32_t used = 0;
            size_t divisor = 1;
            for(size_t word = 0; word < words_per_chunk; ++word) {
                uint16_t index = ((chunk / divisor) + used) % word_list_length;
                indices.emplace_back(index);
                used = index;
                divisor *= word_list_length;
            }
        }
        indices.shrink_to_fit();
        return std::move(indices);
    }

    std::vector<std::string> splitString(const std::string &input, const std::string &separator, size_t fixedWidth) {
        std::vector<std::string> result;
        result.reserve(25); // most likely we will split seed phrases or related which will be between 12 and 25 elements
        if(separator.empty()) { // use fixed width
            for(size_t i = 0; i < input.size(); i += fixedWidth)
                result.emplace_back(input.substr(i, fixedWidth));
            return result;
        }
        size_t prevPos = 0, pos = 0;
        // push substrings before separator or before input end
        while((pos = input.find(separator, prevPos)) != std::string::npos || prevPos < input.length()) {
            if(pos == std::string::npos) pos = input.length(); // no more separator use end of input
            result.emplace_back(input.substr(prevPos, pos - prevPos));
            prevPos = pos + separator.length();
        }
        result.shrink_to_fit();
        return result;
    }

    const std::map<ots::Network, ots::data::NetworkData> NETWORK_DATA_MAP = {
        {Network::MAIN, ots::data::MAINNET_DATA},
        {Network::TEST, ots::data::TESTNET_DATA},
        {Network::STAGE, ots::data::STAGENET_DATA}
    };

    const ots::data::NetworkData* getNetworkData(Network network) {
        auto it = NETWORK_DATA_MAP.find(network);
        if (it == NETWORK_DATA_MAP.end())
            throw ots::exception::UnknownNetwork();
        return &it->second;
    }
} // namespace ots
