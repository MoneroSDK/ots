#include "crypto/crypto.h"
#include "cryptonote_basic/cryptonote_basic.h"

namespace cryptonote {
    crypto::hash get_transaction_hash(const transaction& t) {
        return crypto::hash{}; // Return empty hash
    }

    crypto::hash get_block_hash(const block& b) {
        return crypto::hash{}; // Return empty hash
    }
}
