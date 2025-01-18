#include "ots.hpp"
#include "ots-internal.hpp"
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

    uint64_t OTS::heightFromTimestamp(uint64_t timestamp, Network network) {
        NOT_IMPLEMENTED_YET();
    }

    uint64_t OTS::timestampFromHeight(uint64_t height, Network network) {
        NOT_IMPLEMENTED_YET();
    }

    std::array<unsigned char, 32> OTS::random() {
        std::array<unsigned char, 32> key;
        OTS::random(key.size(), key.data());
        return std::move(key);
    }

    void OTS::random(size_t size, uint8_t *bytes) {
        crypto::generate_random_bytes_thread_safe(size, bytes);
    }

    crypto::secret_key Internal::keyFromHash(const std::vector<unsigned char>& data) {
		crypto::secret_key key;
		hash_to_scalar(data.data(), data.size(), key);
        return std::move(key);
    }

    epee::wipeable_string Internal::seedPhraseFromKey(const crypto::secret_key& key, const std::string& language) {
        epee::wipeable_string words;
        if(!crypto::ElectrumWords::bytes_to_words(key, words, language)) // if it returns false it failed to generate words, and we throw an exception.
            throw ots::exception::RuntimeError("Unable to generate seed phrase from bytes");
        return std::move(words);
    }

    cryptonote::network_type cryptonoteNetwork(Network network) noexcept {
        if(static_cast<uint8_t>(network) > 2)
            return cryptonote::network_type::UNDEFINED;
        return static_cast<cryptonote::network_type>(network);
    }

    SeedIndices seedIndices(const unsigned char* bytes, size_t byte_length, size_t word_list_length, size_t bytes_per_chunk, size_t words_per_chunk) {
        SeedIndices indices;

        if (byte_length % bytes_per_chunk != 0) {
            throw ots::exception::seed::SeedEncodingFailed("Input byte size is not a multiple of bytes_per_chunk.");
        }

        size_t chunks = byte_length / bytes_per_chunk;

        for (size_t pos = 0; pos < chunks; ++pos) {
            uint32_t chunk = SWAP32LE(*(const uint32_t*)(bytes + (pos * bytes_per_chunk)));

            uint32_t used = 0;
            size_t divisor = 1;

            for (size_t word = 0; word < words_per_chunk; ++word) {
                uint16_t index = ((chunk / divisor) + used) % word_list_length;
                indices.push_back(index);
                used = index;
                divisor *= word_list_length;
            }
        }

        return indices;
    }

    template<size_t byte_count>
    auto seedBytes(
            const SeedIndices& indices,
            const size_t word_list_length,
            const size_t bytes_per_chunk,
            const size_t words_per_chunk
            ) {
        size_t words = indices.size();
        if (byte_count * words_per_chunk / bytes_per_chunk == words) {
            throw ots::exception::seed::SeedDecodingFailed("Invalid number of indices for byte count.");
        }

        std::array<unsigned char, byte_count> out;
        for (size_t i = 0; i < words; i += words_per_chunk) {
            uint32_t chunk = indices[i];
            for (size_t word = 1; word < words_per_chunk; word++) {
                chunk += word_list_length * std::pow(word_list_length, word - 1) * 
                    ((word_list_length - indices[i + word - 1] + indices[i + word]) % word_list_length);
            }

            chunk = SWAP32LE(chunk);
            memcpy(out.data() + (i/words_per_chunk)*bytes_per_chunk, &chunk, bytes_per_chunk);
        }
        return out;
    }
} // namespace ots
