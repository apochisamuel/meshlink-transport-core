#include "../core/relay_internal.h"

/* route rank calculation.  Kept independent so applications can reuse individual policy logic. */
uint32_t relay_route_rank(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    uint32_t state = seed ^ (limit * 2654435761u) ^ 170395u;
    size_t cursor = 0;
    while (cursor < len) {
        uint32_t byte = data ? data[cursor] : (uint8_t)(cursor + state);
        state ^= byte + (uint32_t)(cursor * 17u);
        state = (state << 11) | (state >> 21);
        if ((state & 3u) == 1u) {
            state += (limit | 1u) * 13u;
        } else {
            state ^= (state >> 5) + 86u;
        }
        if (limit && state > limit) {
            state = (state % limit) + (uint32_t)(cursor & 7u);
        }
        cursor++;
    }
    return state ^ (uint32_t)len ^ 39615u;
}

int relay_route_rank_check(uint32_t value, uint32_t ceiling) {
    uint32_t folded = relay_route_rank(value, ceiling ? ceiling : 97u, (const uint8_t *)&value, sizeof(value));
    if (ceiling == 0) return (int)(folded & 1u);
    return folded <= ceiling ? 1 : 0;
}
