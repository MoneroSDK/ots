#pragma once
#include "ots.hpp"
#include "ots-internal.hpp"
#include "ots-exceptions.hpp"
#include "key-store.hpp"
#include <wipeable_string.h>
#include <cryptonote_basic/cryptonote_format_utils.h>
#include <cryptonote_basic/account.h>
#include <utility>
#include <unordered_map>
#include <set>

#define DEFAULT_MAX_ACCOUNT_DEPTH 10
#define DEFAULT_MAX_INDEX_DEPTH 100
/**
 * @file account.hpp
 * @brief Header for the Monero Account class
 * @internal
 *
 * This file provides the Monero Account to encapsulate monero inner workings
 * from the Wallet class which is public and delared in ots.hpp.
 * We don't want to bother the user with monero stuff in the library.
 * So we use Account class to do all actual wallet operations.
 */
namespace ots {
    class Account {
        public:
            /**
             * @brief Construct a new Monero Account object
             * @param key is the secret spend key
             * @param network is the network type
             */
            explicit Account(const std::array<unsigned char, 32>& key, const Network network);

            /**
             * @brief Construct a new Monero Account object
             * @param key is the secret spend key, crypto::secret_key is a monero secure container for the key
             * @param network is the network type
             */
            explicit Account(const crypto::secret_key& key, const Network network);

            /**
             * @brief Construct a new Monero Account object
             * @param key is the secret spend key, KeyStore is the secure container for the secret spend key
             * @param network is the network type
             */
            explicit Account(const KeyStore& key, const Network network);

            /**
             * @brief Construct a new Monero Account object
             * @param account is the account object, account_base is the monero secure container for the account
             * @param network is the network type
             */
            explicit Account(const cryptonote::account_base& account, const Network network);

            /**
             * @brief Construct a new Monero Account object
             * @param account is the account object, it simply duplicates the account from another Account object
             */
            explicit Account(const Account& account);

            /**
             * @brief the public address of the wallet/account(0)
             * @return Address the public address
             */
            Address address() const noexcept;

            /**
             * @brief address of a certain account and indexe
             * @param account the account
             */
            Address address(uint32_t account, uint32_t index) const noexcept;

            /**
             * @brief check if the address is in the wallet, but restricted to the search depth
             * @param address the address to check
             * @param maxAccountDepth the maximum account depth to search
             * @param maxIndexDepth the maximum index depth to search
             * @return bool true if the address is in the wallet, false otherwise
             */
            bool hasAddress(const Address& address, uint32_t maxAccountDepth = DEFAULT_MAX_ACCOUNT_DEPTH, uint32_t maxIndexDepth = DEFAULT_MAX_INDEX_DEPTH) const noexcept;

            /**
             * @brief get the account and subindex of a address in the Wallet
             * @param address the address to check
             * @param maxAccountDepth the maximum account depth to search
             * @param maxIndexDepth the maximum index depth to search
             * @return first number is the account, second number the index
             * @throws ots::exception::address:NotFound if the address is not in the wallet
             */
            std::pair<uint32_t, uint32_t> addressIndex(const Address& address, uint32_t maxAccountDepth = DEFAULT_MAX_ACCOUNT_DEPTH, uint32_t maxIndexDepth = DEFAULT_MAX_INDEX_DEPTH) const;

            /**
             * @brief secret spend key of the wallet in hex digits
             */
            WipeableString secretSpendKey() const noexcept;

            /**
             * @brief public spend key of the wallet in hex digits
             */
            WipeableString publicSpendKey() const noexcept;

            /**
             * @brief secret view key of the wallet in hex digits
             */
            WipeableString secretViewKey() const noexcept;

            /**
             * @brief public view key of the wallet in hex digits
             */
            WipeableString publicViewKey() const noexcept;

            /**
             * @brief clear the address cache
             */
            void clearAddressCache() const noexcept;

