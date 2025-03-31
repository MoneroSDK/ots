#pragma once
#include "ots.hpp"
#include "ots-internal.hpp"
#include "ots-exceptions.hpp"
#include "key-store.hpp"
#include "account-dependencies.hpp"
#include "account-dependencies-rpc.hpp" // TODO: remove this file after dependencies are resolved
#include <wipeable_string.h>
#include <utility>
#include <unordered_map>
#include <set>

// transfer_details and exported_transfer_details stuff following
#include <vector>
#include <utility>
#include <crypto/crypto.h>          // For crypto::public_key, crypto::hash, crypto::key_image
#include <crypto/hash.h>            // For crypto::hash (if needed separately)

/**
 * @file account.hpp
 * @brief Header for the Monero Account class
 * @internal
 *
 * This file provides the Monero Account to encapsulate monero inner workings
 * from the Wallet class which is public and delared in ots.hpp.
 * We don't want to bother the user with monero stuff in the library.
 * So we use Account class to do all actual wallet operations.
 *
 * @todo TODO: seems that importOutputs which is source is from wallet2,
 *             cleaned down a bit, has still some unnecessary code.
 * @todo TODO: exportKeyImages seems finished yet, but should be
 *             double checked, cleaned and refactored.
 * @todo TODO: describeTransaction
 * @todo TODO: checkTransaction
 * @todo TODO: signTransaction
 */
namespace ots {
    class Account {

        public:
            /**
             * @brief Construct a new Monero Account object
             * @param key is the secret spend key
             * @param network is the network type
             * @param kdfRounds is the number of rounds for the key derivation
             *        function used for encryption and decryption
             */
            explicit Account(
                const std::array<unsigned char, 32>& key,
                const Network network,
                const uint64_t kdfRounds = 1
            );

            /**
             * @brief Construct a new Monero Account object
             * @param key is the secret spend key, crypto::secret_key is a monero secure container for the key
             * @param network is the network type
             * @param kdfRounds is the number of rounds for the key derivation
             *        function used for encryption and decryption
             */
            explicit Account(
                const crypto::secret_key& key,
                const Network network,
                const uint64_t kdfRounds = 1
            );

            /**
             * @brief Construct a new Monero Account object
             * @param key is the secret spend key, KeyStore is the secure container for the secret spend key
             * @param network is the network type
             * @param kdfRounds is the number of rounds for the key derivation
             *        function used for encryption and decryption
             */
            explicit Account(
                const KeyStore& key,
                const Network network,
                const uint64_t kdfRounds = 1
            );

            /**
             * @brief Construct a new Monero Account object
             * @param account is the account object, account_base is the monero secure container for the account
             * @param network is the network type
             * @param kdfRounds is the number of rounds for the key derivation
             *        function used for encryption and decryption
             */
            explicit Account(
                const cryptonote::account_base& account,
                const Network network,
                const uint64_t kdfRounds = 1
            );

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
             * @brief address of a certain account and index
             * @param account the account
             */
            Address address(uint32_t account, uint32_t index) const noexcept;

            /**
             * @brief address of a certain subaddr_index
             */
            Address address(const cryptonote::subaddress_index index) const noexcept;

            /**
             * @brief check if the address is in the wallet, but restricted to the search depth
             * @param address the address to check
             * @param maxAccountDepth the maximum account depth to search
             * @param maxIndexDepth the maximum index depth to search
             * @return bool true if the address is in the wallet, false otherwise
             */
            bool hasAddress(
                    const Address& address,
                    uint32_t maxAccountDepth = DEFAULT_MAX_ACCOUNT_DEPTH,
                    uint32_t maxIndexDepth = DEFAULT_MAX_INDEX_DEPTH
                    ) const noexcept;

