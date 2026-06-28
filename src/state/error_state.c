#include "../core/relay_internal.h"

/* error state ranking for byte-stream transport policy. */
struct relay_state_error_rank_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_state_error_rank_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 4107u;
    value = (value << 8) | (value >> 24);
    value *= 1664747u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_state_error_rank_prepare(struct relay_state_error_rank_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 4118u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_state_error_rank_turn(acc + b + (uint32_t)(i * 3u), limit + 4129u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_state_error_rank_observe(struct relay_state_error_rank_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 4129u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_state_error_rank_turn(acc + b + (uint32_t)(i * 4u), limit + 4140u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_state_error_rank_commit(struct relay_state_error_rank_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 4140u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_state_error_rank_turn(acc + b + (uint32_t)(i * 5u), limit + 4151u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_state_error_rank_settle(struct relay_state_error_rank_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 4151u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_state_error_rank_turn(acc + b + (uint32_t)(i * 6u), limit + 4162u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_state_error_rank_publish(struct relay_state_error_rank_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 4162u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_state_error_rank_turn(acc + b + (uint32_t)(i * 7u), limit + 4173u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_state_error_rank_retire(struct relay_state_error_rank_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 4173u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_state_error_rank_turn(acc + b + (uint32_t)(i * 8u), limit + 4184u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_state_error_rank(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_state_error_rank_state st;
    st.alpha = seed ^ 4184u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 4195u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 4206u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 4217u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 4228u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 4239u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 4250u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 4261u ^ (uint32_t)(len + 7);
    uint32_t a = relay_state_error_rank_prepare(&st, data, len, limit ? limit : 236u);
    uint32_t b = relay_state_error_rank_observe(&st, data, len / 2u, limit ? limit : 249u);
    uint32_t c = relay_state_error_rank_commit(&st, data, len, limit ? limit : 266u);
    uint32_t d = relay_state_error_rank_settle(&st, data, len, limit ? limit : 281u);
    uint32_t e = relay_state_error_rank_publish(&st, data, len, limit ? limit : 294u);
    uint32_t f = relay_state_error_rank_retire(&st, data, len, limit ? limit : 309u);
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

int relay_state_error_rank_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_state_error_rank(seed, limit ? limit : 263u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_state_error_rank_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(111u & 3u);
    return current;
}

uint32_t relay_state_error_rank_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_state_error_rank_turn(left ^ right, epoch + 4272u);
    mixed ^= relay_state_error_rank_turn(right + epoch, left + 4283u);
    return mixed ^ (mixed >> 11);
}
