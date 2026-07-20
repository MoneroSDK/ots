#include <gtest/gtest.h>
#include "ots.hpp"
#include "ots-exceptions.hpp"
#include "ots-version.h"
#include <vector>
#include <array>
#include <string>
#include <algorithm>
#include <map>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <numeric>
#include <cmath>
#include <unordered_map>
#include "data-address.hpp"
#include "data-seed-language.hpp"
#include "data-seed-legacy.hpp"
#include "data-seed-monero.hpp"
#include "data-seed-polyseed.hpp"
#include "data-wallet-sign.hpp"
#include "data-wallet.hpp"
#include "data-blocktime.hpp"
#include <utf8proc.h>
#include <cstdlib>
#include "json-tx-description.hpp"
#include "test-tx-description.hpp"

#include <fstream> // TODO: for key images to file, do we want to test like that?

// 3600 * 24 * 15, 15 days
#define OTS_HEIGHT_FROM_TIMESTAMP_MAX_DIFF 1296000
// 3600 * 24 * 5, 5 days
#define OTS_TIMESTAMP_FROM_HEIGHT_MAX_DIFF 432000

class OTSTest : public ::testing::Test {
protected:
    ots::OTS ots;
};

TEST_F(OTSTest, OtsVersion) {
    std::string version = ots::OTS::version();
    EXPECT_FALSE(version.empty()) << "Generated version should not be empty";
    EXPECT_TRUE(version == OTS_VERSION_STRING) << "Version string must be the same as defined int ots-version.h";
}

TEST_F(OTSTest, OtsConstructor) {
    // Test default constructor
    EXPECT_NO_THROW({
        ots::OTS defaultOTS;
    }) << "Default constructor should not throw";

}

double shannonEntropy(const uint8_t* data, size_t size) noexcept {
    if(size == 0)
        return 0.0;
    std::unordered_map<uint8_t, size_t> frequency;
    for(size_t i = 0; i < size; ++i)
        frequency[data[i]]++;
    double entropy = 0.0;
    for(const auto& pair : frequency) {
        double prob = static_cast<double>(pair.second) / size;
        entropy -= prob * std::log2(prob);
    }
    return entropy;
}

TEST_F(OTSTest, OtsRandom) {
    EXPECT_NO_THROW(auto r1 = ots::OTS::random()) << "Get random r1 should work";
    EXPECT_NO_THROW(auto r2 = ots::OTS::random()) << "Get random r2 should work";
    auto r1 = ots::OTS::random();
    auto r2 = ots::OTS::random();
    EXPECT_FALSE(r1.empty()) << "Random should have 32 bytes";
    EXPECT_FALSE(r2.empty()) << "Random should have 32 bytes";
    EXPECT_FALSE(r1 == r2) << "Randoms should not be the same";
}

TEST_F(OTSTest, LowEntropyAllZeros) {
    std::vector<uint8_t> data(32, 0x00);
    EXPECT_TRUE(ots::OTS::lowEntropy(data.size(), data.data(), 3.5))
        << "All zeros should be low entropy";
}

TEST_F(OTSTest, LowEntropyAllOnes) {
    std::vector<uint8_t> data(32, 0xFF);
    EXPECT_TRUE(ots::OTS::lowEntropy(data.size(), data.data(), 3.5))
        << "All ones should be low entropy";
}

TEST_F(OTSTest, LowEntropyIncrementalSequence) {
    std::vector<uint8_t> data(32);
    std::iota(data.begin(), data.end(), 0);
    EXPECT_TRUE(ots::OTS::lowEntropy(data.size(), data.data(), 3.5))
        << "Incremental sequence should be low entropy";
}

TEST_F(OTSTest, LowEntropyRepeatingPattern) {
    std::vector<uint8_t> data(32);
    std::generate(data.begin(), data.end(), [n=0]() mutable { return n++ % 2 ? 0xAA : 0x55; });
    EXPECT_TRUE(ots::OTS::lowEntropy(data.size(), data.data(), 3.5))
        << "Alternating pattern should be low entropy";
}

TEST_F(OTSTest, LowEntropyHighEntropyData) {
    std::vector<uint8_t> data(32);
    ots::OTS::random(data.size(), data.data()); // Use actual random data
    EXPECT_FALSE(ots::OTS::lowEntropy(data.size(), data.data(), 3.5))
        << "Proper random data should not be low entropy";
}

TEST_F(OTSTest, LowEntropyThresholdBoundary) {
    // Data with exactly 2 unique bytes (entropy = 1.0)
    std::vector<uint8_t> data(32);
    ots::OTS::random(data.size(), data.data());
    const double calculated_entropy = shannonEntropy(data.data(), data.size());
    EXPECT_TRUE(ots::OTS::lowEntropy(data.size(), data.data(), calculated_entropy + 0.1))
        << "Should detect entropy below threshold";
    EXPECT_FALSE(ots::OTS::lowEntropy(data.size(), data.data(), calculated_entropy - 0.1))
        << "Should pass when threshold is below actual entropy";
}

TEST_F(OTSTest, RandomGenerationWithEnforcement) {
    // OTS::random will try three times to generate high entropy data before it throws
    ots::OTS::enforceEntropy(true);
    // Test successful generation
    EXPECT_NO_THROW({
        std::vector<uint8_t> data(32);
        for(size_t i = 0; i < 100; ++i)
            ots::OTS::random(data.size(), data.data());
    }) << "Should generate valid random data";
}

TEST_F(OTSTest, OtsHeightTimeEstimationRoundTripNow) {
    // test estimation from current time to height and back
    uint64_t timestamp = time(nullptr);
    uint64_t height = ots::OTS::heightFromTimestamp(timestamp);
    EXPECT_LE(timestamp - ots::OTS::timestampFromHeight(height), OTS_HEIGHT_FROM_TIMESTAMP_MAX_DIFF) << "Timestamp should be within " << (OTS_HEIGHT_FROM_TIMESTAMP_MAX_DIFF / (3600 * 24)) << " days";
}

TEST_F(OTSTest, OtsHeightFromTimestamp) {
    uint64_t height = 0;
    long long int acceptable_diff = OTS_HEIGHT_FROM_TIMESTAMP_MAX_DIFF; // 15 days, but should also pass 11 days until 2025-01-01 at least
    for(const uint64_t& t: ots::blocktime::main::timestamps) {
        if(t == 0)
            continue;
        uint64_t estimated_block = ots::OTS::heightFromTimestamp(t);
        EXPECT_LE(estimated_block, ++height) << "Height should be less or equal to the current height";
        EXPECT_LE((long long int)(t - ots::blocktime::main::timestampByHeight(estimated_block!=0?estimated_block:1)), acceptable_diff) << "Timestamp from Height difference should be less than " << acceptable_diff << " seconds (" << (acceptable_diff / (3600 * 24)) << " days)";
    }
}

