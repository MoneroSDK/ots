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
                2, 100000000000
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
            {0, 0, "42q2w6k8pcWiYViMdnqPRTQeyigWkJmBr1LsHh5KPYKrFWFZniZKgTTbx8ePnK4yHpFSbijdAHqkM9iGoVsFhtqTHsxHyBq"},
            {0, 1, "88sbRotyapZKUXVErLMoU1d4SafnuTmbCA8A3ZpA6qP7bcXPB4Sp7ccTTFmryScyXDCW7rDkCdrBCFiC4Xi5zzTm3vqEJYX"},
            {0, 2, "899Kz2eNbywXAX8ZRc8YvBVJRkc4r6hPFhJVro1B4qYCGTeour4aUeYVn5NfSP1fXn28E8vzNN6p5N98eJvfc87HEsXSs5i"},
            {0, 3, "89z8ra3ev3vbNHZZHXA4NLBLYv7o5j1y9hsGWYtL6orDM58TSVh5XDj4K5Ng6AKt2J4ibJkADkGPUV4LqFDCdjbz4CyVLsi"},
            {0, 4, "8982UMcvdzLikWMdBYh9ELK8HV2nHve3bYBvsk1EgV6wcrSXk3Na76rgtFcrCnB84c8EBVTzoJ4pWfKQBMuPk5ktCcrFdMG"},
            {0, 5, "884koVcaU39FFAXbu6UNe9365oksg6oM82g2yiun4xKTSQE8GDpJFnXTzcAG9LDSgNdEEX4da6X8o4JGHhwrDW9uK8XrZ2B"},
            {0, 6, "87kjQsoZZfY45oQm4mLGK91Lm13YV5dqx5Tzw5SrnwA6HsMTkS8AAHFUY4W81PDmknTwEZHrhVvbeFcdwjZmkusPKd7RGCv"},
            {0, 7, "88BGVECSKB879ZqgFUNbMx7eRKcAswu7pjTFMx1XYgsujQnr5epVxTAZW6Jsn1Yns8GLmtbQHAahDFznsMgD9V3hJHeouEH"},
            {0, 8, "82mgRPRyHNkAVu5wykzjLdaMeWRdXXLtpRfUN4vHCXcL9bDZYduTrugXvmc5D36zMCFeAj7m4B2nfZUSNPBWhv22S9vPzYJ"},
            {0, 9, "8BsgwkhykzTdkwzbZt42CW9kaaTTgpo2wXDGUf9CdPTvihc4W9PkaypaRktgUwbWv2hvR9Qm1T2ypawxzTcY25r9MxLJ1LP"},
            {0, 10, "87swuHwsfE2ZSSDfqXtcyrYoVryd7hMqLHHKWnvK9qyR5ivHDVp2GLNb3XbiQRXUHT6W722RNB2rJEJ5RjL4TM9DTS2zRQG"},
            {1, 0, std::string("85xEj65Pu4x7Ktwghic7tMbNN5JvFjueNae4FWqkmMctfSXoUUueTPuVrYDGsCYmUJ1yZVETweSd5PxRYxUbm4PkCS7k6Ly")},
            {1, 1, "89ZbpoeNghXUSa3AMS3e1vFrzfNv7dBm7f6ZdAzGo9bvYEkPqxiHsM6Ffwou3J6HfXNEN75RNqu6wZbX8Yb4j5gD1w6nU9b"},
            {1, 2, "8Bz74GgPfKsd2ivLStEVkEfpJkBASCsFhNinPSUmWhUyLh8wUfFkT8hiEXNjbLjj1AcXTpA7GZb3iS1EjjnnhstdG2uYUiB"},
            {1, 3, "89FUqQD4LHTHw8gKLULxX5SrEg5hEXi6R9ndxiwQ1JcrX1rNyyidc5NLpAQqygXXzvBVf9TMSVT7rEMikWNsAWq96gB9KYK"},
            {1, 4, "89HVDTzdrPq345pPd673UVbqatboHZHKkdmqr5gtPxcZgSZ36sfSVg2dJECsky9jgiH77yDCUJq5SC9TFAt1Jhg7C3isiG4"},
            {1, 5, "8Buh2YYomkVbMWefsCdfJFLzBkftxHSMkUuBadSRwMCkNRm9eDkdT5KTojYZ9TVeXL82tmx2FJPw6gvo8QtJe8YeVNXGe2c"},
            {1, 6, "85hgaZiP1XyEV1RMuDydrGPKNNt4BSmXuEciKzLa1Zbdb73cN6XqeKNBBQSEACM25u2jJHRGF6Cx1J3JdKD2NjLi3q4vuCj"},
            {1, 7, "84BA1QoJD7AQjUZ9ibemY7F6VAPmy9sVjCtFyf7iVY85dHVF7RN7ircgV6CcWj6jpkKT1VnxuEzzM7mxYv6V53ApPN2U9kj"},
            {1, 8, "8C2ajqaaZXNa3MnqEr1z13XRnwaZystp9UW4WSrWDT1hQbFDy2Qm7Wa2MwzhJFroXsJRoSwbPqmPe7PjLSTTGjYcPmKZVJL"},
            {1, 9, "87oSafoY8uQYd4cXezUa2dTnsVEcFKBcVLeNjZhx646NJkzC8BgUiBNGsjzbkVRfqgeDgLanT8M68PyxFxSnsGCFA37E4MD"},
            {1, 10, "83jeYf23YB2MXf3Lh7A1Qh7v5JRyMjbxAUQprRWXpqLe5t41fkvmx28biu3jbUHzTVCLy1QyGjRZATySYFKw6SKs3MUpxSy"},
            {2, 0, "85sjLZM4U2iM5BaiBAfYmPhiKxTLfNYNvQTEo8Tdi5RHBFijjq9Bpu6F7rYG1ZdvEPWq5jWnXs7X8DYhZUTWFaeMRW7GVRy"},
            {2, 1, "89kzz1bB4uiKGE8hqZgbPDcsddPTgpbXd8zaUnHqdxRPdAT1xdJ4v4zBBqomGPwrf4eET7gcxzQhDScip2UR7tBY91xc1Gq"},
            {2, 2, "84TxJpYwLCUPiQ7KvYzuQD3VMz5tC1AMEYm9iAUFgHSud6LKodM3bpeV6oaQXJYKhNA2oEtSPc4u46MfJvxfHtDpSjJZydk"},
            {2, 3, "88zN97Sv9ut9b4vn1CU2VxZJxRMB9Yr7h6mfhpSr8R3zbZHXtWrfGvgTWNsXPazqqTi2Fy7tZLWQ8UX9qQBefzJrNPTr3BP"},
            {2, 4, "83adH77bLT477Hy4nthpdBNkhPFmg5VZHWR78MWCtViuiavDNpoVWHy3MkDMMGLgJNjAs2arSFgAu4zTCv2qasHULgxYUgs"},
            {2, 5, "89LhVmEBjpjZDTpGbw3m4jJqwE6emnfnriEKm2YHnNzycjnKqJea9PXCvKAWcRE1VfbXn1qNzjiTdjSjVWvpVw614Fw5riS"},
            {2, 6, "8BHnK5SDAfdAn6NoVWZKFMH84euWZPZjydVPFUqp7B6p23wESeNAyfh3CX282iCgSAU1UVjkfxB5nTHCzLA4bg899KCutLd"},
            {2, 7, "85qwyGHbDppiLkLBE5DgDNLNDDxVMBKSXhqzxTHcRbazjcqFxL7svk7gVdahGrhsdZ9Xsrgw2WYBFhRdVKu2ZoSvBCuGWmm"},
            {2, 8, "89guMR95R7fWJLJ2mYBaL8jHwxfq829YbZu6nukeoZWg74249H1egjqZgahGYdQ5vXM2cuUGaB2hGjeoaNBMwR2aDy3Qn14"},
            {2, 9, "89imHpo1oroMXFRSiWpRSzJ3NpHuduq3BZ5F7d55KLB3fj8D1uBTEUm7isLQfwsQi4iok7sjkD7Yb2Fm7D4cZU7N8K9ak57"},
            {2, 10, "86rjkJeKFRL4hdoHwdB9TBj7Z2quzz8vwRAsT4ya445zPQ8ShggkUTVARaXrEPNiPYM9EaVhdWrNteQguhDAtwtF2MfKm7G"}
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
