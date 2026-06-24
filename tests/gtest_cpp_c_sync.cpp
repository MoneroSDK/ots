#include <gtest/gtest.h>
#include "ots.hpp"
#include "ots.h"
#include "cryptonote_basic/cryptonote_basic.h" // for cryptonote::network_type

class OtsCppCSyncTest : public ::testing::Test {
protected:
    ots::OTS ots;
};

TEST_F(OtsCppCSyncTest, EnumNetworkInSync) {
    // check that ots::Network is the same size as OTS_NETWORK
    EXPECT_EQ(static_cast<int>(ots::Network::MAIN), static_cast<int>(OTS_NETWORK_MAIN));
    EXPECT_EQ(static_cast<int>(ots::Network::TEST), static_cast<int>(OTS_NETWORK_TEST));
    EXPECT_EQ(static_cast<int>(ots::Network::STAGE), static_cast<int>(OTS_NETWORK_STAGE));

    // check that ots::Network is in sync with cryptonote::network_type
    EXPECT_EQ(static_cast<int>(ots::Network::MAIN), static_cast<int>(cryptonote::MAINNET));
    EXPECT_EQ(static_cast<int>(ots::Network::TEST), static_cast<int>(cryptonote::TESTNET));
    EXPECT_EQ(static_cast<int>(ots::Network::STAGE), static_cast<int>(cryptonote::STAGENET));
    // check if the last entry is below the cryptonote network FAKECHAIN, if not you need to handle the fact that cryptonote::network_type::FAKECHAIN is 3, and cryptonote::network_type::UNDEFINED is 255, so all what differs must be handled in the empty space between 3 and 255. But that is not enough. Because in ots-internal.hpp the function cryptonoteNetwork must be updated to handle the new network types, and make sure that nothing coliides with the existing network types, and missing types in cryptonote doesn't get feed with not existing network types.
    EXPECT_LT(static_cast<int>(ots::Network::STAGE), static_cast<int>(cryptonote::network_type::FAKECHAIN));
}

TEST_F(OtsCppCSyncTest, EnumAddressTypeInSync) {
    // check that ots::AddressType is in sync with OTS_ADDRESS_TYPE
    EXPECT_EQ(static_cast<int>(ots::AddressType::Standard), static_cast<int>(OTS_ADDRESS_TYPE_STANDARD));
    EXPECT_EQ(static_cast<int>(ots::AddressType::SubAddress), static_cast<int>(OTS_ADDRESS_TYPE_SUBADDRESS));
    EXPECT_EQ(static_cast<int>(ots::AddressType::Integrated), static_cast<int>(OTS_ADDRESS_TYPE_INTEGRATED));
}

TEST_F(OtsCppCSyncTest, EnumSeedTypeInSync) {
    // check that ots::SeedType is in sync with OTS_SEED_TYPE
    EXPECT_EQ(static_cast<int>(ots::SeedType::Monero), static_cast<int>(OTS_SEED_TYPE_MONERO));
    EXPECT_EQ(static_cast<int>(ots::SeedType::Polyseed), static_cast<int>(OTS_SEED_TYPE_POLYSEED));
}