            /**
             * @brief get the account and subindex of a address in the Wallet
             * @param address the address to check
             * @param maxAccountDepth the maximum account depth to search
             * @param maxIndexDepth the maximum index depth to search
             * @return first number is the account, second number the index
             * @throws ots::exception::address:NotFound if the address is not in the wallet
             */
            std::pair<uint32_t, uint32_t> addressIndex(
                    const Address& address,
                    uint32_t maxAccountDepth = DEFAULT_MAX_ACCOUNT_DEPTH,
                    uint32_t maxIndexDepth = DEFAULT_MAX_INDEX_DEPTH
                    ) const;

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
             * @brief import outputs from a string
             * @param outputs the outputs to import
             * @param checkMagic if true, will check the magic of the outputs
             * @return the number of outputs imported
             * @throws ots::exception::wallet::ImportOutputs if the magic is bad or the data is bad
             *
             * @note not finished yet, still need to hunt some things down and understand the complete
             *       transplanted code from wallet2, can we get rid of code because it doesn't matter
             *       for us?
             * @todo TODO: cleanup
             * @todo TODO: should add a `bool checkMagic = false` parameter to check the magic or not
             *             and add a method to check the magic separately like:
             *             `bool checkMagic(const std::string& data, const std::string& magic) const;`
             */
            size_t importOutputs(const std::string& outputs, bool checkMagic = true);

            /**
             * @brief import outputs from a tuple
             * @param outputs The outputs to import, stored as a tuple containing:
             *     - `uint64_t` for the offset,
             *     - `uint64_t` for the number of outputs,
             *     - `std::vector<exported_transfer_details>` for the details.
             * @return the number of outputs imported
             *
             * @note this one is called on import from the test wallet.
             */
            size_t importOutputs(
                const std::tuple<
                    uint64_t,
                    uint64_t,
                    std::vector<exported_transfer_details>
                > &outputs
            );

            /**
             * @brief import outputs from a tuple
             * @param outputs the outputs to import
             * @param outputs The outputs to import, stored as a tuple containing:
             *     - `uint64_t` for the offset,
             *     - `uint64_t` for the number of outputs,
             *     - `std::vector<transfer_details>` for the details.
             * @return the number of outputs imported
             *
             * @note this one was never called once until now. What is it about, should dig deeper.
             */
            size_t importOutputs(
                const std::tuple<
                    uint64_t,
                    uint64_t,
                    std::vector<transfer_details>
                > &outputs
            );

            /**
             * @brief export key images after outputs are imported
             * @return the key images in the monero-wallet format
             *
             * @todo TODO: change to std::string exportKeyImages(bool all) const;
             *             there is `std::pair<uint64_t, std::vector<std::pair<crypto::key_image, crypto::signature>>> exportKeyImages(bool all) const` which would collide...
             * @todo TODO: return WipeableString instead of std::string?
             * @todo TODO: should add a `bool addMagic = false` parameter to add the magic in front of data or not
             */
            std::string exportKeyImages() const;

            /**
             * @brief export key images after outputs are imported
             * @return key images for the provided outputs
             *
             * @todo TODO: clean up, document properly
             * @todo TODO: move to private?
             * @todo TODO: what to do about the `bool all` parameter?
             */
            std::pair<uint64_t, std::vector<std::pair<crypto::key_image, crypto::signature>>> exportKeyImages(bool all) const;

            /**
             * @brief describe a transaction
             * @param unsignedTransaction the unsigned transaction to describe
             * @param checkMagic if true, will check the magic of the unsigned transaction
             * @return the description of the transaction
             * @throws ots::exception::wallet::UnsignedTransaction if the magic is bad or the data is bad
             *
             * @note: origin is from wallet_rpc_server: `bool on_describe_transfer(const wallet_rpc::COMMAND_RPC_DESCRIBE_TRANSFER::request& req, wallet_rpc::COMMAND_RPC_DESCRIBE_TRANSFER::response& res, epee::json_rpc::error& er, const connection_context *ctx)`
             *
             * @todo TODO: sort things out, adapt to our needs and clean up
             * @todo TODO: should add a `bool addMagic = false` parameter to add the magic in front of data or not
             */
            TxDescription describeTransaction(const std::string& unsignedTransaction, bool checkMagic = true) const;


            /**
             * @brief describe a transaction in the wallet rpc way
             * @param unsignedTransaction the unsigned transaction to describe
             * @return the description of the transaction
             * @throws ots::exception::wallet::UnsignedTransaction if the magic is bad or the data is bad
             *
             * @note: origin is from wallet_rpc_server: `bool on_describe_transfer(const wallet_rpc::COMMAND_RPC_DESCRIBE_TRANSFER::request& req, wallet_rpc::COMMAND_RPC_DESCRIBE_TRANSFER::response& res, epee::json_rpc::error& er, const connection_context *ctx)`
             *
             * @todo TODO: sort things out, adapt to our needs and clean up
             * @todo TODO: remove later, only for understanding and verification purposes
             */
            tx_description describeTransactionLegacy(const std::string& unsignedTransaction) const;

