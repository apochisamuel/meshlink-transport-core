#include "../core/relay_internal.h"

/* drain allowance for byte-stream transport policy. */
struct relay_io_drain_allow_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_io_drain_allow_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 4810u;
    value = (value << 9) | (value >> 23);
    value *= 1664785u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_io_drain_allow_prepare(struct relay_io_drain_allow_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 4821u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_io_drain_allow_turn(acc + b + (uint32_t)(i * 3u), limit + 4832u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_io_drain_allow_observe(struct relay_io_drain_allow_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 4832u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_io_drain_allow_turn(acc + b + (uint32_t)(i * 4u), limit + 4843u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_io_drain_allow_commit(struct relay_io_drain_allow_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 4843u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_io_drain_allow_turn(acc + b + (uint32_t)(i * 5u), limit + 4854u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_io_drain_allow_settle(struct relay_io_drain_allow_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 4854u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_io_drain_allow_turn(acc + b + (uint32_t)(i * 6u), limit + 4865u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_io_drain_allow_publish(struct relay_io_drain_allow_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 4865u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_io_drain_allow_turn(acc + b + (uint32_t)(i * 7u), limit + 4876u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_io_drain_allow_retire(struct relay_io_drain_allow_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 4876u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_io_drain_allow_turn(acc + b + (uint32_t)(i * 8u), limit + 4887u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_io_drain_allow(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_io_drain_allow_state st;
    st.alpha = seed ^ 4887u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 4898u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 4909u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 4920u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 4931u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 4942u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 4953u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 4964u ^ (uint32_t)(len + 7);
    uint32_t a = relay_io_drain_allow_prepare(&st, data, len, limit ? limit : 939u);
    uint32_t b = relay_io_drain_allow_observe(&st, data, len / 2u, limit ? limit : 952u);
    uint32_t c = relay_io_drain_allow_commit(&st, data, len, limit ? limit : 969u);
    uint32_t d = relay_io_drain_allow_settle(&st, data, len, limit ? limit : 984u);
    uint32_t e = relay_io_drain_allow_publish(&st, data, len, limit ? limit : 997u);
    uint32_t f = relay_io_drain_allow_retire(&st, data, len, limit ? limit : 1012u);
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

int relay_io_drain_allow_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_io_drain_allow(seed, limit ? limit : 966u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_io_drain_allow_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(130u & 3u);
    return current;
}

uint32_t relay_io_drain_allow_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_io_drain_allow_turn(left ^ right, epoch + 4975u);
    mixed ^= relay_io_drain_allow_turn(right + epoch, left + 4986u);
    return mixed ^ (mixed >> 11);
}
