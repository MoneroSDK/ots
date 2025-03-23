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
};

const std::vector<WalletTestCase> wallet_test_cases = {
    {"valid wallet with outputs", ots::SeedType::Monero, 5, std::string(WALLET_092F1B_OUTPUTS)}
};