TEST_F(OTSTest, OtsTimestampFromHeight) {
    uint64_t height = 0;
    long long int acceptable_diff = OTS_TIMESTAMP_FROM_HEIGHT_MAX_DIFF;
    for(const uint64_t& t: ots::blocktime::main::timestamps) {
        if(t == 0)
            continue;
        EXPECT_LT(abs((long long int)(t - ots::OTS::timestampFromHeight(++height))), acceptable_diff) << "Timestamp from Height difference should be less than " << acceptable_diff << " seconds (" << (acceptable_diff / (3600 * 24)) << " days)";
    }
}

TEST_F(OTSTest, OtsHeightTimeEstimationRoundTrip) {
    long long int acceptable_diff = OTS_HEIGHT_FROM_TIMESTAMP_MAX_DIFF;
    for(const uint64_t& t: ots::blocktime::main::timestamps) {
        if(t == 0)
            continue;
        EXPECT_LT(abs((long long int)(t - ots::OTS::timestampFromHeight(ots::OTS::heightFromTimestamp(t)))), acceptable_diff) << "Timestamp turn around difference should be closer then " << acceptable_diff << " seconds (" << (acceptable_diff / (3600 * 24)) << " days)";
    }
}

TEST_F(OTSTest, utf8proc) {
    // Test NFC normalization
    const utf8proc_uint8_t* composed = (const utf8proc_uint8_t*)"café";    // é as single character
    const utf8proc_uint8_t* decomposed = (const utf8proc_uint8_t*)"cafe\xCC\x81";  // e + combining acute
    // NFC (composition)
    utf8proc_uint8_t* result_nfc = utf8proc_NFC(decomposed);
    ASSERT_NE(result_nfc, nullptr);
    EXPECT_STREQ((const char*)result_nfc, (const char*)composed);
    free(result_nfc);
    // NFKD (decomposition)
    utf8proc_uint8_t* result_nfkd = utf8proc_NFKD(composed);
    ASSERT_NE(result_nfkd, nullptr);
    EXPECT_STREQ((const char*)result_nfkd, (const char*)decomposed);
    free(result_nfkd);
}

TEST_F(OTSTest, OtsWipeableString) {
    // only explicit WipeableString::insecure() should be allowed
    ots::WipeableString ws("test");
    EXPECT_NO_THROW({
        std::string s = ws.insecure();
    }) << "Insecure should not throw";
    EXPECT_THROW({
        std::string s = ws;
    }, ots::exception::wipeablestring::UnsafeConversion) << "Conversion should throw";
}

TEST_F(OTSTest, WipeableStringBasicConstruction) {
    // construction with c-string
    ots::WipeableString s1;
    EXPECT_TRUE(s1.empty());

    ots::WipeableString s2("test");
    EXPECT_EQ(s2.size(), 4);
    EXPECT_STREQ(s2.c_str(), "test");

    ots::WipeableString s3("test", 2);
    EXPECT_EQ(s3.size(), 2);
    EXPECT_STREQ(s3.c_str(), "te");
}

TEST_F(OTSTest, WipeableStringCopyAndMove) {
    ots::WipeableString original("original");
    // Copy construction
    ots::WipeableString copied(original);
    EXPECT_EQ(copied, original);
    // Move construction
    ots::WipeableString moved(std::move(copied));
    EXPECT_EQ(moved, original);
    // Copy assignment
    ots::WipeableString copyAssigned;
    copyAssigned = original;
    EXPECT_EQ(copyAssigned, original);
    // Move assignment
    ots::WipeableString moveAssigned;
    moveAssigned = std::move(copyAssigned);
    EXPECT_EQ(moveAssigned, original);
}

TEST_F(OTSTest, WipeableStringStringOperations) {
    ots::WipeableString s("Hello");
    // Append
    s.append(" World");
    EXPECT_STREQ(s.c_str(), "Hello World");
    // += operator
    s += "!";
    EXPECT_STREQ(s.c_str(), "Hello World!");
    // Substring
    auto sub = s.substr(6, 5);
    EXPECT_STREQ(sub.c_str(), "World");
    // Find
    EXPECT_EQ(s.find("World"), 6);
    EXPECT_EQ(s.find("xyz"), ots::WipeableString::npos);
    // Compare
    ots::WipeableString s2("Hello World!");
    EXPECT_EQ(s, s2);
    EXPECT_EQ(s.compare(s2), 0);
}

TEST_F(OTSTest, WipeableStringCapacityAndClear) {
    ots::WipeableString s;
    s.reserve(100);
    EXPECT_GE(s.capacity(), 100);
    s = "test string";
    EXPECT_FALSE(s.empty());
    s.clear();
    EXPECT_TRUE(s.empty());
}

TEST_F(OTSTest, WipeableStringStreamOperations) {
    ots::WipeableString s("test string");
    std::stringstream ss;
    ss << s;
    EXPECT_EQ(ss.str(), "test string");
}

TEST_F(OTSTest, WipeableStringInsecureConversion) {
    ots::WipeableString s("sensitive data");
    // Test insecure() method
    std::string insecure = s.insecure();
    EXPECT_EQ(insecure, "sensitive data");
    // Verify that the original WipeableString is still intact
    EXPECT_STREQ(s.c_str(), "sensitive data");
}

bool isMemoryWiped(const char* ptr, size_t length) {
    for(size_t i = 0; i < length; ++i) {
        if(ptr[i] != '\0') {
            return false;
        }
    }
    return true;
}

TEST_F(OTSTest, WipeableStringWiping) {
    const char* ptr = nullptr;
    size_t length = 0;
    // Create a string and let it go out of scope
    {
        // Memory should be wiped when destructed
        ots::WipeableString s("sensitive data");
        ptr = s.c_str();
        length = strlen(ptr);
        EXPECT_FALSE(isMemoryWiped(ptr, length)) << "Memory should not be wiped yet";
    }
    EXPECT_TRUE(isMemoryWiped(ptr, length)) << "Memory should be wiped";
}

