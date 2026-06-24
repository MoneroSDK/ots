#include "blocktime.hpp"
#include <fstream>
#include <algorithm>
#include <vector>
#include <numeric>
#include <iostream>

namespace ots {
    namespace blocktime {
        BlockTimeData::BlockTimeData(size_t maxBlocks) 
            : blocks(new uint64_t[maxBlocks]()), 
            m_highestBlock(0),
            m_maxBlocks(maxBlocks) {
            // Initialize all blocks to zero
            std::fill_n(blocks.get(), maxBlocks, uint64_t{0});
        }

        BlockTimeData::BlockTimeData(const std::string& filename) {
            // get the file size
            std::ifstream file(filename, std::ios::binary | std::ios::ate);
            uint64_t fileSize = file.tellg();
            m_maxBlocks  = fileSize / sizeof(uint64_t);
            blocks = std::make_unique<uint64_t[]>(m_maxBlocks);
            file.close();
            std::fill_n(blocks.get(), m_maxBlocks, uint64_t{0});
            load(filename); // false, why?
        }

        BlockTimeData::~BlockTimeData() = default;

        BlockTimeData::BlockTimeData(BlockTimeData&&) noexcept = default;
        BlockTimeData& BlockTimeData::operator=(BlockTimeData&&) noexcept = default;

        bool BlockTimeData::load(const std::string& filename) {
            std::ifstream file(filename, std::ios::binary);
            if(!file)
                return false;
            // Read the blocks
            file.read(reinterpret_cast<char*>(blocks.get()), m_maxBlocks * sizeof(uint64_t));
            if(!file.good()) // why is file.good() false?
                return false;
            m_highestBlock = m_maxBlocks - 1;
            return true;
        }

        bool BlockTimeData::save(const std::string& filename) const {
            std::ofstream file(filename, std::ios::binary);
            if(!file) return
                false;
            // Only write up to the highest block we've seen, plus one to include that block
            size_t blocks_to_write = m_highestBlock + 1;
            file.write(
                    reinterpret_cast<const char*>(blocks.get()), 
                    blocks_to_write * sizeof(uint64_t)
                    );
            return file.good();
        }

        uint64_t BlockTimeData::heightByTimestamp(uint64_t timestamp) const {
            // Find the first block with timestamp greater than the target
            for(size_t i = 0; i < m_maxBlocks; ++i)
                if(blocks[i] > timestamp)
                    return i > 0 ? i - 1 : 0;
            return m_maxBlocks - 1;
        }

        uint64_t BlockTimeData::timestampByHeight(uint64_t height) const {
            if(height >= m_maxBlocks)
                return blocks[m_maxBlocks - 1];
            return blocks[height];
        }
    } // namespace blocktime
} // namespace ots
