#include <vector>
#include "ots.hpp"

struct LegacySeedTestCase {
    const std::string name;
    const std::string phrase;
    const uint64_t height = 0;
    const uint64_t time = 0;
    const uint64_t expected_height = 0;
    const uint64_t expected_time = 0;
    const ots::Network network = ots::Network::MAIN;
    const std::string address = "";
    const std::string fingerprint = "";
    const bool valid = true;
    const std::string secret_spend_key = "";
    const std::string public_spend_key = "";
    const std::string secret_view_key = "";
    const std::string public_view_key = "";
    const std::string lang_code = "en";
};

const std::vector<LegacySeedTestCase> legacy_seed_test_cases = {
    {"empty seed", "", 0, 0, 0, 0, ots::Network::MAIN, "", "", false},
    {"invalid seed", "invalid seed phrase", 0, 0, 0, 0, ots::Network::MAIN, "", "", false},
    {
        "valid seed mainnet",
        "turnip nearby oneself jaws sovereign tolerant natural bugs object ingested zeal plotting jaws",
        0, 0, 0, 0,
        ots::Network::MAIN,
        "44ZUMETt4NQ5zkjByUSn8a9qwFYpFqGmtJNP8daZsdZs5nf8wkejUApSNjzQvpGtvF3baiVvoeqkVEU8x1if84Qi6P5A4dT", "3E6B71",
        true,
        "714f6346e0620fe7c529029af7180825192c2ea723fd1c6cc25b8a48a0496d0d", "4d82ba0300c17d1dde2059a73ef72334df83df1b29f0c767d721e82f9a0e721c",
        "01b155580782892daf740e5f723a0ab0c169c36f2f63f768c44a42b167e47008", "9eec38d60e17c597b4d07ad13008e40f8597600b3b95e25084e9e399b985632f"
    },
    {
        "valid seed mainnet (without checksum)",
        "turnip nearby oneself jaws sovereign tolerant natural bugs object ingested zeal plotting",
        0, 0, 0, 0,
        ots::Network::MAIN,
        "44ZUMETt4NQ5zkjByUSn8a9qwFYpFqGmtJNP8daZsdZs5nf8wkejUApSNjzQvpGtvF3baiVvoeqkVEU8x1if84Qi6P5A4dT", "3E6B71",
        true,
        "714f6346e0620fe7c529029af7180825192c2ea723fd1c6cc25b8a48a0496d0d", "4d82ba0300c17d1dde2059a73ef72334df83df1b29f0c767d721e82f9a0e721c",
        "01b155580782892daf740e5f723a0ab0c169c36f2f63f768c44a42b167e47008", "9eec38d60e17c597b4d07ad13008e40f8597600b3b95e25084e9e399b985632f"
    },
    {
        "valid seed mainnet (invalid checksum)",
        "",
        0, 0, 0, 0,
        ots::Network::MAIN,
        "turnip nearby oneself jaws sovereign tolerant natural bugs object ingested zeal plotting zeal", "3E6B71",
        false,
        "714f6346e0620fe7c529029af7180825192c2ea723fd1c6cc25b8a48a0496d0d", "4d82ba0300c17d1dde2059a73ef72334df83df1b29f0c767d721e82f9a0e721c",
        "01b155580782892daf740e5f723a0ab0c169c36f2f63f768c44a42b167e47008", "9eec38d60e17c597b4d07ad13008e40f8597600b3b95e25084e9e399b985632f"
    }
};