TEST_F(OTSTest, SeedIndicesBasicConstruction) {
    ots::SeedIndices v1;
    EXPECT_TRUE(v1.empty());

    std::vector<uint16_t> orig{1, 2, 3, 4};
    ots::SeedIndices v2(orig);
    EXPECT_EQ(v2.size(), 4);
    EXPECT_EQ(v2[0], 1);

    ots::SeedIndices v3(3, 42);
    EXPECT_EQ(v3.size(), 3);
    EXPECT_EQ(v3[0], 42);
    EXPECT_EQ(v3[1], 42);
    EXPECT_EQ(v3[2], 42);
}

TEST_F(OTSTest, SeedIndicesCopyAndMove) {
    ots::SeedIndices original{std::vector<uint16_t>{1, 2, 3}};
    // Copy construction
    ots::SeedIndices copied(original);
    EXPECT_EQ(copied.size(), original.size());
    EXPECT_EQ(copied[0], original[0]);
    // Move construction
    ots::SeedIndices moved(std::move(copied));
    EXPECT_EQ(moved.size(), original.size());
    EXPECT_EQ(moved[0], original[0]);
    // Copy assignment
    ots::SeedIndices copyAssigned;
    copyAssigned = original;
    EXPECT_EQ(copyAssigned.size(), original.size());
    EXPECT_EQ(copyAssigned[0], original[0]);
    // Move assignment
    ots::SeedIndices moveAssigned;
    moveAssigned = std::move(copyAssigned);
    EXPECT_EQ(moveAssigned.size(), original.size());
    EXPECT_EQ(moveAssigned[0], original[0]);
}

TEST_F(OTSTest, SeedIndicesOperations) {
    ots::SeedIndices v;
    // Push back
    v.push_back(1);
    // emplace back
    v.emplace_back(2);
    EXPECT_EQ(v.size(), 2);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);

    // Clear
    v.clear();
    EXPECT_TRUE(v.empty());
}

TEST_F(OTSTest, SeedIndicesElementAccess) {
    ots::SeedIndices v{std::vector<uint16_t>{1, 2, 3}};
    // Operator []
    EXPECT_EQ(v[0], 1);
    v[0] = 42;
    EXPECT_EQ(v[0], 42);
    // At
    EXPECT_EQ(v.at(1), 2);
    EXPECT_THROW(v.at(99), std::out_of_range);
}

TEST_F(OTSTest, SeedIndicesNumericString) {
    std::vector<std::string> separators{"", " ", "-", ":", ".", ", "};
    ots::SeedIndices indices(std::vector<uint16_t>{1, 2, 3, 4});
    for(const auto& sep : separators) {
        std::string numeric = indices.numeric(sep);
        auto indices2 = ots::SeedIndices::fromNumeric(numeric, sep);
        EXPECT_EQ(indices, indices2);
    }
}

TEST_F(OTSTest, SeedIndicesHexString) {
    std::vector<std::string> separators{"", " ", "-", ":", ".", ", "};
    ots::SeedIndices indices(std::vector<uint16_t>{1, 2, 3, 4});
    for(const auto& sep : separators) {
        std::string hex = indices.hex(sep);
        auto indices2 = ots::SeedIndices::fromHex(hex, sep);
        EXPECT_EQ(indices, indices2);
    }
}

TEST_F(OTSTest, OtsSeedLanguageList) {
    EXPECT_NO_THROW(ots::SeedLanguage::list()) << "List of SeedLanguages should not throw";
    EXPECT_NO_THROW(ots::SeedLanguage::listFor(ots::SeedType::Monero)) << "List of SeedLanguages for MoneroSeed should not throw";
    EXPECT_NO_THROW(ots::SeedLanguage::listFor(ots::SeedType::Polyseed)) << "List of SeedLanguages for Polyseed should not throw";
    auto allLanguages = ots::SeedLanguage::list();
    auto moneroLanguages = ots::SeedLanguage::listFor(ots::SeedType::Monero);
    auto polyseedlanguages = ots::SeedLanguage::listFor(ots::SeedType::Polyseed);
    EXPECT_EQ(allLanguages.size(), total_languages_count()) << "List of SeedLanguages should be " << total_languages_count();
    EXPECT_EQ(moneroLanguages.size(), monero_languages_count()) << "List of SeedLanguages for MoneroSeed should be " << monero_languages_count();
    EXPECT_EQ(polyseedlanguages.size(), polyseed_languages_count()) << "List of SeedLanguages for Polyseed should be " << polyseed_languages_count();
}

TEST_F(OTSTest, OtsSeedLanguage) {
    for(auto& tc : seed_language_test_cases) {
        if(!tc.valid) {
            EXPECT_THROW(ots::SeedLanguage::fromCode(tc.code), ots::exception::seed::LanguageNotFound) << "Invalid language " << tc.name << "(" << tc.code << ") should throw";
            EXPECT_THROW(ots::SeedLanguage::fromName(tc.name), ots::exception::seed::LanguageNotFound) << "Invalid language " << tc.name << "(" << tc.code << ") should throw";
            continue;
        }
        EXPECT_NO_THROW({
            const ots::SeedLanguage& lang = ots::SeedLanguage::fromCode(tc.code);
            const ots::SeedLanguage& lang2 = ots::SeedLanguage::fromName(tc.name);
            const ots::SeedLanguage& lang3 = ots::SeedLanguage::fromEnglishName(tc.englishName);
            EXPECT_EQ(lang, lang2) << "Language should be the same when using code or name";
            EXPECT_EQ(lang, lang3) << "Language should be the same when using code or english name";
            EXPECT_EQ(lang.name(), tc.name) << "Name should be the same as in the map";
            EXPECT_EQ(lang.englishName(), tc.englishName) << "English name should be the same as in the map";
            EXPECT_EQ(lang.code(), tc.code) << "Code should be the same as in the map";
            EXPECT_EQ(lang.index(ots::SeedType::Monero), tc.index.monero) << "Monero index should be the same as in the map";
            EXPECT_EQ(lang.index(ots::SeedType::Polyseed), tc.index.polyseed) << "Polyseed index should be the same as in the map";
            EXPECT_EQ(lang.supported(ots::SeedType::Monero), tc.supported.monero) << "Monero should be supported as in the map";
            EXPECT_EQ(lang.supported(ots::SeedType::Polyseed), tc.supported.polyseed) << "Polyseed should be supported as in the map";
        }) << "Valid language " << tc.name << "(" << tc.code << ") should not throw";
    }
    auto& lang = ots::SeedLanguage::fromCode("en");
    EXPECT_THROW(ots::SeedLanguage::defaultLanguage(ots::SeedType::Monero), ots::exception::seed::NoDefaultLanguageSet) << "Language should not be found";
    EXPECT_THROW(ots::SeedLanguage::defaultLanguage(ots::SeedType::Polyseed), ots::exception::seed::NoDefaultLanguageSet) << "Language should not be found";
    ots::SeedLanguage::setDefaultLanguage(ots::SeedType::Monero, lang);
    EXPECT_EQ(lang, ots::SeedLanguage::defaultLanguage(ots::SeedType::Monero)) << "Language should be the default for Monero";
    ots::SeedLanguage::setDefaultLanguage(ots::SeedType::Polyseed, lang);
    EXPECT_EQ(lang, ots::SeedLanguage::defaultLanguage(ots::SeedType::Polyseed)) << "Language should be the default for Polyseed";
}

