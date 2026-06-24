#include <time.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <utf8proc.h>

#include "ots.hpp"
#include "ots-internal.hpp"
#include "seed-polyseed-internal.hpp"
#include "ots-exceptions.hpp"
#include "key-store.hpp"
#include "polyseed-key-store.hpp"
#include "cryptonote_basic/cryptonote_format_utils.h" // for cryptonote::decrypt_key

#include "memwipe.h"
#include "crypto/crypto.h"
#include "string_tools.h" // for epee::string_tools::pod_to_hex // TODO: remove

/**
 * @file seed-polyseed.cpp
 * @brief Implementation of the Polyseed class
 * @todo Refactor, the private header file use of polyseed is ugly, the dependency injection is ugly, too.
 * @see monero-seed-internal.hpp for ugly includes and dependency injection wrapper functions
 * @see utf8proc in CMakelists.txt for the utf8proc library which is used for utf8 normalization
 */

namespace ots {
    const WipeableString Polyseed::phrase(const SeedLanguage& language, const std::string& password) const {
        injectPolyseedDependency();
        if(!language.supported(SeedType::Polyseed))
            throw ots::exception::polyseed::UnsupportedLanguage();
        m_seed->unlockInsecureOnce(false); // request insecure access to key store
        polyseed_data pd = *m_seed->getPolyseedDataReadOnlySession();
        if(!password.empty()) {
            polyseed_crypt(&pd, password.c_str());
        }
        polyseed_str phrase; // struct { char str[POLYSEED_STR_SIZE]; } from polyseed.h
        size_t length = polyseed_encode(&pd, polyseed_get_lang(language.index(SeedType::Polyseed)), polyseed_coin::POLYSEED_MONERO, phrase);
        if(length == 0) { // encoding failed
            memwipe(&(pd.secret), sizeof(pd.secret)); // cleanup to wipe memory
            m_seed->discardPolyseedDataSession(); // cleanup to wipe memory
            throw ots::exception::seed::SeedEncodingFailed();
        }
        WipeableString out(phrase);
        memwipe(&(pd.secret), sizeof(pd.secret)); // cleanup to wipe memory
        memwipe(phrase, sizeof(phrase));
        m_seed->discardPolyseedDataSession();
        return std::move(out);
    }

    const SeedIndices Polyseed::indices(const std::string& password) const {
        SeedIndices out;
        out.reserve((size_t)POLYSEED_NUM_WORDS);
        m_seed->unlockInsecureOnce(false); // request insecure access to key store
        polyseed_data pd = *m_seed->getPolyseedDataReadOnlySession(); // copy secret key
        if(!password.empty()) {
            polyseed_crypt(&pd, password.c_str());
        }
        gf_poly poly = { 0 };
        polyseed_data_to_poly(&pd, &poly); // convert secret to polynomial
        for(size_t i=0; i<POLYSEED_NUM_WORDS; i++) {
            out.emplace_back(poly.coeff[i]);
        }
        memwipe(&poly, sizeof(poly));
        memwipe(&(pd.secret), sizeof(pd.secret)); // cleanup to wipe memory
        m_seed->discardPolyseedDataSession();
        return std::move(out);
    }

