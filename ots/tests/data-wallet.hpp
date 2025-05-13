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

struct WalletSubAddress {
    const uint32_t account;
    const uint32_t index;
    const std::string address;
};

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
    const std::vector<WalletSubAddress> subaddresses = {}; // subaddress and account
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
        },
        {
            {0, 1, std::string("88sbRotyapZKUXVErLMoU1d4SafnuTmbCA8A3ZpA6qP7bcXPB4Sp7ccTTFmryScyXDCW7rDkCdrBCFiC4Xi5zzTm3vqEJYX")},
            {0, 2, "899Kz2eNbywXAX8ZRc8YvBVJRkc4r6hPFhJVro1B4qYCGTeour4aUeYVn5NfSP1fXn28E8vzNN6p5N98eJvfc87HEsXSs5i"},
            {1, 0, std::string("85xEj65Pu4x7Ktwghic7tMbNN5JvFjueNae4FWqkmMctfSXoUUueTPuVrYDGsCYmUJ1yZVETweSd5PxRYxUbm4PkCS7k6Ly")},
            {1, 1, "89ZbpoeNghXUSa3AMS3e1vFrzfNv7dBm7f6ZdAzGo9bvYEkPqxiHsM6Ffwou3J6HfXNEN75RNqu6wZbX8Yb4j5gD1w6nU9b"},
            {1, 2, "8Bz74GgPfKsd2ivLStEVkEfpJkBASCsFhNinPSUmWhUyLh8wUfFkT8hiEXNjbLjj1AcXTpA7GZb3iS1EjjnnhstdG2uYUiB"},
            {2, 0, "85sjLZM4U2iM5BaiBAfYmPhiKxTLfNYNvQTEo8Tdi5RHBFijjq9Bpu6F7rYG1ZdvEPWq5jWnXs7X8DYhZUTWFaeMRW7GVRy"},
            {2, 1, "89kzz1bB4uiKGE8hqZgbPDcsddPTgpbXd8zaUnHqdxRPdAT1xdJ4v4zBBqomGPwrf4eET7gcxzQhDScip2UR7tBY91xc1Gq"},
            {2, 2, "84TxJpYwLCUPiQ7KvYzuQD3VMz5tC1AMEYm9iAUFgHSud6LKodM3bpeV6oaQXJYKhNA2oEtSPc4u46MfJvxfHtDpSjJZydk"}
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