TEST_F(OTSTest, OtsAddress) {
    for(auto& tc : address_test_cases) {
        if(!tc.valid) {
            EXPECT_THROW(ots::Address(tc.address), ots::exception::address::Invalid) << "Invalid address " << tc.name << "(" << tc.address << ") should throw";
            continue;
        }
        EXPECT_NO_THROW({
            ots::Address addr(tc.address);
            EXPECT_EQ(addr.fingerprint(), tc.fingerprint) << "Fingerprint should be the same as in the map";
            EXPECT_EQ(addr.network(), tc.network) << "Network should be the same as in the map";
            EXPECT_EQ(addr.type(), tc.type) << "Type should be the same as in the map";
            if(addr.type() == ots::AddressType::Standard)
            if(addr.isIntegrated()) {
                EXPECT_EQ(addr.paymentID(), tc.payment_id) << "Payment ID should be the same as in the map";
                EXPECT_EQ(static_cast<const std::string&>(addr.integratedAddress()), get_standard_address_for(tc.network).address) << "Integrated address should be the same as the standard address";
            }
        }) << "Valid address " << tc.name << "(" << tc.address << ") should not throw";
    }
}

/**
 * @brief Check if two seed phrases matches
 * Helper function to compare two seed phrases, independent of both have a checksum
 * word or not. Without, more fields in the data struct would be necessary.
 */
bool seedPhrasesMatch(const std::string& expected, const std::string& value) {
    std::istringstream iss1(expected);
    std::istringstream iss2(value);
	std::string a, b;
	size_t count = 0;
	while(iss1 >> a && iss2 >> b) {
		++count;
		if(a != b)
			return false;
	}
    return count >= 12 && count <= 25 && !(count > 13 && count < 24);
}

TEST_F(OTSTest, SeedMergeValues) {
    // Test basic XOR merge with hex values for clarity
    {
        const std::vector<uint16_t> v1 = {0x1234, 0x5678};
        const std::vector<uint16_t> v2 = {0x5678, 0x1234};
        const auto merged = ots::Seed::mergeValues(v1, v2);

        // Should be same size as inputs
        EXPECT_EQ(v1.size(), v2.size());
        EXPECT_EQ(merged.size(), v1.size());
        EXPECT_EQ(merged.size(), v2.size());
        EXPECT_EQ(merged[0], v1[0] ^ v2[0]);
        EXPECT_EQ(merged[1], v1[1] ^ v2[1]);

        // Verify XOR reversibility
        const auto restored = ots::Seed::mergeValues(merged, v2);
        EXPECT_EQ(restored, v1);
    }

    // Test empty vectors
    {
        const std::vector<uint16_t> empty;
        EXPECT_NO_THROW(ots::Seed::mergeValues(empty, empty));
    }

    // Test different sizes throw
    {
        std::vector<uint16_t> v1(3, 0x1234);
        std::vector<uint16_t> v2(4, 0x5678);
        EXPECT_THROW(ots::Seed::mergeValues(v1, v2), ots::exception::seed::LengthMismatch);
    }
}

TEST_F(OTSTest, SeedMergeAndZeroize) {
    std::vector<uint16_t> v1 = {0xaaaa, 0xbbbb};
    std::vector<uint16_t> v2 = {0x5555, 0x4444};
    const auto original_v1 = v1;
    const auto original_v2 = v2;

    // Test memory wiping
    const auto merged = ots::Seed::mergeAndZeorizeValues(v1, v2, false);

    // Verify merge
    EXPECT_EQ(merged[0], 0xaaaa ^ 0x5555);
    EXPECT_EQ(merged[1], 0xbbbb ^ 0x4444);

    // Verify wiping
    EXPECT_TRUE(std::all_of(v1.begin(), v1.end(), [](auto x) { return x == 0; }));
    EXPECT_TRUE(std::all_of(v2.begin(), v2.end(), [](auto x) { return x == 0; }));

    // Verify double XOR restores original
    const auto temp = ots::Seed::mergeValues(merged, original_v2);
    EXPECT_EQ(temp, original_v1);
}

TEST_F(OTSTest, SeedMergeAllZeros) {
    // Add test for all-zero merge
    std::vector<std::vector<uint16_t>> vectors = {
        {0x0000, 0x0000},
        {0x0000, 0x0000}
    };
    const auto merged = ots::Seed::mergeValues(vectors);
    EXPECT_EQ(merged, std::vector<uint16_t>({0x0000, 0x0000}));
}

TEST_F(OTSTest, SeedMergeOddCount) {
    // Add test for odd number of vectors
    std::vector<std::vector<uint16_t>> vectors(7, {0xffff, 0xffff});
    const auto merged = ots::Seed::mergeValues(vectors);
    EXPECT_EQ(merged, std::vector<uint16_t>({0xffff, 0xffff}));
}

TEST_F(OTSTest, SeedMergeWithPassword) {
    // Test password-based XOR with verification
    const std::string password = "test_password";
    const std::vector<uint16_t> original = {0x1234, 0x5678, 0x9abc};

    // First merge
    auto merged = ots::Seed::mergeWithPassword(password, original);

    // Second merge should restore original
    auto restored = ots::Seed::mergeWithPassword(password, merged);
    EXPECT_EQ(restored, original);

    // Test with maximum allowed size
    {
        std::vector<uint16_t> max_values(32, 0xbeef);
        EXPECT_NO_THROW(ots::Seed::mergeWithPassword(password, max_values));
    }

    // Test size limit enforcement
    {
        std::vector<uint16_t> too_long(33, 0xdead);
        EXPECT_THROW(ots::Seed::mergeWithPassword(password, too_long),
                ots::exception::seed::MergeError);
    }
}

