#include "common.hpp"

/**
 * @file bin2hppmacro.cpp
 * @brief cli tool to convert binary files to C++ header file macros
 *
 * Usage:
 * bin2hppmacro <output_file> <input_file:macro_name> [input_file:macro_name ...]
 * - output_file: The header file to write the macros to, create if not exists
 * - input_file:macro_name: The binary file to convert and the macro name to use
 *
 * Exits with 0 on success, 1 on invalid arguments
 * already defined macros.
 *
 * @note the header file should be not manually edited, because the macros are appended
 *       simply to the end of the file. If a macro with the same name already exists,
 *       a warning is printed and the macro is skipped.
 * @warning You cannot override existing macros, you have to remove them manually,
 *          if you want to alter them.
 */

int main(int argc, char* argv[]) {
    if(argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <output_file> <input_file:macro_name> [input_file:macro_name ...]" << std::endl;
        return 1;
    }
    std::set<std::string> existingMacros = getExistingMacros(argv[1]);
    std::ofstream ofs(argv[1], std::ios_base::app);
    if(!ofs)
        throw std::runtime_error("Failed to open output file: " + std::string(argv[1]));
    for(int i = 2; i < argc; ++i) {
        std::string arg(argv[i]);
        size_t colonPos = arg.find(':');
        if(colonPos == std::string::npos) {
            std::cerr << "Invalid argument: " << arg << ". Should be in the format input_file:macro_name" << std::endl;
            continue;
        }
        std::string filename = arg.substr(0, colonPos);
        std::string macroName = arg.substr(colonPos + 1);
        if(existingMacros.find(macroName) != existingMacros.end()) {
            std::cerr << "Warning: Macro " << macroName << " is already defined." << std::endl;
            continue;
        }
        try {
            std::string binaryContent = getBinaryContent(filename);
            std::string hexContent = binaryToHex(binaryContent);
            ofs << "#define " << macroName << " {\"" << hexContent << "\", " << binaryContent.size() << "}\n";
        } catch (const std::exception& e) {
            std::cerr << "Error processing input file: " << filename << ". " << e.what() << std::endl;
        }
    }
    return 0;
}
