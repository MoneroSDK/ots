#pragma once

#include <polyseed.h>

/**
 * @file seed-polyseed-internal.hpp
 * @brief Internal polyseed functions, to not expose polyseed internals to the library user
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @note Internal polyseed headers, are used to control variable timestamps on creation
 *       it is ugly as fuck, and without to modify or rewrite or write a own library
 *       the reasonable thing t do now.
 * @note Seen now also that polyseed it LGPL, maybe a reimplementation under a free license
 *       would be also a good idea how polyseed will not be dynamically linked there could be
 *       some troube
 * @todo Think about reimplementation of polyseed library
 */
#include "../../external/polyseed/src/features.h" // TODO: do better, it's ugly
#include "../../external/polyseed/src/birthday.h" // TODO: do better, it's ugly
#include "../../external/polyseed/src/storage.h" // TODO: do better, it's ugly
#include "../../external/polyseed/src/gf.h" // TODO: do better, it's ugly

#ifdef __cplusplus
}
#endif

namespace ots {
    /**
     * @brief Translates status to ots::exception's
     */
    void onPolyseedStatusNotOkThrowException(const polyseed_status& status);

    /**
     * @brief makes sure dependencies are injected before first use, injection
     *        happens only once - first time.
     */
    void injectPolyseedDependency();

    /**
     * @brief provides random bytes to polyseed library
     */
    void polyseed_random_wrapper(void* result, size_t n);

    /**
     * @brief provides pbkdf2-sha256 to polyseed library
     * @param pw password
     * @param pwlen password length
     * @param salt salt
     * @param saltlen salt length
     * @param iterations number of iterations
     * @param[out] key Output. Pre-allocated buffer where the derived key will be stored.
     * @param keylen output key length
     */
    void polyseed_pbkdf2_wrapper(
        const uint8_t* pw, size_t pwlen,
        const uint8_t* salt, size_t saltlen, 
        uint64_t iterations,
        uint8_t* key, size_t keylen
    );

    /**
     * @brief provides memory wipe to polyseed library
     * @note we use the memwipe from monero, because I assumed it to be safe,
     *       we could use sodium_memzero, and I think memwipe is using it anyway,
     *       but I like to keep dependencies low as possible, and consistency
     *       as high as possible.
     * @param ptr pointer to memory
     * @param len length of memory
     */
    void polyseed_memwipe_wrapper(void* const ptr, const size_t len);

    /**
     * @brief provides utf8 normalization to polyseed library
     *        at the moment utf8proc is used, because it seems
     *        that boost::locale will drag more dependencies
     *        with in. @see ots/external/utf8proc
     * @param str input string
     * @param[out] norm output normalized string, esentially it is
     *        char[360] @see https://github.com/tevador/polyseed/blob/dfb05d8edb682b0e8f743b1b70c9131712ff4157/include/polyseed.h#L23
     */
    size_t utf8_nfc(const char* str, polyseed_str norm);

    /**
     * @brief provides utf8 normalization to polyseed library,
     *        uses also utf8proc @see ots/external/utf8proc
     * @param str input string
     * @param[out] norm output normalized string, esentially it is
     *             char[360] @see https://github.com/tevador/polyseed/blob/dfb05d8edb682b0e8f743b1b70c9131712ff4157/include/polyseed.h#L23
     */
    size_t utf8_nfkd(const char* str, polyseed_str norm);
}
