#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

/**
 * @file blocktime.hpp
 * @brief Header for the BlockTimeData class to handle block time data
 *        in a raw format
 */

namespace ots {
    /**
     * @namespace blocktime
     * @brief All classes related to block time data
     * @note first planned to use it directly inside of gtest,
     *       but then generated with blocktime-cli a header
     *       file with the complete implementation
     */
    namespace blocktime {

        /**
         * @class BlockTimeData
         * @brief Class to handle block time data in a raw format
         */
        class BlockTimeData {
            public:
                /**
                 * @brief Construct a new Block Time Data object
                 * @param maxBlocks maximum number of blocks to store
                 */
                explicit BlockTimeData(size_t maxBlocks);

                /**
                 * @brief Construct a new Block Time Data object
                 * @param filename the file to load the data from
                 */
                explicit BlockTimeData(const std::string& filename);
                ~BlockTimeData();

                // Prevent copying
                BlockTimeData(const BlockTimeData&) = delete;
                BlockTimeData& operator=(const BlockTimeData&) = delete;

                // Allow moving
                BlockTimeData(BlockTimeData&&) noexcept;
                BlockTimeData& operator=(BlockTimeData&&) noexcept;

                /**
                 * @brief Load block time data from a file
                 * @param filename the file to load the data from
                 * @return true if the data was loaded successfully
                 */
                bool load(const std::string& filename);

                /**
                 * @brief Save block time data to a file
                 * @param filename the file to save the data to
                 * @return true if the data was saved successfully
                 */
                bool save(const std::string& filename) const;

                /**
                 * @brief Get the height by timestamp
                 * @param timestamp the timestamp to get the height for
                 * @return uint64_t the height for the timestamp
                 */
                uint64_t heightByTimestamp(uint64_t timestamp) const;

                /**
                 * @brief Get the timestamp by height
                 * @param height the height to get the timestamp for
                 * @return uint64_t the timestamp for the height
                 */
                uint64_t timestampByHeight(uint64_t height) const;

                /**
                 * @brief Get the block time data
                 * @return uint64_t* the block time data
                 */
                inline uint64_t* data() const noexcept { return blocks.get(); }

                /**
                 * @brief Get the block time data
                 * @return uint64_t* the block time data
                 */
                inline uint64_t* data() { return blocks.get(); }

                /**
                 * @brief Get the size of the block time data
                 * @return size_t the size of the block time data
                 */
                inline size_t size() const noexcept { return m_maxBlocks; }

                /**
                 * @brief Get the highest block
                 * @return size_t the highest block
                 */
                inline size_t highestBlock() const noexcept { return m_highestBlock; }

                /**
                 * @brief Get the highest timestamp
                 * @return uint64_t the highest timestamp
                 */
                inline uint64_t highestTimestamp() const noexcept { return blocks[m_highestBlock]; }

                /**
                 * @brief Update the highest block
                 * @param height the height to update the highest block with
                 */
                inline void updateHighestBlock(size_t height) { m_highestBlock = std::max(m_highestBlock, height); }
            private:
                std::unique_ptr<uint64_t[]> blocks;
                size_t m_highestBlock{0};
                size_t m_maxBlocks{0};
        };
    } // namespace blocktime
} // namespace ots
