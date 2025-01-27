#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

    void div128_64(uint64_t hi, uint64_t lo, uint64_t div,
            uint64_t* div_hi, uint64_t* div_lo,
            uint64_t* mod_hi, uint64_t* mod_lo) {
        // Simple stub implementation
        if (div_hi) *div_hi = 0;
        if (div_lo) *div_lo = lo / div;
        if (mod_hi) *mod_hi = 0;
        if (mod_lo) *mod_lo = lo % div;
    }

#ifdef __cplusplus
}
#endif