    /**
     * @note Unfortunately the birthday in polyseed is set with the dependency injection
     *       what makes it necessary to inject every time or use a global/static variable
     *       to be flexible with the timestamp.
     *       So for now polyseed_create is not used, but re-implemented in a way the timestamp
     *       can be set on a by case base without writing ugly code.
     * @todo Maybe a PR with a refactoring of tevador/polyseed in the future and if accepted
     *       refactor here again, too.
     */
    Polyseed Polyseed::create(
            const std::array<unsigned char, 19>& random,
            const Network network,
            uint64_t time_,
            const std::string& passphrase
            ) {
        injectPolyseedDependency();
        Polyseed ps;
        ps.m_network = network;
        polyseed_data* pd = new polyseed_data();
        if(time_ == 0)
            time_ = time(nullptr); // use current time if not set
        pd->birthday = birthday_encode(time_); // encode birthday
        pd->features = make_features(0); // no features, so no need to check if features valid neither
        memwipe(&(pd->secret), sizeof(pd->secret)); // cleanup to wipe memory
        memcpy(pd->secret, random.data(), random.size()); // pd->secret[SECRET_SIZE], SECRET_SIZE is 19 bytes
        pd->secret[SECRET_SIZE - 1] &= CLEAR_MASK; // remove special bits from random key
        // encode polynomial
        gf_poly poly = { 0 };
        polyseed_data_to_poly(pd, &poly);
        // calculate checksum
        gf_poly_encode(&poly);
        pd->checksum = poly.coeff[0];
        // wipe memory
        memwipe(&(poly), sizeof(poly));
        crypto::secret_key secret_spend_key;
        polyseed_keygen(pd, polyseed_coin::POLYSEED_MONERO, 32, (uint8_t*)secret_spend_key.data);
        if(!passphrase.empty()) {
            const epee::wipeable_string passphrase_wipeable(passphrase);
            secret_spend_key = cryptonote::encrypt_key(secret_spend_key, passphrase_wipeable);
        }
        ps.m_key->set(secret_spend_key);
        ps.m_timestamp = birthday_decode(pd->birthday);
        ps.m_seed->transfer(pd); // move secret seed into keystore and end session
        cryptonote::account_base account;
        account.generate(secret_spend_key, true, false);
        ps.m_address = std::make_unique<Address>(account.get_public_address_str(cryptonoteNetwork(network)));
        return std::move(ps);
    }

    Polyseed Polyseed::create(
            const std::array<unsigned char, 32>& random,
            const Network network,
            uint64_t time_,
            const std::string& passphrase
            ) {
        std::array<unsigned char, 19> random19;
        std::copy_n(random.data(), 19, random19.data());
        return std::move(Polyseed::create(random19, network, time_, passphrase));
    }

    Polyseed Polyseed::generate(
            const Network network,
            uint64_t time_,
            const std::string& passphrase
            ) {
        std::array<unsigned char, 19> random;
        OTS::random(19, random.data()); // random secret key. CAREFUL SECRET_SIZE is 19 bytes only, not 32!
        return std::move(Polyseed::create(random, network, time_, passphrase));
    }

	Polyseed Polyseed::decode(
            const std::string& phrase,
            const Network network,
            const std::string& password,
            const std::string& passphrase
            ) {
        injectPolyseedDependency();
        Polyseed ps;
        ps.m_network = network;
        polyseed_data* pd = new polyseed_data();
        polyseed_status status = polyseed_decode(phrase.c_str(), polyseed_coin::POLYSEED_MONERO, nullptr, &pd);
        if(status != polyseed_status::POLYSEED_OK) {
            memwipe(&(pd->secret), sizeof(pd->secret)); // cleanup to wipe memory;
            onPolyseedStatusNotOkThrowException(status);
        }
        bool encrypted = polyseed_is_encrypted(pd);
        if(encrypted && password.empty()) {
            memwipe(&(pd->secret), sizeof(pd->secret)); // cleanup to wipe memory;
            throw ots::exception::polyseed::NoPasswordProvided();
        }
        if(encrypted)
            polyseed_crypt(pd, password.c_str());
        crypto::secret_key secret_spend_key;
        polyseed_keygen(pd, polyseed_coin::POLYSEED_MONERO, 32, (uint8_t*)secret_spend_key.data);
        if(!passphrase.empty()) {
            const epee::wipeable_string passphrase_wipeable(passphrase);
            secret_spend_key = cryptonote::encrypt_key(secret_spend_key, passphrase_wipeable);
        }
        ps.m_key->set(secret_spend_key);
        ps.m_timestamp = birthday_decode(pd->birthday);
        ps.m_seed->transfer(pd);
        cryptonote::account_base account;
        account.generate(secret_spend_key, true, false);
        ps.m_address = std::make_unique<Address>(account.get_public_address_str(cryptonoteNetwork(network)));
        return std::move(ps);
	}

