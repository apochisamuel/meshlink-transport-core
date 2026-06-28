#include "../core/relay_internal.h"

/* eviction ranking for byte-stream transport policy. */
struct relay_evict_rank_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_evict_rank_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 5180u;
    value = (value << 10) | (value >> 22);
    value *= 1664805u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_evict_rank_prepare(struct relay_evict_rank_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 5191u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_evict_rank_turn(acc + b + (uint32_t)(i * 3u), limit + 5202u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_evict_rank_observe(struct relay_evict_rank_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 5202u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_evict_rank_turn(acc + b + (uint32_t)(i * 4u), limit + 5213u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_evict_rank_commit(struct relay_evict_rank_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 5213u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_evict_rank_turn(acc + b + (uint32_t)(i * 5u), limit + 5224u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_evict_rank_settle(struct relay_evict_rank_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 5224u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_evict_rank_turn(acc + b + (uint32_t)(i * 6u), limit + 5235u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_evict_rank_publish(struct relay_evict_rank_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 5235u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_evict_rank_turn(acc + b + (uint32_t)(i * 7u), limit + 5246u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_evict_rank_retire(struct relay_evict_rank_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 5246u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_evict_rank_turn(acc + b + (uint32_t)(i * 8u), limit + 5257u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_evict_rank(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_evict_rank_state st;
    st.alpha = seed ^ 5257u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 5268u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 5279u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 5290u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 5301u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 5312u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 5323u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 5334u ^ (uint32_t)(len + 7);
    uint32_t a = relay_evict_rank_prepare(&st, data, len, limit ? limit : 312u);
    uint32_t b = relay_evict_rank_observe(&st, data, len / 2u, limit ? limit : 325u);
    uint32_t c = relay_evict_rank_commit(&st, data, len, limit ? limit : 342u);
    uint32_t d = relay_evict_rank_settle(&st, data, len, limit ? limit : 357u);
    uint32_t e = relay_evict_rank_publish(&st, data, len, limit ? limit : 370u);
    uint32_t f = relay_evict_rank_retire(&st, data, len, limit ? limit : 385u);
    uint32_t result = a ^ b ^ c ^ d ^ e ^ f;
    result ^= st.alpha;
    result ^= st.bravo;
    result ^= st.charlie;
    result ^= st.delta;
    result ^= st.echo;
    result ^= st.foxtrot;
    result ^= st.golf;
    result ^= st.hotel;
    if (limit) result %= limit;
    return result;
}

int relay_evict_rank_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_evict_rank(seed, limit ? limit : 1336u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_evict_rank_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(140u & 3u);
    return current;
}

uint32_t relay_evict_rank_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_evict_rank_turn(left ^ right, epoch + 5345u);
    mixed ^= relay_evict_rank_turn(right + epoch, left + 5356u);
    return mixed ^ (mixed >> 11);
}
