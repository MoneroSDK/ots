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

// transfer_details and exported_transfer_details stuff following
#include <vector>
#include <utility>
#include <crypto/crypto.h>          // For crypto::public_key, crypto::hash, crypto::key_image
#include <crypto/hash.h>            // For crypto::hash (if needed separately)
#include <cryptonote_basic/cryptonote_basic.h>  // For cryptonote::transaction_prefix, cryptonote::subaddress_index
#include <ringct/rctTypes.h>        // For rct::key
#include <serialization/serialization.h> // For serialize support
#include <serialization/variant.h>   // For tools::write_varint
#include <serialization/tuple.h>
#include <serialization/pair.h>
#include <serialization/string.h>

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
 * @todo TODO: continue with importOutputs, source is from wallet2, cleaned down a bit, but the dots
 *       are still NOT CONNECTED yet.
 */
namespace ots {
    class Account {
        struct is_out_data;
        struct tx_scan_info_t;
        struct exported_transfer_details;
        struct transfer_details;
        typedef std::vector<transfer_details> transfer_container;

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
             * @return the number of outputs imported
             * @throws ots::exception::wallet::ImportOutputs if the magic is bad or the data is bad
             *
             * @note not finished yet, still need to hunt some things down and understand the complete
             *       transplanted code from wallet2
             * @todo TODO: continue here to make it work
             */
            size_t importOutputs(const std::string& outputs);

            /**
             * @brief import outputs from a tuple
             * @param outputs the outputs to import
             * @return the number of outputs imported
             *
             * @todo TODO: clarify the tuple format
             */
            size_t importOutputs(const std::tuple<uint64_t, uint64_t, std::vector<exported_transfer_details>> &outputs);

            /**
             * @brief import outputs from a tuple
             * @param outputs the outputs to import
             * @return the number of outputs imported
             *
             * @todo TODO: clarify the tuple format
             */
            size_t importOutputs(const std::tuple<uint64_t, uint64_t, std::vector<transfer_details>> &outputs);

            /**
             * @brief export key images after outputs are imported
             * @return the key images in the monero-wallet format
             *
             * @todo TODO: change to std::string exportKeyImages(bool all) const;
             *             there is `std::pair<uint64_t, std::vector<std::pair<crypto::key_image, crypto::signature>>> exportKeyImages(bool all) const` which would collide...
             * @todo TODO: return WipeableString instead of std::string?
             */
            std::string exportKeyImages() const;

            /**
             * @brief export key images after outputs are imported
             * @return key images for the provided outputs
             *
             * @todo TODO: clean up, document properly
             * @todo TODO: move to private?
             */
            std::pair<uint64_t, std::vector<std::pair<crypto::key_image, crypto::signature>>> exportKeyImages(bool all) const;

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

            struct multisig_info
            {
                struct LR
                {
                    rct::key m_L;
                    rct::key m_R;

                    BEGIN_SERIALIZE_OBJECT()
                        FIELD(m_L)
                        FIELD(m_R)
                        END_SERIALIZE()
                };

                crypto::public_key m_signer;
                std::vector<LR> m_LR;
                std::vector<crypto::key_image> m_partial_key_images; // one per key the participant has

                BEGIN_SERIALIZE_OBJECT()
                    FIELD(m_signer)
                    FIELD(m_LR)
                    FIELD(m_partial_key_images)
                    END_SERIALIZE()
            };

            struct transfer_details
            {
                uint64_t m_block_height;
                cryptonote::transaction_prefix m_tx;
                crypto::hash m_txid;
                uint64_t m_internal_output_index;
                uint64_t m_global_output_index;
                bool m_spent;
                bool m_frozen;
                uint64_t m_spent_height;
                crypto::key_image m_key_image; //TODO: key_image stored twice :(
                rct::key m_mask;
                uint64_t m_amount;
                bool m_rct;
                bool m_key_image_known;
                bool m_key_image_request; // view wallets: we want to request it; cold wallets: it was requested
                uint64_t m_pk_index;
                cryptonote::subaddress_index m_subaddr_index;
                bool m_key_image_partial;
                std::vector<rct::key> m_multisig_k;
                std::vector<multisig_info> m_multisig_info; // one per other participant
                std::vector<std::pair<uint64_t, crypto::hash>> m_uses;

