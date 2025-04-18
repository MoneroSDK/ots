#include "cryptonote-tx-utils.hpp"
#include <ringct/rctSigs.h>
#include <string_tools.h>
#include <common/apply_permutation.h>
#include "ots-exceptions.hpp"

namespace ots {
    // from cryptonote_tx_utils.cpp: cryptonote::construct_tx_and_get_tx_key
    std::pair<crypto::secret_key, std::vector<crypto::secret_key>> constructTxAndGetTxKey(
        const cryptonote::account_keys& senderAccountKeys,
        const std::unordered_map<crypto::public_key, cryptonote::subaddress_index>& subaddresses,
        std::vector<cryptonote::tx_source_entry>& sources,
        std::vector<cryptonote::tx_destination_entry>& destinations,
        const boost::optional<cryptonote::account_public_address>& changeAddress,
        const std::vector<uint8_t> &extra,
        cryptonote::transaction& tx,
        const rct::RCTConfig &rct_config,
        bool use_view_tags
    ) {
        crypto::secret_key tx_key;
        std::vector<crypto::secret_key> additional_tx_keys;
        auto hwDeviceCloseGuard = std::unique_ptr<hw::device, std::function<void(hw::device*)>>(
            &senderAccountKeys.get_device(),
            [](hw::device* p) { p->close_tx(); }
        );
        hwDeviceCloseGuard->open_tx(tx_key);
        // figure out if we need to make additional tx pubkeys
        auto [stdAddressesCount, subAddressesCount, singleDestSubAddress] = classifyAddresses(destinations, changeAddress);
        if( // additonal tx keys are needed
            subAddressesCount > 0
            && (stdAddressesCount > 0 || subAddressesCount > 1)
        ) {
            additional_tx_keys.clear();
            for(size_t i = 0; i < destinations.size(); ++i)
                additional_tx_keys.emplace_back(
                    cryptonote::keypair::generate(senderAccountKeys.get_device()).sec
                );
        }
        constructTxWithTxKey(
            senderAccountKeys,
            subaddresses,
            sources,
            destinations,
            changeAddress,
            extra,
            tx,
            tx_key,
            additional_tx_keys,
            rct_config,
            use_view_tags
        );
        return std::pair(tx_key, additional_tx_keys);
    }