TEST_F(OTSTest, SeedMergeComplexPasswords) {
    // Test with non-ASCII password
    const std::string password = "p@ssw0rd_Æ¥#";
    const std::vector<uint16_t> values = {0x1234, 0x5678};

    auto merged = ots::Seed::mergeWithPassword(password, values);
    auto restored = ots::Seed::mergeWithPassword(password, merged);
    EXPECT_EQ(restored, values);

    // Test with emoji password
    const std::string emoji_pass = "🔑🚪🗝️";
    auto merged_emoji = ots::Seed::mergeWithPassword(emoji_pass, values);
    auto restored_emoji = ots::Seed::mergeWithPassword(emoji_pass, merged_emoji);
    EXPECT_EQ(restored_emoji, values);
}

TEST_F(OTSTest, SeedMergeWithPasswordMemorySafety) {
    // Test memory wiping
    std::string password = "sensitive";
    std::vector<uint16_t> values = {0x1234, 0x5678};

    const auto check_memory = [](const auto& container) {
        return std::all_of(container.begin(), container.end(),
                [](auto x) { return x == 0; });
    };

    // Test regular merge doesn't wipe
    {
        auto copy = values;
        ots::Seed::mergeWithPassword(password, copy);
        EXPECT_FALSE(check_memory(copy)) << "Should not wipe source in regular merge";
    }

    // Test merge+zeroize
    {
        auto copy = values;
        ots::Seed::mergeWithPasswordAndZeorize(password, copy, false);
        EXPECT_TRUE(check_memory(copy)) << "Should wipe source values";
        EXPECT_TRUE(std::all_of(password.begin(), password.end(),
                    [](char c) { return c == '\0'; })) << "Should wipe password";
    }
}

TEST_F(OTSTest, SeedMergeMultipleVectors) {
    // Basic multi-vector merge
    {
        std::vector<std::vector<uint16_t>> vectors = {
            {0x1111, 0x2222},
            {0x3333, 0x4444},
            {0x5555, 0x6666}
        };
        const auto merged = ots::Seed::mergeValues(vectors);

        // Verify cumulative XOR
        const std::vector<uint16_t> expected = {
            0x1111 ^ 0x3333 ^ 0x5555,
            0x2222 ^ 0x4444 ^ 0x6666
        };
        EXPECT_EQ(merged, expected);
    }

    // Merge with duplicate vectors (should cancel out)
    {
        std::vector<std::vector<uint16_t>> vectors = {
            {0xdead, 0xbeef},
            {0xdead, 0xbeef},
            {0x1234, 0x5678}
        };
        const auto merged = ots::Seed::mergeValues(vectors);
        EXPECT_EQ(merged, std::vector<uint16_t>({0x1234, 0x5678}));
    }

    // Empty input list
    {
        std::vector<std::vector<uint16_t>> empty;
        EXPECT_THROW(ots::Seed::mergeValues(empty), ots::exception::seed::TooFewValues);
    }

    // Single vector input
    {
        std::vector<std::vector<uint16_t>> single = {{1, 2, 3}};
        EXPECT_THROW(ots::Seed::mergeValues(single), ots::exception::seed::TooFewValues);
    }

    // Size mismatch detection
    {
        std::vector<std::vector<uint16_t>> mismatched = {
            {1, 2, 3},
            {4, 5}
        };
        EXPECT_THROW(ots::Seed::mergeValues(mismatched), ots::exception::seed::LengthMismatch);
    }
}

TEST_F(OTSTest, SeedMergeAndZeroizeMultipleVectors) {
    // Verify memory wiping for vector of vectors
    std::vector<std::vector<uint16_t>> vectors = {
        {0xa1a1, 0xb2b2},
        {0xc3c3, 0xd4d4},
        {0xe5e5, 0xf6f6}
    };

    // Store original pointers for validation
    const auto original_data_ptrs = {
        vectors[0].data(),
        vectors[1].data(),
        vectors[2].data()
    };

    const auto merged = ots::Seed::mergeAndZeorizeValues(vectors, false);

    // Verify merge correctness
    const std::vector<uint16_t> expected = {
        0xa1a1 ^ 0xc3c3 ^ 0xe5e5,
        0xb2b2 ^ 0xd4d4 ^ 0xf6f6
    };
    EXPECT_EQ(merged, expected);

    // Verify all input vectors were zeroed
    for(const auto& vec : vectors) {
        EXPECT_TRUE(std::all_of(vec.begin(), vec.end(),
            [](uint16_t x) { return x == 0; }));
    }
    // Verify original memory locations were wiped (not reallocated)
    EXPECT_EQ(vectors[0].data(), original_data_ptrs.begin()[0]);
    EXPECT_EQ(vectors[1].data(), original_data_ptrs.begin()[1]);
    EXPECT_EQ(vectors[2].data(), original_data_ptrs.begin()[2]);
}

TEST_F(OTSTest, SeedMergeConsistency) {
    // Verify different merge orders produce same results
    const std::vector<uint16_t> v1 = {0x1234, 0x5678};
    const std::vector<uint16_t> v2 = {0x9abc, 0xdef0};
    const std::vector<uint16_t> v3 = {0x1357, 0x2468};

    // Method 1: Pairwise merge
    const auto merged_pairwise = ots::Seed::mergeValues(
        ots::Seed::mergeValues(v1, v2), v3
    );

    // Method 2: Multi-vector merge
    const auto merged_multi = ots::Seed::mergeValues({v1, v2, v3});

    EXPECT_EQ(merged_pairwise, merged_multi);
}

