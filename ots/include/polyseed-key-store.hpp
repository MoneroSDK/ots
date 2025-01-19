#pragma once

/**
 * @file polyseed-key-store.hpp
 * @brief Specialized KeyStore for polyseed-related cryptographic operations
 * @internal
 */

#include "key-store.hpp"
#include "polyseed.h"
#include "../src/storage.h" // TODO: do better, it's ugly
#include "../src/features.h" // TODO: do better, it's ugly

namespace ots {
    /**
     * @class PolyseedKeyStore
     * @brief Extends KeyStore with specific functionality for polyseed-based cryptographic operations
     */
    class PolyseedKeyStore : public KeyStore {
        public:
            /**
             * @brief Default constructor for an empty seed key store
             * @note Initializes an empty, secure key storage
             */
            explicit PolyseedKeyStore() noexcept;

            /**
             * @brief Construct a PolyseedKeyStore from an existing key
             * @param key Byte array representing the seed
             */
            explicit PolyseedKeyStore(const std::array<uint8_t, 32>& key) noexcept;

            /**
             * @brief check if polyseed_data secret in encrypted
             * @return true if encrypted
             * @note using is_encrypted from features.h of polyseed to check
             *       without taking the secret out of the key store.
             */
            bool isEncypted() const noexcept;
        protected:
            /**
             * @brief Conversion to const byte pointer (read-only access)
             * @throw ots::exception::keystore::LockedAccessAttempt if not unlocked for access
             * @note You need first unlock the usage!
             * 
             * Usage example:
             * ```
             * keyStore.unlockInsecureOnce();
             * some_const_byte_function(keyStore, 32); // some_const_byte_function(const uint8_t *data, size_t size);
             * ```
             */
            operator const uint8_t*();

            /**
             * @brief Get mutable byte access 
             * @throw ots::exception::keystore::LockedAccessAttempt if not unlocked for access
             * @throw ots::exception::keystore::LockedWriteAttempt if not unlocked for write access
             * @warning Only for critical library-internal operations
             * @note You need first unlock the usage!
             * 
             * Usage example:
             * ```
             * keyStore.unlockInsecureOnce(true);
             * some_byte_manipulating_function(keyStore, 32); // some_byte_manipulating_function(uint8_t *data, size_t size);
             * ```
             */
            uint8_t* unsafe_mutable_bytes();

            /**
             * @note friend class is used to ensure that only following classes can access key data
             *       in the KeyStore
             */

            /**
             * @brief Internal method to set key data from polyseed_data->secret
             * @param data Polyseed data structure
             */
            void set(const polyseed_data* data);

            /**
             * @brief Internal method to set key data from polyseed_data->secret, and wipes polyseed_data->secret
             * @param data Polyseed data structure
             */
            void transfer(polyseed_data* data);

            /**
             * @brief Provide a polyseed_data struct const pointer to work with
             * @return pointer of the polyseed_data struct enriched containing the secret from the PolyseedKeyStore
             * @throw ots::exception::keystore::polyseed::ActivePolyseedDataSession if there is already a session running
             * @throw ots::exception::keystore::LockedAccessAttempt if not unlocked for access
             *
             * @note Need to unlockInsecure or better unlockInsecureOnce
             * @note Need to call discardPolyseedDataSession after finishing working with the const
             *       pointer to ensure the secret get's wiped out of insecure memory.
             * @warning Do NOT copy the secret data from the pointer ever!
             */
            const polyseed_data* getPolyseedDataReadOnlySession();

            /**
             * @brief Provide a polyseed_data struct pointer to work On
             * @return pointer of the polyseed_data struct enriched containing the secret from the PolyseedKeyStore
             * @throw ots::exception::keystore::polyseed::ActivePolyseedDataSession if there is already a session running
             * @throw ots::exception::keystore::LockedAccessAttempt if not unlocked for access
             * @throw ots::exception::keystore::LockedWriteAttempt if not unlocked for write access
             *
             * @warning You most like need only getPolyseedDataReadOnlySession, except for polyseed_crypt
             * @note Need to unlockInsecure or unlockInsecureOnce for write
             * @note Need to call commitPolyseedDataSession or discardPolyseedDataSession after
             *       finishing working on the pointer to ensure the secret get's wiped out of insecure
             *       memory. Do NOT copy the secret data from the pointer.
             * @warning Do NOT copy the secret data from the pointer ever!
             */
            polyseed_data* getPolyseedDataSession();

            /**
             * @brief update PolyseedKeyStore data from the pointers secret and wipe secure
             * @throw ots::exception::keystore::polyseed::NoActivePolyseedDataSession if there is no active session
             * @throw ots::exception::keystore::polyseed::PolyseedDataReadOnlySession if called on a readonly session
             *        the pointers secret
             */
            void commitPolyseedDataSession();

            /**
             * @brief wipe secure the pointers secret
             * @throw ots::exception::keystore::polyseed::NoActivePolyseedDataSession if there is no active session
             */
            void discardPolyseedDataSession();

            /**
             * @note friend class is used to ensure that only following classes can access key data
             *       in the PolyseedKeyStore
             */
            friend class Polyseed;

        private:

            /**
             * @brief wipes securely the polyseed_data secret (which is stored in the key store)
             * @internal
             */
            void wipePolyseedData() noexcept;

            /**
             * @brief bring session end to a sane state
             * @internal
             */
            void cleanupPolyseedDataSession() noexcept;

            /**
             * @brief checks and setup for a new PolyseedDataSession
             * @param write true for write access on polyseed_data
             * @throw ots::exception::keystore::polyseed::ActivePolyseedDataSession if there is already a session running
             * @throw ots::exception::keystore::LockedAccessAttempt if not unlocked for access
             * @throw ots::exception::keystore::LockedWriteAttempt if not unlocked for write access
             * @internal
             */
            void startPolyseedDataSession(bool write);

            /**
             * @brief checks to end PolyseedDataSession
             * @param write true for write access on polyseed_data
             * @throw ots::exception::keystore::polyseed::NoActivePolyseedDataSession if there is no active session
             * @throw ots::exception::keystore::polyseed::PolyseedDataReadOnlySession if called with write==true but on a readonly session
             * @internal
             */
            void endPolyseedDataSession(bool write);

            /** @brief is true if there is a polyseed_data session ongoing */
            mutable bool m_pd_session = false;

            /** @brief is true if there is a polyseed_data session ongoing */
            mutable bool m_pd_session_rw = false;

            /**
             * @brief internal polyseed_data struct, to access it with the secret
             *                 getPolyseedDataSession is used.
             */
            std::unique_ptr<polyseed_data> m_data = std::make_unique<polyseed_data>();
    };
}
