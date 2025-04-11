#include "ots.hpp"
#include "key-store.hpp"
#include "account.hpp"

namespace ots {

    Address Wallet::address(uint32_t account, uint32_t index) const noexcept {
        return m_account->address(account, index);
    }

    std::vector<Address> Wallet::accounts(uint32_t max, uint32_t offset) const noexcept {
        std::vector<Address> accounts;
        uint32_t stop = max + offset;
        for(uint32_t i = offset; i < stop; i++)
            accounts.push_back(address(i, 0));
        return accounts;
    }

    std::vector<Address> Wallet::subAddresses(
            uint32_t account,
            uint32_t max,
            uint32_t offset
            ) const noexcept {
        std::vector<Address> subAddresses;
        uint32_t stop = max + offset;
        for(uint32_t i = offset; i < stop; i++)
            subAddresses.push_back(address(account, i));
        return subAddresses;
    }

    bool Wallet::hasAddress(
            const std::string& address,
            uint32_t maxAccountDepth,
            uint32_t maxIndexDepth
            ) const noexcept {
        try {
            return hasAddress(Address(address), maxAccountDepth, maxIndexDepth);
        } catch(ots::exception::address::Invalid e) {
            return false;
        }
    }

    bool Wallet::hasAddress(
            const Address& address,
            uint32_t maxAccountDepth,
            uint32_t maxIndexDepth
            ) const noexcept {
        return m_account->hasAddress(address,
                OTS::maxAccountDepth(maxAccountDepth),
                OTS::maxIndexDepth(maxIndexDepth)
                );
    }

    std::pair<uint32_t, uint32_t> Wallet::addressIndex(
            const std::string& address,
            uint32_t maxAccountDepth,
            uint32_t maxIndexDepth
            ) const {
        return m_account->addressIndex(
                Address(address), // throws ots::exception::address::Invalid if not valid, and ots::exception::wallet::AddressNotFound if not in wallet
                OTS::maxAccountDepth(maxAccountDepth),
                OTS::maxIndexDepth(maxIndexDepth)
                );
    }

    std::pair<uint32_t, uint32_t> Wallet::addressIndex(
            const Address& address,
            uint32_t maxAccountDepth,
            uint32_t maxIndexDepth
            ) const {
        return m_account->addressIndex(
                address, // throws ots::exception::wallet::AddressNotFound if not in wallet
                OTS::maxAccountDepth(maxAccountDepth),
                OTS::maxIndexDepth(maxIndexDepth)
                );
    }

    WipeableString Wallet::secretViewKey() const noexcept {
        return m_account->secretViewKey();
    };

    WipeableString Wallet::publicViewKey() const noexcept {
        return m_account->publicViewKey();
    };

    WipeableString Wallet::secretSpendKey() const noexcept {
        return m_account->secretSpendKey();
    };

    WipeableString Wallet::publicSpendKey() const noexcept {
        return m_account->publicSpendKey();
    };

    uint64_t Wallet::importOutputs(const std::string& outputs) {
        return m_account->importOutputs(outputs);
    }

    std::string Wallet::exportKeyImages() const {
        return m_account->exportKeyImages();
    }


    // Transaction-related methods
    TxDescription Wallet::describeTransaction(const std::string& unsignedTransaction) const {
        return m_account->describeTransaction(unsignedTransaction);
    }


    std::vector<TxWarning> Wallet::checkTransaction(const std::string& unsignedTransaction) const {
        return checkTransaction(describeTransaction(unsignedTransaction));
    }

    std::vector<TxWarning> Wallet::checkTransaction(const TxDescription& description) const noexcept {
        return m_account->checkTransaction(description);
    }


    std::string Wallet::signTransaction(const std::string& unsignedTransaction) const {
        return m_account->signTransaction(unsignedTransaction);
    }


    std::string Wallet::signData(const std::string& data) const noexcept {
        return m_account->signData(data);
    }

    std::string Wallet::signData(const std::string& data, const std::pair<uint32_t, uint32_t>& index) const {
        return m_account->signData(data, index);
    }

    std::string Wallet::signData(const std::string& data, const Address& address) const {
        return m_account->signData(data, address);
    }

    std::string Wallet::signData(const std::string& data, const std::string& address) const {
        return m_account->signData(data, address);
    }

    bool Wallet::verifyData(
            const std::string& data,
            const std::string& address,
            const std::string& signature,
            bool legacyFallback
            ) {
        return Account::verifyData(data, address, signature) || (legacyFallback && Account::verifyDataLegacy(data, address, signature));
    }

    bool Wallet::verifyData(
            const std::string& data,
            const Address& address,
            const std::string& signature,
            bool legacyFallback
            ) {
        return Account::verifyData(data, address, signature) || (legacyFallback && Account::verifyDataLegacy(data, address, signature));
    }

    bool Wallet::verifyData(
            const std::string& data,
            const std::pair<uint32_t, uint32_t>& index,
            const std::string& signature,
            bool legacyFallback
            ) const {
        const Address& address = m_account->address(index.first, index.second);
        return m_account->verifyData(data, address, signature) || (legacyFallback && m_account->verifyDataLegacy(data, address, signature));
    }

    bool Wallet::verifyData(
            const std::string& data,
            const std::string& signature,
            bool legacyFallback
            ) const {
        return m_account->verifyData(data, m_account->address(), signature) || (legacyFallback && m_account->verifyDataLegacy(data, m_account->address(), signature));
    }

    Wallet::Wallet(const std::array<unsigned char, 32>& key, uint64_t height, const Network network) noexcept :
        m_key(new KeyStore(key), KeyStoreDeleter()),
        m_account(new Account(key, network), AccountDeleter()),
        m_height(height),
        m_network(network) {}

    Wallet::Wallet(const KeyStore& key, uint64_t height, const Network network) noexcept :
        m_key(new KeyStore(key), KeyStoreDeleter()),
        m_account(new Account(key, network), AccountDeleter()),
        m_height(height),
        m_network(network) {}

    Wallet::Wallet(const Account& account, const KeyStore& key, uint64_t height, const Network network) noexcept :
        m_key(new KeyStore(key), KeyStoreDeleter()),
        m_account(new Account(account), AccountDeleter()),
        m_height(height),
        m_network(network) {}
}
