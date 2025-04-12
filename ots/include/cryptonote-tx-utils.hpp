#pragma once

#include <crypto/crypto.h>
#include <cryptonote_basic/cryptonote_basic.h>
#include <cryptonote_core/cryptonote_tx_utils.h> // For tx_source_entry, tx_destination_entry

namespace ots {

    bool constructTxAndGetTxKey(
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
    );

    bool constructTxWithTxKey(
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
            bool use_view_tags);

    void classifyAddresses(
        const std::vector<cryptonote::tx_destination_entry> &destinations,
        const boost::optional<cryptonote::account_public_address>& change_addr,
        size_t &num_stdaddresses,
        size_t &num_subaddresses,
        cryptonote::account_public_address &single_dest_subaddress
    );

    crypto::public_key getDestinationViewKeyPub(
        const std::vector<cryptonote::tx_destination_entry> &destinations,
        const boost::optional<cryptonote::account_public_address>& change_addr
    );
} // namespace ots
