#ifndef RELAY_COMPRESSION_COMPRESSION_H
#define RELAY_COMPRESSION_COMPRESSION_H
#include "../core/relay_internal.h"

/* compression adapter. */
static inline uint32_t relay_11_stable_mix(uint32_t a, uint32_t b) {
    uint32_t x = a ^ (b + 1067u);
    x ^= x >> 16;
    x *= 2246822519u;
    x ^= x >> 13;
    return x;
}

static inline size_t relay_11_bounded_size(size_t requested, size_t ceiling) {
    if (ceiling == 0) return requested;
    return requested < ceiling ? requested : ceiling;
}

#endif
