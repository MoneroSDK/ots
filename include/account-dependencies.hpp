#pragma once

/**
 * @file account-dependencies.hpp
 * @brief struct dependencies from transplanting source
 *        from wallet2
 */

#include <cryptonote_basic/cryptonote_format_utils.h>
#include <cryptonote_basic/account.h>
#include <ringct/rctTypes.h>        // For rct::key
#include <cryptonote_basic/cryptonote_basic.h>  // For cryptonote::transaction_prefix, cryptonote::subaddress_index
#include <cryptonote_core/cryptonote_tx_utils.h> // For tx_source_entry, tx_destination_entry
#include <serialization/serialization.h> // For serialize support
#include <serialization/variant.h>   // For tools::write_varint
#include <serialization/tuple.h>
#include <serialization/pair.h>
#include <serialization/string.h>

namespace ots {
    struct is_out_data;
    struct tx_scan_info_t;
    struct exported_transfer_details;
    struct transfer_details;
    typedef std::vector<transfer_details> transfer_container;
    struct transfers;

    /**
     * @note dependency from the source code of wallet2
     */
    struct multisig_info
    {
        struct LR
        {
            rct::key m_L;
            rct::key m_R;

            BEGIN_SERIALIZE_OBJECT()
                FIELD(m_L)
                FIELD(m_R)
                END_SERIALIZE()
        };

        crypto::public_key m_signer;
        std::vector<LR> m_LR;
        std::vector<crypto::key_image> m_partial_key_images; // one per key the participant has

        BEGIN_SERIALIZE_OBJECT()
            FIELD(m_signer)
            FIELD(m_LR)
            FIELD(m_partial_key_images)
            END_SERIALIZE()
    };

    /**
     * @note dependency from the source code of wallet2
     */
    struct transfer_details
    {
        uint64_t m_block_height;
        cryptonote::transaction_prefix m_tx;
        crypto::hash m_txid;
        uint64_t m_internal_output_index;
        uint64_t m_global_output_index;
        bool m_spent;
        bool m_frozen;
        uint64_t m_spent_height;
        crypto::key_image m_key_image; //TODO: key_image stored twice :(
        rct::key m_mask;
        uint64_t m_amount;
        bool m_rct;
        bool m_key_image_known;
        bool m_key_image_request; // view wallets: we want to request it; cold wallets: it was requested
        uint64_t m_pk_index;
        cryptonote::subaddress_index m_subaddr_index;
        bool m_key_image_partial;
        std::vector<rct::key> m_multisig_k;
        std::vector<multisig_info> m_multisig_info; // one per other participant
        std::vector<std::pair<uint64_t, crypto::hash>> m_uses;

        bool is_rct() const { return m_rct; }
        uint64_t amount() const { return m_amount; }
        const crypto::public_key get_public_key() const {
            crypto::public_key output_public_key;
            if(m_tx.vout.size() <= m_internal_output_index)
                throw ots::exception::wallet::InternalError("Too few outputs, outputs may be corrupted");
            if(!get_output_public_key(m_tx.vout[m_internal_output_index], output_public_key))
                throw ots::exception::wallet::InternalError("Unable to get output public key from output");
            return output_public_key;
        };

        BEGIN_SERIALIZE_OBJECT()
            FIELD(m_block_height)
            FIELD(m_tx)
            FIELD(m_txid)
            FIELD(m_internal_output_index)
            FIELD(m_global_output_index)
            FIELD(m_spent)
            FIELD(m_frozen)
            FIELD(m_spent_height)
            FIELD(m_key_image)
            FIELD(m_mask)
            FIELD(m_amount)
            FIELD(m_rct)
            FIELD(m_key_image_known)
            FIELD(m_key_image_request)
            FIELD(m_pk_index)
            FIELD(m_subaddr_index)
            FIELD(m_key_image_partial)
            FIELD(m_multisig_k)
            FIELD(m_multisig_info)
            FIELD(m_uses)
            END_SERIALIZE()
    };

