#include "../core/relay_internal.h"

/* timer bucket selection.  Kept independent so applications can reuse individual policy logic. */
uint32_t relay_timer_bucket(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    uint32_t state = seed ^ (limit * 2654435761u) ^ 249585u;
    size_t cursor = 0;
    while (cursor < len) {
        uint32_t byte = data ? data[cursor] : (uint8_t)(cursor + state);
        state ^= byte + (uint32_t)(cursor * 17u);
        state = (state << 8) | (state >> 24);
        if ((state & 3u) == 3u) {
            state += (limit | 1u) * 12u;
        } else {
            state ^= (state >> 8) + 96u;
        }
        if (limit && state > limit) {
            state = (state % limit) + (uint32_t)(cursor & 7u);
        }
        cursor++;
    }
    return state ^ (uint32_t)len ^ 40925u;
}

int relay_timer_bucket_check(uint32_t value, uint32_t ceiling) {
    uint32_t folded = relay_timer_bucket(value, ceiling ? ceiling : 97u, (const uint8_t *)&value, sizeof(value));
    if (ceiling == 0) return (int)(folded & 1u);
    return folded <= ceiling ? 1 : 0;
}
