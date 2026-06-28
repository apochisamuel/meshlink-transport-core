#ifndef RELAY_POLICY_POLICY_H
#define RELAY_POLICY_POLICY_H
#include "../core/relay_internal.h"

/* policy validation. */
static inline uint32_t relay_18_stable_mix(uint32_t a, uint32_t b) {
    uint32_t x = a ^ (b + 1746u);
    x ^= x >> 16;
    x *= 2246822519u;
    x ^= x >> 13;
    return x;
}

static inline size_t relay_18_bounded_size(size_t requested, size_t ceiling) {
    if (ceiling == 0) return requested;
    return requested < ceiling ? requested : ceiling;
}

#endif
