#pragma once

#include "account-dependencies.hpp"
#include <memory>
#include <tuple>
#include <serialization/serialization.h> // For serialize support
#include <serialization/variant.h>   // For tools::write_varint
#include <serialization/tuple.h>
#include <serialization/pair.h>
#include <serialization/string.h>
#include <boost/serialization/vector.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
// #if BOOST_VERSION >= 107400
#include <boost/serialization/library_version_type.hpp>
// #endif
#include <boost/serialization/list.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/thread/lock_guard.hpp>
#include <atomic>
#include <random>

#include "include_base_utils.h"
#include "cryptonote_basic/account.h"
#include "cryptonote_basic/account_boost_serialization.h"
#include "cryptonote_basic/cryptonote_basic_impl.h"
#include "net/http.h"
#include "cryptonote_basic/cryptonote_format_utils.h"
// #include "cryptonote_core/cryptonote_tx_utils.h"
#include "common/unordered_containers_boost_serialization.h"
#include "common/util.h"
#include "crypto/chacha.h"
#include "crypto/hash.h"
#include "multisig/multisig_account.h"
#include "ringct/rctTypes.h"
#include "ringct/rctOps.h"
#include "serialization/crypto.h"
#include "serialization/string.h"
#include "serialization/pair.h"
#include "serialization/tuple.h"
#include "serialization/containers.h"

BOOST_CLASS_VERSION(ots::transfer_details, 12)
// BOOST_CLASS_VERSION(ots::payment_details, 5)
// BOOST_CLASS_VERSION(tools::wallet2::unconfirmed_transfer_details, 8)
// BOOST_CLASS_VERSION(tools::wallet2::confirmed_transfer_details, 6)
BOOST_CLASS_VERSION(ots::unsigned_tx_set, 1)
BOOST_CLASS_VERSION(ots::signed_tx_set, 1)
BOOST_CLASS_VERSION(ots::tx_construction_data, 4)
BOOST_CLASS_VERSION(ots::pending_tx, 3)

namespace boost
{
    namespace serialization
    {
        template<class Archive, class F, class S, class T>
            inline void serialize(
                    Archive & ar,
                    std::tuple<F, S, T> & t,
                    const unsigned int /* file_version */
                    ){
                ar & ::boost::serialization::make_nvp("f", std::get<0>(t));
                ar & ::boost::serialization::make_nvp("s", std::get<1>(t));
                ar & ::boost::serialization::make_nvp("t", std::get<2>(t));
            }

        template <class Archive>
            inline typename std::enable_if<!Archive::is_loading::value, void>::type initialize_transfer_details(Archive &a, ots::transfer_details &x, const boost::serialization::version_type ver)
            {
            }
        template <class Archive>
            inline typename std::enable_if<Archive::is_loading::value, void>::type initialize_transfer_details(Archive &a, ots::transfer_details &x, const boost::serialization::version_type ver)
            {
                if (ver < 1)
                {
                    x.m_mask = rct::identity();
                    x.m_amount = x.m_tx.vout[x.m_internal_output_index].amount;
                }
                if (ver < 2)
                {
                    x.m_spent_height = 0;
                }
                if (ver < 4)
                {
                    x.m_rct = x.m_tx.vout[x.m_internal_output_index].amount == 0;
                }
                if (ver < 6)
                {
                    x.m_key_image_known = true;
                }
                if (ver < 7)
                {
                    x.m_pk_index = 0;
                }
                if (ver < 8)
                {
                    x.m_subaddr_index = {};
                }
                if (ver < 9)
                {
                    x.m_key_image_partial = false;
                    x.m_multisig_k.clear();
                    x.m_multisig_info.clear();
                }
                if (ver < 10)
                {
                    x.m_key_image_request = false;
                }
                if (ver < 12)
                {
                    x.m_frozen = false;
                }
            }

        template <class Archive>
            inline void serialize(Archive &a, ots::transfer_details &x, const boost::serialization::version_type ver)
            {
                a & x.m_block_height;
                a & x.m_global_output_index;
                a & x.m_internal_output_index;
                if (ver < 3)
                {
                    cryptonote::transaction tx;
                    a & tx;
                    x.m_tx = (const cryptonote::transaction_prefix&)tx;
                    x.m_txid = cryptonote::get_transaction_hash(tx);
                }
                else
                {
                    a & x.m_tx;
                }
                a & x.m_spent;
                a & x.m_key_image;
                if (ver < 1)
                {
                    // ensure mask and amount are set
                    initialize_transfer_details(a, x, ver);
                    return;
                }
                a & x.m_mask;
                a & x.m_amount;
                if (ver < 2)
                {
                    initialize_transfer_details(a, x, ver);
                    return;
                }
                a & x.m_spent_height;
                if (ver < 3)
                {
                    initialize_transfer_details(a, x, ver);
                    return;
                }
                a & x.m_txid;
                if (ver < 4)
                {
                    initialize_transfer_details(a, x, ver);
                    return;
                }
                a & x.m_rct;
                if (ver < 5)
                {
                    initialize_transfer_details(a, x, ver);
                    return;
                }
                if (ver < 6)
                {
                    // v5 did not properly initialize
                    uint8_t u;
                    a & u;
                    x.m_key_image_known = true;
                    return;
                }
                a & x.m_key_image_known;
                if (ver < 7)
                {
                    initialize_transfer_details(a, x, ver);
                    return;
                }
                a & x.m_pk_index;
                if (ver < 8)
                {
                    initialize_transfer_details(a, x, ver);
                    return;
                }
                a & x.m_subaddr_index;
                if (ver < 9)
                {
                    initialize_transfer_details(a, x, ver);
                    return;
                }
                a & x.m_multisig_info;
                a & x.m_multisig_k;
                a & x.m_key_image_partial;
                if (ver < 10)
                {
                    initialize_transfer_details(a, x, ver);
                    return;
                }
                a & x.m_key_image_request;
                if (ver < 11)
                {
                    initialize_transfer_details(a, x, ver);
                    return;
                }
                a & x.m_uses;
                if (ver < 12)
                {
                    initialize_transfer_details(a, x, ver);
                    return;
                }
                a & x.m_frozen;
            }

