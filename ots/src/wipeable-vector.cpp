#include "wipeable-vector.hpp"

namespace ots {
    // Explicit instantiations for our use cases
    template class WipeableVector<uint16_t>;
    template class WipeableVector<uint8_t>;
}