    /**
     * @note dependency from the source code of wallet2
     */
    struct exported_transfer_details
    {
        crypto::public_key m_pubkey;
        uint64_t m_internal_output_index;
        uint64_t m_global_output_index;
        crypto::public_key m_tx_pubkey;
        union
        {
            struct
            {
                uint8_t m_spent: 1;
                uint8_t m_frozen: 1;
                uint8_t m_rct: 1;
                uint8_t m_key_image_known: 1;
                uint8_t m_key_image_request: 1; // view wallets: we want to request it; cold wallets: it was requested
                uint8_t m_key_image_partial: 1;
            };
            uint8_t flags;
        } m_flags;
        uint64_t m_amount;
        std::vector<crypto::public_key> m_additional_tx_keys;
        uint32_t m_subaddr_index_major;
        uint32_t m_subaddr_index_minor;

        BEGIN_SERIALIZE_OBJECT()
            VERSION_FIELD(1)
            if (version < 1)
                return false;
        FIELD(m_pubkey)
            VARINT_FIELD(m_internal_output_index)
            VARINT_FIELD(m_global_output_index)
            FIELD(m_tx_pubkey)
            FIELD(m_flags.flags)
            VARINT_FIELD(m_amount)
            FIELD(m_additional_tx_keys)
            VARINT_FIELD(m_subaddr_index_major)
            VARINT_FIELD(m_subaddr_index_minor)
            END_SERIALIZE()
    };

    /**
     * @note dependency from the source code of wallet2
     */
    struct tx_construction_data
    {
        std::vector<cryptonote::tx_source_entry> sources;
        cryptonote::tx_destination_entry change_dts;
        std::vector<cryptonote::tx_destination_entry> splitted_dsts; // split, includes change
        std::vector<size_t> selected_transfers;
        std::vector<uint8_t> extra;
        uint64_t unlock_time;
        bool use_rct;
        rct::RCTConfig rct_config;
        bool use_view_tags;
        std::vector<cryptonote::tx_destination_entry> dests; // original setup, does not include change
        uint32_t subaddr_account;   // subaddress account of your wallet to be used in this transfer
        std::set<uint32_t> subaddr_indices;  // set of address indices used as inputs in this transfer

        enum construction_flags_ : uint8_t
        {
            _use_rct          = 1 << 0, // 00000001
            _use_view_tags    = 1 << 1  // 00000010
                                        // next flag      = 1 << 2  // 00000100
                                        // ...
                                        // final flag     = 1 << 7  // 10000000
        };
        uint8_t construction_flags;

        BEGIN_SERIALIZE_OBJECT()
            FIELD(sources)
            FIELD(change_dts)
            FIELD(splitted_dsts)
            FIELD(selected_transfers)
            FIELD(extra)
            FIELD(unlock_time)

            // converted `use_rct` field into construction_flags when view tags
            // were introduced to maintain backwards compatibility
            if (!typename Archive<W>::is_saving())
            {
                FIELD_N("use_rct", construction_flags)
                    use_rct = (construction_flags & _use_rct) > 0;
                use_view_tags = (construction_flags & _use_view_tags) > 0;
            }
            else
            {
                construction_flags = 0;
                if (use_rct)
                    construction_flags ^= _use_rct;
                if (use_view_tags)
                    construction_flags ^= _use_view_tags;

                FIELD_N("use_rct", construction_flags)
            }

        FIELD(rct_config)
            FIELD(dests)
            FIELD(subaddr_account)
            FIELD(subaddr_indices)
            END_SERIALIZE()
    };
    /**
     * @note dependency from the source code of wallet2
     *
     * Original comment:
     * The term "Unsigned tx" is not really a tx since it's not signed yet.
     * It doesnt have tx hash, key and the integrated address is not
     * separated into addr + payment id.
     */
    struct unsigned_tx_set
    {
        std::vector<tx_construction_data> txes;
        std::tuple<uint64_t, uint64_t, transfer_container> transfers;
        std::tuple<uint64_t, uint64_t, std::vector<exported_transfer_details>> new_transfers;

