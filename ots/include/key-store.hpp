#pragma once

/**
 * @file key-store.hpp
 * @brief decoupling of crypto/crypto.h of the monero source
 * @internal
 */

#include "crypto/crypto.h"

namespace ots {
    /**
     * @class KeyStore
     * @brief essentialy wraps crypto::secret_key
     * @internal
     * Used to keep monero source out of the ots.hpp throuh forward declaration of KeyStore
     */
    class KeyStore {
        public:
            /**
             * @brief Destroy KeyStore
             */
            ~KeyStore() noexcept;

            /**
             * @brief empty KeyStore
             */
            explicit KeyStore() noexcept;

            /**
             * @brief create KeyStore from byte array
             */
            explicit KeyStore(const std::array<uint8_t, 32>& key) noexcept;

            /**
             * @brief create KeyStore from byte pointer and size
             * @param key pointer to bytes
             * @param size size of bytes, must be 32 or less, but should be always 32
             * @throws ots::exception::BufferOverflowException if size > 32
             */
            explicit KeyStore(const uint8_t* key, size_t size);

            /**
             * @brief create a KeyStore by copying a crypto::secret_key into the KeyStore
             * @note KeyStore uses internally crypto::secret_key
             */
            explicit KeyStore(crypto::secret_key key) noexcept;

            /**
             * @brief set key from byte array
             */
            void set(const std::array<uint8_t, 32>& key) noexcept;

            /**
             * @brief set key from byte pointer and size
             * @param key pointer to bytes
             * @param size size of bytes, must be 32 or less, but should be always 32
             * @throws ots::exception::BufferOverflowException if size > 32
             */
            void set(const uint8_t* key, size_t size);

            /**
             * @brief copy key from crypto::secret_key
             */
            void set(crypto::secret_key key) noexcept;

            /**
             * @brief secure wipe of the keystore
             */
            void wipe() noexcept;

            /**
             * @brief moves 32 bytes into the KeyStore and wipes the insecure memory
             * @param key the unsecured bytes pointer
             * @param size size of bytes, must be 32 or less, but should be always 32
             * @throws ots::exception::BufferOverflowException if size > 32
             */
            static KeyStore transfer(uint8_t* key, size_t size);
        protected:
            crypto::secret_key m_key;


            /**
             * @brief unlock insecure access to the stored bytes in crypto::secret_key
             * @param writeable by default false, on true permits to write to the bytes
             */
            void unlockInsecure(bool writeable = false) const noexcept;

            /**
             * @brief unlock insecure access once to the stored bytes in crypto::secret_key
             *               and locks the insecure access directly after
             * @param writeable by default false, on true permits to write to the bytes
             */
            void unlockInsecureOnce(bool writeable = false) const noexcept;

            /**
             * @brief locks insecure access complete
             */
            void lockInsecure() const noexcept;

            /**
             * @brief access the underlying crypto::secret_key by reference
             * @throw ots::exception::keystore::LockedWriteAttempt if not unlocked for write access.
             *        The reason behind is that with the crypto::secret_key object you are able to
             *        modify the data
             * @note You need first unlock the usage!
             * 
             * Usage example:
             * ```
             * keyStore.unlockInsecureOnce(true);
             * some_crypto_secret_key_function(keyStore, pub&); // some_crypto_secret_key_function(crypto::secret_key& skey, crypto::public_key& pkey);
             * ```
             */
            explicit operator crypto::secret_key&();

            /**
             * @brief Implicit conversion to const crypto::secret_key reference
             * @throw ots::exception::keystore::LockedAccessAttempt if not unlocked for access
             * @note You need first unlock the usage!
             * @warning Do NOT use explicit static_cast or similar as it will prefer the non-const operator
             *          which requires write access. Let the implicit conversion work instead.
             * 
             * Usage example:
             * ```
             * keyStore.unlockInsecureOnce();
             * // Good:
             * const crypto::secret_key& key = keyStore;  // Uses this implicit const operator
             * some_crypto_secret_key_function(keyStore); // some_crypto_secret_key_function(const crypto::secret_key& skey);
             * 
             * // Bad:
             * static_cast<const crypto::secret_key&>(keyStore);  // Will try to use non-const operator!
             * ```
             */
            operator const crypto::secret_key&() const;

            /**
             * @brief Implicit conversion to const unsigned char pointer for direct byte access
             * @throw ots::exception::keystore::LockedAccessAttempt if not unlocked for access
             * @note You need first unlock the usage!
             * @return Pointer to the underlying byte data of the secret key
             * 
             * Usage example:
             * ```
             * keyStore.unlockInsecureOnce();
             * const unsigned char* bytes = keyStore;  // Direct access to key bytes
             * ```
             */
            operator const unsigned char*() const;

            /**
             * @brief Implicit conversion to const char pointer for direct byte access
             * @throw ots::exception::keystore::LockedAccessAttempt if not unlocked for access
             * @note You need first unlock the usage!
             * @return Pointer to the underlying byte data of the secret key
             * 
             * Usage example:
             * ```
             * keyStore.unlockInsecureOnce();
             * const char* bytes = keyStore;  // Direct access to key bytes
             * ```
             */
            operator const char*() const;

            /**
             * @brief guard the insecure access attempts
             * @throw ots::exception::keystore::LockedWriteAttempt if not unlocked for write access,
             *        if write access is requested
             * @throw ots::exception::keystore::LockedAccessAttempt if not unlocked for access
             * @internal
             */
            void insecureAccessRequest(bool write) const;

            /**
             * @note friend classes is used to ensure that only following classes can access key data
             *       in the KeyStore
             */
            friend class LegacySeed;
            friend class MoneroSeed;
            friend class Polyseed;
            friend class Account;

        private:
            /** @brief if locked key data can not be accessed insecure */
            mutable bool m_lock = true;

            /** @brief if insecure access is unlock, can be written to it? */
            mutable bool m_writeable = false;

            /** @brief key data is accessable exactly once, before locked again */
            mutable bool m_unlock_once = false;

            /** @brief next insecure access to key data will be writeable */
            mutable bool m_writeable_once = false;
    };
}
