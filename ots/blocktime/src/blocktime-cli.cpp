#include "blocktime.hpp"
#include <iostream>
#include <iomanip>
#include <string>
#include <chrono>
#include <ctime>
#include <fstream>
#include <filesystem>

/**
 * @file blocktime_cli.cpp
 * @brief Command line interface for blocktime data
 * @see blocktime.hpp for the BlockTimeData class
 *
 * Usage: blocktime_cli <blocktime_file> <command> [args...]
 * Commands:
 *  timestamp <height>           Get timestamp for block Height
 *  height <timestamp>           Get block Height for Timestamp
 *  date <height>                Show date for block Height
 *  heightdate <date>            Get Height for date (format: YYYY-MM-DD)
 *  dump                         Dump all Heights and Timestamps
 *  header <network> <output>    Generate header file for network (main/test/stage)
 *
 * @todo Pretty ugly, should clean up and make it prettier some day
 */

using namespace ots::blocktime;

/**
 * @brief Print usage information
 */
static void print_usage() {
    std::cout << "Usage: blocktime_cli <blocktime_file> <command> [args...]\n"
              << "Commands:\n"
              << "  timestamp <height>           Get timestamp for block height\n"
              << "  height <timestamp>           Get block height for timestamp\n"
              << "  date <height>                Show date for block height\n"
              << "  heightdate <date>            Get height for date (format: YYYY-MM-DD)\n"
              << "  dump                         Dump all heights and timestamps\n"
              << "  header <network> <output>    Generate header file for network (main/test/stage)\n"
              << std::endl;
}

/**
 * @brief Print all block heights, timestamps, and UTC time side by side
 * @param data BlockTimeData object
 */
static void print_dump(const BlockTimeData& data) {
    const size_t max_height_width = std::to_string(data.highestBlock()).size();
    const size_t max_timestamp_width = std::to_string(data.highestTimestamp()).size();
    const uint64_t now = time(nullptr);
    std::cout << std::setfill('0');
    for(size_t i = 0; i <= data.highestBlock(); ++i) {
        uint64_t timestamp = data.timestampByHeight(i);
        std::time_t time = timestamp;
        std::cout << std::setw(max_height_width) << i << " "
                  << std::setw(max_timestamp_width) << timestamp << " ";
        // Add UTC time format
        if(timestamp > 0 && timestamp <= now) // Sanity check for valid timestamp
            std::cout << std::put_time(std::gmtime(&time), "%Y-%m-%d %H:%M:%S UTC");
        else
            std::cout << "INVALID TIMESTAMP";
        std::cout << "\n";
    }
    std::cout << std::flush;
}

/**
 * @brief Print UTC time for a given timestamp
 * @param timestamp Unix timestamp
 */
static void print_time(uint64_t timestamp) {
    std::time_t time = timestamp;
    std::cout << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << std::endl;
}

/**
 * @brief Parse date string in format YYYY-MM-DD to Unix timestamp
 * @param date Date string in format YYYY-MM-DD
 * @return Unix timestamp
 */
static uint64_t parse_date(const std::string& date) {
    std::tm tm = {};
    std::istringstream ss(date);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    if(ss.fail())
        throw std::runtime_error("Failed to parse date. Format should be YYYY-MM-DD");
    return std::mktime(&tm);
}

/**
 * @brief Get the number of blocks in a file
 * @param filename File to read
 * @return Number of blocks
 */
static size_t get_block_count(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if(!file)
        throw std::runtime_error("Failed to open file: " + filename);
    // Get file size
    file.seekg(0, std::ios::end);
    size_t filesize = file.tellg();
    // Calculate and return number of blocks
    return filesize / sizeof(uint64_t);
}

int main(int argc, char* argv[]) {
    if(argc < 3) {
        print_usage();
        return 1;
    }
    try {
        // Get the number of blocks from the file
        size_t num_blocks = get_block_count(argv[1]);
        if(num_blocks == 0) {
            std::cerr << "Invalid or empty blockchain data file" << std::endl;
            return 1;
        }
        // Create BlockTimeData with exact size needed
        BlockTimeData data(num_blocks);
        if(!data.load(argv[1])) {
            std::cerr << "Failed to load blockchain data from " << argv[1] << std::endl;
            return 1;
        }
        std::string command = argv[2];
        if(command == "timestamp" && argc == 4) {
            uint64_t height = std::stoull(argv[3]);
            uint64_t timestamp = data.timestampByHeight(height);
            std::cout << timestamp << std::endl;
            return 0;
        }
        if(command == "height" && argc == 4) {
            uint64_t timestamp = std::stoull(argv[3]);
            uint64_t height = data.heightByTimestamp(timestamp);
            std::cout << height << std::endl;
            return 0;
        }
        if(command == "dump") {
            print_dump(data);
            return 0;
        }
        if(command == "date" && argc == 4) {
            uint64_t height = std::stoull(argv[3]);
            uint64_t timestamp = data.timestampByHeight(height);
            print_time(timestamp);
            return 0;
        }
        if(command == "heightdate" && argc == 4) {
            uint64_t timestamp = parse_date(argv[3]);
            uint64_t height = data.heightByTimestamp(timestamp);
            std::cout << height << std::endl;
            return 0;
        }
        if(command == "header" && argc == 5) {
            std::string network = argv[3];
            std::string output_file = argv[4];
            if(network != "main" && network != "test" && network != "stage") {
                std::cerr << "Invalid network name. Use main, test, or stage." << std::endl;
                return 1;
            }
            std::ofstream out(output_file);
            if(!out) {
                std::cerr << "Failed to open output file: " << output_file << std::endl;
                return 1;
            }
            const BlockTimeData& const_data = data;
            const uint64_t* blocks = const_data.data();
            const size_t max_timestamp_width = std::to_string(data.highestTimestamp()).size();
            size_t data_size = const_data.size();
            out << "#pragma once\n\n";
            out << "/**\n";
            out << " * This file is auto-generated by blocktime\n";
            out << " */\n\n";
            out << "#include <cstdint>\n\n";
            out << "namespace ots::blocktime::" << network << " {\n\n";
            out << "    static const uint64_t timestamps[" << data_size << "] = {\n";
            for(size_t i = 0; i < data_size; ++i) {
                if(i% 5 == 0 && i != 0)
                    out << "\n";
                out << (i % 5 == 0?"":" ");
                out << std::setw(max_timestamp_width) << blocks[i] << ((i != data_size-1) ? "," : "");
            }
            out << "\n};\n";
            out << "    uint64_t highestBlock() { return sizeof(timestamps) / sizeof(uint64_t); }\n\n";
            out << "    uint64_t highestTimestamp() { return timestamps[highestBlock() - 1]; }\n\n";
            out << "    uint64_t timestampByHeight(uint64_t height) {\n";
            out << "        if(height >= highestBlock()) return highestTimestamp();\n";
            out << "        return timestamps[height];\n";
            out << "    };\n\n";
            out << "    uint64_t heightByTimestamp(uint64_t timestamp) {\n";
            out << "        for(size_t i = 0; i < highestBlock(); ++i)\n";
            out << "            if(timestamps[i] > timestamp)\n";
            out << "                return i > 0 ? i - 1 : 0;\n";
            out << "        return highestBlock() - 1;\n";
            out << "    };\n}";
            out.close();
            return 0;
        }
        print_usage();
        return 1;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
