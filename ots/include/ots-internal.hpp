#pragma once

/**
 * @file ots-internal.hpp
 * @brief Internal declarations for library
 * @internal
 *
 * @todo resolve dependencies to compile everything static inside.
 *       boost, easylogger (remove and wrapper?)
 */

#include "ots.hpp"
#include "memwipe.h" // for memwipe
#include "crypto/crypto.h" // for crypto::secret_key
#include "mnemonics/electrum-words.h"
#include "cryptonote_basic/cryptonote_basic.h" // for cryptonote::network_type
// #include "cryptonote_core/cryptonote_core.h" // for cryptonote::address_parse_info

namespace ots {
    class Internal {
        /**
         * @brief Returns a key from a monero hash
         * @todo think if right place and if it makes even sense here, think it should be in MoneroSeed
         */
        static crypto::secret_key keyFromHash(const std::vector<unsigned char>& data);

        /**
         * @brief Returns a seed phrase from a key
         * @todo think if right place and if it makes even sense here, think it should be in MoneroSeed
         */
        static epee::wipeable_string seedPhraseFromKey(const crypto::secret_key& key, const std::string& language);
    };

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
}