	Polyseed Polyseed::decode(
			const std::string& phrase, 
			const SeedLanguage& language, 
			const Network network,
            const std::string& password,
            const std::string& passphrase
			) {
        injectPolyseedDependency();
        Polyseed ps;
        ps.m_network = network;
        polyseed_data* pd = new polyseed_data();
        polyseed_status status = polyseed_decode_explicit(phrase.c_str(), polyseed_coin::POLYSEED_MONERO, polyseed_get_lang(language.index(SeedType::Polyseed)), &pd);
        if(status != polyseed_status::POLYSEED_OK) {
            memwipe(&(pd->secret), sizeof(pd->secret)); // cleanup to wipe memory;
            onPolyseedStatusNotOkThrowException(status);
        }
        bool encrypted = polyseed_is_encrypted(pd);
        if(encrypted && password.empty()) {
            memwipe(&(pd->secret), sizeof(pd->secret)); // cleanup to wipe memory;
            throw ots::exception::polyseed::NoPasswordProvided();
        }
        if(encrypted)
            polyseed_crypt(pd, password.c_str());
        crypto::secret_key secret_spend_key;
        polyseed_keygen(pd, polyseed_coin::POLYSEED_MONERO, 32, (uint8_t*)secret_spend_key.data);
        if(!passphrase.empty()) {
            const epee::wipeable_string passphrase_wipeable(passphrase);
            secret_spend_key = cryptonote::encrypt_key(secret_spend_key, passphrase_wipeable);
        }
        ps.m_key->set(secret_spend_key);
        ps.m_timestamp = birthday_decode(pd->birthday);
        ps.m_seed->transfer(pd);
        cryptonote::account_base account;
        account.generate(secret_spend_key, true, false);
        ps.m_address = std::make_unique<Address>(account.get_public_address_str(cryptonoteNetwork(network)));
        return std::move(ps);
	}

	Polyseed Polyseed::decode(
            const std::vector<uint16_t>& indices,
            Network network,
            const std::string& password,
            const std::string& passphrase
            ) {
        return decode(SeedIndices(indices), network, password, passphrase);
    }

	Polyseed Polyseed::decode(
            const SeedIndices& indices,
            Network network,
            const std::string& password,
            const std::string& passphrase
            ) {
        injectPolyseedDependency();
		Polyseed ps;
        ps.m_network = network;
        polyseed_data* pd = new polyseed_data();

        /* encode polynomial */
        gf_poly poly = { 0 };
        for(size_t i=0; i<indices.size(); i++) {
            poly.coeff[i] = indices[i];
        }
        gf_poly_encode(&poly);
        pd->checksum = poly.coeff[0];
        if (!gf_poly_check(&poly)) {
            memwipe(&(poly), sizeof(poly));
            memwipe(&(pd->secret), sizeof(pd->secret)); // cleanup to wipe memory;
            throw ots::exception::polyseed::ChecksumMismatch();
        }
        polyseed_poly_to_data(&poly, pd);
        // no features to check, so we will not test anything here!
        bool encrypted = polyseed_is_encrypted(pd);
        if(encrypted && password.empty()) {
            memwipe(&(poly), sizeof(poly));
            memwipe(&(pd->secret), sizeof(pd->secret)); // cleanup to wipe memory;
            throw ots::exception::polyseed::NoPasswordProvided();
        }
        if(encrypted)
            polyseed_crypt(pd, password.c_str());
        memwipe(&(poly), sizeof(poly));
        crypto::secret_key secret_spend_key;
        polyseed_keygen(pd, polyseed_coin::POLYSEED_MONERO, 32, (uint8_t*)secret_spend_key.data);
        if(!passphrase.empty()) {
            const epee::wipeable_string passphrase_wipeable(passphrase);
            secret_spend_key = cryptonote::encrypt_key(secret_spend_key, passphrase_wipeable);
        }
        ps.m_key->set(secret_spend_key);
        ps.m_timestamp = birthday_decode(pd->birthday);
        ps.m_seed->transfer(pd);
        cryptonote::account_base account;
        account.generate(secret_spend_key, true, false);
        ps.m_address = std::make_unique<Address>(account.get_public_address_str(cryptonoteNetwork(network)));
        return std::move(ps);
	}

