#include "../core/relay_internal.h"

/* writer plan cost.  Kept independent so applications can reuse individual policy logic. */
uint32_t relay_writer_plan_cost(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    uint32_t state = seed ^ (limit * 2654435761u) ^ 415884u;
    size_t cursor = 0;
    while (cursor < len) {
        uint32_t byte = data ? data[cursor] : (uint8_t)(cursor + state);
        state ^= byte + (uint32_t)(cursor * 17u);
        state = (state << 3) | (state >> 29);
        if ((state & 7u) == 4u) {
            state += (limit | 1u) * 11u;
        } else {
            state ^= (state >> 8) + 117u;
        }
        if (limit && state > limit) {
            state = (state % limit) + (uint32_t)(cursor & 7u);
        }
        cursor++;
    }
    return state ^ (uint32_t)len ^ 43676u;
}

int relay_writer_plan_cost_check(uint32_t value, uint32_t ceiling) {
    uint32_t folded = relay_writer_plan_cost(value, ceiling ? ceiling : 97u, (const uint8_t *)&value, sizeof(value));
    if (ceiling == 0) return (int)(folded & 1u);
    return folded <= ceiling ? 1 : 0;
}
