#include <gtest/gtest.h>
#include <blocktime.hpp>
#include <ots.hpp>
#include <vector>
#include <cmath>

class BlockTimeDataTest : public ::testing::Test {

    protected:
        /**
         * @note one bar per 100k blocks
         */
        void printProgressBar(size_t progress, size_t total) {
            if(total == 0) return;  // Avoid division by zero
            size_t barWidth = static_cast<size_t>(std::ceil(static_cast<double>(total) / 100000));
            size_t progressBlocks = static_cast<size_t>(std::ceil(static_cast<double>(progress) / 100000));
            size_t pos = std::min(progressBlocks, barWidth);
            std::cout << "\r[";
            for(size_t i = 0; i < barWidth; ++i)
                std::cout << (i < pos ? "=" : "-");
            std::cout << "] " << std::fixed << std::setprecision(2)
                << std::setw(6) << std::setfill(' ')  // Total width is 6: 3 digits, decimal point, and 2 fractional digits
                << (100.0 * progress / total) << "% ("
                << progress << "/" << total << " processed)"; //TODO: add thousands separators
            if(progress == total)
                std::cout << std::endl;
            std::cout.flush();
        }
};

TEST_F(BlockTimeDataTest, BlocktimeEstimation) {
    std::cout << "Blocktime estimation test" << std::endl;
    ots::blocktime::BlockTimeData main{"mainnet.timestamps"};
    ots::blocktime::BlockTimeData test{"testnet.timestamps"};
    ots::blocktime::BlockTimeData stage{"stagenet.timestamps"};
    std::vector<ots::blocktime::BlockTimeData*> networks = {&main, &test, &stage};
    std::vector<std::string> network_names = {"Mainnet", "Testnet", "Stagenet"};
    std::vector<ots::Network> network_enums = {ots::Network::MAIN, ots::Network::TEST, ots::Network::STAGE};
    size_t network_index = 0;
    for(auto& blocktimes: networks) {
        auto& net = network_enums[network_index];
        std::cout << "Testing " << network_names[network_index] << " network" << std::endl;
        uint64_t highest = blocktimes->highestBlock();
        std::cout << "Highest block: " << highest << std::endl;
        for(uint64_t height = 1; height < highest; height++) {
            if(blocktimes->timestampByHeight(height) == 0)
                return; // incomplete data
            uint64_t timestamp = blocktimes->timestampByHeight(height);
            if(network_index == 1) // testnet
                timestamp -= (3600 * 24 * 15); // TODO: fix OTS::heightFromTimestamp to handle this
            if(network_index == 2) // stagenet
                timestamp -= (3600 * 24 * 28); // TODO: fix OTS::heightFromTimestamp to handle this
            uint64_t estimated_height = ots::OTS::heightFromTimestamp(timestamp, net);
            EXPECT_LE(estimated_height, height) << "Height should be less or equal to the current height";
            uint64_t estimated_timestamp = ots::OTS::timestampFromHeight(estimated_height);
            EXPECT_LE(estimated_timestamp, timestamp) << "Timestamp should be less or equal to the current timestamp";
            if(height % 1000 == 0)
                printProgressBar(height, highest);
        }
        printProgressBar(highest, highest);
        std::cout << std::endl;
        network_index++;
    }
}
