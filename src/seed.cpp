#include <openssl/evp.h>
#include <openssl/sha.h>

#include "ots.hpp"
#include "key-store.hpp"
#include "memwipe.h"

#define SEED_PBKDF2_ITERATIONS 100000
#define SEED_PBKDF2_SALT "OTS-SeedMerge-v1" // Fixed application salt
#define SEED_PBKDF2_SALT_LEN 16 // strlen(SEED_PBKDF2_SALT)
   
namespace ots {

	const uint64_t Seed::timestamp() const noexcept {
		if(m_timestamp != 0)
			return m_timestamp;
		return OTS::timestampFromHeight(m_height, m_network);
	}

	const uint64_t Seed::height() const noexcept {
		if(m_height == 0 && m_timestamp != 0)
			return OTS::heightFromTimestamp(m_timestamp, m_network);
		return m_height;
	}

    const std::string& Seed::fingerprint() const noexcept {
        return m_address->fingerprint();
    }

    const Address& Seed::address() const noexcept {
        return *m_address;
    }

    Seed::Seed(): m_key(new KeyStore(), KeyStoreDeleter()), m_network(Network::MAIN) {};

    std::vector<uint16_t> Seed::mergeValues(const std::vector<uint16_t>& values1, const std::vector<uint16_t>& values2) {
        if(values1.size() != values2.size())
            throw ots::exception::seed::LengthMismatch();
        std::vector<uint16_t> values;
        values.reserve(values1.size());
        for(size_t i = 0; i < values1.size(); ++i)
            values.emplace_back(values1[i] ^ values2[i]);
        return std::move(values);
    }

    std::vector<uint16_t> Seed::mergeValues(const std::vector<std::vector<uint16_t>>& values) {
        if(values.size() < 2)
            throw ots::exception::seed::TooFewValues();
        size_t size = values[0].size(); // take size from first vector
        for(const auto& v: values)
            if(size != v.size()) // and make sure all vectors have the same size
                throw ots::exception::seed::LengthMismatch();
        std::vector<uint16_t> merged = values[0];
        for(size_t i = 1; i < values.size(); ++i)
            merged = mergeValues(merged, values[i]);
        return std::move(merged);
    }

    std::vector<uint16_t> Seed::mergeAndZeorizeValues(std::vector<uint16_t>& values1, std::vector<uint16_t>& values2, bool del) {
        std::vector<uint16_t> merged = mergeValues(values1, values2);
        memwipe(values1.data(), values1.size() * sizeof(uint16_t));
        memwipe(values2.data(), values2.size() * sizeof(uint16_t));
        if(del) {
            delete &values1;
            delete &values2;
        }
        return std::move(merged);
    }

    std::vector<uint16_t> Seed::mergeAndZeorizeValues(std::vector<std::vector<uint16_t>>& values, bool del) {
        std::vector<uint16_t> merged = mergeValues(values);
        for(auto& v: values) {
            memwipe(v.data(), v.size() * sizeof(uint16_t));
            if(del)
                delete &v;
        }
        return std::move(merged);
    }

    std::vector<uint16_t> Seed::mergeWithPassword(const std::string& password,
            const std::vector<uint16_t>& values) 
    {
        if(values.size() > 32)
            throw ots::exception::seed::MergeError("Seed values too long. Seed values exceed the length of the password hash.");
        std::vector<unsigned char> derived_key(values.size() * 2);
        // Derive key material
        const int rc = PKCS5_PBKDF2_HMAC(
                password.c_str(), password.size(),
                reinterpret_cast<const uint8_t*>(SEED_PBKDF2_SALT), (size_t)SEED_PBKDF2_SALT_LEN,
                SEED_PBKDF2_ITERATIONS,
                EVP_sha256(),
                derived_key.size(), derived_key.data()
                );
        if(rc != 1)
            throw exception::seed::MergeError("PBKDF2 key derivation failed");
        // Process values
        std::vector<uint16_t> result;
        result.reserve(values.size());
        for(size_t i = 0; i < values.size(); ++i) {
            /*
            uint16_t key_val = static_cast<uint16_t>(derived_key[2*i]) << 8 
                | derived_key[2*i + 1];
            result.emplace_back(values[i] ^ key_val);
            */
            result.emplace_back(values[i] ^ (static_cast<uint16_t>(derived_key[2 * i]) << 8 | derived_key[2 * i + 1]));
        }
        // Cleanup
        memwipe(derived_key.data(), derived_key.size());
        return std::move(result);
    }

    std::vector<uint16_t> Seed::mergeWithPasswordAndZeorize(
            std::string& password,
            std::vector<uint16_t>& values,
            bool del)
    {
            auto result = mergeWithPassword(password, values); // if it throws, we let it happen and don't cleanup
            // Wipe original values
            memwipe(values.data(), values.size() * sizeof(uint16_t));
            memwipe(password.data(), password.size());
            if(del) {
                delete &values;  // Matches existing zerodize pattern
                delete &password;
            }
            return std::move(result);
    }

    std::vector<uint16_t> Seed::mergeWithPassword(
            const WipeableString& password,
            const std::vector<uint16_t>& values)
    {
        return std::move(mergeWithPassword(password.insecure(), values));
    }

    std::vector<uint16_t> Seed::mergeWithPasswordAndZeorize(
            const WipeableString& password,
            std::vector<uint16_t>& values,
            bool del)
    {
        std::vector<uint16_t> out = std::move(mergeWithPassword(password.insecure(), values));
        memwipe(values.data(), values.size() * sizeof(uint16_t));
        if(del)
            delete &values;
        return std::move(out);
    }

    std::shared_ptr<Wallet> Seed::wallet() noexcept {
        if(m_wallet == nullptr)
            m_wallet = std::make_shared<Wallet>(*m_key, m_height, m_network);
        return m_wallet;
    }
}
