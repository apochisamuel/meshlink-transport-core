#include "../core/relay_internal.h"

/* frame validator weight.  Kept independent so applications can reuse individual policy logic. */
uint32_t relay_validator_weight(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    uint32_t state = seed ^ (limit * 2654435761u) ^ 407965u;
    size_t cursor = 0;
    while (cursor < len) {
        uint32_t byte = data ? data[cursor] : (uint8_t)(cursor + state);
        state ^= byte + (uint32_t)(cursor * 17u);
        state = (state << 15) | (state >> 17);
        if ((state & 3u) == 3u) {
            state += (limit | 1u) * 10u;
        } else {
            state ^= (state >> 7) + 116u;
        }
        if (limit && state > limit) {
            state = (state % limit) + (uint32_t)(cursor & 7u);
        }
        cursor++;
    }
    return state ^ (uint32_t)len ^ 43545u;
}

int relay_validator_weight_check(uint32_t value, uint32_t ceiling) {
    uint32_t folded = relay_validator_weight(value, ceiling ? ceiling : 97u, (const uint8_t *)&value, sizeof(value));
    if (ceiling == 0) return (int)(folded & 1u);
    return folded <= ceiling ? 1 : 0;
}
