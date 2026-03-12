#pragma once

#include "blocktime.hpp"
#include <lmdb.h>
#include <string>

/**
 * @file blocktime-extractor.hpp
 * @brief Header for the blocktime extractor, to read the blocktime data from the LMDB database
 */

namespace ots {
    /**
     * @namespace blocktime
     * @brief All blocktime related classes and functions
     */
    namespace blocktime {

        /**
         * @brief LMDBReader reads the blocktime data from the LMDB database
         */
        class LMDBReader {
            public:
                /**
                 * @brief Construct a new LMDBReader object
                 * @param blockchainPath the path to the blockchain LMDB database
                 */
                explicit LMDBReader(const std::string& blockchainPath);
                ~LMDBReader();

                /**
                 * @brief Initialize the LMDB database
                 * @return true if the database was initialized successfully
                 */
                bool init();
                /**
                 * @brief read the blocktime data from the LMDB database into data
                 * @param data the blocktime data
                 * @return true if the data was read successfully
                 */
                bool read(BlockTimeData& data);

                /**
                 * @brief get the maximum height of the blockchain from the LMDB database
                 * @return the maximum height of the blockchain
                 */
                size_t maxHeight() const;

            private:
                MDB_env* env;
                MDB_dbi dbi;
                MDB_txn* txn;
                std::string path;
        };

        /**
         * @brief read a varint from the buffer
         * @param buffer the buffer to read from
         * @param size the size of the buffer
         * @param value the value to read into
         * @return the number of bytes read
         */
        size_t readVarint(const uint8_t* buffer, size_t size, uint64_t& value);

        /**
         * @brief print a progress bar
         * @param progress the current progress
         * @param total the total progress
         */
        void printProgressBar(size_t progress, size_t total);
    } // namespace blocktime
} // namespace ots
