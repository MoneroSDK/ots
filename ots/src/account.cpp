#include "account.hpp"
#include <string_tools.h> // for epee::string_tools::pod_to_hex
#include <common/base58.h> // For tools::base58::decode
#include <cryptonote_basic/cryptonote_basic.h> // For cryptonote::address_parse_info
#include <cryptonote_basic/tx_extra.h> // For cryptonote::tx_extra_pub_key
#include "ringct/rctOps.h"
#include <crypto/chacha.h> // For crypto::generate_chacha_key
#include <wipeable_string.h>
#include <tuple>

namespace ots {

    Account::Account(
        const std::array<unsigned char, 32>& key,
        const Network network,
        const uint64_t kdfRounds
    ): m_network(network), mKdfRounds(kdfRounds) {
        m_account = cryptonote::account_base();
        crypto::secret_key sk;
        std::copy_n(key.data(), key.size(), sk.data);
        crypto::secret_key secretSpendKey = m_account.generate(sk, true, false);
    }

    Account::Account(
        const crypto::secret_key& key,
        const Network network,
        const uint64_t kdfRounds
    ): m_network(network), mKdfRounds(kdfRounds) {
        m_account = cryptonote::account_base();
        crypto::secret_key secretSpendKey = m_account.generate(key, true, false);
    }

    Account::Account(
        const KeyStore& key,
        const Network network,
        const uint64_t kdfRounds
    ): m_network(network), mKdfRounds(kdfRounds) {
        m_account = cryptonote::account_base();
        key.unlockInsecureOnce();
        crypto::secret_key secretSpendKey = m_account.generate(static_cast<const crypto::secret_key&>(key), true, false);
    }

    Account::Account(
        const cryptonote::account_base& account,
        const Network network,
        const uint64_t kdfRounds
    ): m_account(account), m_network(network), mKdfRounds(kdfRounds) {}

    Account::Account(const Account& account):
        m_account(account.m_account),
        m_network(account.m_network),
        mKdfRounds(account.mKdfRounds) {}

    Address Account::address() const noexcept {
        return Address(m_account.get_public_address_str(cryptonoteNetwork(m_network)));
    }

    Address Account::address(uint32_t account, uint32_t index) const noexcept {
        if(!addressIndexInCache(account, index)) {
            cryptonote::subaddress_index subaddress_index = {account, index};
            cryptonote::account_public_address addr = m_account.get_device().get_subaddress(m_account.get_keys(), subaddress_index);
            Address address = Address(cryptonote::get_account_address_as_str(cryptonoteNetwork(m_network), account != 0 || index != 0, addr));
            cacheAddress(address, account, index);
            return address;
        }
        return cachedAddress(account, index);
    }

    Address Account::address(const cryptonote::subaddress_index index) const noexcept {
        if(!addressIndexInCache(index.major, index.minor)) {
            cryptonote::account_public_address addr = m_account.get_device().get_subaddress(m_account.get_keys(), index);
            Address address = Address(cryptonote::get_account_address_as_str(cryptonoteNetwork(m_network), index.major != 0 || index.minor != 0, addr));
            cacheAddress(address, index.major, index.minor);
            return address;
        }
        return cachedAddress(index);
    }

    void Account::cacheAddress(const cryptonote::subaddress_index index) const noexcept {
        if(addressIndexInCache(index))
            return;
        cacheAddress(address(index), index);
    }

    void Account::cacheAddress(const Address& address, cryptonote::subaddress_index index) const noexcept {
        cacheAddress(address, index.major, index.minor);
    }

