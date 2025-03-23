#pragma once

#include <fstream>
#include <iostream>
#include <set>
#include <string>

/**
 * @file common.hpp
 * @brief Common functions for the tools
 */

/**
 * @brief Convert binary content to hex string
 * @param binaryContent Binary content to convert
 * @return Hex string
 */
std::string binaryToHex(const std::string& binaryContent);

/**
 * @brief Convert hex string to binary content
 * @param hexContent Hex string to convert
 * @return Binary content
 */
std::string hexToBinary(const std::string& hexContent);

/** 
 * @brief Extract the content of a macro from a header file
 * @param file Header file to extract the macro from
 * @param macroName Name of the macro to extract
 * @return Pair of content and data size of the macro
 */
std::pair<std::string, size_t> extractMacroContent(
    const std::string& file,
    const std::string& macroName
);

/**
 * @brief Get the content of a file
 * @param file File to get the content from
 * @return Content of the file
 */
std::string getBinaryContent(const std::string& file);

/** 
 * @brief Get the macros already defined in a header
 * @param file File to get the content from
 * @return set of the names of the macros already defined in the header
 */
std::set<std::string> getExistingMacros(const std::string& file);
