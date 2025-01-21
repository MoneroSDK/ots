#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace ots {
    namespace blocktime {

        class BlockTimeData {
            public:
                explicit BlockTimeData(size_t max_blocks);  // Changed constructor
                ~BlockTimeData();

                // Prevent copying
                BlockTimeData(const BlockTimeData&) = delete;
                BlockTimeData& operator=(const BlockTimeData&) = delete;

                // Allow moving
                BlockTimeData(BlockTimeData&&) noexcept;
                BlockTimeData& operator=(BlockTimeData&&) noexcept;

                bool load(const std::string& filename);
                bool save(const std::string& filename) const;
                uint64_t get_height_by_timestamp(uint64_t timestamp) const;
                uint64_t get_timestamp_by_height(uint64_t height) const;

                uint64_t* data() { return blocks.get(); }
                const uint64_t* data() const { return blocks.get(); }
                size_t size() const { return max_blocks; }
                size_t get_highest_block() const { return highest_block; }
                void update_highest_block(size_t height) { highest_block = std::max(highest_block, height); }
            private:
                std::unique_ptr<uint64_t[]> blocks;
                size_t highest_block{0};
                size_t max_blocks{0};  // Added max_blocks member
        };
    } // namespace blocktime
} // namespace ots
