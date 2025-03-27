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

struct WalletTestCase {
    const std::string name;
    const ots::SeedType seed_type = ots::SeedType::Monero;
    const size_t seed_test_case = 0;
    const std::string outputs = "";
    const uint64_t outputs_count = 0;
    const bool valid = true;
};

const std::vector<WalletTestCase> wallet_test_cases = {
    {"valid wallet with invalid outputs", ots::SeedType::Monero, 5, std::string(WALLET_E00000_OUTPUTS), 0, false},
    {"valid wallet with defective outputs", ots::SeedType::Monero, 5, std::string(WALLET_E00001_OUTPUTS), 0, false},
    {"valid wallet with valid outputs", ots::SeedType::Monero, 5, std::string(WALLET_092F1B_OUTPUTS), 13},
    {"valid wallet with outputs for other wallet", ots::SeedType::Monero, 3, std::string(WALLET_092F1B_OUTPUTS), 13, false}
};