        /*
           template <class Archive>
           inline void serialize(Archive& a, ots::payment_details& x, const boost::serialization::version_type ver)
           {
           a & x.m_tx_hash;
           a & x.m_amount;
           a & x.m_block_height;
           a & x.m_unlock_time;
           if (ver < 1)
           return;
           a & x.m_timestamp;
           if (ver < 2)
           {
           x.m_coinbase = false;
           x.m_subaddr_index = {};
           return;
           }
           a & x.m_subaddr_index;
           if (ver < 3)
           {
           x.m_coinbase = false;
           x.m_fee = 0;
           return;
           }
           a & x.m_fee;
           if (ver < 4)
           {
           x.m_coinbase = false;
           return;
           }
           a & x.m_coinbase;
           if (ver < 5)
           return;
           a & x.m_amounts;
           }
           */
        template <class Archive>
            inline void serialize(Archive &a, ots::unsigned_tx_set &x, const boost::serialization::version_type ver)
            {
                a & x.txes;
                if (ver == 0)
                {
                    // load old version
                    std::pair<size_t, ots::transfer_container> old_transfers;
                    a & old_transfers;
                    std::get<0>(x.transfers) = std::get<0>(old_transfers);
                    std::get<1>(x.transfers) = std::get<0>(old_transfers) + std::get<1>(old_transfers).size();
                    std::get<2>(x.transfers) = std::get<1>(old_transfers);
                    return;
                }
                throw std::runtime_error("Boost serialization not supported for newest unsigned_tx_set");
            }

        template <class Archive>
            inline void serialize(Archive &a, ots::signed_tx_set &x, const boost::serialization::version_type ver)
            {
                a & x.ptx;
                a & x.key_images;
                if (ver < 1)
                    return;
                a & x.tx_key_images;
            }

        template <class Archive>
            inline void serialize(Archive &a, ots::tx_construction_data &x, const boost::serialization::version_type ver)
            {
                a & x.sources;
                a & x.change_dts;
                a & x.splitted_dsts;
                if (ver < 2)
                {
                    // load list to vector
                    std::list<size_t> selected_transfers;
                    a & selected_transfers;
                    x.selected_transfers.clear();
                    x.selected_transfers.reserve(selected_transfers.size());
                    for (size_t t: selected_transfers)
                        x.selected_transfers.push_back(t);
                }
                a & x.extra;
                a & x.unlock_time;
                a & x.use_rct;
                a & x.dests;
                if (ver < 1)
                {
                    x.subaddr_account = 0;
                    return;
                }
                a & x.subaddr_account;
                a & x.subaddr_indices;
                if (ver < 2)
                {
                    if (!typename Archive::is_saving())
                        x.rct_config = { rct::RangeProofBorromean, 0 };
                    return;
                }
                a & x.selected_transfers;
                if (ver < 3)
                {
                    if (!typename Archive::is_saving())
                        x.rct_config = { rct::RangeProofBorromean, 0 };
                    return;
                }
                if (ver < 4)
                {
                    bool use_bulletproofs = x.rct_config.range_proof_type != rct::RangeProofBorromean;
                    a & use_bulletproofs;
                    if (!typename Archive::is_saving())
                        x.rct_config = { use_bulletproofs ? rct::RangeProofBulletproof : rct::RangeProofBorromean, 0 };
                    return;
                }
                a & x.rct_config;
            }

        template <class Archive>
            inline void serialize(Archive &a, ots::multisig_sig &x, const boost::serialization::version_type ver)
            {
                a & x.sigs;
                a & x.ignore;
                a & x.used_L;
                a & x.signing_keys;
                a & x.msout;
                if (ver < 1)
                    return;
                a & x.total_alpha_G;
                a & x.total_alpha_H;
                a & x.c_0;
                a & x.s;
            }

        template <class Archive>
            inline void serialize(Archive &a, ots::pending_tx &x, const boost::serialization::version_type ver)
            {
                a & x.tx;
                a & x.dust;
                a & x.fee;
                a & x.dust_added_to_fee;
                a & x.change_dts;
                if (ver < 2)
                {
                    // load list to vector
                    std::list<size_t> selected_transfers;
                    a & selected_transfers;
                    x.selected_transfers.clear();
                    x.selected_transfers.reserve(selected_transfers.size());
                    for (size_t t: selected_transfers)
                        x.selected_transfers.push_back(t);
                }
                a & x.key_images;
                a & x.tx_key;
                a & x.dests;
                a & x.construction_data;
                if (ver < 1)
                    return;
                a & x.additional_tx_keys;
                if (ver < 2)
                    return;
                a & x.selected_transfers;
                if (ver < 3)
                    return;
                a & x.multisig_sigs;
            }
    } // namespace serialization
} // namespace boost
