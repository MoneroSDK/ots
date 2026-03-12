#include "blocktime-extractor.hpp"
#include <iostream>
#include <filesystem>
#include <cstdio>
#include <iomanip>
#include <cmath>

/**
 * @file blocktime-extractor.cpp
 * @brief Implementation of the blocktime extractor
 * Reads the blocktime data from the blockchain database
 * and writes the timestamp of each block as uint64_t
 * to a binary file.
 * This is used by @see blocktime-cli to generate the
 * blocktime header file and provide a way to easy
 * convert between block height and timestamp.
 *
 * @note It is ugly as fuck but it works and don't want to waste time
 *       on it for now.
 * @note I had an idea to write sequential only the first timestamp
 *       and after that only the block height of the first block
 *       any given day, which should be still pretty accurate but
 *       save a lot of space. Anyway, for now I will not implement
 *       it how it is out of scope.
 * @todo One day, clean up and make it nice
 */

namespace ots {
    namespace blocktime {

        LMDBReader::LMDBReader(const std::string& blockchainPath)
            : env(nullptr), dbi(0), txn(nullptr), path(blockchainPath) {}

        LMDBReader::~LMDBReader() {
            if(txn) mdb_txn_abort(txn);
            if(env) mdb_env_close(env);
        }

        bool LMDBReader::init() {
            if(!std::filesystem::exists(path)) {
                std::cerr << "Path does not exist: " << path << std::endl;
                return false;
            }
            // If path is a file, use its parent directory
            std::string db_path = path;
            if(std::filesystem::is_regular_file(path)) {
                db_path = std::filesystem::path(path).parent_path().string();
            }

            int rc = mdb_env_create(&env);
            if(rc) {
                std::cerr << "Failed to create env: " << mdb_strerror(rc) << std::endl;
                return false;
            }
            rc = mdb_env_set_maxdbs(env, 1);
            rc = mdb_env_set_mapsize(env, 1ULL << 33); // 8GB
            rc = mdb_env_open(env, db_path.c_str(), MDB_RDONLY | MDB_NOTLS, 0644);
            if(rc) {
                std::cerr << "Failed to open env: " << mdb_strerror(rc) << std::endl;
                return false;
            }
            rc = mdb_txn_begin(env, nullptr, MDB_RDONLY, &txn);
            if(rc) {
                std::cerr << "Failed to begin txn: " << mdb_strerror(rc) << std::endl;
                return false;
            }
            rc = mdb_dbi_open(txn, "blocks", 0, &dbi);
            if(rc) {
                std::cerr << "Failed to open dbi: " << mdb_strerror(rc) << std::endl;
                return false;
            }
            return true;
        }

        size_t LMDBReader::maxHeight() const {
            MDB_cursor* cursor;
            int rc = mdb_cursor_open(txn, dbi, &cursor);
            if(rc)
                throw std::runtime_error("Failed to create cursor");
            MDB_val key, value;
            size_t max_height = 0;
            // Position cursor at the last key
            if(mdb_cursor_get(cursor, &key, &value, MDB_LAST) == 0) {
                if(key.mv_size == sizeof(uint64_t))
                    max_height = *(uint64_t*)key.mv_data;
            }
            mdb_cursor_close(cursor);
            return max_height;
        }

        bool LMDBReader::read(BlockTimeData& data) {

            MDB_cursor* cursor;
            int rc = mdb_cursor_open(txn, dbi, &cursor);
            if(rc) {
                std::cerr << "Failed to create cursor: " << mdb_strerror(rc) << std::endl;
                return false;
            }

            uint64_t* blocks = data.data();
            size_t count = 0;

            MDB_val key, value;
            while(mdb_cursor_get(cursor, &key, &value, MDB_NEXT) == 0) {
                if(key.mv_size != sizeof(uint64_t))
                    continue;
                uint64_t height = *(uint64_t*)key.mv_data;
                if(height >= data.size())
                    continue;
                const uint8_t* buffer = static_cast<const uint8_t*>(value.mv_data);
                size_t buffer_size = value.mv_size;
                size_t pos = 0;
                uint64_t major_version, minor_version, timestamp;
                // Read the varints
                pos += readVarint(buffer + pos, buffer_size - pos, major_version);
                pos += readVarint(buffer + pos, buffer_size - pos, minor_version);
                pos += readVarint(buffer + pos, buffer_size - pos, timestamp);

                blocks[height] = timestamp;

                count++;
                data.updateHighestBlock(height);

                if(count % 1000 == 0) {
                    printProgressBar(count, data.size());
                }
            }
            printProgressBar(data.size(), data.size());
            mdb_cursor_close(cursor);
            return count > 0;
        }

        // Read a varint from a buffer, returns bytes read
        size_t readVarint(const uint8_t* buffer, size_t size, uint64_t& value) {
            value = 0;
            size_t read = 0;
            int shift = 0;
            while(read < size) {
                uint8_t byte = buffer[read++];
                value |= static_cast<uint64_t>(byte & 0x7f) << shift;
                if(!(byte & 0x80))
                    break;
                shift += 7;
            }
            return read;
        }

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
    }
}
int main(int argc, char* argv[]) {
    if(argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <blockchain_path> <output_file>" << std::endl;
        return 1;
    }
    try {
        ots::blocktime::LMDBReader reader(argv[1]);
        if(!reader.init()) {
            std::cerr << "Failed to initialize LMDB reader" << std::endl;
            return 1;
        }
        // Get the actual max height from the blockchain
        size_t max_height = reader.maxHeight();
        ots::blocktime::BlockTimeData data(max_height + 1);  // +1 because height is 0-based
        if(!reader.read(data)) {
            std::cerr << "Failed to read blockchain" << std::endl;
            return 1;
        }
        if(!data.save(argv[2])) {
            std::cerr << "Failed to save data" << std::endl;
            return 1;
        }
        return 0;
    } catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch(...) {
        std::cerr << "Unknown error" << std::endl;
        return 1;
    }
}
