#include "ots.hpp"
#include "ots-internal.hpp"

#include "crypto/hash.h" // For crypto::hash
#include "common/util.h" // For tools::sha256sum
#include "cryptonote_basic/account.h" // For cryptonote::get_account_address_from_str
#include "cryptonote_config.h" // For cryptonote::MAINNET
#include "cryptonote_basic/cryptonote_basic.h" // For cryptonote::address_parse_info
#include "cryptonote_basic/cryptonote_format_utils.h" // For cryptonote::address_parse_info

#include "common/base58.h" // For tools::base58::decode
#include "string_tools.h" // For epee::string_tools::pod_to_hex

#include <algorithm> // For std::transform
#include <cctype>    // For std::toupper

namespace ots {

	Address::Address(const std::string& address) {
        m_network = Address::network(address); // throw ots::exception::address::Invalid(); if not valid
        m_type = Address::type(address);
		m_address = address;
	}

	Address::Address(const std::string& address, const Network network) {
		if(!isValid(address, network))
			throw ots::exception::address::Invalid();
		m_address = address;
        m_network = network;
        m_type = Address::type(address); // throw ots::exception::address::Invalid(); if not valid, what should never happen, how we validate it before
	}

	const Network Address::network() const noexcept {
        return m_network;
    }

    const AddressType Address::type() const noexcept {
        return m_type;
    }

	bool Address::isValid(const std::string& address, const Network network) noexcept {
        cryptonote::address_parse_info info;
        return cryptonote::get_account_address_from_str(info, cryptonoteNetwork(network), address);
	}

	Network Address::network(const std::string& address) {
        for(Network net : {Network::MAIN, Network::TEST, Network::STAGE}) {
            if(isValid(address, net))
                return net;
        }
        throw ots::exception::address::Invalid();
	}

    AddressType Address::type(const std::string& address) {
        std::string unused;
        uint64_t prefix;
        if(!tools::base58::decode_addr(address, prefix, unused))
            throw ots::exception::address::Invalid();
        switch(prefix) {
            case config::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX:
                return AddressType::Standard;
            case config::CRYPTONOTE_PUBLIC_SUBADDRESS_BASE58_PREFIX:
                return AddressType::SubAddress;
            case config::CRYPTONOTE_PUBLIC_INTEGRATED_ADDRESS_BASE58_PREFIX:
                return AddressType::Integrated;

            case config::testnet::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX:
                return AddressType::Standard;
            case config::testnet::CRYPTONOTE_PUBLIC_SUBADDRESS_BASE58_PREFIX:
                return AddressType::SubAddress;
            case config::testnet::CRYPTONOTE_PUBLIC_INTEGRATED_ADDRESS_BASE58_PREFIX:
                return AddressType::Integrated;

            case config::stagenet::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX:
                return AddressType::Standard;
            case config::stagenet::CRYPTONOTE_PUBLIC_SUBADDRESS_BASE58_PREFIX:
                return AddressType::SubAddress;
            case config::stagenet::CRYPTONOTE_PUBLIC_INTEGRATED_ADDRESS_BASE58_PREFIX:
                return AddressType::Integrated;
        }
        throw ots::exception::address::Invalid(); // if it is none of the above must be invalid
    }

    std::string Address::fingerprint(const std::string& address) {
        return Address(address).fingerprint();
    }

    bool Address::isIntegrated(const std::string& address) {
        std::string unused;
        uint64_t prefix;
        if(!tools::base58::decode_addr(address, prefix, unused))
            throw ots::exception::address::Invalid();
        switch(prefix) {
            case config::CRYPTONOTE_PUBLIC_INTEGRATED_ADDRESS_BASE58_PREFIX:
            case config::testnet::CRYPTONOTE_PUBLIC_INTEGRATED_ADDRESS_BASE58_PREFIX:
            case config::stagenet::CRYPTONOTE_PUBLIC_INTEGRATED_ADDRESS_BASE58_PREFIX:
                return true;
            default:
                return false;
        }
    }

    std::string Address::paymentID(const std::string& address, Network network) {
        cryptonote::address_parse_info info;
        if(!cryptonote::get_account_address_from_str(info, cryptonoteNetwork(network), address))
            throw ots::exception::address::Invalid();
        if(!info.has_payment_id)
            return "";
        return epee::string_tools::pod_to_hex(info.payment_id);
    }

    std::string Address::integratedAddress(const std::string& address, Network network) {
        cryptonote::address_parse_info info;
        if(!cryptonote::get_account_address_from_str(info, cryptonoteNetwork(network), address))
            throw ots::exception::address::Invalid();
        if(!info.has_payment_id)
            throw ots::exception::address::NotIntegrated();
        return cryptonote::get_account_address_as_str(cryptonoteNetwork(network),false, info.address);
    }

	Address::operator std::string() const noexcept {
		return m_address;
	}

	Address::operator const std::string&() const noexcept {
		return m_address;
	}

	Address::operator const uint8_t*() const noexcept {
        return reinterpret_cast<const uint8_t*>(m_address.data());
	}

    bool Address::operator==(const Address& other) const noexcept {
        return m_address == other.m_address;
    }

    bool Address::operator==(const std::string& other) const noexcept {
        return m_address == other;
    }

    const size_t Address::length() const noexcept {
        return m_address.size();
    }

    const std::string& Address::fingerprint() const noexcept {
        if(m_fingerprint.empty()) {
            crypto::hash hash; // struct hash { char data[HASH_SIZE]; };
            tools::sha256sum(reinterpret_cast<const uint8_t*>(m_address.data()), m_address.size(), hash);
            std::string hash_str = epee::string_tools::pod_to_hex(hash.data); // convert to hexdigest
            std::string fp = hash_str.substr(hash_str.length() - 6); // last 6 digits
            std::transform(fp.begin(), fp.end(), fp.begin(), ::toupper); // convert to uppercase
            m_fingerprint = std::move(fp);
        }
        return m_fingerprint;
    }

    bool Address::isIntegrated() const noexcept {
        try {
            return isIntegrated(m_address);
        } catch (const ots::exception::address::Invalid&) { // should never happen anyway.
            return false;
        }
    }

    std::string Address::paymentID() const noexcept {
        try {
            return paymentID(m_address, m_network);
        } catch (const ots::exception::address::Invalid&) { // should never happen anyway.
            return "";
        }
    }

    Address Address::integratedAddress() const {
        return Address(integratedAddress(m_address, m_network)); // throws ots::exception::address::Invalid if not valid (but should never happen), and ots::exception::address::NotIntegrated if not integrated
    }
}
