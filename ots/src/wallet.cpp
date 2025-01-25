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
        for(uint32_t i = offset; i < stop; i++) {
            accounts.push_back(address(i, 0));
        }
		return accounts;
	}

	std::vector<Address> Wallet::subAddresses(uint32_t account, uint32_t max, uint32_t offset) const noexcept {
        std::vector<Address> subAddresses;
        uint32_t stop = max + offset;
        for(uint32_t i = offset; i < stop; i++) {
            subAddresses.push_back(address(account, i));
        }
        return subAddresses;
	}

	bool Wallet::hasAddress(const std::string& address) const noexcept {
        try {
            return hasAddress(Address(address));
        } catch(ots::exception::address::Invalid e) {
            return false;
        }
	}

	bool Wallet::hasAddress(const Address& address) const noexcept {
        return m_account->hasAddress(address);
	}

	std::pair<uint32_t, uint32_t> Wallet::addressIndex(const std::string& address) const {
        return m_account->addressIndex(Address(address), DEFAULT_MAX_ACCOUNT_DEPTH, DEFAULT_MAX_INDEX_DEPTH); // throws ots::exception::address::Invalid if not valid, and ots::exception::wallet::AddressNotFound if not in wallet TODO: max depths?
	}

	std::pair<uint32_t, uint32_t> Wallet::addressIndex(const Address& address) const {
        return m_account->addressIndex(address, DEFAULT_MAX_ACCOUNT_DEPTH, DEFAULT_MAX_INDEX_DEPTH); // throws ots::exception::wallet::AddressNotFound if not in wallet TODO: max depths?
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
		NOT_IMPLEMENTED_YET();
	}

	WipeableString Wallet::exportKeyImages() const {
		NOT_IMPLEMENTED_YET();
	}


	// Transaction-related methods
	TxDescription Wallet::describeTransaction(const std::string& unsignedTransaction) const {
		NOT_IMPLEMENTED_YET();
	}


	std::vector<TxWarning> Wallet::checkTransaction(const std::string& unsignedTransaction) const {
		NOT_IMPLEMENTED_YET();
	}

	std::vector<TxWarning> Wallet::checkTransaction(const TxDescription& description) const noexcept {
		NOT_IMPLEMENTED_YET();
	}


	std::string Wallet::signTransaction(const std::string& unsignedTransaction) const {
		NOT_IMPLEMENTED_YET();
	}


	std::string Wallet::signData(const std::string& data) const noexcept {
		NOT_IMPLEMENTED_YET();
	}

	bool Wallet::verifyData(
		const std::string& data, 
		const std::string& address, 
		const std::string& signature
		) const {
		NOT_IMPLEMENTED_YET();
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
