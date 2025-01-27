#include "account.hpp"
#include "string_tools.h" // for epee::string_tools::pod_to_hex
#include "common/base58.h" // For tools::base58::decode
#include "cryptonote_basic/cryptonote_basic.h" // For cryptonote::address_parse_info

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

    crypto::hash Account::hashData(const std::string& data, const crypto::public_key& spendKey, const crypto::public_key& viewKey) {
        KECCAK_CTX ctx;
        keccak_init(&ctx);
        keccak_update(&ctx, (const uint8_t*)config::HASH_KEY_MESSAGE_SIGNING, sizeof(config::HASH_KEY_MESSAGE_SIGNING)); // includes NUL
        keccak_update(&ctx, (const uint8_t*)&spendKey, sizeof(crypto::public_key));
        keccak_update(&ctx, (const uint8_t*)&viewKey, sizeof(crypto::public_key));
        const uint8_t mode = 0;
        keccak_update(&ctx, (const uint8_t*)&mode, sizeof(uint8_t));
        char len_buf[(sizeof(size_t) * 8 + 6) / 7];
        char *ptr = len_buf;
        tools::write_varint(ptr, data.size());
        if(!(ptr > len_buf && ptr <= len_buf + sizeof(len_buf)))
            throw ots::exception::BufferOverflowException("Length overflow, hashing data caused by tools::write_varint()");
        keccak_update(&ctx, (const uint8_t*)len_buf, ptr - len_buf);
        keccak_update(&ctx, (const uint8_t*)data.data(), data.size());
        crypto::hash hash;
        keccak_finish(&ctx, (uint8_t*)&hash);
        return hash;
    }

    std::string Account::signData(const std::string& data) const {
        if(data.empty())
            throw ots::exception::sign::EmptyMessage();
        const cryptonote::account_keys &keys = m_account.get_keys();
        crypto::hash hash = hashData(data, keys.m_account_address.m_spend_public_key, keys.m_account_address.m_view_public_key);
        crypto::signature signature;
        crypto::generate_signature(hash, keys.m_account_address.m_spend_public_key, keys.m_spend_secret_key, signature);
        return std::string("SigV2") + tools::base58::encode(std::string((const char *)&signature, sizeof(signature)));
    }

    std::string Account::signData(const std::string& data, const std::pair<uint32_t, uint32_t>& index) const {
        if(data.empty())
            throw ots::exception::sign::EmptyMessage();
        cryptonote::subaddress_index idx = {index.first, index.second};
        const cryptonote::account_keys &keys = m_account.get_keys();
        crypto::secret_key skey_spend = keys.m_spend_secret_key;
        crypto::secret_key m = m_account.get_device().get_subaddress_secret_key(keys.m_view_secret_key, idx);
        sc_add((unsigned char*)&skey_spend, (unsigned char*)&m, (unsigned char*)&skey_spend);
        crypto::public_key pkey_spend;
        secret_key_to_public_key(skey_spend, pkey_spend);
        crypto::secret_key skey_view;
        sc_mul((unsigned char*)&skey_view, (unsigned char*)&keys.m_view_secret_key, (unsigned char*)&skey_spend);
        crypto::public_key pkey_view;
        secret_key_to_public_key(skey_view, pkey_view);
        crypto::hash hash = hashData(data, pkey_spend, pkey_view);
        crypto::signature signature;
        crypto::generate_signature(hash, pkey_spend, skey_spend, signature);
        return std::string("SigV2") + tools::base58::encode(std::string((const char *)&signature, sizeof(signature)));
    }

    std::string Account::signData(const std::string& data, const Address& address, uint32_t maxAccountDepth, uint32_t maxIndexDepth) const {
        std::pair<uint32_t, uint32_t> index = addressIndex(address, maxAccountDepth, maxIndexDepth); // throws ots::exception::wallet::AddressNotFound if address is not in the wallet until maxAccountDepth and maxIndexDepth
        return signData(data, index);
    }

    std::string Account::signData(const std::string& data, const std::string& address, uint32_t maxAccountDepth, uint32_t maxIndexDepth) const {
        return signData(data, Address(address), maxAccountDepth, maxIndexDepth);
    }

    bool Account::verifyData(const std::string& data, const Address& address, const std::string& signature) {
        if(data.empty())
            throw ots::exception::sign::EmptyMessage();
        static const std::string& header("SigV2");
        cryptonote::address_parse_info info;
        const Network network = address.network();
        if(!get_account_address_from_str(info, cryptonoteNetwork(network), address))
            throw ots::exception::address::Invalid();
        if(signature.size() < header.size() || signature.substr(0, header.size()) != header)
            throw ots::exception::sign::InvalidSignature();
        std::string decoded;
        if(!tools::base58::decode(signature.substr(header.size()), decoded))
            throw ots::exception::sign::InvalidSignature();
        crypto::signature s;
        if (sizeof(s) != decoded.size())
            throw ots::exception::sign::InvalidSignature();
        memcpy(&s, decoded.data(), sizeof(s));
        crypto::hash hash = hashData(data, info.address.m_spend_public_key, info.address.m_view_public_key);
        return crypto::check_signature(hash, info.address.m_spend_public_key, s);
    }

    bool Account::verifyData(const std::string& data, const std::string& address, const std::string& signature) {
        return verifyData(data, Address(address), signature); // throws ots::exception::address::Invalid if address is not valid
    }

    bool Account::verifyDataLegacy(const std::string& data, const Address& address, const std::string& signature) {
        if(data.empty())
            throw ots::exception::sign::EmptyMessage();
        static const std::string& header("SigV1");
        cryptonote::address_parse_info info;
        const Network network = address.network();
        if(!get_account_address_from_str(info, cryptonoteNetwork(network), address))
            throw ots::exception::address::Invalid();
        if(signature.size() < header.size() || signature.substr(0, header.size()) != header)
            throw ots::exception::sign::InvalidSignature();
        std::string decoded;
        if(!tools::base58::decode(signature.substr(header.size()), decoded))
            throw ots::exception::sign::InvalidSignature();
        crypto::signature s;
        if (sizeof(s) != decoded.size())
            throw ots::exception::sign::InvalidSignature();
        memcpy(&s, decoded.data(), sizeof(s));
        crypto::hash hash;
        crypto::cn_fast_hash(data.data(), data.size(), hash);
        return crypto::check_signature(hash, info.address.m_spend_public_key, s);
    }

    bool Account::verifyDataLegacy(const std::string& data, const std::string& address, const std::string& signature) {
        return verifyDataLegacy(data, Address(address), signature); // throws ots::exception::address::Invalid if address is not valid
    }

    void Account::clearAddressCache() const noexcept {
        m_addressCache.clear();
        m_addressIndexCache.clear();
    }

    void AccountDeleter::operator()(Account* account) const {
        delete account;
    }
}
