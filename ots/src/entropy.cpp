#include "entropy.hpp"
#include <cmath>
#include <unordered_map>

/**
 * @file entropy.cpp
 * @brief Implementation of the functions to check entropy of data
 */
namespace ots {
    bool Entropy::allBytesIdentical(const uint8_t* data, size_t size) noexcept {
        if(size == 0)
            return false;
        const uint8_t first = data[0];
        for(size_t i = 1; i < size; ++i)
            if(data[i] != first)
                return false;
        return true;
    }

    bool Entropy::isIncrementalSequence(const uint8_t* data, size_t size) noexcept {
        if(size < 2)
            return false;
        for(size_t i = 1; i < size; ++i)
            if(data[i] != static_cast<uint8_t>(data[i-1] + 1))
                return false;
        return true;
    }

    bool Entropy::hasRepeatingSubpattern(const uint8_t* data, size_t size, uint8_t pattern) noexcept {
        for(size_t i = 0; i < size; ++i)
            if(data[i] != pattern)
                return false;
        return true;
    }

    double Entropy::shannonEntropy(const uint8_t* data, size_t size) noexcept {
        if(size == 0)
            return 0.0;
        std::unordered_map<uint8_t, size_t> frequency;
        for(size_t i = 0; i < size; ++i)
            frequency[data[i]]++;
        double entropy = 0.0;
        for(const auto& pair : frequency) {
            double prob = static_cast<double>(pair.second) / size;
            entropy -= prob * std::log2(prob);
        }
        return entropy;
    }

    bool Entropy::mostlyExtremes(const uint8_t* data, size_t size) noexcept {
        if(size == 0)
            return false;
        size_t extremes = 0;
        for(size_t i = 0; i < size; ++i)
            if(data[i] == 0x00 || data[i] == 0xFF)
                extremes++;
        return (static_cast<double>(extremes) / size) > 0.9;
    }
} // namespace ots
