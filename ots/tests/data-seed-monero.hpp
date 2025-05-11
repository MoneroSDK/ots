#include <vector>
#include "ots.hpp"

struct MoneroSeedTestCase {
    const std::string name;
    const std::string phrase;
    const uint64_t height = 0;
    const uint64_t time = 0;
    const uint64_t expected_height = 0;
    const uint64_t expected_time = 0;
    const ots::Network network = ots::Network::MAIN;
    const std::string password = "";
    const bool encrypted = false;
    const std::string address = "";
    const std::string fingerprint = "";
    const bool valid = true;
    const std::string secret_spend_key = "";
    const std::string public_spend_key = "";
    const std::string secret_view_key = "";
    const std::string public_view_key = "";
    const std::string lang_code = "en";
};

const std::vector<MoneroSeedTestCase> monero_seed_test_cases = {
    {"empty seed", "", 0, 0, 0, 0, ots::Network::MAIN, "", false, "", "", false},
    {"invalid seed", "invalid seed phrase", 0, 0, 0, 0, ots::Network::MAIN, "", false, "", "", false},
    {
        "valid seed mainnet",
        "jetting keyboard unwind cupcake cogs adapt otter honked biplane pencil peaches faked daft psychic impel ferry maul siren dwarf optical asleep maul money devoid adapt",
        0, 0, 0, 0,
        ots::Network::MAIN,
        "", false,
        "46pkkPM7tNQZFNZXqWkm7LUxL9F5zSg7i5wPQ7o9aND6QaYgCQi3EACbhTZBiEtLyKYfNVywemmRCWDDH7HkBwZLPNuptsZ", "9DE02D",
        true,
        "c2b2b4cf4827b2693292ed5b710ac3d1500d59ba9fe697419717bb735902bb06", "893d94e3c28e35c0cc03ae33201da7a720374ee7f9af791d8541e0146eba198c",
        "1a33c2e55bb34b293eee308caf7b22e56713ee893df1904c0fbdf6f0c6a44a08", "f747686952bd69cf716e751f833d56bd4ac8660738ca9baea3712ead0363abc6"
    },
    {
        "valid seed mainnet (without checksum)",
        "jetting keyboard unwind cupcake cogs adapt otter honked biplane pencil peaches faked daft psychic impel ferry maul siren dwarf optical asleep maul money devoid",
        0, 0, 0, 0,
        ots::Network::MAIN,
        "", false,
        "46pkkPM7tNQZFNZXqWkm7LUxL9F5zSg7i5wPQ7o9aND6QaYgCQi3EACbhTZBiEtLyKYfNVywemmRCWDDH7HkBwZLPNuptsZ", "9DE02D",
        true,
        "c2b2b4cf4827b2693292ed5b710ac3d1500d59ba9fe697419717bb735902bb06", "893d94e3c28e35c0cc03ae33201da7a720374ee7f9af791d8541e0146eba198c",
        "1a33c2e55bb34b293eee308caf7b22e56713ee893df1904c0fbdf6f0c6a44a08", "f747686952bd69cf716e751f833d56bd4ac8660738ca9baea3712ead0363abc6"
    },
    {
        "valid seed mainnet (invalid checksum)",
        "jetting keyboard unwind cupcake cogs adapt otter honked biplane pencil peaches faked daft psychic impel ferry maul siren dwarf optical asleep maul money devoid unwind",
        0, 0, 0, 0,
        ots::Network::MAIN,
        "", false,
        "46pkkPM7tNQZFNZXqWkm7LUxL9F5zSg7i5wPQ7o9aND6QaYgCQi3EACbhTZBiEtLyKYfNVywemmRCWDDH7HkBwZLPNuptsZ", "9DE02D",
        false,
        "c2b2b4cf4827b2693292ed5b710ac3d1500d59ba9fe697419717bb735902bb06", "893d94e3c28e35c0cc03ae33201da7a720374ee7f9af791d8541e0146eba198c",
        "1a33c2e55bb34b293eee308caf7b22e56713ee893df1904c0fbdf6f0c6a44a08", "f747686952bd69cf716e751f833d56bd4ac8660738ca9baea3712ead0363abc6"
    },
    {
        "wallet test (mainnet)",
        "large nuisance tiger drying runway january cavernous powder adopt boil vehicle tidy talent fetches future trendy usual basin hiker smuggled when pheasants cinema diplomat hiker",
        3164653, 0, 3164653, 1717596972,
        ots::Network::MAIN,
        "", false,
        "42q2w6k8pcWiYViMdnqPRTQeyigWkJmBr1LsHh5KPYKrFWFZniZKgTTbx8ePnK4yHpFSbijdAHqkM9iGoVsFhtqTHsxHyBq", "092F1B",
        true,
        "c16b184475f827b26b76995675f459ec9d2d5207d479eb2c1834732d8286af0c", "1fd06af906b88ff85e961441d5f5f28d6c39e130eabe5d020c420fcf593eb156",
        "ad1614fd12c53ec774ed8350832aefc003d8a5785efe41326149cd2207c2ec00", "b7398b7040772ad0f4a72dd52cbd675656d74cbc099c2234154dae66b29a8695"
    },
    {
        "empty wallet test (mainnet)",
        "light wiggle different evolved tiers tamper austere vortex beer oncoming twice macro quick hamburger foiled bugs object thumbs haystack january turnip uphill gown inflamed oncoming",
        3360383, 0, 3360383, 0,
        ots::Network::MAIN,
        "", false,
        "42CxKHFP8X5D5RQxTZXH9aNrNdRDg6aLhe9jXLd3qoJaiFadSrPXCzJ49vwDximrozeHgmZQGrZ3YftBqsse8P888yg6uPq", "08D795",
        true,
        "19fbe8397f4049fb54fb4526a660baa1863de6f082e87841663b6b72bc245b0f", "0f6620cb2a561c4832f2d76901726182a3df7913600cd2de1bd76c57abec53f6",
        "ff75ebdb8add78dcffb4be9809a7ce80d611f90b53d1f97e17f15c8066f0b900", "a03881823e787712db37b3ec759d59deedb1e451626d0be876a691ab83d6dd46"
    },
};