        BEGIN_SERIALIZE_OBJECT()
            VERSION_FIELD(2)
            FIELD(txes)
            if (version == 0)
            {
                std::pair<size_t, transfer_container> v0_transfers;
                FIELD(v0_transfers);
                std::get<0>(transfers) = std::get<0>(v0_transfers);
                std::get<1>(transfers) = std::get<0>(v0_transfers) + std::get<1>(v0_transfers).size();
                std::get<2>(transfers) = std::get<1>(v0_transfers);
                return true;
            }
        if (version == 1)
        {
            std::pair<size_t, std::vector<exported_transfer_details>> v1_transfers;
            FIELD(v1_transfers);
            std::get<0>(new_transfers) = std::get<0>(v1_transfers);
            std::get<1>(new_transfers) = std::get<0>(v1_transfers) + std::get<1>(v1_transfers).size();
            std::get<2>(new_transfers) = std::get<1>(v1_transfers);
            return true;
        }

        FIELD(new_transfers)
            END_SERIALIZE()
    };

    /**
     * @note dependency from the source code of wallet2
     */
    struct multisig_sig
    {
        rct::rctSig sigs;
        std::unordered_set<crypto::public_key> ignore;
        std::unordered_set<rct::key> used_L;
        std::unordered_set<crypto::public_key> signing_keys;
        rct::multisig_out msout;

        rct::keyM total_alpha_G;
        rct::keyM total_alpha_H;
        rct::keyV c_0;
        rct::keyV s;

        BEGIN_SERIALIZE_OBJECT()
            VERSION_FIELD(1)
            if (version < 1)
                return false;
        FIELD(sigs)
            FIELD(ignore)
            FIELD(used_L)
            FIELD(signing_keys)
            FIELD(msout)
            FIELD(total_alpha_G)
            FIELD(total_alpha_H)
            FIELD(c_0)
            FIELD(s)
            END_SERIALIZE()
    };

    /**
     * @note dependency from the source code of wallet2
     *
     * Original comment:
     * The convention for destinations is:
     * dests does not include change
     * splitted_dsts (in construction_data) does
     */
    struct pending_tx
    {
        cryptonote::transaction tx;
        uint64_t dust, fee;
        bool dust_added_to_fee;
        cryptonote::tx_destination_entry change_dts;
        std::vector<size_t> selected_transfers;
        std::string key_images;
        crypto::secret_key tx_key;
        std::vector<crypto::secret_key> additional_tx_keys;
        std::vector<cryptonote::tx_destination_entry> dests;
        std::vector<multisig_sig> multisig_sigs;
        crypto::secret_key multisig_tx_key_entropy;

        tx_construction_data construction_data;

        BEGIN_SERIALIZE_OBJECT()
            VERSION_FIELD(1)
            FIELD(tx)
            FIELD(dust)
            FIELD(fee)
            FIELD(dust_added_to_fee)
            FIELD(change_dts)
            FIELD(selected_transfers)
            FIELD(key_images)
            FIELD(tx_key)
            FIELD(additional_tx_keys)
            FIELD(dests)
            FIELD(construction_data)
            FIELD(multisig_sigs)
            if (version < 1)
            {
                multisig_tx_key_entropy = crypto::null_skey;
                return true;
            }
        FIELD(multisig_tx_key_entropy)
            END_SERIALIZE()
    };

    /**
     * @note dependency from the source code of wallet2
     */
    struct signed_tx_set
    {
        std::vector<pending_tx> ptx;
        std::vector<crypto::key_image> key_images;
        std::unordered_map<crypto::public_key, crypto::key_image> tx_key_images;

        BEGIN_SERIALIZE_OBJECT()
            VERSION_FIELD(0)
            FIELD(ptx)
            FIELD(key_images)
            FIELD(tx_key_images)
            END_SERIALIZE()
    };

    /**
     * @note dependency from the source code of wallet2
     */
    struct tx_scan_info_t
    {
        cryptonote::keypair in_ephemeral;
        crypto::key_image ki;
        rct::key mask;
        uint64_t amount;
        uint64_t money_transfered;
        bool error;
        boost::optional<cryptonote::subaddress_receive_info> received;

        tx_scan_info_t(): amount(0), money_transfered(0), error(true) {}
    };

    /**
     * @note dependency from the source code of wallet2
     */
    struct is_out_data
    {
        crypto::public_key pkey;
        crypto::key_derivation derivation;
        std::vector<boost::optional<cryptonote::subaddress_receive_info>> received;
    };

    typedef std::vector<transfer_details> transfer_container;
}
