#include "../core/relay_internal.h"

/* probe interval choice for byte-stream transport policy. */
struct relay_probe_interval_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_probe_interval_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 4514u;
    value = (value << 10) | (value >> 22);
    value *= 1664769u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_probe_interval_prepare(struct relay_probe_interval_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 4525u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_probe_interval_turn(acc + b + (uint32_t)(i * 3u), limit + 4536u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_probe_interval_observe(struct relay_probe_interval_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 4536u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_probe_interval_turn(acc + b + (uint32_t)(i * 4u), limit + 4547u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_probe_interval_commit(struct relay_probe_interval_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 4547u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_probe_interval_turn(acc + b + (uint32_t)(i * 5u), limit + 4558u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_probe_interval_settle(struct relay_probe_interval_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 4558u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_probe_interval_turn(acc + b + (uint32_t)(i * 6u), limit + 4569u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_probe_interval_publish(struct relay_probe_interval_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 4569u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_probe_interval_turn(acc + b + (uint32_t)(i * 7u), limit + 4580u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_probe_interval_retire(struct relay_probe_interval_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 4580u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_probe_interval_turn(acc + b + (uint32_t)(i * 8u), limit + 4591u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_probe_interval(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_probe_interval_state st;
    st.alpha = seed ^ 4591u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 4602u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 4613u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 4624u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 4635u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 4646u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 4657u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 4668u ^ (uint32_t)(len + 7);
    uint32_t a = relay_probe_interval_prepare(&st, data, len, limit ? limit : 643u);
    uint32_t b = relay_probe_interval_observe(&st, data, len / 2u, limit ? limit : 656u);
    uint32_t c = relay_probe_interval_commit(&st, data, len, limit ? limit : 673u);
    uint32_t d = relay_probe_interval_settle(&st, data, len, limit ? limit : 688u);
    uint32_t e = relay_probe_interval_publish(&st, data, len, limit ? limit : 701u);
    uint32_t f = relay_probe_interval_retire(&st, data, len, limit ? limit : 716u);
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

int relay_probe_interval_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_probe_interval(seed, limit ? limit : 670u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_probe_interval_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(122u & 3u);
    return current;
}

uint32_t relay_probe_interval_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_probe_interval_turn(left ^ right, epoch + 4679u);
    mixed ^= relay_probe_interval_turn(right + epoch, left + 4690u);
    return mixed ^ (mixed >> 11);
}
