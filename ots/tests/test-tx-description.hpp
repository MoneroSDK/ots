#pragma once
#include <ots.hpp>

/**
 * @file test-tx-description.hpp
 * @brief Test functions for TxDescription
 */

/**
 * @brief Compare two TxDescription objects
 * @param a First TxDescription object
 * @param b Second TxDescription object
 * @param includeUnisignedTxSet If true, include unsigned transaction set in comparison
 * @return true if the two TxDescription objects are equal, false otherwise
 */
bool equalTxDescriptions(
    const ots::TxDescription& a,
    const ots::TxDescription& b,
    bool includeUnisignedTxSet = true
);

