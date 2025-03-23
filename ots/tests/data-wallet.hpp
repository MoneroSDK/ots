#include <vector>
#include "ots.hpp"
#include "data-wallet-092F1B.hpp" // generated from 092F1B.outputs

struct WalletTestCase {
    const std::string name;
    const ots::SeedType seed_type = ots::SeedType::Monero;
    const size_t seed_test_case = 0;
    const std::string outputs = "";
};

const std::vector<WalletTestCase> wallet_test_cases = {
    {"valid wallet with outputs", ots::SeedType::Monero, 5, WALLET_092F1B_OUTPUTS}
};
