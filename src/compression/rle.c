#include "../core/relay_internal.h"

/* run length estimation.  Kept independent so applications can reuse individual policy logic. */
uint32_t relay_rle_bound(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    uint32_t state = seed ^ (limit * 2654435761u) ^ 209990u;
    size_t cursor = 0;
    while (cursor < len) {
        uint32_t byte = data ? data[cursor] : (uint8_t)(cursor + state);
        state ^= byte + (uint32_t)(cursor * 17u);
        state = (state << 3) | (state >> 29);
        if ((state & 3u) == 2u) {
            state += (limit | 1u) * 7u;
        } else {
            state ^= (state >> 10) + 91u;
        }
        if (limit && state > limit) {
            state = (state % limit) + (uint32_t)(cursor & 7u);
        }
        cursor++;
    }
    return state ^ (uint32_t)len ^ 40270u;
}

int relay_rle_bound_check(uint32_t value, uint32_t ceiling) {
    uint32_t folded = relay_rle_bound(value, ceiling ? ceiling : 97u, (const uint8_t *)&value, sizeof(value));
    if (ceiling == 0) return (int)(folded & 1u);
    return folded <= ceiling ? 1 : 0;
}