TEST_F(OTSTest, OTSLegacySeed) {
    auto lang = ots::SeedLanguage::fromCode("en");
    for(auto& tc : legacy_seed_test_cases) {
        if(!tc.valid) {
            EXPECT_THROW(ots::LegacySeed::decode(tc.phrase), ots::exception::seed::SeedDecodingFailed) << "Invalid seed \"" << tc.name << "\" should throw";
            continue;
        }
        EXPECT_NO_THROW({
            auto seed = ots::LegacySeed::decode(tc.phrase, tc.height, tc.time, tc.network);
            auto phrase = seed.phrase(lang).insecure();
            EXPECT_TRUE(seedPhrasesMatch(tc.phrase, phrase)) << "Phrase should be the same as in the map";
            EXPECT_EQ(seed.height(), tc.height) << "Height should be the same as in the map";
            // EXPECT_EQ(seed.timestamp(), tc.time) << "Timestamp should be the same as in the map";
            EXPECT_EQ(seed.network(), tc.network) << "Network should be the same as in the map";
            EXPECT_EQ(seed.address(), tc.address) << "Address should be the same as in the map";
            EXPECT_EQ(seed.address().fingerprint(), tc.fingerprint) << "Fingerprint should be the same as in the map";
        }) << "Valid seed \"" << tc.name << "\"(" << tc.phrase << ") should not throw";
    }
    auto mstc = monero_seed_test_cases[2];
    EXPECT_THROW({
        auto seed = ots::LegacySeed::decode(mstc.phrase, mstc.height, mstc.time, mstc.network);
    }, ots::exception::legacyseed::InvalidSeedFormat) << "Monero seed \"" << mstc.name << "\" should throw";
}

TEST_F(OTSTest, OTSLegacySeedIndices) {
    auto lang = ots::SeedLanguage::fromCode("en");
    EXPECT_NO_THROW({
        auto tc = legacy_seed_test_cases[2];
        auto seed = ots::LegacySeed::decode(tc.phrase, tc.height, tc.time, tc.network);
        ots::SeedIndices indices = seed.indices();
        auto seedFromIndices = ots::LegacySeed::decode(indices, tc.height, tc.time, tc.network);
        EXPECT_EQ(seedFromIndices.phrase(ots::SeedLanguage::fromCode(tc.lang_code)).insecure(), tc.phrase) << "Phrase should be the same as in the map";
    }) << "Seed indices should lead to the same seed";
}

TEST_F(OTSTest, OtsMoneroSeedCreate) {
    auto password = "password";
    auto lang = ots::SeedLanguage::fromCode("en");
    auto seed = ots::MoneroSeed::generate();
    auto seed2 = ots::MoneroSeed::decode(seed.phrase(ots::SeedLanguage::fromCode("en")).insecure());
    EXPECT_EQ(seed.phrase(lang).insecure(), seed2.phrase(lang).insecure()) << "Generated seed should be the same as the decoded seed";
    EXPECT_EQ(seed.phrase(lang, password).insecure(), seed2.phrase(lang, password).insecure()) << "Generated seed should be the same as the decoded seed";
    EXPECT_EQ(seed.indices(password), seed2.indices(password)) << "Seed indices should be the same as the decoded seed";
}

TEST_F(OTSTest, OtsMoneroSeed) {
    for(auto& tc : monero_seed_test_cases) {
        if(!tc.valid) {
            EXPECT_THROW(ots::MoneroSeed::decode(tc.phrase, tc.height, tc.time, tc.network, tc.password), ots::exception::seed::SeedDecodingFailed) << "Invalid seed " << tc.name << " should throw";
            continue;
        }
        EXPECT_NO_THROW({
            auto seed = ots::MoneroSeed::decode(tc.phrase, tc.height, tc.time, tc.network, tc.password);
            EXPECT_EQ(seed.height(), tc.height) << "Height should be the same as in the map";
            EXPECT_EQ(seed.network(), tc.network) << "Network should be the same as in the map";
            EXPECT_EQ(seed.address(), tc.address) << "Address should be the same as in the map";
            EXPECT_EQ(seed.address().fingerprint(), tc.fingerprint) << "Fingerprint should be the same as in the map";
        }) << "Valid seed " << tc.name << "(" << tc.phrase << ") should not throw";
        EXPECT_NO_THROW({
            auto tc = monero_seed_test_cases[2];
            auto seed = ots::MoneroSeed::decode(tc.phrase, tc.height, tc.time, tc.network, tc.password);
            ots::SeedIndices indices = seed.indices(tc.password);
            auto seedFromIndices = ots::MoneroSeed::decode(indices, tc.height, tc.time, tc.network, tc.password);
            EXPECT_EQ(seedFromIndices.phrase(ots::SeedLanguage::fromCode(tc.lang_code), tc.password).insecure(), tc.phrase) << "Phrase should be the same as in the map";
        }) << "Seed indices should lead to the same seed";
    }
}

TEST_F(OTSTest, OtsPolyseedPlain) {
    auto lang = ots::SeedLanguage::fromCode("en");
    auto ps = ots::Polyseed::generate();
    auto phrase = ps.phrase(lang).insecure();
    auto ps2 = ots::Polyseed::decode(phrase, lang, ots::Network::MAIN);
    EXPECT_EQ(phrase, ps2.phrase(lang).insecure()) << "Decoded phrase should be the same as the original";
    EXPECT_EQ(ps2.address(), ps.address()) << "Address should be the same as the original";
    EXPECT_EQ(ps.indices(), ps2.indices()) << "Seed indices should be the same as the original";
    auto ps3 = ots::Polyseed::decode(ps.indices());
    EXPECT_EQ(phrase, ps3.phrase(lang).insecure()) << "Decoded phrase should be the same as the original";
    auto ps4 = ots::Polyseed::decode("october embark potato lecture make pair reunion deliver energy soccer trim film monitor ethics gesture grab", ots::Network::MAIN, "", "passphrase");
    auto ms = ps.moneroSeed();
    EXPECT_EQ(ms.address(), ps.address()) << "Monero seed address should be the same as the polyseed address";
    EXPECT_EQ(ms.wallet()->secretSpendKey(), ps.wallet()->secretSpendKey()) << "Monero seed secret spend key should be the same as the polyseed secret spend key";
}

TEST_F(OTSTest, OtsPolyseedEncrypted) {
    EXPECT_NO_THROW({
        auto password = "password";
        auto lang = ots::SeedLanguage::fromCode("en");
        auto ps = ots::Polyseed::generate(ots::Network::MAIN);
        auto phrase = ps.phrase(lang, password).insecure();
        auto ps2 = ots::Polyseed::decode(phrase, ots::Network::MAIN, password);
        EXPECT_THROW(ots::Polyseed::decode(phrase, ots::Network::MAIN), ots::exception::polyseed::NoPasswordProvided) << "Decryption without password should throw";
        EXPECT_EQ(phrase, ps2.phrase(lang, password).insecure()) << "Decrypted phrase should be the same as the original";
        EXPECT_EQ(ps.indices(password), ps2.indices(password)) << "Seed indices should be the same as the original";
        EXPECT_EQ(ps.indices(), ps2.indices()) << "Seed indices should be the same as the original";
        EXPECT_NE(ps.indices(), ps2.indices(password)) << "Seed indices should be different with different passwords";
        EXPECT_EQ(ps2.address(), ps.address()) << "Address should be the same as the original";
    });
}

