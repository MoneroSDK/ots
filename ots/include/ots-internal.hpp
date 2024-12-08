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
#include "ea_config.h"
#include "crypto/crypto.h"
#include "mnemonics/electrum-words.h"
#include "cryptonote_basic/cryptonote_basic.h"
#include "cryptonote_core/cryptonote_core.h"

#define ELPP_THREAD_SAFE
#define ELPP_NO_DEFAULT_LOG_FILE
#define ELPP_CUSTOM_COUT std::cout
#define ELPP_STACKTRACE_ON_CRASH
#define ELPP_LOGGING_FLAGS_FILENAME_DISPLAY
#define ELPP_DISABLE_PERFORMANCE_TRACKING

#include "easylogging++.h"
INITIALIZE_EASYLOGGINGPP

namespace ots {
    crypto::secret_key keyFromHash(const std::vector<unsigned char>& data);
    crypto::secret_key randomKey();
    epee::wipeable_string seedPhraseFromKey(const crypto::secret_key& key, const std::string& language);
}
