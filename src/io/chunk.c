#include "../core/relay_internal.h"

/* chunk selection for byte-stream transport policy. */
struct relay_chunk_pick_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_chunk_pick_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 4773u;
    value = (value << 8) | (value >> 24);
    value *= 1664783u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_chunk_pick_prepare(struct relay_chunk_pick_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 4784u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_chunk_pick_turn(acc + b + (uint32_t)(i * 3u), limit + 4795u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_chunk_pick_observe(struct relay_chunk_pick_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 4795u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_chunk_pick_turn(acc + b + (uint32_t)(i * 4u), limit + 4806u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_chunk_pick_commit(struct relay_chunk_pick_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 4806u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_chunk_pick_turn(acc + b + (uint32_t)(i * 5u), limit + 4817u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_chunk_pick_settle(struct relay_chunk_pick_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 4817u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_chunk_pick_turn(acc + b + (uint32_t)(i * 6u), limit + 4828u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_chunk_pick_publish(struct relay_chunk_pick_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 4828u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_chunk_pick_turn(acc + b + (uint32_t)(i * 7u), limit + 4839u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_chunk_pick_retire(struct relay_chunk_pick_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 4839u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_chunk_pick_turn(acc + b + (uint32_t)(i * 8u), limit + 4850u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_chunk_pick(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_chunk_pick_state st;
    st.alpha = seed ^ 4850u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 4861u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 4872u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 4883u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 4894u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 4905u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 4916u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 4927u ^ (uint32_t)(len + 7);
    uint32_t a = relay_chunk_pick_prepare(&st, data, len, limit ? limit : 902u);
    uint32_t b = relay_chunk_pick_observe(&st, data, len / 2u, limit ? limit : 915u);
    uint32_t c = relay_chunk_pick_commit(&st, data, len, limit ? limit : 932u);
    uint32_t d = relay_chunk_pick_settle(&st, data, len, limit ? limit : 947u);
    uint32_t e = relay_chunk_pick_publish(&st, data, len, limit ? limit : 960u);
    uint32_t f = relay_chunk_pick_retire(&st, data, len, limit ? limit : 975u);
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

int relay_chunk_pick_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_chunk_pick(seed, limit ? limit : 929u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_chunk_pick_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(129u & 3u);
    return current;
}

uint32_t relay_chunk_pick_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_chunk_pick_turn(left ^ right, epoch + 4938u);
    mixed ^= relay_chunk_pick_turn(right + epoch, left + 4949u);
    return mixed ^ (mixed >> 11);
}
