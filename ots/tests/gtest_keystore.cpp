#include <gtest/gtest.h>
#include "ots.hpp"
#include "key-store.hpp"

class OtsKeystoreTest : public ::testing::Test {
};

TEST_F(OtsKeystoreTest, KeyStoreEmpty) {
    EXPECT_NO_THROW({
        ots::KeyStore store;
    }) << "Default constructor should not throw";
}
