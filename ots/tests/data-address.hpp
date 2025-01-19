#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include "ots.hpp"

struct AddressTestCase {
    std::string name;
    std::string address;
    std::string fingerprint = "";
    ots::Network network = ots::Network::MAIN;
    ots::AddressType type = ots::AddressType::Standard;
    std::string payment_id = ""; // default empty, for no integrated addresses
    bool valid = true;
};

inline const std::vector<AddressTestCase> address_test_cases = {
    {
        "Empty",
        "",
        "",
        ots::Network::MAIN,
        ots::AddressType::Standard,
        "",
        false
    },
    {
        "Invalid",
        "invalid",
        "",
        ots::Network::MAIN,
        ots::AddressType::Standard,
        "",
        false
    },
    {
        "MainStandard",
        "4957vKkr9wUAA4a2rRjLmbT4uJadSZxzrW1nJh3NJYDr87hEdiFhaCcGyK87kb8u1i1DWtwKTUnoZ6uobbotLGqX3zZKdtK",
        "35B3F5",
        ots::Network::MAIN,
        ots::AddressType::Standard
    },
    {
        "MainSubAddress",
        "83HfRN12ujdNR9AtzmMotUaKo3avrzjfbHefaZ4muku5cJuBc3qaf81Xovo88FxRgoGYqp1cJycSiZF4554cd5Lt6PfQBXm",
        "DCB56E",
        ots::Network::MAIN,
        ots::AddressType::SubAddress
    },
    {
        "MainIntegrated",
        "4Jmnw8aLmCzAA4a2rRjLmbT4uJadSZxzrW1nJh3NJYDr87hEdiFhaCcGyK87kb8u1i1DWtwKTUnoZ6uobbotLGqX5QeCPeUbcLb1iqv4E7",
        "CF8863",
        ots::Network::MAIN,
        ots::AddressType::Integrated,
        "59f3832901727c06"
    },
    {
        "TestStandard",
        "9xftLeckEQ5S5S2FHDGKZAUAHZKPdYRtVJAgyYERcEvaa8YjV7z5yXrVKmfse2mnePUCJUB6L8yCWfvUj1LBQHyRDhg7bzw",
        "00F069",
        ots::Network::TEST,
        ots::AddressType::Standard
    },
    {
        "TestSubAddress",
        "BaswxFneurncD8EanZiasqLYdB2wLBPKUEpGvfTyoymEH933uibnpuWTjQA2ThJiirVSMgbYVWuGVUePddR2v9WmNHDwdPJ",
        "59984B",
        ots::Network::TEST,
        ots::AddressType::SubAddress
    },
    {
        "TestIntegrated",
        "A8NZMTSEqfbS5S2FHDGKZAUAHZKPdYRtVJAgyYERcEvaa8YjV7z5yXrVKmfse2mnePUCJUB6L8yCWfvUj1LBQHyRKrDheASom7LFc6SsTx",
        "8E8255",
        ots::Network::TEST,
        ots::AddressType::Integrated,
        "b03d44b903993f81"
    },
    {
        "StageStandard",
        "5BCb2ZfN7Jybmqjgb3QbCyYpPgF2s9ygS2xJ3wKM1jVyKmaX1XHtAieiaHeWx7CwirKvTA1PEHZtA37FqKaDDowoTC4MjxA",
        "D01628",
        ots::Network::STAGE,
        ots::AddressType::Standard
    },
    {
        "StageSubAddress",
        "79yuUvURCcUDSis5CJxkqiSBKs6YC64nJXoo9dYWaLtcQr179fZwHxJQYnGyDViHb6EWLNDUCJ2kh25X5kkCyu6aLDfd1cX",
        "7D8356",
        ots::Network::STAGE,
        ots::AddressType::SubAddress
    },
    {
        "StageIntegrated",
        "5LuG3NUriaVbmqjgb3QbCyYpPgF2s9ygS2xJ3wKM1jVyKmaX1XHtAieiaHeWx7CwirKvTA1PEHZtA37FqKaDDowofucwGND291HHTMRikw",
        "83B77F",
        ots::Network::STAGE,
        ots::AddressType::Integrated,
        "9c749f464a3df891"
    }
};

inline AddressTestCase get_standard_address_for(ots::Network network) {
    auto it = std::find_if(address_test_cases.begin(), address_test_cases.end(),
                           [network](const AddressTestCase& tc) {
                               return tc.network == network &&
                                      tc.type == ots::AddressType::Standard &&
                                      tc.valid;
                           });
    if (it != address_test_cases.end())
        return *it;
    return {"", "", "", ots::Network::MAIN, ots::AddressType::Standard, "", false};
}
