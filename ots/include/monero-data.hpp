#pragma once

/**
 * @file monero-data.hpp
 * @brief Constants for the Monero network to calculate the height from a timestamp and reverse
 * @todo TODO: should search directly in monero source to collect the data from there
 */

#include <cstdint>

#define V1_SECONDS_PER_BLOCK 60
#define V2_SECONDS_PER_BLOCK 120

// Mainnet constants
#define MAINNET_BIRTH_TIME 1397818193
#define MAINNET_V2_TIME 1458748658
#define MAINNET_V2_BLOCK 1009827
#define MAINNET_ROLLBACK 0

// Testnet constants
#define TESTNET_BIRTH_TIME 1410295020
#define TESTNET_V2_TIME 1448285909
#define TESTNET_V2_BLOCK 624634
#define TESTNET_ROLLBACK 342100

// Stagenet constants
#define STAGENET_BIRTH_TIME 1518932025
#define STAGENET_V2_TIME 1520937818
#define STAGENET_V2_BLOCK 32000
#define STAGENET_ROLLBACK 30000

struct NetworkData {
    uint64_t birth;
    uint64_t v2_time;
    uint64_t v2_block;
    uint64_t rollback;
};

static const NetworkData MAINNET_DATA = {
    MAINNET_BIRTH_TIME,
    MAINNET_V2_TIME,
    MAINNET_V2_BLOCK,
    MAINNET_ROLLBACK
};

static const NetworkData TESTNET_DATA = {
    TESTNET_BIRTH_TIME,
    TESTNET_V2_TIME,
    TESTNET_V2_BLOCK,
    TESTNET_ROLLBACK
};

static const NetworkData STAGENET_DATA = {
    STAGENET_BIRTH_TIME,
    STAGENET_V2_TIME,
    STAGENET_V2_BLOCK,
    STAGENET_ROLLBACK
};
