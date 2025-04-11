#include "account.hpp"
#include <string_tools.h> // for epee::string_tools::pod_to_hex
#include <common/base58.h> // For tools::base58::decode
#include <cryptonote_basic/cryptonote_basic.h> // For cryptonote::address_parse_info
#include <cryptonote_basic/tx_extra.h> // For cryptonote::tx_extra_pub_key
#include "ringct/rctOps.h"
#include <crypto/chacha.h> // For crypto::generate_chacha_key
#include <wipeable_string.h>
#include <tuple>
#include <optional>
#include <functional> // for std::reference_wrapper

#include "ringct/rctSigs.h"
#include "string_tools.h"
#include "common/apply_permutation.h"

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
        if(addressInCache(address))
            return;
        std::pair<uint32_t, uint32_t> pair = {account, index};
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
        return addressIndexInCache(std::pair(account, index));
    }

    bool Account::addressIndexInCache(const cryptonote::subaddress_index& index) const noexcept {
        return addressIndexInCache(std::pair(index.major, index.minor));
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
        return Address(m_indexToAddressCache[std::pair(account, index)]);
    }

    Address Account::cachedAddress(const cryptonote::subaddress_index index) const {
        return cachedAddress(index.major, index.minor);
    }

    std::pair<uint32_t, uint32_t> Account::addressIndex(const Address& address, uint32_t maxAccountDepth, uint32_t maxIndexDepth) const {
        if(address == this->address())
            return {0, 0};
        if(addressInCache(address))
            return cachedAddressIndex(address);
        for(uint32_t acc = 0; acc < maxAccountDepth; acc++) {
            for(uint32_t idx = 0; idx < maxIndexDepth; idx++) {
                if(addressIndexInCache(acc, idx))
                    continue;
                Address addr = this->address(acc, idx);
                cacheAddress(addr, acc, idx);
                if(addr == address)
                    return std::pair(acc, idx);
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

    size_t Account::importOutputs(const std::string& outputs, bool withMagic) {
        //std::string data = outputs;
        // const size_t magiclen = strlen(OUTPUT_EXPORT_FILE_MAGIC);
        // if(data.size() < magiclen || memcmp(data.data(), OUTPUT_EXPORT_FILE_MAGIC, magiclen))
        if(withMagic && isBadMagic(outputs, OUTPUT_EXPORT_FILE_MAGIC))
            throw ots::exception::wallet::ImportOutputs("Bad magic in data");
        std::string data;
        try {
            //data = decryptWithViewSecretKey(std::string(data, magiclen));
            data = decryptWithViewSecretKey(withMagic?std::string(outputs, strlen(OUTPUT_EXPORT_FILE_MAGIC)):outputs);
        } catch(const std::exception &e) {
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
        std::string body(data, headerlen);
        try { // assume this is the current format
            std::tuple<uint64_t, uint64_t, std::vector<exported_transfer_details>> new_outputs;
            binary_archive<false> ar{epee::strspan<std::uint8_t>(body)};
            if(::serialization::serialize(ar, new_outputs) && ::serialization::check_stream_state(ar))
                return importOutputs(new_outputs);
        } catch(...) {}
        try { // assume this is a deprecated format, TODO: remove as soon we are sure it's not needed anymore
            std::tuple<uint64_t, uint64_t, std::vector<transfer_details>> outputs;
            binary_archive<false> ar{epee::strspan<std::uint8_t>(body)};
            if(::serialization::serialize(ar, outputs) && ::serialization::check_stream_state(ar))
                return importOutputs(outputs);
        } catch(...) {}
        // Thor removed fallback to boost serialization (dependencies for nothing) from the original (monero) source
        throw ots::exception::wallet::ImportOutputs("Failed to import outputs");
    }

    size_t Account::importOutputs(const std::tuple<uint64_t, uint64_t, std::vector<exported_transfer_details>> &outputs) {
        // TODO: dislike the silent uint64_t to size_t conversion
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

    size_t Account::importOutputs(const std::tuple<uint64_t, uint64_t, std::vector<transfer_details>> &outputs) {
        // TODO: seems to be a deprecated format, remove as soon as we are sure it's not needed anymore
        // TODO: dislike the silent uint64_t to size_t conversion
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
            std::cout << "Account::importOutputs(transferdetails): Importing output " << i << std::endl; // TODO: debug only, remove
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
        data += std::string(reinterpret_cast<const char *>(&keys.m_spend_public_key), sizeof(crypto::public_key));
        data += std::string(reinterpret_cast<const char *>(&keys.m_view_public_key), sizeof(crypto::public_key));
        for(const auto &i: ski.second) {
            data += std::string(reinterpret_cast<const char *>(&i.first), sizeof(crypto::key_image));
            data += std::string(reinterpret_cast<const char *>(&i.second), sizeof(crypto::signature));
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
        for(const transfer_details &td: m_transfers) {
            // get ephemeral public key
            const crypto::public_key pkey = td.get_public_key();
            // get tx pub key
            std::vector<cryptonote::tx_extra_field> tx_extra_fields;
            parse_tx_extra(td.m_tx.extra, tx_extra_fields); // comment from monero source: Extra may only be partially parsed, it's OK if tx_extra_fields contains public key. THOR: removed empty if statement, if we don't care about the result, no need to check it. This comment is only for code review. TODO: remove this comment later, and check if parse_tx_extra is even still needed.
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
            key_ptrs.emplace_back(&pkey);
            crypto::generate_ring_signature(
                    (const crypto::hash&)td.m_key_image,
                    td.m_key_image,
                    key_ptrs,
                    in_ephemeral.sec,
                    0,
                    &signature
                    );
            ski.emplace_back(td.m_key_image, signature);
        }
        return std::pair(offset, ski);
    }

    TxDescription Account::describeTransaction(const std::string& unsignedTransaction, bool withMagic) const {
        TxDescription txDescription{unsignedTransaction};
        std::vector <tx_construction_data> tx_constructions = parseUnsignedTransaction(unsignedTransaction, withMagic).txes;
        std::unordered_map<cryptonote::account_public_address, FlowVector> allAddressFlows;
        try {
            std::optional<std::reference_wrapper<const tx_construction_data>> firstKnownNonZeroChange;
            for(const tx_construction_data &cd: tx_constructions) {
                TransferDescription transferDescription;
                std::unordered_map<cryptonote::account_public_address, FlowVector> addressFlows;
                std::vector<cryptonote::tx_extra_field> tx_extra_fields;
                crypto::hash8 payment_id = crypto::null_hash8;
                cryptonote::tx_extra_nonce extra_nonce;
                if(
                    cryptonote::parse_tx_extra(cd.extra, tx_extra_fields) // have tx extras
                    && find_tx_extra_field_by_type(tx_extra_fields, extra_nonce) // have nonce
                    && cryptonote::get_encrypted_payment_id_from_tx_extra_nonce(
                        extra_nonce.nonce, payment_id) // have payment id
                    && payment_id != crypto::null_hash8 // actually have a payment id
                  )
                    transferDescription.paymentId = epee::string_tools::pod_to_hex(payment_id);
                for(auto &src: cd.sources) {
                    transferDescription.amountIn += src.amount;
                    if(src.outputs.size() < transferDescription.ringSize)
                        transferDescription.ringSize = src.outputs.size();
                }
                for(const auto &entry: cd.splitted_dsts) {
                    std::string address = cryptonote::get_account_address_as_str(
                        cryptonoteNetwork(m_network), entry.is_subaddress, entry.addr
                    );
                    if(!transferDescription.paymentId.empty() && !entry.is_subaddress && address != entry.original)
                        address = cryptonote::get_account_integrated_address_as_str(
                                cryptonoteNetwork(m_network), entry.addr, payment_id);
                    auto tx_dest = addressFlows.find(entry.addr);
                    if(tx_dest == addressFlows.end()) // tx dest not existing yet, let's create it
                        addressFlows.emplace(entry.addr, FlowVector{Address(address), entry.amount});
                    else // tx dest already exists, let's add the amount to it
                        tx_dest->second.amount += entry.amount;
                    transferDescription.amountOut += entry.amount;
                }
                if(cd.change_dts.amount > 0) {
                    auto it = addressFlows.find(cd.change_dts.addr);
                    if(it == addressFlows.end())
                        throw ots::exception::tx::Change("Claimed change does not go to a paid address");
                    if(it->second.amount < cd.change_dts.amount)
                        throw ots::exception::tx::Change("Claimed change is larger than payment to the change address");
                    if(!firstKnownNonZeroChange.has_value()) // on first time, set to currenct construction data
                        firstKnownNonZeroChange = std::cref(cd);
                    if(
                        memcmp( // if the change address is different from the first one, we have a multi change addresses
                            &cd.change_dts.addr,
                            &firstKnownNonZeroChange.value().get().change_dts.addr,
                            sizeof(cd.change_dts.addr)
                        )
                    )
                        throw ots::exception::tx::Change("Change goes to more than one address");
                    if(!transferDescription.change.has_value())
                        transferDescription.change = FlowVector{
                            Address(
                                get_account_address_as_str(
                                    cryptonoteNetwork(m_network),
                                    tx_constructions.front().subaddr_account > 0,
                                    tx_constructions.front().change_dts.addr
                                ) // address of change of the first `cd`
                            ),
                            0 // amount will be added below
                        };
                    if(!txDescription.change.has_value())
                        txDescription.change = transferDescription.change;
                    transferDescription.change.value().amount += cd.change_dts.amount;
                    txDescription.change.value().amount += cd.change_dts.amount;
                    it->second.amount -= cd.change_dts.amount;
                    if(it->second.amount == 0)
                        addressFlows.erase(cd.change_dts.addr);
                }
                for(const auto &addressFlow: addressFlows) {
                    if(addressFlow.second.amount == 0) { // no amount sent to this address, so it's a dummy output
                        transferDescription.dummyOutputs++;
                        continue;
                    }
                    transferDescription.flows.emplace_back(addressFlow.second);
                    auto it_in_all = allAddressFlows.find(addressFlow.first);
                    if(it_in_all == allAddressFlows.end()) // addressFlow not existing yet, let's copy it
                        allAddressFlows.emplace(addressFlow.first, addressFlow.second);
                    else // addressFlow already exists, let's add the amount to it
                        it_in_all->second.amount += addressFlow.second.amount;
                }
                transferDescription.fee = transferDescription.amountIn - transferDescription.amountOut;
                transferDescription.unlockTime = cd.unlock_time;
                transferDescription.extra = epee::to_hex::string({cd.extra.data(), cd.extra.size()});
                txDescription.amountIn += transferDescription.amountIn;
                txDescription.amountOut += transferDescription.amountOut;
                txDescription.fee += transferDescription.fee;
                txDescription.transfers.emplace_back(transferDescription);
            }
            // Populate the summary recipients list
            for(auto i = allAddressFlows.begin(); i != allAddressFlows.end(); ++i) {
                txDescription.flows.emplace_back(i->second);
            }
        } catch(const std::exception &e) {
            throw ots::exception::tx::Parse("failed to parse unsigned transfers" + std::string(e.what()));
        }
        return txDescription;
    }

    tx_description Account::describeTransactionLegacy(const std::string& unsignedTransaction) const {
        tx_description res;
        std::vector <tx_construction_data> tx_constructions = parseUnsignedTransaction(unsignedTransaction).txes;
        try {
            std::unordered_map<cryptonote::account_public_address, std::pair<std::string, uint64_t>> all_dests;
            std::optional<std::reference_wrapper<const tx_construction_data>> first_known_non_zero_change;
            for(const tx_construction_data &cd : tx_constructions) {
                transfer_description desc;
                std::unordered_map<cryptonote::account_public_address, std::pair<std::string, uint64_t>> tx_dests;
                std::vector<cryptonote::tx_extra_field> tx_extra_fields;
                crypto::hash8 payment_id = crypto::null_hash8;
                cryptonote::tx_extra_nonce extra_nonce;
                if(
                    cryptonote::parse_tx_extra(cd.extra, tx_extra_fields) // have tx extras
                    && find_tx_extra_field_by_type(tx_extra_fields, extra_nonce) // have nonce
                    && cryptonote::get_encrypted_payment_id_from_tx_extra_nonce(
                        extra_nonce.nonce, payment_id) // have payment id
                    && payment_id != crypto::null_hash8 // actually have a payment id
                  )
                    desc.payment_id = epee::string_tools::pod_to_hex(payment_id);
                for(auto &src: cd.sources) {
                    desc.amount_in += src.amount;
                    if(src.outputs.size() < desc.ring_size)
                        desc.ring_size = src.outputs.size();
                }
                for(const auto &entry : cd.splitted_dsts) {
                    std::string address = cryptonote::get_account_address_as_str(
                        cryptonoteNetwork(m_network), entry.is_subaddress, entry.addr
                    );
                    if(!desc.payment_id.empty() && !entry.is_subaddress && address != entry.original)
                        address = cryptonote::get_account_integrated_address_as_str(
                                cryptonoteNetwork(m_network), entry.addr, payment_id);
                    auto tx_dest = tx_dests.find(entry.addr);
                    if(tx_dest == tx_dests.end()) // tx dest not existing yet, let's create it
                        tx_dests.emplace(entry.addr, std::pair(address, entry.amount));
                    else // tx dest already exists, let's add the amount to it
                        tx_dest->second.second += entry.amount;
                    desc.amount_out += entry.amount;
                }
                if(cd.change_dts.amount > 0) {
                    auto it = tx_dests.find(cd.change_dts.addr);
                    if(it == tx_dests.end())
                        throw ots::exception::tx::Change("Claimed change does not go to a paid address");
                    if(it->second.second < cd.change_dts.amount)
                        throw ots::exception::tx::Change("Claimed change is larger than payment to the change address");
                    if(!first_known_non_zero_change.has_value()) // on first time, set to currenct construction data
                        first_known_non_zero_change = std::cref(cd);
                    if(
                        memcmp( // if the change address is different from the first one, we have a multi change addresses
                            &cd.change_dts.addr,
                            &first_known_non_zero_change.value().get().change_dts.addr,
                            sizeof(cd.change_dts.addr)
                        )
                    )
                        throw ots::exception::tx::Change("Change goes to more than one address");
                    desc.change_amount += cd.change_dts.amount;
                    it->second.second -= cd.change_dts.amount;
                    if(it->second.second == 0)
                        tx_dests.erase(cd.change_dts.addr);
                }
                for(const auto &tx_dest: tx_dests) {
                    if(tx_dest.second.second == 0) { // no amount sent to this address, so it's a dummy output
                        ++desc.dummy_outputs;
                        continue;
                    }
                    desc.recipients.emplace_back(tx_dest.second.first, tx_dest.second.second);
                    auto it_in_all = all_dests.find(tx_dest.first);
                    if(it_in_all == all_dests.end()) // tx dest not existing yet, let's create it
                        all_dests.emplace(tx_dest.first, tx_dest.second);
                    else // tx dest already exists, let's add the amount to it
                        it_in_all->second.second += tx_dest.second.second;
                }
                if(desc.change_amount > 0) {
                    desc.change_address = get_account_address_as_str( // address of the first change
                        cryptonoteNetwork(m_network),
                        tx_constructions.front().subaddr_account > 0,
                        tx_constructions.front().change_dts.addr
                    );
                    res.summary.change_address = desc.change_address;
                }
                desc.fee = desc.amount_in - desc.amount_out;
                desc.unlock_time = cd.unlock_time;
                desc.extra = epee::to_hex::string({cd.extra.data(), cd.extra.size()});
                // Update summary items
                res.summary.amount_in += desc.amount_in;
                res.summary.amount_out += desc.amount_out;
                res.summary.change_amount += desc.change_amount;
                res.summary.fee += desc.fee;
                res.desc.emplace_back(desc);
            }
            // Populate the summary recipients list
            for(auto i = all_dests.begin(); i != all_dests.end(); ++i) {
                res.summary.recipients.emplace_back(i->second.first, i->second.second);
            }
        } catch(const std::exception &e) {
            throw ots::exception::tx::Parse("failed to parse unsigned transfers" + std::string(e.what()));
        }
        return res;
    }

    std::string Account::signTransaction(const std::string &unsignedTransaction, bool withMagic) {
        unsigned_tx_set exported_txs = parseUnsignedTransaction(unsignedTransaction, withMagic);
        // sign
        auto [signed_txes, ptx] = signTransaction(exported_txs);
        // save as binary
        std::ostringstream oss;
        binary_archive<true> ar(oss);
        try {
            if(!::serialization::serialize(ar, signed_txes))
                throw ots::exception::tx::Sign("Failed to serialize signed transaction");
        } catch(...) {
            throw ots::exception::tx::Sign("Failed to serialize signed transaction");
        }
        return (withMagic?std::string(SIGNED_TX_PREFIX):std::string("")) + encryptWithViewSecretKey(oss.str());
    }

    std::pair<signed_tx_set, std::vector<pending_tx>> Account::signTransaction(
        unsigned_tx_set &exported_txs
    ) {
        std::vector<pending_tx> txs;
        signed_tx_set signed_txes;
        if(!std::get<2>(exported_txs.new_transfers).empty())
            importOutputs(exported_txs.new_transfers);
        else if(!std::get<2>(exported_txs.transfers).empty()) // TODO: think that can be removed, seems to be deprecated
            importOutputs(exported_txs.transfers);
        // sign the transactions
        for(size_t n = 0; n < exported_txs.txes.size(); ++n) {
            tx_construction_data &sd = exported_txs.txes[n];
            if(sd.sources.empty())
                throw ots::exception::tx::Sign("Empty sources");
            if(sd.unlock_time)
                throw ots::exception::tx::UnlockTime("Non-zero unlock time");
            signed_txes.ptx.push_back(pending_tx());
            pending_tx &ptx = signed_txes.ptx.back();
            rct::RCTConfig rct_config = sd.rct_config;
            crypto::secret_key tx_key;
            std::vector<crypto::secret_key> additional_tx_keys;
            if(!construct_tx_and_get_tx_key(
                m_account.get_keys(),
                m_subaddresses,
                sd.sources,
                sd.splitted_dsts,
                sd.change_dts.addr,
                sd.extra,
                ptx.tx,
                tx_key,
                additional_tx_keys,
                sd.use_rct,
                rct_config,
                sd.use_view_tags
            ))
                throw ots::exception::tx::Construct("Failed to construct transaction");
            // we don't test tx size, because we don't know the current limit, due to not having a blockchain,
            // and it's a bit pointless to fail there anyway, since it'd be a (good) guess only. We sign anyway,
            // and if we really go over limit, the daemon will reject when it gets submitted. Chances are it's
            // OK anyway since it was generated in the first place, and rerolling should be within a few bytes.

            // normally, the tx keys are saved in commit_tx, when the tx is actually sent to the daemon.
            // we can't do that here since the tx will be sent from the compromised wallet, which we don't want
            // to see that info, so we save it here
            /* TODO: THOR seem not to be used anywhere
            if(tx_key != crypto::null_skey) {
                const crypto::hash txid = get_transaction_hash(ptx.tx);
                m_tx_keys[txid] = tx_key;
                m_additional_tx_keys[txid] = additional_tx_keys;
            }
            */

            std::string key_images;
            bool all_are_txin_to_key = std::all_of(
                ptx.tx.vin.begin(),
                ptx.tx.vin.end(),
                [&](const cryptonote::txin_v& s_e
            ) -> bool {
                CHECKED_GET_SPECIFIC_VARIANT(s_e, const cryptonote::txin_to_key, in, false);
                key_images += boost::to_string(in.k_image) + " ";
                return true;
            });
            // THROW_WALLET_EXCEPTION_IF(!all_are_txin_to_key, error::unexpected_txin_type, ptx.tx);
            if(!all_are_txin_to_key)
                throw ots::exception::tx::UnexpectedInputType("Unexpected input type");
            ptx.key_images = key_images;
            ptx.fee = 0;
            for(const auto &i: sd.sources) ptx.fee += i.amount;
            for(const auto &i: sd.splitted_dsts) ptx.fee -= i.amount;
            ptx.dust = 0;
            ptx.dust_added_to_fee = false;
            ptx.change_dts = sd.change_dts;
            ptx.selected_transfers = sd.selected_transfers;
            ptx.tx_key = rct::rct2sk(rct::identity()); // don't send it back to the untrusted view wallet
            ptx.dests = sd.dests;
            ptx.construction_data = sd;

            txs.push_back(ptx);
            // add tx keys only to ptx
            txs.back().tx_key = tx_key;
            txs.back().additional_tx_keys = additional_tx_keys;
        }
        // add key image mapping for these txes
        const cryptonote::account_keys &keys = m_account.get_keys();
        hw::device &hwdev = m_account.get_device();
        for(size_t n = 0; n < exported_txs.txes.size(); ++n) {
            const cryptonote::transaction &tx = signed_txes.ptx[n].tx;
            crypto::key_derivation derivation;
            std::vector<crypto::key_derivation> additional_derivations;
            crypto::public_key tx_pub_key = get_tx_pub_key_from_extra(tx);
            std::vector<crypto::public_key> additional_tx_pub_keys;
            for(const crypto::secret_key &skey: txs[n].additional_tx_keys) {
                additional_tx_pub_keys.resize(additional_tx_pub_keys.size() + 1);
                crypto::secret_key_to_public_key(skey, additional_tx_pub_keys.back());
            }
            // compute derivations
            hwdev.set_mode(hw::device::TRANSACTION_PARSE);
            if(!hwdev.generate_key_derivation(tx_pub_key, keys.m_view_secret_key, derivation))
                throw ots::exception::tx::Sign("Failed to generate key derivation from tx pubkey" + boost::lexical_cast<std::string>(cryptonote::get_transaction_hash(tx)));
            for(size_t i = 0; i < additional_tx_pub_keys.size(); ++i) {
                additional_derivations.push_back({});
                if(!hwdev.generate_key_derivation(
                    additional_tx_pub_keys[i],
                    keys.m_view_secret_key,
                    additional_derivations.back()
                )) {
                    std::cerr << "Failed to generate key derivation from additional tx pubkey in " << cryptonote::get_transaction_hash(tx) << ", skipping" << std::endl; // TODO: debug only, remove or throw
                    memcpy(&additional_derivations.back(), rct::identity().bytes, sizeof(crypto::key_derivation));
                }
            }
            for(size_t i = 0; i < tx.vout.size(); ++i) {
                crypto::public_key output_public_key;
                if(
                    !get_output_public_key(tx.vout[i], output_public_key)
                
                    || !is_out_to_acc_precomp(
                        m_subaddresses,
                        output_public_key,
                        derivation,
                        additional_derivations,
                        i,
                        hwdev,
                        get_output_view_tag(tx.vout[i])
                    ) // if this output is back to this wallet, we can calculate its key image already
                )
                    continue;
                crypto::key_image ki;
                cryptonote::keypair in_ephemeral;
                if(!generate_key_image_helper(
                    keys,
                    m_subaddresses,
                    output_public_key,
                    tx_pub_key,
                    additional_tx_pub_keys,
                    i,
                    in_ephemeral,
                    ki,
                    hwdev
                ))
                    throw ots::exception::tx::Sign("Failed to calculate key image");
                signed_txes.tx_key_images[output_public_key] = ki;
            }
        }
        // add key images
        signed_txes.key_images.resize(m_transfers.size());
        for(size_t i = 0; i < m_transfers.size(); ++i) {
            if(!m_transfers[i].m_key_image_known || m_transfers[i].m_key_image_partial)
                std::cout << "WARNING: key image not known in signing wallet at index " << i << std::endl; // TODO: debug only, remove or throw
            signed_txes.key_images[i] = m_transfers[i].m_key_image;
        }
        return std::pair(signed_txes, txs);
    }

    std::vector<TxWarning> Account::checkTransaction(const TxDescription &txDescription) const noexcept {
        std::vector<TxWarning> warnings;
        for(const auto &transfer: txDescription.transfers) {
            if(transfer.unlockTime > 0) {
                // TODO: implement actual TxWarning's
                // warnings.emplace_back();
                break;
            }
        }
        return warnings;
    }

    std::string Account::encrypt(const char *plaintext, size_t len, const crypto::secret_key &skey, bool authenticated) const {
        crypto::chacha_key key;
        crypto::generate_chacha_key(&skey, sizeof(skey), key, mKdfRounds);
        std::string ciphertext;
        crypto::chacha_iv iv = crypto::rand<crypto::chacha_iv>();
        ciphertext.resize(len + sizeof(iv) + (authenticated ? sizeof(crypto::signature) : 0));
        crypto::chacha20(plaintext, len, key, iv, &ciphertext[sizeof(iv)]);
        memcpy(&ciphertext[0], &iv, sizeof(iv));
        if(authenticated) {
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
        boost::unique_lock<hw::device> hwdev_lock(hwdev);
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
        if(!is_out_data || i >= is_out_data->received.size())
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

    bool Account::isBadMagic(const std::string& data, const std::string& magic) {
        return data.size() < magic.size() || data.substr(0, magic.size()) != magic;
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

    unsigned_tx_set Account::parseUnsignedTransaction(const std::string &unsigned_tx, bool withMagic) const {
        unsigned_tx_set exported_txs;
        if(withMagic && isBadMagic(unsigned_tx, UNSIGNED_TX_PREFIX))
            throw ots::exception::wallet::UnsignedTransaction("Bad magic in data");
        std::string s = withMagic?unsigned_tx.substr(strlen(UNSIGNED_TX_PREFIX) -1):unsigned_tx;
        const char version = s[0];
        s = s.substr(1);
        // THOR: version bytes '\003' and '\004' are deprecated, we will not support them
        // deleted code how it will be handled below. But maybe we should tell
        // the user that the file is deprecated and he should use the new format
        // instead serving him a generic exception below.
        if(version != '\005')
            throw ots::exception::wallet::UnsignedTransaction("Unsupported version in unsigned tx");
        try {
            s = decryptWithViewSecretKey(s);
        } catch(const std::exception &e) {
            throw ots::exception::wallet::UnsignedTransaction(e.what());
        }
        try {
            binary_archive<false> ar{epee::strspan<std::uint8_t>(s)};
            if(!::serialization::serialize(ar, exported_txs))
                throw ots::exception::wallet::UnsignedTransaction("Failed to parse data from unsigned tx");
        }
        catch(...) {
            throw ots::exception::wallet::UnsignedTransaction("Failed to parse data from unsigned tx");
        }
        return exported_txs;
    }

    // from cryptonote_tx_utils.cpp
    bool Account::construct_tx_and_get_tx_key(
        const cryptonote::account_keys& sender_account_keys,
        const std::unordered_map<crypto::public_key, cryptonote::subaddress_index>& subaddresses,
        std::vector<cryptonote::tx_source_entry>& sources,
        std::vector<cryptonote::tx_destination_entry>& destinations,
        const boost::optional<cryptonote::account_public_address>& change_addr,
        const std::vector<uint8_t> &extra,
        cryptonote::transaction& tx,
        crypto::secret_key &tx_key,
        std::vector<crypto::secret_key> &additional_tx_keys,
        bool rct,
        const rct::RCTConfig &rct_config,
        bool use_view_tags
    ) {
        hw::device &hwdev = sender_account_keys.get_device();
        hwdev.open_tx(tx_key);
        try {
            // figure out if we need to make additional tx pubkeys
            size_t num_stdaddresses = 0;
            size_t num_subaddresses = 0;
            cryptonote::account_public_address single_dest_subaddress;
            classify_addresses(destinations, change_addr, num_stdaddresses, num_subaddresses, single_dest_subaddress);
            bool need_additional_txkeys = num_subaddresses > 0 && (num_stdaddresses > 0 || num_subaddresses > 1);
            if (need_additional_txkeys)
            {
                additional_tx_keys.clear();
                for (size_t i = 0; i < destinations.size(); ++i)
                {
                    additional_tx_keys.push_back(cryptonote::keypair::generate(sender_account_keys.get_device()).sec);
                }
            }

            bool shuffle_outs = true;
            bool r = construct_tx_with_tx_key(sender_account_keys, subaddresses, sources, destinations, change_addr, extra, tx, tx_key, additional_tx_keys, rct, rct_config, shuffle_outs, use_view_tags);
            hwdev.close_tx();
            return r;
        } catch(...) {
            hwdev.close_tx();
            throw;
        }
    }

    // from cryptonote_tx_utils.cpp
    bool Account::construct_tx_with_tx_key(
        const cryptonote::account_keys& sender_account_keys,
        const std::unordered_map<crypto::public_key, cryptonote::subaddress_index>& subaddresses,
        std::vector<cryptonote::tx_source_entry>& sources,
        std::vector<cryptonote::tx_destination_entry>& destinations,
        const boost::optional<cryptonote::account_public_address>& change_addr,
        const std::vector<uint8_t> &extra,
        cryptonote::transaction& tx,
        const crypto::secret_key &tx_key,
        const std::vector<crypto::secret_key> &additional_tx_keys,
        bool rct,
        const rct::RCTConfig &rct_config,
        bool shuffle_outs,
        bool use_view_tags
    ) {
        hw::device &hwdev = sender_account_keys.get_device();
        if(sources.empty()) {
            std::cout << "Empty sources" << std::endl; // TODO: debug only, remove or throw
            return false;
        }
        std::vector<rct::key> amount_keys;
        tx.set_null();
        amount_keys.clear();
        tx.version = rct ? 2 : 1;
        tx.unlock_time = 0;
        tx.extra = extra;
        crypto::public_key txkey_pub;
        // if we have a stealth payment id, find it and encrypt it with the tx key now
        std::vector<cryptonote::tx_extra_field> tx_extra_fields;
        if(parse_tx_extra(tx.extra, tx_extra_fields)) {
            bool add_dummy_payment_id = true;
            cryptonote::tx_extra_nonce extra_nonce;
            if(find_tx_extra_field_by_type(tx_extra_fields, extra_nonce)) {
                crypto::hash payment_id = crypto::null_hash;
                crypto::hash8 payment_id8 = crypto::null_hash8;
                if(cryptonote::get_encrypted_payment_id_from_tx_extra_nonce(extra_nonce.nonce, payment_id8)) {
                    crypto::public_key view_key_pub = get_destination_view_key_pub(destinations, change_addr);
                    if(view_key_pub == crypto::null_pkey) {
                        std::cout << "Destinations have to have exactly one output to support encrypted payment ids" << std::endl; // TODO: debug only, remove or throw
                        return false;
                    }
                    if(!hwdev.encrypt_payment_id(payment_id8, view_key_pub, tx_key)) {
                        std::cout << "Failed to encrypt payment id" << std::endl; // TODO: debug only, remove or throw
                        return false;
                    }
                    std::string extra_nonce;
                    cryptonote::set_encrypted_payment_id_to_tx_extra_nonce(extra_nonce, payment_id8);
                    cryptonote::remove_field_from_tx_extra(tx.extra, typeid(cryptonote::tx_extra_nonce));
                    if(!cryptonote::add_extra_nonce_to_tx_extra(tx.extra, extra_nonce)) {
                        std::cout << "Failed to add encrypted payment id to tx extra" << std::endl; // TODO: debug only, remove or throw
                        return false;
                    }
                    add_dummy_payment_id = false;
                } else if(cryptonote::get_payment_id_from_tx_extra_nonce(extra_nonce.nonce, payment_id)) {
                    add_dummy_payment_id = false;
                }
            }
            // we don't add one if we've got more than the usual 1 destination plus change
            if(destinations.size() > 2)
                add_dummy_payment_id = false;
            if(add_dummy_payment_id) {
                // if we have neither long nor short payment id, add a dummy short one,
                // this should end up being the vast majority of txes as time goes on
                std::string extra_nonce;
                crypto::hash8 payment_id8 = crypto::null_hash8;
                crypto::public_key view_key_pub = get_destination_view_key_pub(destinations, change_addr);
                if(view_key_pub == crypto::null_pkey) {
                    std::cout << "Failed to get key to encrypt dummy payment id with" << std::endl; // TODO: debug only, remove or throw
                } else {
                    hwdev.encrypt_payment_id(payment_id8, view_key_pub, tx_key);
                    cryptonote::set_encrypted_payment_id_to_tx_extra_nonce(extra_nonce, payment_id8);
                    if(!cryptonote::add_extra_nonce_to_tx_extra(tx.extra, extra_nonce)) // continue anyway
                        std::cerr << "Failed to add dummy encrypted payment id to tx extra" << std::endl; // TODO: debug only, remove or throw)
                }
            }
        } else {
            std::cerr << "Failed to parse tx extra" << std::endl; // TODO: debug only, remove or throw
            tx_extra_fields.clear();
        }
        struct input_generation_context_data {
            cryptonote::keypair in_ephemeral;
        };
        std::vector<input_generation_context_data> in_contexts;
        uint64_t summary_inputs_money = 0;
        //fill inputs
        int idx = -1;
        for(const cryptonote::tx_source_entry& src_entr:  sources) {
            ++idx;
            if(src_entr.real_output >= src_entr.outputs.size()) {
                std::cerr << "real_output index (" << src_entr.real_output << ")bigger than output_keys.size()=" << src_entr.outputs.size() << std::endl; // TODO: debug only, remove or throw
                return false;
            }
            summary_inputs_money += src_entr.amount;
            //key_derivation recv_derivation;
            in_contexts.push_back(input_generation_context_data());
            cryptonote::keypair& in_ephemeral = in_contexts.back().in_ephemeral;
            crypto::key_image img;
            const auto& out_key = reinterpret_cast<const crypto::public_key&>(src_entr.outputs[src_entr.real_output].second.dest);
            if(!generate_key_image_helper(
                    sender_account_keys,
                    subaddresses,
                    out_key,
                    src_entr.real_out_tx_key,
                    src_entr.real_out_additional_tx_keys,
                    src_entr.real_output_in_tx_index,
                    in_ephemeral,
                    img,
                    hwdev
                )
            ) {
                std::cerr << "Key image generation failed!" << std::endl; // TODO: debug only, remove or throw
                return false;
            }
            //check that derivated key is equal with real output key
            if(!(src_entr.outputs[src_entr.real_output].second.dest == in_ephemeral.pub)) {
                // TODO: debug only, remove or throw
                std::cerr << "derived public key mismatch with output public key at index "
                    << idx << ", real out " << src_entr.real_output << "! "<< std::endl
                    << "derived_key:"
                        << epee::string_tools::pod_to_hex(in_ephemeral.pub) << std::endl << "real output_public_key:"
                        << epee::string_tools::pod_to_hex(src_entr.outputs[src_entr.real_output].second.dest)
                        << std::endl
                    << "amount " << src_entr.amount << ", rct " << src_entr.rct << std::endl
                    << "tx pubkey " << src_entr.real_out_tx_key
                        << ", real_output_in_tx_index " << src_entr.real_output_in_tx_index << std::endl;
                return false;
            }
            //put key image into tx input
            cryptonote::txin_to_key input_to_key;
            input_to_key.amount = src_entr.amount;
            input_to_key.k_image = img;
            //fill outputs array and use relative offsets
            for(const cryptonote::tx_source_entry::output_entry& out_entry: src_entr.outputs)
                input_to_key.key_offsets.push_back(out_entry.first);
            input_to_key.key_offsets = cryptonote::absolute_output_offsets_to_relative(input_to_key.key_offsets);
            tx.vin.push_back(input_to_key);
        }
        if(shuffle_outs)
            std::shuffle(destinations.begin(), destinations.end(), crypto::random_device{});
        // sort ins by their key image
        std::vector<size_t> ins_order(sources.size());
        for(size_t n = 0; n < sources.size(); ++n)
            ins_order[n] = n;
        std::sort(ins_order.begin(), ins_order.end(), [&](const size_t i0, const size_t i1) {
            const cryptonote::txin_to_key &tk0 = boost::get<cryptonote::txin_to_key>(tx.vin[i0]);
            const cryptonote::txin_to_key &tk1 = boost::get<cryptonote::txin_to_key>(tx.vin[i1]);
            return memcmp(&tk0.k_image, &tk1.k_image, sizeof(tk0.k_image)) > 0;
        });
        tools::apply_permutation(ins_order, [&] (size_t i0, size_t i1) {
            std::swap(tx.vin[i0], tx.vin[i1]);
            std::swap(in_contexts[i0], in_contexts[i1]);
            std::swap(sources[i0], sources[i1]);
        });
        // figure out if we need to make additional tx pubkeys
        size_t num_stdaddresses = 0;
        size_t num_subaddresses = 0;
        cryptonote::account_public_address single_dest_subaddress;
        classify_addresses(destinations, change_addr, num_stdaddresses, num_subaddresses, single_dest_subaddress);
        // if this is a single-destination transfer to a subaddress, we set the tx pubkey to R=s*D
        txkey_pub = (num_stdaddresses == 0 && num_subaddresses == 1) ? rct::rct2pk(
            hwdev.scalarmultKey(
                rct::pk2rct(single_dest_subaddress.m_spend_public_key),
                rct::sk2rct(tx_key)
            )
        ): rct::rct2pk(
            hwdev.scalarmultBase(rct::sk2rct(tx_key))
        );
        cryptonote::remove_field_from_tx_extra(tx.extra, typeid(cryptonote::tx_extra_pub_key));
        add_tx_pub_key_to_extra(tx, txkey_pub);
        std::vector<crypto::public_key> additional_tx_public_keys;
        // we don't need to include additional tx keys if:
        //   - all the destinations are standard addresses
        //   - there's only one destination which is a subaddress
        bool need_additional_txkeys = num_subaddresses > 0 && (num_stdaddresses > 0 || num_subaddresses > 1);
        if(need_additional_txkeys)
            CHECK_AND_ASSERT_MES(destinations.size() == additional_tx_keys.size(), false, "Wrong amount of additional tx keys");
        uint64_t summary_outs_money = 0;
        //fill outputs
        size_t output_index = 0;
        for(const cryptonote::tx_destination_entry& dst_entr: destinations) {
            CHECK_AND_ASSERT_MES(dst_entr.amount > 0 || tx.version > 1, false, "Destination with wrong amount: " << dst_entr.amount);
            crypto::public_key out_eph_public_key;
            crypto::view_tag view_tag;
            hwdev.generate_output_ephemeral_keys(
                tx.version,
                sender_account_keys,
                txkey_pub, tx_key,
                dst_entr, change_addr, output_index,
                need_additional_txkeys, additional_tx_keys,
                additional_tx_public_keys, amount_keys, out_eph_public_key,
                use_view_tags, view_tag
            );
            cryptonote::tx_out out;
            cryptonote::set_tx_out(dst_entr.amount, out_eph_public_key, use_view_tags, view_tag, out);
            tx.vout.push_back(out);
            output_index++;
            summary_outs_money += dst_entr.amount;
        }
        CHECK_AND_ASSERT_MES(additional_tx_public_keys.size() == additional_tx_keys.size(), false, "Internal error creating additional public keys");
        cryptonote::remove_field_from_tx_extra(tx.extra, typeid(cryptonote::tx_extra_additional_pub_keys));
        if(need_additional_txkeys)
            cryptonote::add_additional_tx_pub_keys_to_extra(tx.extra, additional_tx_public_keys);
        if(!cryptonote::sort_tx_extra(tx.extra, tx.extra))
            return false;
        CHECK_AND_ASSERT_MES(tx.extra.size() <= MAX_TX_EXTRA_SIZE, false, "TX extra size (" << tx.extra.size() << ") is greater than max allowed (" << MAX_TX_EXTRA_SIZE << ")");
        //check money
        if(summary_outs_money > summary_inputs_money ) {
            std::cerr << "Transaction inputs money ("<< summary_inputs_money << ") less than outputs money (" << summary_outs_money << ")" << std::endl; // TODO: debug only, remove or throw
            return false;
        }
        // check for watch only wallet
        bool zero_secret_key = true;
        for(size_t i = 0; i < sizeof(sender_account_keys.m_spend_secret_key); ++i)
            zero_secret_key &= (sender_account_keys.m_spend_secret_key.data[i] == 0);
        if(zero_secret_key)
            std::cerr << "Null secret key, skipping signatures" << std::endl; // TODO: debug only, remove or throw
        if(tx.version == 1) { 
            //generate ring signatures
            crypto::hash tx_prefix_hash;
            get_transaction_prefix_hash(tx, tx_prefix_hash);
            std::stringstream ss_ring_s;
            size_t i = 0;
            for(const cryptonote::tx_source_entry& src_entr:  sources) {
                ss_ring_s << "pub_keys:" << std::endl;
                std::vector<const crypto::public_key*> keys_ptrs;
                std::vector<crypto::public_key> keys(src_entr.outputs.size());
                size_t ii = 0;
                for(const cryptonote::tx_source_entry::output_entry& o: src_entr.outputs) {
                    keys[ii] = rct2pk(o.second.dest);
                    keys_ptrs.push_back(&keys[ii]);
                    ss_ring_s << o.second.dest << std::endl;
                    ++ii;
                }
                tx.signatures.push_back(std::vector<crypto::signature>());
                std::vector<crypto::signature>& sigs = tx.signatures.back();
                sigs.resize(src_entr.outputs.size());
                if(!zero_secret_key)
                    crypto::generate_ring_signature(
                        tx_prefix_hash,
                        boost::get<cryptonote::txin_to_key>(tx.vin[i]).k_image,
                        keys_ptrs,
                        in_contexts[i].in_ephemeral.sec,
                        src_entr.real_output,
                        sigs.data()
                    );
                ss_ring_s << "signatures:" << std::endl;
                std::for_each(sigs.begin(), sigs.end(), [&](const crypto::signature& s){ss_ring_s << s << std::endl;});
                ss_ring_s << "prefix_hash:" << tx_prefix_hash << std::endl << "in_ephemeral_key: " << crypto::secret_key_explicit_print_ref{in_contexts[i].in_ephemeral.sec} << std::endl << "real_output: " << src_entr.real_output << std::endl;
                i++;
            }
            std::cerr << "construct_tx, transaction_created: " << get_transaction_hash(tx) << std::endl << obj_to_json_str(tx) << std::endl << ss_ring_s.str() << std::endl; // TODO: debug only remove!
        } else {
            size_t n_total_outs = sources[0].outputs.size(); // only for non-simple rct
            // the non-simple version is slightly smaller, but assumes all real inputs
            // are on the same index, so can only be used if there just one ring.
            bool use_simple_rct = sources.size() > 1 || rct_config.range_proof_type != rct::RangeProofBorromean;
            if(!use_simple_rct) {
                // non simple ringct requires all real inputs to be at the same index for all inputs
                for(const cryptonote::tx_source_entry& src_entr:  sources) {
                    if(src_entr.real_output != sources.begin()->real_output) {
                        std::cerr << "All inputs must have the same index for non-simple ringct" << std::endl; // TODO: debug only, remove or throw
                        return false;
                    }
                }
                // enforce same mixin for all outputs
                for(size_t i = 1; i < sources.size(); ++i) {
                    if(n_total_outs != sources[i].outputs.size()) {
                        std::cerr << "Non-simple ringct transaction has varying ring size" << std::endl; // TODO: debug only, remove or throw
                        return false;
                    }
                }
            }
            uint64_t amount_in = 0, amount_out = 0;
            rct::ctkeyV inSk;
            inSk.reserve(sources.size());
            // mixRing indexing is done the other way round for simple
            rct::ctkeyM mixRing(use_simple_rct ? sources.size() : n_total_outs);
            rct::keyV destinations;
            std::vector<uint64_t> inamounts, outamounts;
            std::vector<unsigned int> index;
            for(size_t i = 0; i < sources.size(); ++i) {
                rct::ctkey ctkey;
                amount_in += sources[i].amount;
                inamounts.push_back(sources[i].amount);
                index.push_back(sources[i].real_output);
                // inSk: (secret key, mask)
                ctkey.dest = rct::sk2rct(in_contexts[i].in_ephemeral.sec);
                ctkey.mask = sources[i].mask;
                inSk.push_back(ctkey);
                memwipe(&ctkey, sizeof(rct::ctkey));
                // inPk: (public key, commitment)
                // will be done when filling in mixRing
            }
            for(size_t i = 0; i < tx.vout.size(); ++i) {
                crypto::public_key output_public_key;
                get_output_public_key(tx.vout[i], output_public_key);
                destinations.push_back(rct::pk2rct(output_public_key));
                outamounts.push_back(tx.vout[i].amount);
                amount_out += tx.vout[i].amount;
            }
            if(use_simple_rct) {
                // mixRing indexing is done the other way round for simple
                for(size_t i = 0; i < sources.size(); ++i) {
                    mixRing[i].resize(sources[i].outputs.size());
                    for(size_t n = 0; n < sources[i].outputs.size(); ++n) {
                        mixRing[i][n] = sources[i].outputs[n].second;
                    }
                }
            } else {
                for(size_t i = 0; i < n_total_outs; ++i) { // same index assumption
                    mixRing[i].resize(sources.size());
                    for(size_t n = 0; n < sources.size(); ++n) {
                        mixRing[i][n] = sources[n].outputs[i].second;
                    }
                }
            }
            // fee
            if(!use_simple_rct && amount_in > amount_out)
                outamounts.push_back(amount_in - amount_out);
            // zero out all amounts to mask rct outputs, real amounts are now encrypted
            for(size_t i = 0; i < tx.vin.size(); ++i) {
                if(sources[i].rct)
                    boost::get<cryptonote::txin_to_key>(tx.vin[i]).amount = 0;
            }
            for(size_t i = 0; i < tx.vout.size(); ++i)
                tx.vout[i].amount = 0;
            crypto::hash tx_prefix_hash;
            get_transaction_prefix_hash(tx, tx_prefix_hash, hwdev);
            rct::ctkeyV outSk;
            tx.rct_signatures = use_simple_rct ? rct::genRctSimple(
                    rct::hash2rct(tx_prefix_hash),
                    inSk,
                    destinations,
                    inamounts,
                    outamounts,
                    amount_in - amount_out,
                    mixRing,
                    amount_keys,
                    index,
                    outSk,
                    rct_config,
                    hwdev
            ) : rct::genRct( // same index assumption
                    rct::hash2rct(tx_prefix_hash),
                    inSk,
                    destinations,
                    outamounts,
                    mixRing,
                    amount_keys,
                    sources[0].real_output,
                    outSk,
                    rct_config,
                    hwdev
            );
            memwipe(inSk.data(), inSk.size() * sizeof(rct::ctkey));
            CHECK_AND_ASSERT_MES(tx.vout.size() == outSk.size(), false, "outSk size does not match vout");

            std::cerr << "construct_tx, transaction_created: " << get_transaction_hash(tx) << std::endl << obj_to_json_str(tx) << std::endl; // TODO: debug only remove!
        }
        tx.invalidate_hashes();
        return true;
    }

    // from cryptonote_tx_utils.cpp
    void Account::classify_addresses(
        const std::vector<cryptonote::tx_destination_entry> &destinations,
        const boost::optional<cryptonote::account_public_address>& change_addr,
        size_t &num_stdaddresses,
        size_t &num_subaddresses,
        cryptonote::account_public_address &single_dest_subaddress
    ) {
        num_stdaddresses = 0;
        num_subaddresses = 0;
        std::unordered_set<cryptonote::account_public_address> unique_dst_addresses;
        for(const cryptonote::tx_destination_entry& dst_entr: destinations) {
            if(change_addr && dst_entr.addr == change_addr)
                continue;
            if(unique_dst_addresses.count(dst_entr.addr) == 0) {
                unique_dst_addresses.insert(dst_entr.addr);
                if(dst_entr.is_subaddress) {
                    ++num_subaddresses;
                    single_dest_subaddress = dst_entr.addr;
                } else {
                    ++num_stdaddresses;
                }
            }
        }
        std::cerr << "destinations include " << num_stdaddresses << " standard addresses and " << num_subaddresses << " subaddresses" << std::endl; // TODO: debug only, remove!
    }

    // from cryptonote_tx_utils.cpp
    crypto::public_key Account::get_destination_view_key_pub(
        const std::vector<cryptonote::tx_destination_entry> &destinations,
        const boost::optional<cryptonote::account_public_address>& change_addr
    ) {
        cryptonote::account_public_address addr = {crypto::null_pkey, crypto::null_pkey};
        size_t count = 0;
        for (const auto &i : destinations)
        {
            if (i.amount == 0)
                continue;
            if (change_addr && i.addr == *change_addr)
                continue;
            if (i.addr == addr)
                continue;
            if (count > 0)
                return crypto::null_pkey;
            addr = i.addr;
            ++count;
        }
        if (count == 0 && change_addr)
            return change_addr->m_view_public_key;
        return addr.m_view_public_key;
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
