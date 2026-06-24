#pragma once

/**
 * @file account-dependencies-rpc.hpp
 * @brief struct dependencies from transplanting source
 *        from wallet-rpc-server
 * @internal temporary file to untagle dependencie
 * @todo TODO: remove this file after dependencies are resolved
 */

#include "account-dependencies.hpp"

namespace ots {
    struct recipient {
        std::string address = "";
        uint64_t amount = 0;

        recipient(std::string address, uint64_t amount)
            : address(address), amount(amount) {};
    };

    struct transfer_description {
        uint64_t amount_in = 0;
        uint64_t amount_out = 0;
        uint32_t ring_size = std::numeric_limits<uint32_t>::max();
        uint64_t unlock_time = 0;
        std::vector<recipient> recipients = {};
        std::string payment_id = "";
        uint64_t change_amount = 0;
        std::string change_address = "";
        uint64_t fee = 0;
        uint32_t dummy_outputs = 0;
        std::string extra = "";
    };

    struct txset_summary {
        uint64_t amount_in = 0;
        uint64_t amount_out = 0;
        std::vector<recipient> recipients;
        uint64_t change_amount = 0;
        std::string change_address = "";
        uint64_t fee = 0;
    };

    struct tx_description {
        std::vector<transfer_description> desc = {};
        struct txset_summary summary = {};
    };
}
