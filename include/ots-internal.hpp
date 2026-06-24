#pragma once

/**
 * @file ots-internal.hpp
 * @brief Internal declarations for library
 * @see ots.cpp for implementations
 * @internal
 *
 * @todo resolve dependencies to compile everything static inside.
 *       boost, easylogger (remove and wrapper?)
 */

#include "ots.hpp"
#include "data.hpp"
#include <memwipe.h> // for memwipe
// TODO: remove #include "crypto/crypto.h" // for crypto::secret_key
#include <int-util.h> // for SWAP32LE
// TODO: remove #include "mnemonics/electrum-words.h"
#include <cryptonote_basic/cryptonote_basic.h> // for cryptonote::network_type
// TODO: remove #include "cryptonote_core/cryptonote_core.h" // for cryptonote::address_parse_info

namespace ots {
    /**
     * @brief A class to handle a reference copy of a value and zeroize it on destruction
     * @tparam T type of the value
     * @param value the const reference to copy and zeroize
     * @internal
     */
    template <typename T>
    class ZeroizeReferenceCopy {
        public:
            explicit ZeroizeReferenceCopy(const T& value): ptr_(new T(value)) {}
            ~ZeroizeReferenceCopy() {
                if(ptr_) {
                    // TODO: investigate if memwipe realy does more than std::memset
                    // std::memset(ptr_.get(), 0, sizeof(T));
                    memwipe(ptr_.get(), sizeof(T));
                    delete ptr_.release();
                }
            }
            operator T&() { return *ptr_; }
            std::unique_ptr<T> ptr_;
    };

    /**
     * @brief Converts the network type from the library network type to cryptonote network type
     * @param network the library network type
     * @return the cryptonote network type
     * @internal
     *
     * @note There is the possibility that there are other network types in the library than in cryptonote, so the function returns UNDEFINED if the network type is not in cryptonote. This function should be revised if the library network types are extended.
     */
    cryptonote::network_type cryptonoteNetwork(Network network) noexcept;

    /**
     * @brief Converts a string by a separator or fixed 4 char width
     * @param input the string to split
     * @param separator the separator to split the string, if empty the fixed width is used
     * @param fixedWidth the fixed width to split the string, by default 4 chars
     * @return the vector of strings
     */
    std::vector<std::string> splitString(const std::string &input, const std::string &separator = "", size_t fixedWidth = 4);

    /**
     * @brief Returns the indices of a seed phrase
     * @param bytes the bytes to convert
     * @param byte_length the length of the bytes
     * @param word_list_length the length of the word list, monero wordlist length is 1626
     * @param bytes_per_chunk the number of bytes per chunk, monero uses 4 bytes per chunk is electrum default
     * @param words_per_chunk the number of words per chunk, monero uses 3 words per chunk is electrum default
     * @return the seed indices
     * @throws ots::exception::seed::LengthMismatch If more bits provided than needed
     * @internal
     */
    SeedIndices seedIndices(const unsigned char* bytes, size_t byte_length, size_t word_list_length = 1626, size_t bytes_per_chunk = 4, size_t words_per_chunk = 3);

    /** 
     * @brief Function to convert seed indices to bytes, the monero way.
     * @param indices A collection of seed indices.
     * @param word_list_length The length of the word list (default 1626).
     * @param bytes_per_chunk The number of bytes per chunk (default 4).
     * @param words_per_chunk The number of words per chunk (default 3).
     * @return An array of seed bytes.
     * @throws ots::exception::seed::SeedDecodingFailed If the number of indices is invalid for the byte count.
     * @note The function is copying the behaviour extracted from the monero codebase.
     *       I tried to make the function easier understandable, how I needed to think
     *       some time to understand the monero code. Also tried to make it somehow reusable
     *       for other seed types, although not used, so at the moment the parameters
     *       `word_list_length`, `bytes_per_chunk` and `words_per_chunk` are always the
     *       default because this applies to monero seeds (includes legacy seeds (12/13 words)).
     *       Highly dislike this function but found no more elegant and simpler way to
     *       reimplemnt.
     * @see https://github.com/monero-project/monero/blob/84df77404e8bcbe1cf409f64c81e4e4f9c84885b/src/mnemonics/electrum-words.cpp#L264
     * @note added it here as an internal template function in the header to avoid code
     *       duplication. Not very happy with it but probably best I can do about it.
     *       @see src/seed-legacy.cpp `LegacySeed::decode(const SeedIndices&, uint64_t, uint64_t, Network)`
     *       @see src/seed-monero.cpp `MoneroSeed::decode(const SeedIndices&, uint64_t, uint64_t, Network, const std::string&)`
     * @internal
     */
    template<size_t byte_count>
    std::array<unsigned char, byte_count> seedBytes(
            const SeedIndices& indices,
            const size_t word_list_length = 1626,
            const size_t bytes_per_chunk = 4,
            const size_t words_per_chunk = 3
            ) {
        size_t words = indices.size();
        if (byte_count * words_per_chunk / bytes_per_chunk != words)
            throw ots::exception::seed::SeedDecodingFailed("Invalid number of indices for byte count.");
        std::array<unsigned char, byte_count> out;
        for (size_t pos = 0; pos < words; pos += words_per_chunk) { // iterates over the chunks
            uint32_t chunk = 0;
            uint32_t previous = 0;
            uint32_t multiplier = 1;
            for (size_t word = 0; word < words_per_chunk; word++) { // iterate over words in the current chunk
                // current index for word in chunk
                uint32_t current = indices[pos + word];
                // add to current chunk:
                // word_list_length^word * ((word_list_length - previous + current) mod word_list_length)
                chunk += (multiplier * ((word_list_length - previous + current) % word_list_length));
                // we store the current value to substract it in the next iteration from the word_list_length
                previous = current;
                // it is uglier then using pow, but pow is float and could fuck up the correct result,
                // additional it is more efficient. But mathematically or logically it seem easier (to me)
                // to think about word_list_length^word
                multiplier *= word_list_length;
            }
            chunk = SWAP32LE(chunk); // make sure that byte order is correct
            // copy the calculated chunk into the right position of the secret spent key
            memcpy(out.data() + (pos / words_per_chunk) * bytes_per_chunk, &chunk, bytes_per_chunk);
        }
        return out;
    }

    extern const std::map<Network, ots::data::NetworkData> NETWORK_DATA_MAP;
    const ots::data::NetworkData* getNetworkData(Network network);
}