            /**
             * @brief parse an unsigned transaction from a string
             * @param unsigned_tx the unsigned transaction to parse
             * @param checkMagic if true, will check the magic of the unsigned transaction
             * @return the parsed unsigned transaction
             * @throws ots::exception::wallet::UnsignedTransaction if the magic is bad or the data is bad
             *
             * @note: origin is from wallet2: `bool parse_unsigned_tx_from_str(const std::string &unsigned_tx_st, unsigned_tx_set &exported_txs) const;`
             * @todo TODO: cleanup
             */
            unsigned_tx_set parseUnsignedTransaction(const std::string &unsigned_tx, bool checkMagic = true) const;

            /**
             * @brief decrypt a ciphertext with the secret key
             * @param ciphertext the ciphertext to decrypt
             * @param skey the secret key to decrypt with
             * @param authenticated if the ciphertext is authenticated
             * @return the decrypted plaintext
             *
             * @todo TODO: clean up, document properly
             * @todo TODO: move to private?
             */
            std::string decrypt(const std::string &ciphertext, const crypto::secret_key &skey, bool authenticated) const;

            /**
             * @brief encrypt a message with a secret key
             * @param plaintext the message to encrypt
             * @param len the length of the message
             * @param skey the secret key to encrypt with
             * @param authenticated if true, will use authenticated encryption
             * @return the encrypted message
             *
             * @todo TODO: clean up, document properly
             */
            std::string encrypt(const char *plaintext, size_t len, const crypto::secret_key &skey, bool authenticated) const;

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
            std::string signData(
                    const std::string& data,
                    const std::string& address,
                    uint32_t maxAccountDepth = DEFAULT_MAX_ACCOUNT_DEPTH,
                    uint32_t maxIndexDepth = DEFAULT_MAX_INDEX_DEPTH
                    ) const;

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
             * @brief the kdf rounds, default in wallet2 is 1
             * @todo: TODO: see how we enable to change it, what we need to change
             *              in wallet.cpp, ots.hpp and elsewhere...
             */
            uint64_t mKdfRounds = 1;

            /**
            * @todo TODO: temporary helper function to make code more readable, decide later what to do with it
            * @internal
            */
            static void setupTd(const exported_transfer_details &etd, transfer_details &td);

            /**
             * @brief hash the data with the spend and view keys
             * @param data the data to hash
             * @param spendKey the public spend key
             * @param viewKey the public view key
             * @return the hash of the data
             * @throws ots::exception::BufferOverflowException if tools:write_varint fails (should not happen)
             */
            static crypto::hash hashData(const std::string& data, const crypto::public_key& spendKey, const crypto::public_key& viewKey);
            /**
             * @brief decrypt a ciphertext with the view secret key
             * @param ciphertext the ciphertext to decrypt
             * @param authenticated if the ciphertext is authenticated
             * @return the decrypted plaintext
             * @note temporary function, will be refactored
             * @todo TODO: refactor!
             * @note: origin is from wallet2: decrypt_with_view_secret_key
             */
            std::string decryptWithViewSecretKey(const std::string& ciphertext, bool authenticated = true) const;

            /**
             * @brief encrypt a message with the view secret key
             * @param plaintext the message to encrypt
             * @return the encrypted message
             * @note temporary function, will be refactored
             * @todo TODO: refactor!
             * @note: origin is from wallet2: encrypt_with_view_secret_key
             */
            std::string encryptWithViewSecretKey(const std::string &plaintext) const;

            /**
             * @brief authenticate a message with the public view key
             * @param data the data to authenticate
             * @param signature the signature to authenticate with
             * @note temporary function, will be refactored
             * @todo TODO: refactor!
             */
            void authenticateWithViewPublicKey(const std::string& data, const crypto::signature& signature) const;

            /**
             * @brief check the magic of the data
             * @param data the data to check
             * @param magic the magic to check
             * @return true if the magic is bad
             */
            bool static isBadMagic(const std::string& data, const std::string& magic);

            /**
             * @note temporary function, will be refactored
             * @todo TODO: refactor!
             */
            crypto::public_key get_tx_pub_key_from_received_outs(const transfer_details &td) const;

    void check_acc_out_precomp(const cryptonote::tx_out &o, const crypto::key_derivation &derivation, const std::vector<crypto::key_derivation> &additional_derivations, size_t i, tx_scan_info_t &tx_scan_info) const;
    void check_acc_out_precomp(const cryptonote::tx_out &o, const crypto::key_derivation &derivation, const std::vector<crypto::key_derivation> &additional_derivations, size_t i, const is_out_data *is_out_data, tx_scan_info_t &tx_scan_info) const;

