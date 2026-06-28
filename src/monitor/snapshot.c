#include "../core/relay_internal.h"

/* snapshot ranking for byte-stream transport policy. */
struct relay_snapshot_rank_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_snapshot_rank_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 4477u;
    value = (value << 9) | (value >> 23);
    value *= 1664767u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_snapshot_rank_prepare(struct relay_snapshot_rank_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 4488u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_snapshot_rank_turn(acc + b + (uint32_t)(i * 3u), limit + 4499u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_snapshot_rank_observe(struct relay_snapshot_rank_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 4499u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_snapshot_rank_turn(acc + b + (uint32_t)(i * 4u), limit + 4510u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_snapshot_rank_commit(struct relay_snapshot_rank_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 4510u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_snapshot_rank_turn(acc + b + (uint32_t)(i * 5u), limit + 4521u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_snapshot_rank_settle(struct relay_snapshot_rank_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 4521u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_snapshot_rank_turn(acc + b + (uint32_t)(i * 6u), limit + 4532u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_snapshot_rank_publish(struct relay_snapshot_rank_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 4532u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_snapshot_rank_turn(acc + b + (uint32_t)(i * 7u), limit + 4543u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_snapshot_rank_retire(struct relay_snapshot_rank_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 4543u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_snapshot_rank_turn(acc + b + (uint32_t)(i * 8u), limit + 4554u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_snapshot_rank(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_snapshot_rank_state st;
    st.alpha = seed ^ 4554u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 4565u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 4576u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 4587u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 4598u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 4609u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 4620u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 4631u ^ (uint32_t)(len + 7);
    uint32_t a = relay_snapshot_rank_prepare(&st, data, len, limit ? limit : 606u);
    uint32_t b = relay_snapshot_rank_observe(&st, data, len / 2u, limit ? limit : 619u);
    uint32_t c = relay_snapshot_rank_commit(&st, data, len, limit ? limit : 636u);
    uint32_t d = relay_snapshot_rank_settle(&st, data, len, limit ? limit : 651u);
    uint32_t e = relay_snapshot_rank_publish(&st, data, len, limit ? limit : 664u);
    uint32_t f = relay_snapshot_rank_retire(&st, data, len, limit ? limit : 679u);
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

int relay_snapshot_rank_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_snapshot_rank(seed, limit ? limit : 633u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_snapshot_rank_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(121u & 3u);
    return current;
}

uint32_t relay_snapshot_rank_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_snapshot_rank_turn(left ^ right, epoch + 4642u);
    mixed ^= relay_snapshot_rank_turn(right + epoch, left + 4653u);
    return mixed ^ (mixed >> 11);
}
