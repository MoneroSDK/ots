#include "ots.hpp"
#include "ots-exceptions.hpp"
#include "polyseed-key-store.hpp"

namespace ots {

    bool PolyseedKeyStore::isEncypted() const noexcept {
        return is_encrypted(m_data->features);
    }

    PolyseedKeyStore::operator const uint8_t*() {
        insecureAccessRequest(false);
        return (uint8_t*)m_key.data;
    }

    uint8_t* PolyseedKeyStore::unsafe_mutable_bytes() {
        insecureAccessRequest(true);
        return (uint8_t*)m_key.data;
    }

    void PolyseedKeyStore::set(const polyseed_data* data) {
        KeyStore::set(data->secret, (size_t) 32);
        m_data->birthday = data->birthday;
        m_data->features = data->features;
        m_data->checksum = data->checksum;
    }

    void PolyseedKeyStore::transfer(polyseed_data* data) {
        set(data);
        memwipe(data->secret, 32);
    }

    const polyseed_data* PolyseedKeyStore::getPolyseedDataReadOnlySession() {
        startPolyseedDataSession(false);
        std::copy_n(m_key.data, 32, m_data->secret); // copy from key store to polyseed_data (m_data->secret)
        return m_data.get(); // return raw (insecure) polyseed_data
    }

    polyseed_data* PolyseedKeyStore::getPolyseedDataSession() {
        startPolyseedDataSession(true);
        std::copy_n(m_key.data, 32, m_data->secret); // copy from key store to polyseed_data (m_data->secret)
        return m_data.get(); // return raw (insecure) polyseed_data
    }

    void PolyseedKeyStore::commitPolyseedDataSession() {
        endPolyseedDataSession(true);
        std::copy_n(m_data->secret, 32, m_key.data); // copy from polyseed_data (m_data->secret) back to key store 
        cleanupPolyseedDataSession();
    }

    void PolyseedKeyStore::discardPolyseedDataSession() {
        endPolyseedDataSession(false);
        cleanupPolyseedDataSession();
    }

    void PolyseedKeyStore::startPolyseedDataSession(bool write) {
        insecureAccessRequest(write);
        if(m_pd_session)
            throw ots::exception::keystore::polyseed::ActivePolyseedDataSession();
        m_pd_session = true; // start session
        m_pd_session_rw = write;
    }

    void PolyseedKeyStore::endPolyseedDataSession(bool write) {
        if(!m_pd_session) {
            wipePolyseedData(); // Always wipe, better safe then sorry even it should never happen!
            throw ots::exception::keystore::polyseed::NoActivePolyseedDataSession();
        }
        if(write && !m_pd_session_rw) {
            wipePolyseedData(); // Always wipe, better safe then sorry even it should never happen!
            throw ots::exception::keystore::polyseed::PolyseedDataReadOnlySession();
        }
    }

    void PolyseedKeyStore::cleanupPolyseedDataSession() noexcept {
        wipePolyseedData();
        m_pd_session = false; // end session
        m_pd_session_rw = false;
    }

    void PolyseedKeyStore::wipePolyseedData() noexcept {
        memwipe(m_data->secret, sizeof(m_data->secret));
    }

    PolyseedKeyStore::PolyseedKeyStore() noexcept {
        m_data = std::make_unique<polyseed_data>();
    }

    void PolyseedKeyStoreDeleter::operator()(PolyseedKeyStore* store) const {
        delete store;
    }
}
