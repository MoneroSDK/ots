#include "ots.hpp"
#include "ots-internal.hpp"
#include "ots-exceptions.hpp"
#include "key-store.hpp"
#include "int-util.h" // for SWAP32LE
#include "mnemonics/electrum-words.h" // for crypto::ElectrumWords
#include "wipeable_string.h" // for epee::wipeable_string
#include "cryptonote_basic/cryptonote_format_utils.h" // for cryptonote::decrypt_key
#include "cryptonote_basic/account.h" // for cryptonote::account_base
#include "memwipe.h" // for memwipe

#define MONERO_SEED_BYTES 32
#define MONERO_SEED_INDICES 24

namespace ots {
	const WipeableString MoneroSeed::phrase(const SeedLanguage& language, const std::string& passphrase) const {
        epee::wipeable_string phrase;
        m_key->unlockInsecureOnce(false);
        crypto::secret_key key = *m_key;
        if(!passphrase.empty()) {
            const epee::wipeable_string passphrase_wipeable(passphrase);
            key = cryptonote::encrypt_key(key, passphrase_wipeable);
        }
        if(!crypto::ElectrumWords::bytes_to_words(key, phrase, language.englishName()))
            throw ots::exception::seed::SeedDecodingFailed();
        return WipeableString(phrase.data(), phrase.size());
	}

    const SeedIndices MoneroSeed::indices(const std::string& passphrase) const {
        SeedIndices indices;
        m_key->unlockInsecureOnce(false);
        crypto::secret_key key = *m_key;
        if(!passphrase.empty()) {
            const epee::wipeable_string passphrase_wipeable(passphrase);
            key = cryptonote::encrypt_key(key, passphrase_wipeable);
        }
        return std::move(ots::seedIndices(reinterpret_cast<const unsigned char*>(key.data), (size_t)(size_t)MONERO_SEED_BYTES));
    }

	MoneroSeed MoneroSeed::decode(
			const std::string& phrase,
			uint64_t height,
			uint64_t time,
			const Network network,
            const std::string& passphrase
			) {
        crypto::secret_key recovery_key;
        std::string lang;
        const epee::wipeable_string phrase_wipeable(phrase);
        if(!crypto::ElectrumWords::words_to_bytes(phrase_wipeable, recovery_key, lang))
            throw ots::exception::seed::SeedDecodingFailed();
        if(!passphrase.empty()) {
            const epee::wipeable_string passphrase_wipeable(passphrase);
            recovery_key = cryptonote::decrypt_key(recovery_key, passphrase_wipeable);
        }
        MoneroSeed seed;
        seed.m_height = height;
        seed.m_timestamp = time;
        seed.m_network = network;
        cryptonote::account_base account;
        crypto::secret_key secret_spend_key = account.generate(recovery_key, true, false);
        seed.m_key->set(secret_spend_key);
        seed.m_address = std::make_unique<Address>(account.get_public_address_str(cryptonoteNetwork(network)));
        return std::move(seed);
	}

    MoneroSeed MoneroSeed::decode(
            const std::vector<uint16_t>& indices, 
            uint64_t height,
            uint64_t time,
            const Network network,
            const std::string& passphrase
            ) {
        return std::move(decode(SeedIndices(indices), height, time, network, passphrase));
    }

    MoneroSeed MoneroSeed::decode(
            const SeedIndices& indices,
            uint64_t height,
            uint64_t time,
            Network network,
            const std::string& passphrase
            ) {
        if(indices.size() != MONERO_SEED_INDICES)
            throw ots::exception::seed::SeedDecodingFailed();
        std::array<unsigned char, (size_t)MONERO_SEED_BYTES> recovery_bytes = ots::seedBytes<(size_t)MONERO_SEED_BYTES>(indices); // @see ots-internal.hpp
        MoneroSeed seed;
        seed.m_height = height;
        seed.m_timestamp = time;
        seed.m_network = network;
        crypto::secret_key secret_spend_key;
        memcpy(secret_spend_key.data, recovery_bytes.data(), (size_t)MONERO_SEED_BYTES);
        if(!passphrase.empty()) {
            const epee::wipeable_string passphrase_wipeable(passphrase);
            secret_spend_key = cryptonote::decrypt_key(secret_spend_key, passphrase_wipeable);
        }
        cryptonote::account_base account;
        account.generate(secret_spend_key, true, false);
        seed.m_key->set(secret_spend_key);
        seed.m_address = std::make_unique<Address>(account.get_public_address_str(cryptonoteNetwork(network)));
        return std::move(seed);
    }

    MoneroSeed MoneroSeed::create(
            const std::array<unsigned char, (size_t)MONERO_SEED_BYTES>& random,
            uint64_t height,
            uint64_t time,
            const Network network
            ) {
        MoneroSeed seed;
        seed.m_height = height;
        seed.m_timestamp = time;
        seed.m_network = network;
        crypto::secret_key secret_key;
        memcpy(secret_key.data, random.data(), (size_t)MONERO_SEED_BYTES);
        cryptonote::account_base account;
        crypto::secret_key secret_spend_key = account.generate(secret_key, true, false);
        seed.m_key->set(secret_spend_key);
        seed.m_address = std::make_unique<Address>(account.get_public_address_str(cryptonoteNetwork(network)));
        return std::move(seed);
    }

	MoneroSeed MoneroSeed::generate(
			uint64_t height,
			uint64_t time,
			const Network network
			) {
        std::array<unsigned char, (size_t)MONERO_SEED_BYTES> random = OTS::random();
        MoneroSeed out = create(random, height, time, network);
        memwipe(random.data(), random.size());
        return std::move(out);
	}
}
