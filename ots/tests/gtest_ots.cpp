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
#include "data-address.hpp"
#include "data-seed-language.hpp"
#include "data-seed-monero.hpp"

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

TEST_F(OTSTest, OtsRandom) {
    EXPECT_NO_THROW(auto r1 = ots::OTS::random()) << "Get random r1 should work";
    EXPECT_NO_THROW(auto r2 = ots::OTS::random()) << "Get random r2 should work";
    auto r1 = ots::OTS::random();
    auto r2 = ots::OTS::random();
    EXPECT_FALSE(r1.empty()) << "Random should have 32 bytes";
    EXPECT_FALSE(r2.empty()) << "Random should have 32 bytes";
    EXPECT_FALSE(r1 == r2) << "Randoms should not be the same";
}

TEST_F(OTSTest, OtsWipeableString) {
    ots::WipeableString ws("test");
    EXPECT_NO_THROW({
        std::string s = ws.insecure();
    }) << "Insecure should not throw";
    EXPECT_THROW({
        std::string s = ws;
    }, ots::exception::wipeablestring::UnsafeConversion) << "Conversion should throw";
}

TEST_F(OTSTest, WipeableStringBasicConstruction) {
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
    for (size_t i = 0; i < length; ++i) {
        if (ptr[i] != '\0') {
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

TEST_F(OTSTest, WipeableVectorBasicConstruction) {
    ots::WipeableVector<uint16_t> v1;
    EXPECT_TRUE(v1.empty());
    
    std::vector<uint16_t> orig{1, 2, 3, 4};
    ots::WipeableVector<uint16_t> v2(orig);
    EXPECT_EQ(v2.size(), 4);
    EXPECT_EQ(v2[0], 1);
    
    ots::WipeableVector<uint16_t> v3(3, 42);
    EXPECT_EQ(v3.size(), 3);
    EXPECT_EQ(v3[0], 42);
    EXPECT_EQ(v3[1], 42);
    EXPECT_EQ(v3[2], 42);
}

TEST_F(OTSTest, WipeableVectorCopyAndMove) {
    ots::WipeableVector<uint16_t> original{std::vector<uint16_t>{1, 2, 3}};
    
    // Copy construction
    ots::WipeableVector<uint16_t> copied(original);
    EXPECT_EQ(copied.size(), original.size());
    EXPECT_EQ(copied[0], original[0]);
    
    // Move construction
    ots::WipeableVector<uint16_t> moved(std::move(copied));
    EXPECT_EQ(moved.size(), original.size());
    EXPECT_EQ(moved[0], original[0]);
    
    // Copy assignment
    ots::WipeableVector<uint16_t> copyAssigned;
    copyAssigned = original;
    EXPECT_EQ(copyAssigned.size(), original.size());
    EXPECT_EQ(copyAssigned[0], original[0]);
    
    // Move assignment
    ots::WipeableVector<uint16_t> moveAssigned;
    moveAssigned = std::move(copyAssigned);
    EXPECT_EQ(moveAssigned.size(), original.size());
    EXPECT_EQ(moveAssigned[0], original[0]);
}

TEST_F(OTSTest, WipeableVectorOperations) {
    ots::WipeableVector<uint16_t> v;
    
    // Push back
    v.push_back(1);
    v.push_back(2);
    EXPECT_EQ(v.size(), 2);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    
    // Resize
    v.resize(4);
    EXPECT_EQ(v.size(), 4);
    
    // Reserve
    size_t oldCapacity = v.size();
    v.reserve(100);
    EXPECT_GE(v.size(), oldCapacity);
    
    // Clear
    v.clear();
    EXPECT_TRUE(v.empty());
}

TEST_F(OTSTest, WipeableVectorIterators) {
    std::vector<uint16_t> orig{1, 2, 3, 4};
    ots::WipeableVector<uint16_t> v(orig);
    
    uint16_t sum = 0;
    for(const auto& val : v) {
        sum += val;
    }
    EXPECT_EQ(sum, 10);
}

TEST_F(OTSTest, WipeableVectorConversion) {
    std::vector<uint16_t> orig{1, 2, 3, 4};
    ots::WipeableVector<uint16_t> v(orig);
    
    // Test that explicit conversion works
    std::vector<uint16_t> safe = v.insecure();
    EXPECT_EQ(safe, orig);
    
    // Test that implicit conversion throws
    EXPECT_THROW({
        const std::vector<uint16_t>& unsafe = v.operator std::vector<uint16_t>();
        (void)unsafe;  // prevent unused variable warning
    }, ots::exception::wipeablevector::UnsafeConversion);
}

TEST_F(OTSTest, WipeableVectorElementAccess) {
    ots::WipeableVector<uint16_t> v{std::vector<uint16_t>{1, 2, 3}};
    
    // Operator []
    EXPECT_EQ(v[0], 1);
    v[0] = 42;
    EXPECT_EQ(v[0], 42);
    
    // At
    EXPECT_EQ(v.at(1), 2);
    EXPECT_THROW(v.at(99), std::out_of_range);
    
    // Data
    const uint16_t* data = v.data();
    EXPECT_EQ(data[0], 42);
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

TEST_F(OTSTest, OtsMoneroSeed) {
    for(auto& tc : monero_seed_test_cases) {
        if(!tc.valid) {
            EXPECT_THROW(ots::MoneroSeed::decode(tc.phrase, tc.height, tc.time, tc.network, tc.password), ots::exception::seed::SeedDecodingFailed) << "Invalid seed " << tc.name << " should throw";
            continue;
        }
        EXPECT_NO_THROW({
            auto seed = ots::MoneroSeed::decode(tc.phrase, tc.height, tc.time, tc.network, tc.password);
            EXPECT_EQ(seed.height(), tc.height) << "Height should be the same as in the map";
            // EXPECT_EQ(seed.timestamp(), tc.time) << "Timestamp should be the same as in the map";
            EXPECT_EQ(seed.network(), tc.network) << "Network should be the same as in the map";
            EXPECT_EQ(seed.encrypted(), tc.encrypted) << "Encrypted should be the same as in the map";
            EXPECT_EQ(seed.address(), tc.address) << "Address should be the same as in the map";
            EXPECT_EQ(seed.address().fingerprint(), tc.fingerprint) << "Fingerprint should be the same as in the map";
        }) << "Valid seed " << tc.name << "(" << tc.phrase << ") should not throw";
    }
}
