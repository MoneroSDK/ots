#pragma once
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif
    void* tx_description_from_json(
        const char* json,
        size_t json_size,
        const char* unsigned_tx_set,
        size_t unsigned_tx_set_size
    );
#ifdef __cplusplus
}
#endif
