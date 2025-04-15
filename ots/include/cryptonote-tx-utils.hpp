#pragma once

/**
 * @file cryptonote-tx-utils.hpp
 * @brief substituting functions in `<cryptonote_core/cryptonote_tx_utils.h>`
 *
 * using functions from `<cryptonote_core/cryptonote_tx_utils.h>`
 * to construct transactions drags more dependencies inside, like
 * blockchain.cpp which itself drags in a lot of other dependencies
 * which clearly are not needed for offline transactions signing.
 */

#include <crypto/crypto.h>
#include <cryptonote_basic/cryptonote_basic.h>
#include <cryptonote_core/cryptonote_tx_utils.h> // For tx_source_entry, tx_destination_entry

namespace ots {

    std::pair<crypto::secret_key, std::vector<crypto::secret_key>> constructTxAndGetTxKey(
        const cryptonote::account_keys& senderAccountKeys,
        const std::unordered_map<crypto::public_key, cryptonote::subaddress_index>& subaddresses,
        std::vector<cryptonote::tx_source_entry>& sources,
        std::vector<cryptonote::tx_destination_entry>& destinations,
        const boost::optional<cryptonote::account_public_address>& changeAddress,
        const std::vector<uint8_t> &extra,
        cryptonote::transaction& tx,
        bool rct,
        const rct::RCTConfig &rct_config,
        bool use_view_tags
    );

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
        bool rct,
        const rct::RCTConfig &rct_config,
        bool shuffle_outs,
        bool use_view_tags
    );

    std::tuple<
        size_t, /** @brief count of standard addresses */
        size_t, /** @brief count of subaddresses */
        cryptonote::account_public_address /** @brief single destination subaddress */
    > classifyAddresses(
        const std::vector<cryptonote::tx_destination_entry> &destinations,
        const boost::optional<cryptonote::account_public_address>& changeAddress
    );

    crypto::public_key getDestinationViewKeyPub(
        const std::vector<cryptonote::tx_destination_entry> &destinations,
        const boost::optional<cryptonote::account_public_address>& changeAddress
    );
} // namespace ots