TEST_F(OTSTest, OtsPolyseedOffsetphrase) {
    EXPECT_NO_THROW({
        auto lang = ots::SeedLanguage::fromCode("en");
        auto passphrase = "";
        auto ps = ots::Polyseed::generate(ots::Network::MAIN, 0);
        auto phrase = ps.phrase(lang);
        auto ps2 = ots::Polyseed::decode(phrase.insecure(), ots::Network::MAIN);
        EXPECT_EQ(phrase.insecure(), ps2.phrase(lang).insecure()) << "Decrypted phrase should be the same as the original";
        EXPECT_EQ(ps2.address(), ps.address()) << "Address should be the same as the original";
        auto ps3 = ots::Polyseed::decode(ps.indices(), ots::Network::MAIN);
        EXPECT_EQ(phrase.insecure(), ps3.phrase(lang).insecure()) << "Decrypted phrase should be the same as the original";
        EXPECT_EQ(ps3.address(), ps.address()) << "Address should be the same as the original";
        auto ps4 = ots::Polyseed::decode("october embark potato lecture make pair reunion deliver energy soccer trim film monitor ethics gesture grab", ots::Network::MAIN, "", "passphrase");
        auto ps5 = ots::Polyseed::decode(ps4.indices(), ots::Network::MAIN, "", "passphrase");
        EXPECT_EQ(ps4.address(), ps5.address()) << "Address should be the same as the original";
    });
}

TEST_F(OTSTest, OtsPolyseedPasswordAndPassphrase) {
    EXPECT_NO_THROW({
        auto en = ots::SeedLanguage::fromCode("en");
        auto password = "password";
        auto passphrase = "passphrase";
        auto ps_created = ots::Polyseed::generate(ots::Network::MAIN, 0, passphrase);
        auto phrase = ps_created.phrase(en, password).insecure();
        auto indices = ps_created.indices(password);
        auto ps_phrase = ots::Polyseed::decode(phrase, en, ots::Network::MAIN, password, passphrase);
        auto ps_indices = ots::Polyseed::decode(indices, ots::Network::MAIN, password, passphrase);
        EXPECT_EQ(phrase, ps_phrase.phrase(en, password).insecure()) << "Decoded phrase should be the same as the original";
        EXPECT_EQ(indices, ps_indices.indices(password)) << "Seed indices should be the same as the original";
        EXPECT_NE(phrase, ps_phrase.phrase(en).insecure()) << "Decoded phrase should be different without password";
        EXPECT_NE(indices, ps_indices.indices()) << "Seed indices should be different with different passwords";
        EXPECT_EQ(ps_phrase.address(), ps_indices.address()) << "Address should be the same as the original";
        EXPECT_EQ(ps_phrase.address(), ps_created.address()) << "Address should be the same as the original";
    });
}

TEST_F(OTSTest, OtsPolyseedTestCases) {
    for(auto& tc: polyseed_test_cases) {
        if(!tc.valid) {
            EXPECT_THROW(
                ots::Polyseed::decode(
                    tc.phrase,
                    tc.network,
                    tc.password,
                    tc.passphrase
                ),
                ots::exception::polyseed::InvalidSeedFormat
            ) << "Invalid seed " << tc.name << " should throw";
            continue;
        }
        EXPECT_NO_THROW({
            auto seed = ots::Polyseed::decode(
                tc.phrase,
                tc.network,
                tc.password,
                tc.passphrase
            );
            EXPECT_EQ(
                seed.address(),
                tc.address
            ) << "Address should be the same as in the map";
            EXPECT_EQ(
                seed.address().fingerprint(),
                tc.fingerprint
            ) << "Fingerprint should be the same as in the map";
            EXPECT_EQ(
                seed.wallet()->secretSpendKey().insecure(),
                tc.secret_spend_key
            ) << "Secret spend key should be the same as in the map";
            EXPECT_EQ(
                seed.wallet()->secretViewKey().insecure(),
                tc.secret_view_key
            ) << "Secret view key should be the same as in the map";
            EXPECT_EQ(
                seed.wallet()->publicSpendKey().insecure(),
                tc.public_spend_key
            ) << "Public spend key should be the same as in the map";
            EXPECT_EQ(
                seed.wallet()->publicViewKey().insecure(),
                tc.public_view_key
            ) << "Public view key should be the same as in the map";
        }) << "Valid seed " << tc.name << "(" << tc.phrase << ") should not throw";

    }
}

TEST_F(OTSTest, WalletVerifySignedMessageFromTestData) {
    for(const auto& test_case : wallet_verify_signed_message_test_cases) {
        // Create a wallet from a seed (use the valid seed from monero_seed_test_cases)
        if(test_case.throws) {
            EXPECT_ANY_THROW(ots::Wallet::verifyData(
                test_case.message,
                test_case.address,
                test_case.signature,
                test_case.version == 1?true:false
            ));
            continue;
        }
        EXPECT_EQ(ots::Wallet::verifyData(
            test_case.message,
            test_case.address,
            test_case.signature,
            test_case.version == 1?true:false
        ), test_case.valid);
    }
}

TEST_F(OTSTest, WalletSignDataWithMoneroSeed) {
    const auto& seed_data = monero_seed_test_cases[2]; // "valid seed mainnet"
    ots::MoneroSeed seed = ots::MoneroSeed::decode(
        seed_data.phrase,
        seed_data.height,
        seed_data.time,
        seed_data.network,
        seed_data.password
    );
    auto wallet = seed.wallet();
    std::string message = "Test message from seed";
    // Sign with the wallet's primary address
    std::string signature = wallet->signData(message);
    auto wrong_signature = wallet_verify_signed_message_test_cases[0].signature;
    auto modified_message = message + "modified";
    // Verify against the test case's expected address
    EXPECT_TRUE(wallet->verifyData(message, seed_data.address, signature))
        << "Signature from seed should verify with its own address";
    for(size_t acc = 0; acc < 10; acc++) {
        for(size_t sub = 0; sub < 10; sub++) {
            if(acc == 0 && sub == 0)
                continue;
            std::string signature = wallet->signData(message, std::pair(acc, sub));
            auto address = wallet->address(acc, sub);
            EXPECT_TRUE(ots::Wallet::verifyData(message, address, signature))
                << "Signature from subaddress " << address << " should be valid!";
            EXPECT_TRUE(ots::Wallet::verifyData(message, (const std::string&)address, signature))
                << "Signature from subaddress (as string) " << address << " should be valid!";
            EXPECT_TRUE(wallet->verifyData(message, std::pair(acc, sub), signature))
                << "Signature from subaddress " << address << " (" << acc << ", " << sub <<") should verify with its own address";
            EXPECT_FALSE(wallet->verifyData(modified_message, std::pair(acc, sub), signature))
                << "Signature from subaddress " << address << " (" << acc << ", " << sub <<") should not verify with a modified message";
            EXPECT_FALSE(wallet->verifyData(message, std::pair(acc, sub), wrong_signature))
                << "Signature from subaddress " << address << " (" << acc << ", " << sub <<") should not verify with a wrong signature";
        }
    }
}

