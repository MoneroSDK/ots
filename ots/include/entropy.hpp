#pragma once

#include <cstdint>
#include <cstddef>

/**
 * @file entropy.hpp
 * @brief Entropy check functions
 */
namespace ots {
    /**
     * @class Entropy
     * @brief Entropy check functions
     * @internal
     *
     * @todo TODO: think about to move to a own lib, so don't need duplicate code in tests
     */
    class Entropy {
        public:
            /**
             * @brief Check if all bytes are identical
             * @data the data to check
             * @size the size of the data
             * @return true if all bytes are identical
             */
            static bool allBytesIdentical(const uint8_t* data, size_t size) noexcept;

            /**
             * @brief Check if bytes are an incremental sequence
             * @data the data to check
             * @size the size of the data
             * @return true if the data is an incremental sequence
             */
            static bool isIncrementalSequence(const uint8_t* data, size_t size) noexcept;

            /**
             * @brief Check if bytes have a repeating subpattern
             * @data the data to check
             * @size the size of the data
             * @return true if the data has a repeating subpattern
             */
            static bool hasRepeatingSubpattern(const uint8_t* data, size_t size, uint8_t pattern) noexcept;

            /**
             * @brief Check the Shannon entropy index of the data
             * @note The entropy is calculated as -Σ p(x) * log2(p(x))
             * @see https://en.wikipedia.org/wiki/Entropy_(information_theory)
             */
            static double shannonEntropy(const uint8_t* data, size_t size) noexcept;

            /**
             * @brief Check if the data is mostly extremes
             * @note The data is mostly 0x00 or 0xFF
             */
            static bool mostlyExtremes(const uint8_t* data, size_t size) noexcept;
    };
} // namespace ots