    // from cryptonote_tx_utils.cpp: cryptonote::construct_tx_with_tx_key
    void constructTxWithTxKey(
        const cryptonote::account_keys& senderAccountKeys,
        const std::unordered_map<crypto::public_key, cryptonote::subaddress_index>& subaddresses,
        std::vector<cryptonote::tx_source_entry>& sources,
        std::vector<cryptonote::tx_destination_entry>& destinations,
        const boost::optional<cryptonote::account_public_address>& changeAddress,
        const std::vector<uint8_t> &extra,
        cryptonote::transaction& tx,
        const crypto::secret_key &tx_key,
        const std::vector<crypto::secret_key> &additional_tx_keys,
        const rct::RCTConfig &rct_config,
        bool use_view_tags
    ) {
        hw::device &hwdev = senderAccountKeys.get_device();
        if(sources.empty())
            throw ots::exception::tx::Construct("Empty sources");
        std::vector<rct::key> amount_keys;
        tx.set_null();
        amount_keys.clear();
        tx.version = 2; // we always use version rct (version > 2)
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
                if(cryptonote::get_encrypted_payment_id_from_tx_extra_nonce(
                        extra_nonce.nonce,
                        payment_id8
                    )
                ) {
                    crypto::public_key view_key_pub = getDestinationViewKeyPub(
                        destinations,
                        changeAddress
                    );
                    if(view_key_pub == crypto::null_pkey)
                        throw ots::exception::tx::Construct("Destinations have to have exactly one output to support encrypted payment ids");
                    if(!hwdev.encrypt_payment_id(payment_id8, view_key_pub, tx_key))
                        throw ots::exception::tx::Construct("Failed to encrypt payment id");
                    std::string extra_nonce;
                    cryptonote::set_encrypted_payment_id_to_tx_extra_nonce(extra_nonce, payment_id8);
                    cryptonote::remove_field_from_tx_extra(tx.extra, typeid(cryptonote::tx_extra_nonce));
                    if(!cryptonote::add_extra_nonce_to_tx_extra(tx.extra, extra_nonce))
                        throw ots::exception::tx::Construct("Failed to add encrypted payment id to tx extra");
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
                crypto::public_key view_key_pub = getDestinationViewKeyPub(destinations, changeAddress);
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
            if(src_entr.real_output >= src_entr.outputs.size())
                throw ots::exception::tx::Construct("real_output index (" + std::to_string(src_entr.real_output) + ") bigger than output_keys.size()=" + std::to_string(src_entr.outputs.size()));
            summary_inputs_money += src_entr.amount;
            //key_derivation recv_derivation;
            in_contexts.push_back(input_generation_context_data());
            cryptonote::keypair& in_ephemeral = in_contexts.back().in_ephemeral;
            crypto::key_image img;
            const auto& out_key = reinterpret_cast<const crypto::public_key&>(src_entr.outputs[src_entr.real_output].second.dest);
            if(!generate_key_image_helper(
                    senderAccountKeys,
                    subaddresses,
                    out_key,
                    src_entr.real_out_tx_key,
                    src_entr.real_out_additional_tx_keys,
                    src_entr.real_output_in_tx_index,
                    in_ephemeral,
                    img,
                    hwdev
                )
            )
                throw ots::exception::tx::Construct("Key image generation failed");
            //check that derivated key is equal with real output key
            if(!(src_entr.outputs[src_entr.real_output].second.dest == in_ephemeral.pub)) {
                std::ostringstream oss;
                oss << "derived public key mismatch with output public key at index "
                    << idx << ", real out " << src_entr.real_output << "! "<< std::endl
                    << "derived_key:"
                        << epee::string_tools::pod_to_hex(in_ephemeral.pub) << std::endl
                    << "real output_public_key:"
                        << epee::string_tools::pod_to_hex(src_entr.outputs[src_entr.real_output].second.dest)
                        << std::endl
                    << "amount " << src_entr.amount << ", rct " << src_entr.rct << std::endl
                    << "tx pubkey " << src_entr.real_out_tx_key
                        << ", real_output_in_tx_index " << src_entr.real_output_in_tx_index << std::endl;
                throw ots::exception::tx::Construct(oss.str());
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
        // shuffle outs
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
        auto [stdAddressesCount, subAddressesCount, singleDestSubAddress] = classifyAddresses(destinations, changeAddress);
        // if this is a single-destination transfer to a subaddress,
        // we set the tx pubkey to R=s*D
        txkey_pub = (stdAddressesCount == 0 && subAddressesCount == 1) ? rct::rct2pk(
            hwdev.scalarmultKey(
                rct::pk2rct(singleDestSubAddress.m_spend_public_key),
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
        bool need_additional_txkeys = subAddressesCount > 0 && (stdAddressesCount > 0 || subAddressesCount > 1);
        if(need_additional_txkeys && destinations.size() != additional_tx_keys.size())
            throw ots::exception::tx::Construct("Wrong amount of additional tx keys");
        uint64_t summary_outs_money = 0;
        //fill outputs
        size_t output_index = 0;
        for(const cryptonote::tx_destination_entry& destination: destinations) {
            crypto::public_key out_eph_public_key;
            crypto::view_tag view_tag;
            hwdev.generate_output_ephemeral_keys(
                tx.version,
                senderAccountKeys,
                txkey_pub, tx_key,
                destination, changeAddress, output_index,
                need_additional_txkeys, additional_tx_keys,
                additional_tx_public_keys, amount_keys, out_eph_public_key,
                use_view_tags, view_tag
            );
            cryptonote::tx_out out;
            cryptonote::set_tx_out(destination.amount, out_eph_public_key, use_view_tags, view_tag, out);
            tx.vout.push_back(out);
            output_index++;
            summary_outs_money += destination.amount;
        }
        if(additional_tx_public_keys.size() != additional_tx_keys.size())
            throw ots::exception::tx::Construct("Internal error creating additional public keys");
        cryptonote::remove_field_from_tx_extra(tx.extra, typeid(cryptonote::tx_extra_additional_pub_keys));
        if(need_additional_txkeys)
            cryptonote::add_additional_tx_pub_keys_to_extra(tx.extra, additional_tx_public_keys);
        if(!cryptonote::sort_tx_extra(tx.extra, tx.extra))
            throw ots::exception::tx::Construct("Failed to sort tx extra");
        if(tx.extra.size() > MAX_TX_EXTRA_SIZE)
            throw ots::exception::tx::Construct("Transaction extra size (" + std::to_string(tx.extra.size()) + ") is greater than max allowed: " + std::to_string(MAX_TX_EXTRA_SIZE));
        //check money
        if(summary_outs_money > summary_inputs_money )
            throw ots::exception::tx::Construct("Transaction inputs money (" + std::to_string(summary_inputs_money) + ") less than outputs money (" + std::to_string(summary_outs_money) + ")");
        // check for watch only wallet
        bool zero_secret_key = true;
        for(size_t i = 0; i < sizeof(senderAccountKeys.m_spend_secret_key); ++i)
            zero_secret_key &= (senderAccountKeys.m_spend_secret_key.data[i] == 0);
        size_t n_total_outs = sources.begin()->outputs.size(); // only for non-simple rct
        // the non-simple version is slightly smaller, but assumes all real inputs
        // are on the same index, so can only be used if there just one ring.
        bool use_simple_rct = sources.size() > 1 || rct_config.range_proof_type != rct::RangeProofBorromean;
        if(!use_simple_rct) {
            // non simple ringct requires all real inputs to be at the same index for all inputs
            for(const cryptonote::tx_source_entry& source:  sources)
                if(source.real_output != sources.begin()->real_output)
                    throw ots::exception::tx::Construct("All inputs must have the same index for non-simple ringct");
            // enforce same mixin for all outputs
            for(const cryptonote::tx_source_entry& source:  sources)
                if(n_total_outs != source.outputs.size())
                    throw ots::exception::tx::Construct("Non-simple ringct transaction has varying ring size");
        }
        uint64_t amount_in = 0, amount_out = 0;
        rct::ctkeyV inSk;
        inSk.reserve(sources.size());
        // mixRing indexing is done the other way round for simple
        rct::ctkeyM mixRing(use_simple_rct ? sources.size() : n_total_outs);
        rct::keyV ringDestinations;
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
            ringDestinations.push_back(rct::pk2rct(output_public_key));
            outamounts.push_back(tx.vout[i].amount);
            amount_out += tx.vout[i].amount;
        }
        if(use_simple_rct) {
            // mixRing indexing is done the other way round for simple
            for(size_t i = 0; i < sources.size(); ++i) {
                mixRing[i].resize(sources[i].outputs.size());
                for(size_t n = 0; n < sources[i].outputs.size(); ++n)
                    mixRing[i][n] = sources[i].outputs[n].second;
            }
        } else {
            for(size_t i = 0; i < n_total_outs; ++i) { // same index assumption
                mixRing[i].resize(sources.size());
                for(size_t n = 0; n < sources.size(); ++n)
                    mixRing[i][n] = sources[n].outputs[i].second;
            }
        }
        // fee
        if(!use_simple_rct && amount_in > amount_out)
            outamounts.push_back(amount_in - amount_out);
        // zero out all amounts to mask rct outputs, real amounts are now encrypted
        for(size_t i = 0; i < tx.vin.size(); ++i)
            if(sources[i].rct)
                boost::get<cryptonote::txin_to_key>(tx.vin[i]).amount = 0;
        for(size_t i = 0; i < tx.vout.size(); ++i)
            tx.vout[i].amount = 0;
        crypto::hash tx_prefix_hash;
        get_transaction_prefix_hash(tx, tx_prefix_hash, hwdev);
        rct::ctkeyV outSk;
        tx.rct_signatures = use_simple_rct ? rct::genRctSimple(
                rct::hash2rct(tx_prefix_hash),
                inSk,
                ringDestinations,
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
                ringDestinations,
                outamounts,
                mixRing,
                amount_keys,
                sources.begin()->real_output,
                outSk,
                rct_config,
                hwdev
        );
        memwipe(inSk.data(), inSk.size() * sizeof(rct::ctkey));
        if(tx.vout.size() != outSk.size())
            throw ots::exception::tx::Construct("outSk size does not match vout");
        tx.invalidate_hashes();
    }

    // from cryptonote_tx_utils.cpp: cryptonote::classify_addresses
    std::tuple<size_t, size_t, cryptonote::account_public_address> classifyAddresses(
        const std::vector<cryptonote::tx_destination_entry> &destinations,
        const boost::optional<cryptonote::account_public_address>& changeAddress
    ) {
        size_t stdAddressesCount = 0, subAddressesCount = 0;
        cryptonote::account_public_address singleDestSubAddress;
        bool uniqueDestAddresses = false;
        for(const cryptonote::tx_destination_entry& destination: destinations) {
            if(changeAddress && destination.addr == changeAddress)
                continue;
            if(!uniqueDestAddresses) {
                uniqueDestAddresses = true;
                if(destination.is_subaddress) {
                    subAddressesCount++;
                    singleDestSubAddress = destination.addr;
                    continue;
                }
                stdAddressesCount++;
            }
        }
        return {stdAddressesCount, subAddressesCount, singleDestSubAddress};
    }

    // from cryptonote_tx_utils.cpp: cryptonote::get_destination_view_key_pub
    crypto::public_key getDestinationViewKeyPub(
        const std::vector<cryptonote::tx_destination_entry> &destinations,
        const boost::optional<cryptonote::account_public_address>& changeAddress
    ) {
        cryptonote::account_public_address address = {crypto::null_pkey, crypto::null_pkey};
        size_t count = 0;
        for(const auto &destination : destinations) {
            if(
                destination.amount == 0
                || changeAddress && destination.addr == *changeAddress
                || destination.addr == address
            )
                continue;
            if(count > 0)
                return crypto::null_pkey;
            address = destination.addr;
            count++;
        }
        return (count == 0 && changeAddress)
            ? changeAddress->m_view_public_key
            : address.m_view_public_key;
    }
} // namespace ots
