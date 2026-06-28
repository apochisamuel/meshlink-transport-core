#include "../core/relay_internal.h"

/* variable length integer measurement for byte-stream transport policy. */
struct relay_varint_measure_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_varint_measure_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 3737u;
    value = (value << 7) | (value >> 25);
    value *= 1664727u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_varint_measure_prepare(struct relay_varint_measure_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 3748u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_varint_measure_turn(acc + b + (uint32_t)(i * 3u), limit + 3759u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_varint_measure_observe(struct relay_varint_measure_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 3759u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_varint_measure_turn(acc + b + (uint32_t)(i * 4u), limit + 3770u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_varint_measure_commit(struct relay_varint_measure_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 3770u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_varint_measure_turn(acc + b + (uint32_t)(i * 5u), limit + 3781u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_varint_measure_settle(struct relay_varint_measure_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 3781u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_varint_measure_turn(acc + b + (uint32_t)(i * 6u), limit + 3792u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_varint_measure_publish(struct relay_varint_measure_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 3792u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_varint_measure_turn(acc + b + (uint32_t)(i * 7u), limit + 3803u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_varint_measure_retire(struct relay_varint_measure_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 3803u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_varint_measure_turn(acc + b + (uint32_t)(i * 8u), limit + 3814u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_varint_measure(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_varint_measure_state st;
    st.alpha = seed ^ 3814u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 3825u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 3836u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 3847u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 3858u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 3869u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 3880u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 3891u ^ (uint32_t)(len + 7);
    uint32_t a = relay_varint_measure_prepare(&st, data, len, limit ? limit : 863u);
    uint32_t b = relay_varint_measure_observe(&st, data, len / 2u, limit ? limit : 876u);
    uint32_t c = relay_varint_measure_commit(&st, data, len, limit ? limit : 893u);
    uint32_t d = relay_varint_measure_settle(&st, data, len, limit ? limit : 908u);
    uint32_t e = relay_varint_measure_publish(&st, data, len, limit ? limit : 921u);
    uint32_t f = relay_varint_measure_retire(&st, data, len, limit ? limit : 936u);
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

int relay_varint_measure_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_varint_measure(seed, limit ? limit : 3988u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_varint_measure_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(101u & 3u);
    return current;
}

uint32_t relay_varint_measure_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_varint_measure_turn(left ^ right, epoch + 3902u);
    mixed ^= relay_varint_measure_turn(right + epoch, left + 3913u);
    return mixed ^ (mixed >> 11);
}
