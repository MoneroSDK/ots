#pragma once

/**
 * @file ots-internal.hpp
 * @brief Internal declarations for library
 * @internal
 *
 * @todo resolve dependencies to compile everything static inside.
 *       boost, easylogger (remove and wrapper?)
 */

#include "ots.hpp"
#include "crypto/crypto.h"
#include "mnemonics/electrum-words.h"
#include "cryptonote_basic/cryptonote_basic.h"
#include "cryptonote_core/cryptonote_core.h"

namespace ots {
    crypto::secret_key keyFromHash(const std::vector<unsigned char>& data);
    crypto::secret_key randomKey();
    epee::wipeable_string seedPhraseFromKey(const crypto::secret_key& key, const std::string& language);
}
