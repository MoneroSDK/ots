#include <vector>
#include "ots.hpp"

/**
 * @file data-wallet-outputs.hpp is auto generated from *.outputs files
 * @brief Contains the outputs for the wallet test cases
 *
 * WALLET_E00000_OUTPUTS contains random carbage
 * WALLET_E00001_OUTPUTS contains a defective outputs of 092F1B
 * WALLET_092F1B_OUTPUTS is the valid outputs for the wallet 092F1B
 */
#include "data-wallet-outputs.hpp"
#include "data-wallet-unsigned-tx.hpp"
#include "data-wallet-unsigned-tx-json.hpp"

struct WalletOutputs {
    const std::string outputs;
    const uint64_t count;
};

struct WalletUnsignedTransaction {
    const std::string tx;
    const std::string json;
    const size_t forOutputs = 0;
    const uint64_t amount = 0;
    const bool hasChange = true;
};

struct WalletTestCase {
    const std::string name;
    const ots::SeedType seed_type = ots::SeedType::Monero;
    const size_t seed_test_case = 0;
    const std::vector<WalletOutputs> outputs = {}; // outputs and count of the outputs
    const bool valid = true;
    const std::vector<WalletUnsignedTransaction> unsigned_transactions = {}; // tx and forOutputs
};

const std::vector<WalletTestCase> wallet_test_cases = {
    {
        "valid wallet with invalid outputs",
        ots::SeedType::Monero, 5,
        {
            {std::string(WALLET_E00000_OUTPUTS), 0}
        },
        false
    },
    {
        "valid wallet with defective outputs",
        ots::SeedType::Monero, 5,
        {
            {std::string(WALLET_E00001_OUTPUTS), 0}
        },
        false
    },
    {
        "valid wallet with valid outputs",
        ots::SeedType::Monero, 5,
        {
            {std::string(WALLET_092F1B_OUTPUTS), 13},
            {std::string(WALLET_092F1B_01_OUTPUTS), 14},
            {std::string(WALLET_092F1B_02_OUTPUTS), 15},
            {std::string(WALLET_092F1B_03_OUTPUTS), 16}
        },
        true,
        {
            {
                std::string(WALLET_092F1B_UNSIGNED_TX),
                std::string(WALLET_092F1B_UNSIGNED_TX_JSON),
                1, 100000000000
            },
            {
                std::string(WALLET_092F1B_01_UNSIGNED_TX),
                std::string(WALLET_092F1B_01_UNSIGNED_TX_JSON),
                1, 150000000000
            },
            {
                std::string(WALLET_092F1B_02_UNSIGNED_TX),
                std::string(WALLET_092F1B_02_UNSIGNED_TX_JSON),
                1, 20000000000
            },
            {
                std::string(WALLET_092F1B_03_UNSIGNED_TX),
                std::string(WALLET_092F1B_03_UNSIGNED_TX_JSON),
                1, 19799670000,
                false
            }
        }
    },
    {
        "valid wallet with outputs for other wallet",
        ots::SeedType::Monero, 3,
        {
            {std::string(WALLET_092F1B_OUTPUTS), 13}
        },
        false
    },
    {
        "valid empty wallet with outputs",
        ots::SeedType::Monero, 6,
        {
            {std::string(WALLET_08D795_OUTPUTS), 0}
        }
    }
};
