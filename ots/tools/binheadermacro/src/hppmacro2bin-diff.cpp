#include "common.hpp"

/**
 * @file hppmacro2bin-diff.cpp
 * @brief cli tool to Compare binary data from a C++ header file macro with a file
 *
 * Usage: hppmacro2bin-diff <header file> <macro name> <binary file to compare>
 * - header file: the header file containing the macro
 * - macro name: the macro name to extract
 * - binary file to compare: the file to compare the extracted binary data to
 * Exits with 0 on no difference, 1 on difference, -1 on error
 *
 * @note Works only for macros created with bin2hppmacro, or to be more
 *       precise, for macros that contain the following format:
 *       `#define MACRO_NAME {"hexdata", size}`
 */

int main(int argc, char* argv[]) {
    if(argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <header file> <macro name> <binary file to compare>" << std::endl;
        return -1;
    }
    std::string headerFile = argv[1];
    std::string macroName = argv[2];
    std::string binaryFile = argv[3];
    try {
        auto [hexContent, expectedSize] = extractMacroContent(headerFile, macroName);
        std::string macroBinaryContent = hexToBinary(hexContent);
        std::string fileBinaryContent = getBinaryContent(binaryFile);
        if(macroBinaryContent.size() != fileBinaryContent.size()) {
            std::cerr << "Files differ in size: macro contains " << macroBinaryContent.size() 
                      << " bytes, file contains " << fileBinaryContent.size() << " bytes" << std::endl;
            return 1;
        }
        for(size_t i = 0; i < macroBinaryContent.size(); i++) {
            if(macroBinaryContent[i] != fileBinaryContent[i]) {
                std::cerr << "Files differ at position " << i << std::endl;
                return 1;
            }
        }
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
}