TEST_F(OTSTest, WalletImportOutputs) {
    for(const auto& tc : wallet_test_cases) {
        auto seed_tc = monero_seed_test_cases[tc.seed_test_case]; // "valid seed mainnet"
        auto seed = ots::MoneroSeed::decode(
            seed_tc.phrase,
            seed_tc.height,
            seed_tc.time,
            seed_tc.network,
            seed_tc.password
        );
        auto wallet = seed.wallet();
        if(!tc.valid) {
            for(const auto& [outputs, outputs_count]: tc.outputs) {
                EXPECT_THROW(wallet->importOutputs(outputs), ots::exception::wallet::ImportOutputs);
            }
            continue;
        }
        EXPECT_NO_THROW({
            for(const auto& [outputs, outputs_count]: tc.outputs) {
                auto count = wallet->importOutputs(outputs);
                EXPECT_EQ(count, outputs_count);
            }
        });
    }
}

TEST_F(OTSTest, WalletExportKeyImages) {
    for(const auto& tc : wallet_test_cases) {
        auto seed_tc = monero_seed_test_cases[tc.seed_test_case]; // "valid seed mainnet"
        auto seed = ots::MoneroSeed::decode(
            seed_tc.phrase,
            seed_tc.height,
            seed_tc.time,
            seed_tc.network,
            seed_tc.password
        );
        auto wallet = seed.wallet();
        if(!tc.valid)
            continue;
        EXPECT_NO_THROW({
            int i = 0;
            for(const auto& [outputs, outputs_count]: tc.outputs) {
                auto count = wallet->importOutputs(outputs);
                auto keyImages = wallet->exportKeyImages();
                std::ostringstream filename;
                if(i == 0)
                    filename << seed.fingerprint() << ".keyimages";
                else
                    filename << seed.fingerprint() << "_" << std::setw(2) << std::setfill('0') << std::setw(2) << i << ".keyimages";
                i++;
                std::ofstream keyImagesFile(filename.str());
                keyImagesFile << keyImages;
                    keyImagesFile.close();
            }
        });
    }
}

TEST_F(OTSTest, WalletDescribeTransaction) {
    for(const auto& tc : wallet_test_cases) {
        auto seed_tc = monero_seed_test_cases[tc.seed_test_case]; // "valid seed mainnet"
        auto seed = ots::MoneroSeed::decode(
            seed_tc.phrase,
            seed_tc.height,
            seed_tc.time,
            seed_tc.network,
            seed_tc.password
        );
        auto wallet = seed.wallet();
        if(!tc.valid || tc.outputs.empty() || tc.unsigned_transactions.empty())
            continue;
        EXPECT_NO_THROW({
            for(const auto&[unsignedTx, unsignedTxJson, fromOutputs, amount, hasChange]: tc.unsigned_transactions) {
                // auto count = wallet->importOutputs(tc.outputs[fromOutputs].outputs); // TODO: is it needed?
                auto tx = wallet->describeTransaction(unsignedTx);
                EXPECT_EQ(tx.amountIn, tx.amountOut + tx.fee) << "Tx amountIn != amountOut + fee";
                if(hasChange) {
                    EXPECT_TRUE(tx.change.has_value()) << "Tx change should not be empty";
                    EXPECT_EQ(tx.amountIn - tx.fee, tx.amountOut) << "Tx amountIn - fee != amountOut";
                    EXPECT_EQ(tx.amountOut, tx.change.value().amount + amount) << "Tx has some wrong amount";
                } else {
                    EXPECT_FALSE(tx.change.has_value()) << "Tx change should be empty";
                    EXPECT_EQ(tx.amountIn - tx.fee, tx.amountOut) << "Tx amountIn - fee != amountOut";
                }
                for(const auto& transfer: tx.transfers) {
                    EXPECT_EQ(transfer.unlockTime, 0) << "Transfer unlockTime should be 0";
                }
                EXPECT_TRUE(equalTxDescriptions(tx, txDescriptionFromJson(unsignedTxJson, unsignedTx))) << "TxDescription should be the same as TxDescription from JSON";
            }
        });
    }
}

TEST_F(OTSTest, WalletSignTransaction) {
    for(const auto& tc : wallet_test_cases) {
        auto seed_tc = monero_seed_test_cases[tc.seed_test_case]; // "valid seed mainnet"
        auto seed = ots::MoneroSeed::decode(
            seed_tc.phrase,
            seed_tc.height,
            seed_tc.time,
            seed_tc.network,
            seed_tc.password
        );
        auto wallet = seed.wallet();
        if(!tc.valid || tc.outputs.empty() || tc.unsigned_transactions.empty())
            continue;
        EXPECT_NO_THROW({
            int i = 0;
            for(const auto&[unsignedTx, unsignedTxJson, fromOutputs, amount, hasChange]: tc.unsigned_transactions) {
                auto count = wallet->importOutputs(tc.outputs[fromOutputs].outputs);
                auto signedTransaction = wallet->signTransaction(unsignedTx);
                std::ostringstream filename;
                if(i == 0)
                    filename << seed.fingerprint() << ".signed_tx";
                else
                    filename << seed.fingerprint() << "_" << std::setw(2) << std::setfill('0') << std::setw(2) << i << ".signed_tx";
                std::ofstream signedTxFile(filename.str());
                signedTxFile << signedTransaction;
                signedTxFile.close();
                auto keyImages = wallet->exportKeyImages();
                filename << "_keyImages";
                std::ofstream keyImagesFile(filename.str());
                keyImagesFile << keyImages;
                    keyImagesFile.close();
                i++;
            }
        });
    }
}
