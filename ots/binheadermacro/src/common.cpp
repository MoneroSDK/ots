#include <regex>
#include "common.hpp"

/**
 * @file common.cpp
 * @brief Common functions for the tools
 */

std::string getBinaryContent(const std::string& file) {
    std::ifstream ifs(file, std::ios_base::binary);
    if(!ifs)
        throw std::runtime_error("Failed to open binary file: " + file);
    return std::string((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
}

std::string binaryToHex(const std::string& binaryContent) {
    std::string hexContent;
    for(char c : binaryContent) {
        char hex[3];
        sprintf(hex, "%02x", static_cast<unsigned char>(c));
        hexContent += "\\x" + std::string(hex);
    }
    return hexContent;
}

std::string hexToBinary(const std::string& hexContent) {
    std::string binaryContent;
    std::string hex;
    for(size_t i = 0; i < hexContent.length(); i++) {
        if(hexContent[i] == '\\' && i + 3 < hexContent.length() && hexContent[i+1] == 'x') {
            hex = hexContent.substr(i+2, 2);
            char byte = static_cast<char>(std::stoi(hex, nullptr, 16));
            binaryContent.push_back(byte);
            i += 3;
        }
    }
    return binaryContent;
}

std::pair<std::string, size_t> extractMacroContent(
        const std::string& file,
        const std::string& macroName
    ) {
    std::ifstream ifs(file);
    if(!ifs)
        throw std::runtime_error("Failed to open header file: " + file);
    std::string line;
    std::regex macroRegex("#define\\s+" + macroName + "\\s+\\{\"(.*)\",\\s*(\\d+)\\}");
    while(std::getline(ifs, line)) {
        std::smatch matches;
        if(std::regex_search(line, matches, macroRegex) && matches.size() >= 3)
            return {matches[1].str(), std::stoul(matches[2].str())};
    }
    throw std::runtime_error("Macro '" + macroName + "' not found in header file");
}

std::set<std::string> getExistingMacros(const std::string& file) {
    std::ifstream ifs(file);
    if(!ifs)
        return {};
    std::set<std::string> macros;
    std::string line;
    while(std::getline(ifs, line)) {
        size_t pos = line.find("#define ");
        if(pos != std::string::npos) {
            size_t endPos = line.find_first_of(" \t", pos + 8);
            if(endPos == std::string::npos)
                continue;
            macros.insert(line.substr(pos + 8, endPos - pos - 8));
        }
    }
    return macros;
}