            /**
             * @brief sign a message with the wallet
             * @param data the message to sign
             * @return the signature of the message
             * @throws ots::exception::BufferOverflowException if tools:write_varint fails in Account::hashData() (should not happen)
             */
            std::string signData(const std::string& data) const;

            /**
             * @brief sign a message with subindex address of the wallet
             * @param data the message to sign
             * @param index the account and index to sign with
             * @return the signature of the message
             * @throws ots::exception::BufferOverflowException if tools:write_varint fails in Account::hashData() (should not happen)
             */
            std::string signData(const std::string& data, const std::pair<uint32_t, uint32_t>& index) const;

            /**
             * @brief sign a message with the provided address of the wallet
             * @param data the message to sign
             * @param address the address of the wallet
             * @param maxAccountDepth the maximum account depth to search
             * @param maxIndexDepth the maximum index depth to search
             * @return the signature of the message
             * @throws ots::exception::address::Invalid if the address is not valid
             * @throws ots::exception::BufferOverflowException if tools:write_varint fails in Account::hashData() (should not happen)
             */
            std::string signData(const std::string& data, const std::string& address, uint32_t maxAccountDepth = DEFAULT_MAX_ACCOUNT_DEPTH, uint32_t maxIndexDepth = DEFAULT_MAX_INDEX_DEPTH) const;

            /**
             * @brief sign a message with the provided address of the wallet
             * @param data the message to sign
             * @param address the address of the wallet
             * @param maxAccountDepth the maximum account depth to search
             * @param maxIndexDepth the maximum index depth to search
             * @return the signature of the message
             * @throws ots::exception::address::Invalid if the address is not valid
             * @throws ots::exception::BufferOverflowException if tools:write_varint fails in Account::hashData() (should not happen)
             */
            std::string signData(const std::string& data, const Address& address, uint32_t maxAccountDepth = DEFAULT_MAX_ACCOUNT_DEPTH, uint32_t maxIndexDepth = DEFAULT_MAX_INDEX_DEPTH) const;

            /**
             * @brief verify a signed message
             * @param data the message to verify
             * @param address of the signer
             * @param signature of the message
             * @return true if the signature is valid
             * @note the signature must be a base58 encoded string
             * @note will only verify signatures of Version 2 and only mode 0 (signed with secret spend key)
             */
            bool static verifyData(const std::string& data, const Address& address, const std::string& signature);

            /**
             * @brief verify a signed message
             * @param data the message to verify
             * @param address of the signer
             * @param signature of the message
             * @return true if the signature is valid
             * @note the signature must be a base58 encoded string
             * @note will only verify signatures of Version 2 and only mode 0 (signed with secret spend key)
             */
            bool static verifyData(const std::string& data, const std::string& address, const std::string& signature);

            /**
             * @brief verify a signed message
             * @param data the message to verify
             * @param address of the signer
             * @param signature of the message
             * @return true if the signature is valid
             * @note the signature must be a base58 encoded string
             * @note will only verify signatures of Version 1 and only mode 0 (signed with secret spend key)
             */
            bool static verifyDataLegacy(const std::string& data, const Address& address, const std::string& signature);

            /**
             * @brief verify a signed message
             * @param data the message to verify
             * @param address of the signer
             * @param signature of the message
             * @return true if the signature is valid
             * @note the signature must be a base58 encoded string
             * @note will only verify signatures of Version 1 and only mode 0 (signed with secret spend key)
             */
            bool static verifyDataLegacy(const std::string& data, const std::string& address, const std::string& signature);

        private:
            /**
             * @brief hash the data with the spend and view keys
             * @param data the data to hash
             * @param spendKey the public spend key
             * @param viewKey the public view key
             * @return the hash of the data
             * @throws ots::exception::BufferOverflowException if tools:write_varint fails (should not happen)
             */
            static crypto::hash hashData(const std::string& data, const crypto::public_key& spendKey, const crypto::public_key& viewKey);
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
