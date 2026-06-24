#include "common.hpp"

/**
 * @file hppmacro2bin.cpp
 * @brief cli tool to Extract binary data from a C++ header file macro into a file
 *
 * Usage: hppmacro2bin <header file> <macro name> [output file]
 * - header file: the header file containing the macro
 * - macro name: the macro name to extract
 * - output file: the file to write the extracted binary data to, if not provided,
 *                write to stdout
 * Exits with 0 on success, 1 on error
 *
 * @note Works only for macros created with bin2hppmacro, or to be more
 *       precise, for macros that contain the following format:
 *       `#define MACRO_NAME {"hexdata", size}`
 */

int main(int argc, char* argv[]) {
    if (argc < 3 || argc > 4) {
        std::cerr << "Usage: " << argv[0] << " <header file> <macro name> [output file]" << std::endl;
        return 1;
    }
    std::string headerFile = argv[1];
    std::string macroName = argv[2];
    try {
        auto [hexContent, expectedSize] = extractMacroContent(headerFile, macroName);
        std::string binaryContent = hexToBinary(hexContent);
        if(binaryContent.size() != expectedSize)
            std::cerr << "Warning: Extracted binary size (" << binaryContent.size() 
                      << ") doesn't match expected size (" << expectedSize << ")" << std::endl;
        if(argc == 3) { // no output file, write to stdout
            std::cout.write(binaryContent.data(), binaryContent.size());
            return 0;
        }
        std::string outputFile = argv[3];
        std::ofstream ofs(outputFile, std::ios::binary);
        if(!ofs)
            throw std::runtime_error("Failed to open output file: " + outputFile);
        ofs.write(binaryContent.data(), binaryContent.size());
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