    MoneroSeed Polyseed::moneroSeed() const {
        m_key->unlockInsecureOnce(false); // request insecure access to key store
        std::array<unsigned char, 32> key = *m_key;
        MoneroSeed seed = MoneroSeed::create(key, 0, timestamp(), m_network);
        memwipe(key.data(), key.size());
        return std::move(seed);
    }

    void onPolyseedStatusNotOkThrowException(const polyseed_status& status) {
        switch(status) {
            case(polyseed_status::POLYSEED_OK): // success, nothing to do
                return;
            case(polyseed_status::POLYSEED_ERR_NUM_WORDS): // wrong number of words in the phrase
                throw ots::exception::polyseed::WordCount();
            case(polyseed_status::POLYSEED_ERR_LANG): // unknown language or unsupported words
                throw ots::exception::polyseed::UnsupportedLanguage();
            case(polyseed_status::POLYSEED_ERR_CHECKSUM): // checksum mismatch
                throw ots::exception::polyseed::ChecksumMismatch();
            case(polyseed_status::POLYSEED_ERR_UNSUPPORTED): // unsupported seed features
                throw ots::exception::polyseed::UnsupportedFeatures();
            case(polyseed_status::POLYSEED_ERR_FORMAT): // Invalid seed format
                throw ots::exception::polyseed::InvalidSeedFormat();
            case(polyseed_status::POLYSEED_ERR_MEMORY): // Memory allocation failure
                throw ots::exception::polyseed::MemoryAllocationError();
            case(polyseed_status::POLYSEED_ERR_MULT_LANG): // Multiple languages detected
                throw ots::exception::polyseed::AmbigousLanguage();
        }
    }

    Polyseed::Polyseed(): m_seed(new PolyseedKeyStore(), PolyseedKeyStoreDeleter()) {}

    /** @todo TODO: holy clusterfuck, I really dislike using the polyseed lib as is */
    void injectPolyseedDependency() {
        static bool injected = false;
        if(!injected) {
            polyseed_dependency pd;
            pd.randbytes = polyseed_random_wrapper;
            pd.pbkdf2_sha256 = polyseed_pbkdf2_wrapper;
            pd.memzero = polyseed_memwipe_wrapper;
            pd.u8_nfc = utf8_nfc;
            pd.u8_nfkd = utf8_nfkd;
            pd.time = nullptr;  // Optional, set to nullptr if not needed
            pd.alloc = nullptr; // Optional, set to nullptr if not needed
            pd.free = nullptr;  // Optional, set to nullptr if not needed
            polyseed_inject(&pd);
            injected = true;
        }
    }

    void polyseed_random_wrapper(void* result, size_t n) {
        OTS::random(n, static_cast<uint8_t*>(result));
    }

    // Wrapper for PBKDF2
    void polyseed_pbkdf2_wrapper(const uint8_t* pw, size_t pwlen,
            const uint8_t* salt, size_t saltlen,
            uint64_t iterations,
            uint8_t* key, size_t keylen) {
        PKCS5_PBKDF2_HMAC(
                reinterpret_cast<const char*>(pw),
                pwlen,
                salt,
                saltlen,
                iterations,
                EVP_sha256(),  // Use SHA256
                keylen,
                key
                );
    }

    // Wrapper for memwipe
    void polyseed_memwipe_wrapper(void* const ptr, const size_t len) {
        memwipe(ptr, len);
    }

    size_t utf8_nfc(const char* str, polyseed_str norm) {
        utf8proc_uint8_t* result = utf8proc_NFC((const utf8proc_uint8_t*)str);
        if (!result) return 0;

        size_t len = strlen((char*)result);
        size_t size = std::min(len, (size_t)POLYSEED_STR_SIZE - 1);
        memcpy(norm, result, size);
        norm[size] = '\0';
        free(result);
        return size;
    }

    size_t utf8_nfkd(const char* str, polyseed_str norm) {
        utf8proc_uint8_t* result = utf8proc_NFKD((const utf8proc_uint8_t*)str);
        if (!result) return 0;

        size_t len = strlen((char*)result);
        size_t size = std::min(len, (size_t)POLYSEED_STR_SIZE - 1);
        memcpy(norm, result, size);
        norm[size] = '\0';
        free(result);
        return size;
    }
}
