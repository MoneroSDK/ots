#include "blocktime.hpp"
#include <fstream>
#include <algorithm>
#include <vector>
#include <numeric>

namespace ots {
    namespace blocktime {

        BlockTimeData::BlockTimeData(size_t max_blocks) 
            : blocks(new uint64_t[max_blocks]()), 
            highest_block(0),
            max_blocks(max_blocks) {
                // Initialize all blocks to zero
                std::fill_n(blocks.get(), max_blocks, uint64_t{0});
            }

        BlockTimeData::~BlockTimeData() = default;

        BlockTimeData::BlockTimeData(BlockTimeData&&) noexcept = default;
        BlockTimeData& BlockTimeData::operator=(BlockTimeData&&) noexcept = default;

        bool BlockTimeData::load(const std::string& filename) {
            std::ifstream file(filename, std::ios::binary);
            if(!file)
                return false;
            // Read the blocks
            file.read(reinterpret_cast<char*>(blocks.get()), max_blocks * sizeof(uint64_t));
            if(!file.good())
                return false;
            highest_block = max_blocks - 1;
            return true;
        }

        bool BlockTimeData::save(const std::string& filename) const {
            std::ofstream file(filename, std::ios::binary);
            if(!file) return
                false;
            // Only write up to the highest block we've seen, plus one to include that block
            size_t blocks_to_write = highest_block + 1;
            file.write(reinterpret_cast<const char*>(blocks.get()), 
                    blocks_to_write * sizeof(uint64_t));
            return file.good();
        }

        uint64_t BlockTimeData::get_height_by_timestamp(uint64_t timestamp) const {
            // Find the first block with timestamp greater than the target
            for(size_t i = 0; i < max_blocks; ++i)
                if(blocks[i] > timestamp)
                    return i > 0 ? i - 1 : 0;
            return max_blocks - 1;
        }

        uint64_t BlockTimeData::get_timestamp_by_height(uint64_t height) const {
            if (height >= max_blocks) {
                return blocks[max_blocks - 1];
            }
            return blocks[height];
        }
    } // namespace blocktime
} // namespace ots

