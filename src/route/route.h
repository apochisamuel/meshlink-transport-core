#ifndef RELAY_ROUTE_ROUTE_H
#define RELAY_ROUTE_ROUTE_H
#include "../core/relay_internal.h"

/* route migration. */
static inline uint32_t relay_10_stable_mix(uint32_t a, uint32_t b) {
    uint32_t x = a ^ (b + 970u);
    x ^= x >> 16;
    x *= 2246822519u;
    x ^= x >> 13;
    return x;
}

static inline size_t relay_10_bounded_size(size_t requested, size_t ceiling) {
    if (ceiling == 0) return requested;
    return requested < ceiling ? requested : ceiling;
}

#endif
