#include "ots.hpp"
#include "ots-internal.hpp"
#include "ots-exceptions.hpp"
#include "account.hpp"
#include "key-store.hpp"
#include "int-util.h"
#include "mnemonics/electrum-words.h"
#include "wipeable_string.h"
#include "cryptonote_basic/cryptonote_basic.h"
#include "cryptonote_basic/cryptonote_format_utils.h"
#include "cryptonote_basic/account.h"
#include "string_tools.h"
#include "crypto/keccak.h"
#include "crypto/crypto-ops.h"

#define LEGACY_SEED_BYTES 16
#define LEGACY_SEED_WORDS 12

namespace ots {

    const WipeableString LegacySeed::phrase(const SeedLanguage& language, const std::string& password) const {
        if(!password.empty())
            throw ots::exception::seed::PasswordNotSupported();
        epee::wipeable_string phrase;
        m_seed->unlockInsecureOnce(false);
        if(!crypto::ElectrumWords::bytes_to_words(*m_seed, (size_t)LEGACY_SEED_BYTES, phrase, language.englishName())) {
            throw ots::exception::seed::SeedDecodingFailed();
        }
        return WipeableString(phrase.data(), phrase.size());
    }

    const SeedIndices LegacySeed::indices(const std::string& password) const {
        if(!password.empty())
            throw ots::exception::seed::PasswordNotSupported();
        SeedIndices indices;
        m_seed->unlockInsecureOnce(false);
        return ots::seedIndices(*m_seed, (size_t)LEGACY_SEED_BYTES);
    }

    LegacySeed LegacySeed::decode(
            const std::string& phrase,
            uint64_t height,
            uint64_t time,
            Network network
            ) {
        std::string unused_lang;
        const epee::wipeable_string phrase_wipeable(phrase);
        epee::wipeable_string out;
        if(!crypto::ElectrumWords::get_is_old_style_seed(phrase_wipeable))
            throw ots::exception::legacyseed::InvalidSeedFormat();
        bool ok = crypto::ElectrumWords::words_to_bytes(phrase_wipeable, out, LEGACY_SEED_BYTES, false, unused_lang);
        if(!ok || out.size() != LEGACY_SEED_BYTES)
            throw ots::exception::seed::SeedDecodingFailed();
        LegacySeed seed;
        const char* data = out.data();
        seed.m_seed->set((uint8_t*)data, (size_t)LEGACY_SEED_BYTES);
        seed.m_height = height;
        seed.m_timestamp = time;
        seed.m_network = network;
        crypto::secret_key recovery_key;
        crypto::hash h;
        keccak((uint8_t *)out.data(), out.size(), (uint8_t *)h.data, sizeof(h));
        memcpy(recovery_key.data, h.data, sizeof(recovery_key));
        sc_reduce32((uint8_t *)&recovery_key.data);

        crypto::secret_key o;
        o = *(const crypto::secret_key*)out.data();
        crypto::secret_key secret_view_key;
        keccak((uint8_t *)h.data, sizeof(h), (uint8_t *)h.data, sizeof(h));
        memcpy(secret_view_key.data, h.data, sizeof(recovery_key));
        sc_reduce32((uint8_t *)&secret_view_key.data);

        crypto::public_key public_spend_key;
        crypto::public_key public_view_key;
        crypto::secret_key_to_public_key(recovery_key, public_spend_key);
        crypto::secret_key_to_public_key(secret_view_key, public_view_key);
        cryptonote::account_public_address addr = {public_spend_key, public_view_key};
        cryptonote::account_base account;
        account.create_from_keys(addr, recovery_key, secret_view_key);
        seed.m_key->set(recovery_key);
        seed.m_address = std::make_unique<Address>(account.get_public_address_str(cryptonoteNetwork(network)));
        seed.m_wallet = std::make_shared<Wallet>(Account(account, network), *seed.m_key, seed.m_height, seed.m_network);
        return std::move(seed);
    }

    /**
     * @todo TODO: should use a protected constructor LegacySeed(const char* data, uint64_t height, uint64_t time, Network network) to avoid code duplication
     */
    LegacySeed LegacySeed::decode(
            const SeedIndices& indices,
            uint64_t height,
            uint64_t time,
            Network network
            ) {
        if(indices.size() != LEGACY_SEED_WORDS)
            throw ots::exception::seed::SeedDecodingFailed();
        auto out = ots::seedBytes<LEGACY_SEED_BYTES>(indices); // @see ots-internal.hpp
        LegacySeed seed;
        const unsigned char* data = out.data();
        seed.m_seed->set((uint8_t*)data, (size_t)LEGACY_SEED_BYTES);
        seed.m_height = height;
        seed.m_timestamp = time;
        seed.m_network = network;
        crypto::secret_key recovery_key;
        crypto::hash h;
        keccak((uint8_t *)out.data(), out.size(), (uint8_t *)h.data, sizeof(h));
        memcpy(recovery_key.data, h.data, sizeof(recovery_key));
        sc_reduce32((uint8_t *)&recovery_key.data);
        crypto::secret_key o;
        o = *(const crypto::secret_key*)out.data();
        crypto::secret_key secret_view_key;
        keccak((uint8_t *)h.data, sizeof(h), (uint8_t *)h.data, sizeof(h));
        memcpy(secret_view_key.data, h.data, sizeof(recovery_key));
        sc_reduce32((uint8_t *)&secret_view_key.data);
        crypto::public_key public_spend_key;
        crypto::public_key public_view_key;
        crypto::secret_key_to_public_key(recovery_key, public_spend_key);
        crypto::secret_key_to_public_key(secret_view_key, public_view_key);
        cryptonote::account_public_address addr = {public_spend_key, public_view_key};
        cryptonote::account_base account;
        account.create_from_keys(addr, recovery_key, secret_view_key);
        seed.m_key->set(recovery_key);
        seed.m_address = std::make_unique<Address>(account.get_public_address_str(cryptonoteNetwork(network)));
        seed.m_wallet = std::make_shared<Wallet>(Account(account, network), *seed.m_key, seed.m_height, seed.m_network);
        return std::move(seed);
    }

    LegacySeed LegacySeed::decode(
            const std::vector<uint16_t>& values,
            uint64_t height,
            uint64_t time,
            Network network
            ) {
        return decode(SeedIndices(values), height, time, network);
    }

    LegacySeed::LegacySeed(): Seed(), m_seed(new KeyStore(), KeyStoreDeleter()) {}

    /**
     * @note Tried to convert legacy seed to Monero seed, but I don't see any way because of different
     *       key generation methods. Both generate the secret view key from the secret spend key, but
     *       in different ways.
     */
}