                bool is_rct() const { return m_rct; }
                uint64_t amount() const { return m_amount; }
                const crypto::public_key get_public_key() const {
                    crypto::public_key output_public_key;
                    if(m_tx.vout.size() <= m_internal_output_index)
                        throw ots::exception::wallet::InternalError("Too few outputs, outputs may be corrupted");
                    if(!get_output_public_key(m_tx.vout[m_internal_output_index], output_public_key))
                        throw ots::exception::wallet::InternalError("Unable to get output public key from output");
                    return output_public_key;
                };

                BEGIN_SERIALIZE_OBJECT()
                    FIELD(m_block_height)
                    FIELD(m_tx)
                    FIELD(m_txid)
                    FIELD(m_internal_output_index)
                    FIELD(m_global_output_index)
                    FIELD(m_spent)
                    FIELD(m_frozen)
                    FIELD(m_spent_height)
                    FIELD(m_key_image)
                    FIELD(m_mask)
                    FIELD(m_amount)
                    FIELD(m_rct)
                    FIELD(m_key_image_known)
                    FIELD(m_key_image_request)
                    FIELD(m_pk_index)
                    FIELD(m_subaddr_index)
                    FIELD(m_key_image_partial)
                    FIELD(m_multisig_k)
                    FIELD(m_multisig_info)
                    FIELD(m_uses)
                    END_SERIALIZE()
            };

            struct exported_transfer_details
            {
                crypto::public_key m_pubkey;
                uint64_t m_internal_output_index;
                uint64_t m_global_output_index;
                crypto::public_key m_tx_pubkey;
                union
                {
                    struct
                    {
                        uint8_t m_spent: 1;
                        uint8_t m_frozen: 1;
                        uint8_t m_rct: 1;
                        uint8_t m_key_image_known: 1;
                        uint8_t m_key_image_request: 1; // view wallets: we want to request it; cold wallets: it was requested
                        uint8_t m_key_image_partial: 1;
                    };
                    uint8_t flags;
                } m_flags;
                uint64_t m_amount;
                std::vector<crypto::public_key> m_additional_tx_keys;
                uint32_t m_subaddr_index_major;
                uint32_t m_subaddr_index_minor;

                BEGIN_SERIALIZE_OBJECT()
                    VERSION_FIELD(1)
                    if (version < 1)
                        return false;
                FIELD(m_pubkey)
                    VARINT_FIELD(m_internal_output_index)
                    VARINT_FIELD(m_global_output_index)
                    FIELD(m_tx_pubkey)
                    FIELD(m_flags.flags)
                    VARINT_FIELD(m_amount)
                    FIELD(m_additional_tx_keys)
                    VARINT_FIELD(m_subaddr_index_major)
                    VARINT_FIELD(m_subaddr_index_minor)
                    END_SERIALIZE()
            };

            struct tx_scan_info_t
            {
                cryptonote::keypair in_ephemeral;
                crypto::key_image ki;
                rct::key mask;
                uint64_t amount;
                uint64_t money_transfered;
                bool error;
                boost::optional<cryptonote::subaddress_receive_info> received;

                tx_scan_info_t(): amount(0), money_transfered(0), error(true) {}
            };

            struct is_out_data
            {
                crypto::public_key pkey;
                crypto::key_derivation derivation;
                std::vector<boost::optional<cryptonote::subaddress_receive_info>> received;
            };

            transfer_container m_transfers;
            std::unordered_map<crypto::key_image, size_t> m_key_images;
            std::unordered_map<crypto::public_key, size_t> m_pub_keys;
            std::unordered_map<crypto::public_key, cryptonote::subaddress_index> m_subaddresses;
            std::vector<std::vector<std::string>> m_subaddress_labels;
            size_t m_subaddress_lookahead_major, m_subaddress_lookahead_minor;
    };
}
