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
#include "memwipe.h" // for memwipe
// TODO: remove #include "crypto/crypto.h" // for crypto::secret_key
#include "int-util.h" // for SWAP32LE
// TODO: remove #include "mnemonics/electrum-words.h"
#include "cryptonote_basic/cryptonote_basic.h" // for cryptonote::network_type
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

    template<size_t byte_count>
    std::array<unsigned char, byte_count> seedBytes(
            const SeedIndices& indices,
            const size_t word_list_length = 1626,
            const size_t bytes_per_chunk = 4,
            const size_t words_per_chunk = 3
            );

    template<size_t byte_count>
    std::array<unsigned char, byte_count> seedBytes(
            const SeedIndices& indices,
            const size_t word_list_length,
            const size_t bytes_per_chunk,
            const size_t words_per_chunk
            ) {
        size_t words = indices.size();
        if (byte_count * words_per_chunk / bytes_per_chunk != words)
            throw ots::exception::seed::SeedDecodingFailed("Invalid number of indices for byte count.");

        std::array<unsigned char, byte_count> out;
        for (size_t pos = 0; pos < words; pos += words_per_chunk) {
            uint32_t chunk = 0;
            uint32_t previous = 0;
            uint32_t multiplier = 1;
            for (size_t word = 0; word < words_per_chunk; word++) {
                uint32_t current = indices[pos + word];
                chunk += (multiplier * (((word_list_length - previous) + current) % word_list_length));
                previous = current;
                multiplier *= word_list_length;
            }

            chunk = SWAP32LE(chunk);
            memcpy(out.data() + (pos / words_per_chunk) * bytes_per_chunk, &chunk, bytes_per_chunk);
        }
        return out;
    }
}
