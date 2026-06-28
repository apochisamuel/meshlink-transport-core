#include "../core/relay_internal.h"

/* ordering gap calculation for byte-stream transport policy. */
struct relay_ordering_gap_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_ordering_gap_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 4366u;
    value = (value << 6) | (value >> 26);
    value *= 1664761u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_ordering_gap_prepare(struct relay_ordering_gap_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 4377u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_ordering_gap_turn(acc + b + (uint32_t)(i * 3u), limit + 4388u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_ordering_gap_observe(struct relay_ordering_gap_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 4388u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_ordering_gap_turn(acc + b + (uint32_t)(i * 4u), limit + 4399u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_ordering_gap_commit(struct relay_ordering_gap_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 4399u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_ordering_gap_turn(acc + b + (uint32_t)(i * 5u), limit + 4410u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_ordering_gap_settle(struct relay_ordering_gap_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 4410u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_ordering_gap_turn(acc + b + (uint32_t)(i * 6u), limit + 4421u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_ordering_gap_publish(struct relay_ordering_gap_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 4421u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_ordering_gap_turn(acc + b + (uint32_t)(i * 7u), limit + 4432u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_ordering_gap_retire(struct relay_ordering_gap_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 4432u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_ordering_gap_turn(acc + b + (uint32_t)(i * 8u), limit + 4443u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_ordering_gap(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_ordering_gap_state st;
    st.alpha = seed ^ 4443u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 4454u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 4465u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 4476u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 4487u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 4498u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 4509u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 4520u ^ (uint32_t)(len + 7);
    uint32_t a = relay_ordering_gap_prepare(&st, data, len, limit ? limit : 495u);
    uint32_t b = relay_ordering_gap_observe(&st, data, len / 2u, limit ? limit : 508u);
    uint32_t c = relay_ordering_gap_commit(&st, data, len, limit ? limit : 525u);
    uint32_t d = relay_ordering_gap_settle(&st, data, len, limit ? limit : 540u);
    uint32_t e = relay_ordering_gap_publish(&st, data, len, limit ? limit : 553u);
    uint32_t f = relay_ordering_gap_retire(&st, data, len, limit ? limit : 568u);
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

int relay_ordering_gap_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_ordering_gap(seed, limit ? limit : 522u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_ordering_gap_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(118u & 3u);
    return current;
}

uint32_t relay_ordering_gap_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_ordering_gap_turn(left ^ right, epoch + 4531u);
    mixed ^= relay_ordering_gap_turn(right + epoch, left + 4542u);
    return mixed ^ (mixed >> 11);
}
