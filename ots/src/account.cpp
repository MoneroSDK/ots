#include "account.hpp"
#include "cryptonote-tx-utils.hpp"
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

    // TODO: IMPORTANT something seems off here if used via C ABI, second call will crash. Investigate
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
            if(!sd.use_rct)
                throw ots::exception::tx::Sign("Legacy Non-RingCT transaction not supported");
            signed_txes.ptx.push_back(pending_tx());
            pending_tx &ptx = signed_txes.ptx.back();
            auto [tx_key, additional_tx_keys] = constructTxAndGetTxKey(
                m_account.get_keys(),
                m_subaddresses,
                sd.sources,
                sd.splitted_dsts,
                sd.change_dts.addr,
                sd.extra,
                ptx.tx,
                sd.rct_config,
                sd.use_view_tags
            );
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
            if(!all_are_txin_to_key)
                throw ots::exception::tx::UnexpectedInputType("Unexpected tx input type");
            ptx.key_images = key_images;
            ptx.fee = 0;
            for(const auto &source: sd.sources) ptx.fee += source.amount;
            for(const auto &destination: sd.splitted_dsts) ptx.fee -= destination.amount;
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

    void Account::clearAddressCache() const noexcept {
        m_addressCache.clear();
        m_addressIndexCache.clear();
        m_indexToAddressCache.clear();
    }

    void AccountDeleter::operator()(Account* account) const {
        delete account;
    }
}