            /** @brief m_account the monero account from cryptonote */
            cryptonote::account_base m_account;

            /** @brief m_addressCache the cache for the addresses */
            mutable std::unordered_map<std::string, std::pair<uint32_t, uint32_t>> m_addressCache;

            /** @brief m_indexToAddressCache the cache for the index to address */
            mutable std::map<std::pair<uint32_t, uint32_t>, std::string> m_indexToAddressCache;

            /** @brief m_addressIndexCache the cache for the address index
             *  @todo TODO: probably should be refactored because duplicate the index
             *        already in @see m_indexToAddressCache
             */
            mutable std::set<std::pair<uint32_t, uint32_t>> m_addressIndexCache;

            /**
             * @brief get the cached address index
             * @param address the address to get the index for
             * @return the account and index
             * @throws ots::exception::wallet::AddressNotFound if the address is not in the cache
             */
            std::pair<uint32_t, uint32_t> cachedAddressIndex(const Address& address) const;

            /**
             * @brief cache the address
             * @param address the address to cache
             * @param account the account
             * @param index the index
             * @internal
             */
            void cacheAddress(const Address& address, uint32_t account, uint32_t index) const noexcept;

            /**
             * @brief cache the address
             * @param address the address to cache
             * @param index the index
             *
             * @note wrapper for @see cacheAddress(const Address& address, uint32_t account, uint32_t index)
             * @internal
             */
            void cacheAddress(const Address& address, cryptonote::subaddress_index index) const noexcept;

            /**
             * @brief cache the address, if not exists in cache
             * @param index the index to cache
             * @internal
             * @note wrapper for @see cacheAddress(const Address& address, cryptonote::subaddress_index index)
             *       if addressIndexInCache(const cryptonote::subaddress_index index) is false
             */
            void cacheAddress(cryptonote::subaddress_index index) const noexcept;

            /**
             * @brief check if the address is in the cache
             * @param address the address to check
             * @return true if the address is in the cache, false otherwise
             * @internal
             */
            bool addressInCache(const Address& address) const noexcept;
            
            /**
             * @brief check if the address index is in the cache
             * @param index the index to check
             * @return true if the index is in the cache, false otherwise
             * @internal
             */
            bool addressIndexInCache(const std::pair<uint32_t, uint32_t>& index) const noexcept;
            
            /**
             * @brief check if the address index is in the cache
             * @param index the index to check
             * @return true if the index is in the cache, false otherwise
             *
             * @note wrapper for @see addressIndexInCache(const std::pair<uint32_t, uint32_t>& index)
             * @internal
             */
            bool addressIndexInCache(const uint32_t account, const uint32_t index) const noexcept;
            
            /**
             * @brief check if the address index is in the cache
             * @param index the index to check
             * @return true if the index is in the cache, false otherwise
             *
             * @note wrapper for @see addressIndexInCache(const uint32_t account, const uint32_t index)
             * @internal
             */
            bool addressIndexInCache(const cryptonote::subaddress_index& index) const noexcept;

            /**
             * @brief get the cached address for a certain account and index
             * @param account the account
             * @param index the index
             * @return the address
             * @throws ots::exception::wallet::AddressNotFound if the address is not in the cache
             *
             * @internal
             */
            Address cachedAddress(const uint32_t account, const uint32_t index) const;

            /**
             * @brief get the cached address for a certain account and index
             * @param account the account
             * @param index the index
             * @return the address
             * @throws ots::exception::wallet::AddressNotFound if the address is not in the cache
             *
             * @note it a wrappe for @see cachedAddress(const uint32_t account, const uint32_t index)
             * @internal
             */
            Address cachedAddress(const cryptonote::subaddress_index index) const;
            Network m_network;

            transfer_container m_transfers;
            std::unordered_map<crypto::key_image, size_t> m_key_images;
            std::unordered_map<crypto::public_key, size_t> m_pub_keys;
            mutable std::unordered_map<crypto::public_key, cryptonote::subaddress_index> m_subaddresses;
            std::vector<std::vector<std::string>> m_subaddress_labels;
            size_t m_subaddress_lookahead_major, m_subaddress_lookahead_minor;
    };
}
