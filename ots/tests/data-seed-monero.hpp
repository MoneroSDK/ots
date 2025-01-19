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
    }
};