    void Account::cacheAddress(const Address& address, uint32_t account, uint32_t index) const noexcept {
        std::pair<uint32_t, uint32_t> pair = std::make_pair(account, index);
        if(addressInCache(address) || addressIndexInCache(pair))
            return;
        m_addressIndexCache.insert(pair);
        m_addressCache[address] = pair;
        m_indexToAddressCache[pair] = address;
        // Add to subaddresses for importOutputs
        cryptonote::subaddress_index subaddress_index = {account, index};
        crypto::public_key pub = m_account.get_device().get_subaddress_spend_public_key(m_account.get_keys(), subaddress_index);
        m_subaddresses[pub] = subaddress_index;
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

    bool Account::addressIndexInCache(const cryptonote::subaddress_index& index) const noexcept {
        return addressIndexInCache(index.major, index.minor);
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

    std::pair<uint32_t, uint32_t> Account::cachedAddressIndex(const Address& address) const {
        if(!addressInCache(address))
            throw ots::exception::wallet::AddressNotFound();
        return m_addressCache[address];
    }

    Address Account::cachedAddress(const uint32_t account, const uint32_t index) const {
        if(!addressIndexInCache(account, index))
            throw ots::exception::wallet::AddressNotFound();
        return Address(m_indexToAddressCache[std::make_pair(account, index)]);
    }

    Address Account::cachedAddress(const cryptonote::subaddress_index index) const {
        return cachedAddress(index.major, index.minor);
    }

    std::pair<uint32_t, uint32_t> Account::addressIndex(const Address& address, uint32_t maxAccountDepth, uint32_t maxIndexDepth) const {
        if(address == this->address())
            return std::make_pair(0, 0);
        if(addressInCache(address))
            return cachedAddressIndex(address);
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

    void Account::setupTd(const exported_transfer_details& etd, transfer_details& td) {
        td.m_block_height = 0;
        td.m_txid = crypto::null_hash;
        td.m_global_output_index = etd.m_global_output_index;
        td.m_spent = etd.m_flags.m_spent;
        td.m_frozen = etd.m_flags.m_frozen;
        td.m_spent_height = 0;
        td.m_mask = rct::identity();
        td.m_amount = etd.m_amount;
        td.m_rct = etd.m_flags.m_rct;
        td.m_key_image_known = etd.m_flags.m_key_image_known;
        td.m_key_image_request = etd.m_flags.m_key_image_request;
        td.m_key_image_partial = false;
        td.m_subaddr_index.major = etd.m_subaddr_index_major;
        td.m_subaddr_index.minor = etd.m_subaddr_index_minor;
    }

    size_t Account::importOutputs(const std::tuple<uint64_t, uint64_t, std::vector<exported_transfer_details>> &outputs) {
        // we can now import piecemeal
        const size_t offset = std::get<0>(outputs);
        const size_t num_outputs = std::get<1>(outputs);
        const std::vector<exported_transfer_details> &output_array = std::get<2>(outputs);
        if(offset > m_transfers.size())
            throw ots::exception::wallet::InternalError("Imported outputs omit more outputs that we know of. Try using export_outputs all.");
        if(offset + output_array.size() > num_outputs)
            throw ots::exception::wallet::InternalError("Imported outputs exceed total outputs");
        const size_t original_size = m_transfers.size();
        if(offset + output_array.size() > m_transfers.size())
            m_transfers.resize(offset + output_array.size());
        else if(num_outputs < m_transfers.size())
            m_transfers.resize(num_outputs);
        for(size_t i = 0; i < output_array.size(); ++i) {
            exported_transfer_details etd = output_array[i];
            transfer_details &td = m_transfers[i + offset];
            // setup td with "cheap" loaded data
            setupTd(etd, td);
            // skip those we've already imported, or which have different data
            if(
                i + offset < original_size &&
                td.m_key_image_known &&
                etd.m_internal_output_index == td.m_internal_output_index &&
                etd.m_pubkey == td.get_public_key()
            )
                continue;
            // construct a synthetix tx prefix that has the info we'll need: the output with its pubkey, the tx pubkey in extra
            td.m_tx = {};
            if(etd.m_internal_output_index >= 65536)
                throw ots::exception::wallet::InternalError("Internal output index seems outrageously high, rejecting");
            td.m_internal_output_index = etd.m_internal_output_index;
            cryptonote::txout_to_key tk;
            tk.key = etd.m_pubkey;
            cryptonote::tx_out out;
            out.amount = etd.m_amount;
            out.target = tk;
            td.m_tx.vout.resize(etd.m_internal_output_index);
            td.m_tx.vout.push_back(out);
            td.m_pk_index = 0;
            add_tx_pub_key_to_extra(td.m_tx, etd.m_tx_pubkey);
            if(!etd.m_additional_tx_keys.empty())
                cryptonote::add_additional_tx_pub_keys_to_extra(td.m_tx.extra, etd.m_additional_tx_keys);
            // the hot wallet wouldn't have known about key images (except if we already exported them)
            cryptonote::keypair in_ephemeral;
            const crypto::public_key &tx_pub_key = etd.m_tx_pubkey;
            const std::vector<crypto::public_key> &additional_tx_pub_keys = etd.m_additional_tx_keys;
            const crypto::public_key& out_key = etd.m_pubkey;
            cacheAddress(td.m_subaddr_index);
            bool r = cryptonote::generate_key_image_helper(
                    m_account.get_keys(),
                    m_subaddresses,
                    out_key,
                    tx_pub_key,
                    additional_tx_pub_keys,
                    td.m_internal_output_index,
                    in_ephemeral,
                    td.m_key_image,
                    m_account.get_device()
            );
            if(!r)
                throw ots::exception::wallet::InternalError("Failed to generate key image");
            td.m_key_image_known = true;
            td.m_key_image_request = true;
            td.m_key_image_partial = false;
            if(in_ephemeral.pub != out_key)
                throw ots::exception::wallet::InternalError("key_image generated ephemeral public key not matched with output_key at index " + boost::lexical_cast<std::string>(i + offset));
            m_key_images[td.m_key_image] = i + offset;
            m_pub_keys[td.get_public_key()] = i + offset;
        }
        return m_transfers.size();
    }

    size_t Account::importOutputs(const std::string& outputs) {
        std::string data = outputs;
        const size_t magiclen = strlen(OUTPUT_EXPORT_FILE_MAGIC);
        if(data.size() < magiclen || memcmp(data.data(), OUTPUT_EXPORT_FILE_MAGIC, magiclen))
            throw ots::exception::wallet::ImportOutputs("Bad magic in data");
        try {
            data = decryptWithViewSecretKey(std::string(data, magiclen));
        } catch (const std::exception &e) {
            throw ots::exception::wallet::ImportOutputs(e.what());
        }
        const size_t headerlen = 2 * sizeof(crypto::public_key);
        if(data.size() < headerlen)
            throw ots::exception::wallet::ImportOutputs("Bad data size for outputs");
        const crypto::public_key &public_spend_key = *(const crypto::public_key*)&data[0];
        const crypto::public_key &public_view_key = *(const crypto::public_key*)&data[sizeof(crypto::public_key)];
        const cryptonote::account_public_address &keys = m_account.get_keys().m_account_address;
        if(public_spend_key != keys.m_spend_public_key || public_view_key != keys.m_view_public_key)
            throw ots::exception::wallet::ImportOutputs("Outputs from are for a different account");
        size_t imported_outputs = 0;
        bool loaded = false;
        try {
            std::string body(data, headerlen);
            std::tuple<uint64_t, uint64_t, std::vector<exported_transfer_details>> new_outputs;
            try {
                binary_archive<false> ar{epee::strspan<std::uint8_t>(body)};
                loaded = ::serialization::serialize(ar, new_outputs) && ::serialization::check_stream_state(ar);
            } catch (...) {}
            if(!loaded)
                std::get<2>(new_outputs).clear();
            std::tuple<uint64_t, uint64_t, std::vector<transfer_details>> outputs;
            if(!loaded)
                try {
                    binary_archive<false> ar{epee::strspan<std::uint8_t>(body)};
                    if(::serialization::serialize(ar, outputs))
                        if(::serialization::check_stream_state(ar))
                            loaded = true;
                }
                catch (...) {}
                // Thor removed fallback to boost serialization (dependencies for nothing)
            if(!loaded) {
                std::get<0>(outputs) = 0;
                std::get<1>(outputs) = 0;
                std::get<2>(outputs) = {};
            }
            imported_outputs = !std::get<2>(new_outputs).empty() ? importOutputs(new_outputs) : !std::get<2>(outputs).empty() ? importOutputs(outputs) : 0;
        } catch (const std::exception &e) {
            throw ots::exception::wallet::ImportOutputs(e.what());
        }
        return imported_outputs;
    }

    size_t Account::importOutputs(const std::tuple<uint64_t, uint64_t, std::vector<transfer_details>> &outputs) {
        // we can now import piecemeal
        const size_t offset = std::get<0>(outputs);
        const size_t num_outputs = std::get<1>(outputs);
        const std::vector<transfer_details> &output_array = std::get<2>(outputs);
        if(offset > m_transfers.size())
            throw ots::exception::wallet::InternalError("Imported outputs omit more outputs that we know of");
        if(offset + output_array.size() > num_outputs)
            throw ots::exception::wallet::InternalError("Offset is larger than total outputs");
        const size_t original_size = m_transfers.size();
        if(offset + output_array.size() > m_transfers.size())
            m_transfers.resize(offset + output_array.size());
        else if(num_outputs < m_transfers.size())
            m_transfers.resize(num_outputs);
        for(size_t i = 0; i < output_array.size(); ++i) {
            std::cout << "Account::importOutputs(transferdetails): Importing output " << i << std::endl;
            transfer_details td = output_array[i];
            if(i + offset < original_size) { // skip those we've already imported, or which have different data
                // compare the data used to create the key image below
                const transfer_details &org_td = m_transfers[i + offset];
                if(!org_td.m_key_image_known)
                    goto process;
#define CMPF(f) if(!(td.f == org_td.f)) goto process
                CMPF(m_txid);
                CMPF(m_key_image);
                CMPF(m_internal_output_index);
#undef CMPF
                if(!(get_transaction_prefix_hash(td.m_tx) == get_transaction_prefix_hash(org_td.m_tx)))
                    goto process;
                // copy anyway, since the comparison does not include ancillary fields which may have changed
                m_transfers[i + offset] = std::move(td);
                continue;
            }
process:
            // the hot wallet wouldn't have known about key images (except if we already exported them)
            cryptonote::keypair in_ephemeral;
            if(td.m_tx.vout.empty())
                throw ots::exception::wallet::InternalError("tx with no outputs at index " + boost::lexical_cast<std::string>(i + offset));
            crypto::public_key tx_pub_key = get_tx_pub_key_from_received_outs(td);
            const std::vector<crypto::public_key> additional_tx_pub_keys = get_additional_tx_pub_keys_from_extra(td.m_tx);
            if(td.m_internal_output_index >= td.m_tx.vout.size())
                throw ots::exception::wallet::InternalError("Internal index is out of range");
            crypto::public_key out_key = td.get_public_key();
            cacheAddress(td.m_subaddr_index); // make sure the subaddress is reserved
            bool r = cryptonote::generate_key_image_helper(m_account.get_keys(), m_subaddresses, out_key, tx_pub_key, additional_tx_pub_keys, td.m_internal_output_index, in_ephemeral, td.m_key_image, m_account.get_device());
            if(!r)
                throw ots::exception::wallet::InternalError("Failed to generate key image");
            td.m_key_image_known = true;
            td.m_key_image_request = true;
            td.m_key_image_partial = false;
            if(in_ephemeral.pub != out_key)
                throw ots::exception::wallet::InternalError("key_image generated ephemeral public key not matched with output_key at index " + boost::lexical_cast<std::string>(i + offset));
            m_key_images[td.m_key_image] = i + offset;
            m_pub_keys[td.get_public_key()] = i + offset;
            m_transfers[i + offset] = std::move(td);
        }
        return m_transfers.size();
    }

    std::string Account::exportKeyImages() const {
        std::pair<uint64_t, std::vector<std::pair<crypto::key_image, crypto::signature>>> ski = exportKeyImages(true); // TODO: see if we want to trigger that automatically to false after transfers, or if we want to keep it true. I prefer personally always true, but the tradeoff is that e.g. the UR codes are bigger, so it takes longer to scan them. But how there is no state exchange other then outputs and keyimages, how to know if the user suddenly tries another view only wallet, and then the key images are not there? This is my reasoning for always true.
        const cryptonote::account_public_address &keys = m_account.get_keys().m_account_address;
        const uint32_t offset = ski.first;
        std::string data;
        data.reserve(4 + ski.second.size() * (sizeof(crypto::key_image) + sizeof(crypto::signature)) + 2 * sizeof(crypto::public_key));
        data.resize(4);
        data[0] = offset & 0xff;
        data[1] = (offset >> 8) & 0xff;
        data[2] = (offset >> 16) & 0xff;
        data[3] = (offset >> 24) & 0xff;
        data += std::string((const char *)&keys.m_spend_public_key, sizeof(crypto::public_key));
        data += std::string((const char *)&keys.m_view_public_key, sizeof(crypto::public_key));
        for (const auto &i: ski.second) {
            data += std::string((const char *)&i.first, sizeof(crypto::key_image));
            data += std::string((const char *)&i.second, sizeof(crypto::signature));
        }
        return std::string(KEY_IMAGE_EXPORT_FILE_MAGIC) + encryptWithViewSecretKey(data);
    }

    std::pair<uint64_t, std::vector<std::pair<crypto::key_image, crypto::signature>>> Account::exportKeyImages(bool all) const {
        std::vector<std::pair<crypto::key_image, crypto::signature>> ski;
        size_t offset = 0;
        if(!all) // TODO: in case all usecases are all == true, we can remove this if statement
            while(offset < m_transfers.size() && !m_transfers[offset].m_key_image_request)
                ++offset;
        ski.reserve(m_transfers.size() - offset);
        for(size_t n = offset; n < m_transfers.size(); ++n) {
            const transfer_details &td = m_transfers[n];
            // get ephemeral public key
            const crypto::public_key pkey = td.get_public_key();
            // get tx pub key
            std::vector<cryptonote::tx_extra_field> tx_extra_fields;
            parse_tx_extra(td.m_tx.extra, tx_extra_fields); // comment from monero source: Extra may only be partially parsed, it's OK if tx_extra_fields contains public key THOR: removed empty if statement, if we don't care about the result, no need to check it. This comment is only for code review. TODO: remove this comment later, and check if parse_tx_extra is even still needed.
            crypto::public_key tx_pub_key = get_tx_pub_key_from_received_outs(td);
            const std::vector<crypto::public_key> additional_tx_pub_keys = get_additional_tx_pub_keys_from_extra(td.m_tx);
            // generate ephemeral secret key
            crypto::key_image ki;
            cryptonote::keypair in_ephemeral;
            if(!cryptonote::generate_key_image_helper(
                    m_account.get_keys(),
                    m_subaddresses,
                    pkey,
                    tx_pub_key,
                    additional_tx_pub_keys,
                    td.m_internal_output_index,
                    in_ephemeral,
                    ki,
                    m_account.get_device()
                ))
                throw ots::exception::wallet::InternalError("Failed to generate key image");
            if(td.m_key_image_known && !td.m_key_image_partial && ki != td.m_key_image)
                throw ots::exception::wallet::InternalError("key_image generated not matched with cached key image");
            if(in_ephemeral.pub != pkey)
                throw ots::exception::wallet::InternalError("key_image generated ephemeral public key not matched with output_key");
            // sign the key image with the output secret key
            crypto::signature signature;
            std::vector<const crypto::public_key*> key_ptrs;
            key_ptrs.push_back(&pkey);
            crypto::generate_ring_signature(
                (const crypto::hash&)td.m_key_image,
                td.m_key_image,
                key_ptrs,
                in_ephemeral.sec,
                0,
                &signature
            );
            ski.push_back(std::make_pair(td.m_key_image, signature));
        }
        return std::make_pair(offset, ski);
    }

    std::string Account::encrypt(const char *plaintext, size_t len, const crypto::secret_key &skey, bool authenticated) const {
        crypto::chacha_key key;
        crypto::generate_chacha_key(&skey, sizeof(skey), key, mKdfRounds);
        std::string ciphertext;
        crypto::chacha_iv iv = crypto::rand<crypto::chacha_iv>();
        ciphertext.resize(len + sizeof(iv) + (authenticated ? sizeof(crypto::signature) : 0));
        crypto::chacha20(plaintext, len, key, iv, &ciphertext[sizeof(iv)]);
        memcpy(&ciphertext[0], &iv, sizeof(iv));
        if (authenticated) {
            crypto::hash hash;
            crypto::cn_fast_hash(ciphertext.data(), ciphertext.size() - sizeof(crypto::signature), hash);
            crypto::public_key pkey;
            crypto::secret_key_to_public_key(skey, pkey);
            crypto::signature &signature = *(crypto::signature*)&ciphertext[ciphertext.size() - sizeof(crypto::signature)];
            crypto::generate_signature(hash, pkey, skey, signature);
        }
        return ciphertext;
    }

    std::string Account::encryptWithViewSecretKey(const std::string &plaintext) const {
        return encrypt(plaintext.c_str(), plaintext.size(), m_account.get_keys().m_view_secret_key, true);
    }

    crypto::public_key Account::get_tx_pub_key_from_received_outs(const transfer_details &td) const {
        std::vector<cryptonote::tx_extra_field> tx_extra_fields;
        if(!parse_tx_extra(td.m_tx.extra, tx_extra_fields)) // THOR: WTF is that construct?????
        {
            // Extra may only be partially parsed, it's OK if tx_extra_fields contains public key
        }
        // Due to a previous bug, there might be more than one tx pubkey in extra, one being
        // the result of a previously discarded signature.
        // For speed, since scanning for outputs is a slow process, we check whether extra
        // contains more than one pubkey. If not, the first one is returned. If yes, they're
        // checked for whether they yield at least one output
        cryptonote::tx_extra_pub_key pub_key_field;
        if(!find_tx_extra_field_by_type(tx_extra_fields, pub_key_field, 0))
            throw ots::exception::wallet::InternalError("Public key wasn't found in the transaction extra");
        const crypto::public_key tx_pub_key = pub_key_field.pub_key;
        bool two_found = find_tx_extra_field_by_type(tx_extra_fields, pub_key_field, 1);
        if(!two_found)
            return tx_pub_key; // easy case, just one found
        // more than one, loop and search
        const cryptonote::account_keys& keys = m_account.get_keys();
        size_t pk_index = 0;
        hw::device &hwdev = m_account.get_device();
        while(find_tx_extra_field_by_type(tx_extra_fields, pub_key_field, pk_index++)) {
            const crypto::public_key tx_pub_key_extra = pub_key_field.pub_key;
            crypto::key_derivation derivation;
            bool r = hwdev.generate_key_derivation(tx_pub_key_extra, keys.m_view_secret_key, derivation);
            if(!r)
                throw ots::exception::wallet::InternalError("Failed to generate key derivation");
            for(size_t i = 0; i < td.m_tx.vout.size(); ++i) {
                tx_scan_info_t tx_scan_info;
                check_acc_out_precomp(td.m_tx.vout[i], derivation, {}, i, tx_scan_info);
                if(!tx_scan_info.error && tx_scan_info.received)
                    return tx_pub_key_extra;
            }
        }
        // we found no key yielding an output, but it might be in the additional
        // tx pub keys only, which we do not need to check, so return the first one
        return tx_pub_key;
    }

    void Account::check_acc_out_precomp(
            const cryptonote::tx_out &o,
            const crypto::key_derivation &derivation,
            const std::vector<crypto::key_derivation> &additional_derivations,
            size_t i,
            tx_scan_info_t &tx_scan_info
            ) const {
        hw::device &hwdev = m_account.get_device();
        boost::unique_lock<hw::device> hwdev_lock (hwdev);
        hwdev.set_mode(hw::device::TRANSACTION_PARSE);
        crypto::public_key output_public_key;
        if(!get_output_public_key(o, output_public_key)) {
            tx_scan_info.error = true;
            LOG_ERROR("wrong type id in transaction out");
            return;
        }
        tx_scan_info.received = is_out_to_acc_precomp(m_subaddresses, output_public_key, derivation, additional_derivations, i, hwdev, get_output_view_tag(o));
        tx_scan_info.money_transfered = tx_scan_info.received ? o.amount : 0; // o.amount may be 0 for ringct outputs
        tx_scan_info.error = false;
    }

    void Account::check_acc_out_precomp(
            const cryptonote::tx_out &o,
            const crypto::key_derivation &derivation,
            const std::vector<crypto::key_derivation> &additional_derivations,
            size_t i,
            const is_out_data *is_out_data,
            tx_scan_info_t &tx_scan_info
            ) const {
        if (!is_out_data || i >= is_out_data->received.size())
            return check_acc_out_precomp(o, derivation, additional_derivations, i, tx_scan_info);
        tx_scan_info.received = is_out_data->received[i];
        tx_scan_info.money_transfered = tx_scan_info.received ? o.amount : 0; // o.amount may be 0 for ringct outputs
        tx_scan_info.error = false;
    }

    std::string Account::decrypt(const std::string &ciphertext, const crypto::secret_key &skey, bool authenticated) const {
        const size_t prefix_size = sizeof(crypto::chacha_iv) + (authenticated ? sizeof(crypto::signature) : 0);
        if(ciphertext.size() < prefix_size)
            throw ots::exception::wallet::InvalidCiphertext("Ciphertext too short");
        crypto::chacha_key key;
        crypto::generate_chacha_key(&skey, sizeof(skey), key, mKdfRounds);
        const crypto::chacha_iv &iv = *(const crypto::chacha_iv*)&ciphertext[0];
        if(authenticated) {
            crypto::hash hash;
            crypto::cn_fast_hash(ciphertext.data(), ciphertext.size() - sizeof(crypto::signature), hash);
            crypto::public_key pkey;
            crypto::secret_key_to_public_key(skey, pkey);
            const crypto::signature &signature = *(const crypto::signature*)&ciphertext[ciphertext.size() - sizeof(crypto::signature)];
            if(!crypto::check_signature(hash, pkey, signature))
                throw ots::exception::wallet::CiphertextAuthenticationFailed();
        }
        std::unique_ptr<char[]> buffer{new char[ciphertext.size() - prefix_size]};
        auto wiper = epee::misc_utils::create_scope_leave_handler([&]() { memwipe(buffer.get(), ciphertext.size() - prefix_size); });
        crypto::chacha20(ciphertext.data() + sizeof(iv), ciphertext.size() - prefix_size, key, iv, buffer.get());
        return std::string(buffer.get(), ciphertext.size() - prefix_size);
    }

    std::string Account::decryptWithViewSecretKey(const std::string& ciphertext, bool authenticated) const {
        return decrypt(ciphertext, m_account.get_keys().m_view_secret_key, authenticated);
    }

    void Account::authenticateWithViewPublicKey(const std::string& data, const crypto::signature& signature) const {
        crypto::hash hash;
        crypto::cn_fast_hash(data.data(), data.size(), hash);
        if(!crypto::check_signature(hash, m_account.get_keys().m_account_address.m_view_public_key, signature))
            throw ots::exception::sign::InvalidSignature();
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
        if(sizeof(s) != decoded.size())
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
        if(sizeof(s) != decoded.size())
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
        m_indexToAddressCache.clear();
    }

    void AccountDeleter::operator()(Account* account) const {
        delete account;
    }
}
