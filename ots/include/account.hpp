#pragma once
#include "ots.hpp"
#include "ots-internal.hpp"
#include "ots-exceptions.hpp"
#include "key-store.hpp"
#include "wipeable_string.h" // for epee::wipeable_string
#include "cryptonote_basic/cryptonote_format_utils.h" // for cryptonote::decrypt_key
#include "cryptonote_basic/account.h" // for cryptonote::account_base
#include <utility> // For std::pair
#include <unordered_map> // For std::unordered_map
#include <set> // For std::set

#define DEFAULT_MAX_ACCOUNT_DEPTH 10
#define DEFAULT_MAX_INDEX_DEPTH 100
/**
 * @todo TODO: documentation missing
 */
namespace ots {
    class Account {
        public:
            explicit Account(const std::array<unsigned char, 32>& key, const Network network);
            explicit Account(const crypto::secret_key& key, const Network network);
            explicit Account(const KeyStore& key, const Network network);
            explicit Account(const cryptonote::account_base& account, const Network network);
            explicit Account(const Account& account);

            Address address() const noexcept;
            Address address(uint32_t account, uint32_t index) const noexcept;
            bool hasAddress(const Address& address, uint32_t maxAccountDepth = DEFAULT_MAX_ACCOUNT_DEPTH, uint32_t maxIndexDepth = DEFAULT_MAX_INDEX_DEPTH) const noexcept;
            std::pair<uint32_t, uint32_t> addressIndex(const Address& address, uint32_t maxAccountDepth = DEFAULT_MAX_ACCOUNT_DEPTH, uint32_t maxIndexDepth = DEFAULT_MAX_INDEX_DEPTH) const;
            WipeableString secretSpendKey() const noexcept;
            WipeableString publicSpendKey() const noexcept;
            WipeableString secretViewKey() const noexcept;
            WipeableString publicViewKey() const noexcept;

        private:
            cryptonote::account_base m_account;
            mutable std::unordered_map<std::string, std::pair<uint32_t, uint32_t>> m_addressCache;
            mutable std::set<std::pair<uint32_t, uint32_t>> m_addressIndexCache;
            std::pair<uint32_t, uint32_t> cachedAddress(const Address& address) const noexcept;
            void cacheAddress(const Address& address, uint32_t account, uint32_t index) const noexcept;
            bool addressInCache(const Address& address) const noexcept;
            bool addressIndexInCache(const std::pair<uint32_t, uint32_t>& index) const noexcept;
            bool addressIndexInCache(const uint32_t account, const uint32_t index) const noexcept;
            Network m_network;
    };
}
