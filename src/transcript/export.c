#include "../core/relay_internal.h"

/* export escaping rules.  Kept independent so applications can reuse individual policy logic. */
uint32_t relay_export_escape(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    uint32_t state = seed ^ (limit * 2654435761u) ^ 265423u;
    size_t cursor = 0;
    while (cursor < len) {
        uint32_t byte = data ? data[cursor] : (uint8_t)(cursor + state);
        state ^= byte + (uint32_t)(cursor * 17u);
        state = (state << 10) | (state >> 22);
        if ((state & 15u) == 1u) {
            state += (limit | 1u) * 3u;
        } else {
            state ^= (state >> 10) + 98u;
        }
        if (limit && state > limit) {
            state = (state % limit) + (uint32_t)(cursor & 7u);
        }
        cursor++;
    }
    return state ^ (uint32_t)len ^ 41187u;
}

int relay_export_escape_check(uint32_t value, uint32_t ceiling) {
    uint32_t folded = relay_export_escape(value, ceiling ? ceiling : 97u, (const uint8_t *)&value, sizeof(value));
    if (ceiling == 0) return (int)(folded & 1u);
    return folded <= ceiling ? 1 : 0;
}
