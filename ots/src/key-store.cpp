#include "ots.hpp"
#include "ots-exceptions.hpp"
#include "key-store.hpp"
#include "memwipe.h"

namespace ots {
    KeyStore::KeyStore() noexcept {};

    KeyStore::KeyStore(const std::array<uint8_t, 32>& key) noexcept {
        set(key.data(), key.size());
    }

    KeyStore::KeyStore(const uint8_t* key, size_t size) {
        if (size > 32)
            throw ots::exception::BufferOverflowException();
        wipe();
        std::copy_n(key, size, m_key.data);
    }

    KeyStore::KeyStore(crypto::secret_key key) noexcept : m_key(key) {}

    void KeyStore::set(const std::array<uint8_t, 32>& key) noexcept {
        set(key.data(), key.size());
    }

    void KeyStore::set(const uint8_t* key, size_t size) {
        if(size > 32)
            throw ots::exception::BufferOverflowException();
        wipe();
        std::copy_n(key, size, m_key.data);
    }

    void KeyStore::set(crypto::secret_key key) noexcept {
        m_key = key;
    }

    KeyStore KeyStore::transfer(uint8_t* key, size_t size) {
        KeyStore store(key, size);
        memwipe(key, size);
        return store;
    }

    void KeyStore::unlockInsecure(bool writeable) const noexcept {
        m_lock = false;
        m_writeable = writeable;
        m_unlock_once = false;
        m_writeable_once = false;
    }

    void KeyStore::unlockInsecureOnce(bool writeable) const noexcept {
        m_lock = true;
        m_writeable = false;
        m_unlock_once = true;
        m_writeable_once = writeable;
    }

    void KeyStore::lockInsecure() const noexcept {
        m_lock = true;
        m_writeable = false;
        m_unlock_once = false;
        m_writeable_once = false;
    }

    void KeyStore::insecureAccessRequest(bool write) const {
        /**
         * @note On write access on of two conditions must be true !m_lock && m_writeable, or m_unlock_once && m_writeable_once
         *       That means m_writeable_once && !m_lock are not sufficient
         */
        if(write && !((m_writeable && !m_lock) || (m_writeable_once && m_unlock_once)))
            throw ots::exception::keystore::LockedWriteAttempt();
        if(m_lock && !m_unlock_once)
            throw ots::exception::keystore::LockedAccessAttempt();
        if(m_unlock_once)
            lockInsecure();
    }

    KeyStore::operator crypto::secret_key&() {
        insecureAccessRequest(true);
        return m_key;
    }

    KeyStore::operator const crypto::secret_key&() const {
        insecureAccessRequest(false);
        return m_key;
    }

    KeyStore::operator const unsigned char*() const {
        insecureAccessRequest(false);
        return reinterpret_cast<const unsigned char*>(m_key.data);
    }

    KeyStore::operator const char*() const {
        insecureAccessRequest(false);
        return reinterpret_cast<const char*>(m_key.data);
    }

    void KeyStore::wipe() noexcept {
        memwipe(m_key.data, 32);
    }

    KeyStore::~KeyStore() noexcept {
        wipe();
    }

    void KeyStoreDeleter::operator()(KeyStore* store) const {
        delete store;
    }
} // namespace ots
