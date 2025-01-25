#include "account.hpp"
#include "string_tools.h" // for epee::string_tools::pod_to_hex

namespace ots {

    Account::Account(const std::array<unsigned char, 32>& key, const Network network): m_network(network) {
        m_account = cryptonote::account_base();
        crypto::secret_key sk;
        std::copy_n(key.data(), key.size(), sk.data);
        crypto::secret_key secretSpendKey = m_account.generate(sk, true, false);
    }

    Account::Account(const crypto::secret_key& key, const Network network): m_network(network) {
        m_account = cryptonote::account_base();
        crypto::secret_key secretSpendKey = m_account.generate(key, true, false);
    }

    Account::Account(const KeyStore& key, const Network network): m_network(network) {
        m_account = cryptonote::account_base();
        key.unlockInsecureOnce();
        crypto::secret_key secretSpendKey = m_account.generate(static_cast<const crypto::secret_key&>(key), true, false);
    }

    Account::Account(const cryptonote::account_base& account, const Network network): m_account(account), m_network(network) {}

    Account::Account(const Account& account): m_account(account.m_account), m_network(account.m_network) {}

    Address Account::address() const noexcept {
        return Address(m_account.get_public_address_str(cryptonoteNetwork(m_network)));
    }

    Address Account::address(uint32_t account, uint32_t index) const noexcept {
        cryptonote::subaddress_index subaddress_index = {account, index};
        cryptonote::account_public_address addr = m_account.get_device().get_subaddress(m_account.get_keys(), subaddress_index);
        return Address(cryptonote::get_account_address_as_str(cryptonoteNetwork(m_network), false, addr));
    }

    void Account::cacheAddress(const Address& address, uint32_t account, uint32_t index) const noexcept {
        std::pair<uint32_t, uint32_t> pair = std::make_pair(account, index);
        if(addressInCache(address) || addressIndexInCache(pair))
            return;
        m_addressIndexCache.insert(pair);
        m_addressCache[address] = pair;
    }

    bool Account::addressInCache(const Address& address) const noexcept {
        return !m_addressCache.empty() && m_addressCache.find(address) != m_addressCache.end();
    }

    bool Account::addressIndexInCache(const std::pair<uint32_t, uint32_t>& index) const noexcept {
        return !m_addressIndexCache.empty() && m_addressIndexCache.find(index) != m_addressIndexCache.end();
    }

    bool Account::addressIndexInCache(const uint32_t account, const uint32_t index) const noexcept {
        return addressIndexInCache(std::make_pair(account, index));
    }

    bool Account::hasAddress(const Address& address, uint32_t maxAccountDepth, uint32_t maxIndexDepth) const noexcept {
        if(address == this->address() || addressInCache(address))
            return true;
        for(uint32_t acc = 0; acc < maxAccountDepth; acc++) {
            for(uint32_t idx = 0; idx < maxIndexDepth; idx++) {
                if(addressIndexInCache(acc, idx))
                    continue;
                Address addr = this->address(acc, idx);
                cacheAddress(addr, acc, idx);
                if(addr == address)
                    return true;
            }
        }
        return false;
    }

    std::pair<uint32_t, uint32_t> Account::cachedAddress(const Address& address) const noexcept {
        return m_addressCache[address];
    }

    std::pair<uint32_t, uint32_t> Account::addressIndex(const Address& address, uint32_t maxAccountDepth, uint32_t maxIndexDepth) const {
        if(address == this->address())
            return std::make_pair(0, 0);
        if(addressInCache(address))
            return cachedAddress(address);
        for(uint32_t acc = 0; acc < maxAccountDepth; acc++) {
            for(uint32_t idx = 0; idx < maxIndexDepth; idx++) {
                if(addressIndexInCache(acc, idx))
                    continue;
                Address addr = this->address(acc, idx);
                cacheAddress(addr, acc, idx);
                if(addr == address)
                    return std::make_pair(acc, idx);
            }
        }
        throw ots::exception::wallet::AddressNotFound();
    }

    WipeableString Account::secretSpendKey() const noexcept {
        return WipeableString(epee::string_tools::pod_to_hex(m_account.get_keys().m_spend_secret_key.data));
    }

    WipeableString Account::publicSpendKey() const noexcept {
        return WipeableString(epee::string_tools::pod_to_hex(m_account.get_keys().m_account_address.m_spend_public_key));
    }

    WipeableString Account::secretViewKey() const noexcept {
        return WipeableString(epee::string_tools::pod_to_hex(m_account.get_keys().m_view_secret_key.data));
    }

    WipeableString Account::publicViewKey() const noexcept {
        return WipeableString(epee::string_tools::pod_to_hex(m_account.get_keys().m_account_address.m_view_public_key));
    }

    void Account::clearAddressCache() const noexcept {
        m_addressCache.clear();
        m_addressIndexCache.clear();
    }

    void AccountDeleter::operator()(Account* account) const {
        delete account;
    }
}
