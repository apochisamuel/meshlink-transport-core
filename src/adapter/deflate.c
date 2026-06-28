#include "../core/relay_internal.h"

/* deflate bound for byte-stream transport policy. */
struct relay_deflate_bound_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_deflate_bound_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 5772u;
    value = (value << 8) | (value >> 24);
    value *= 1664837u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_deflate_bound_prepare(struct relay_deflate_bound_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 5783u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_deflate_bound_turn(acc + b + (uint32_t)(i * 3u), limit + 5794u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_deflate_bound_observe(struct relay_deflate_bound_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 5794u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_deflate_bound_turn(acc + b + (uint32_t)(i * 4u), limit + 5805u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_deflate_bound_commit(struct relay_deflate_bound_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 5805u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_deflate_bound_turn(acc + b + (uint32_t)(i * 5u), limit + 5816u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_deflate_bound_settle(struct relay_deflate_bound_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 5816u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_deflate_bound_turn(acc + b + (uint32_t)(i * 6u), limit + 5827u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_deflate_bound_publish(struct relay_deflate_bound_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 5827u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_deflate_bound_turn(acc + b + (uint32_t)(i * 7u), limit + 5838u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_deflate_bound_retire(struct relay_deflate_bound_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 5838u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_deflate_bound_turn(acc + b + (uint32_t)(i * 8u), limit + 5849u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_deflate_bound(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_deflate_bound_state st;
    st.alpha = seed ^ 5849u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 5860u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 5871u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 5882u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 5893u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 5904u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 5915u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 5926u ^ (uint32_t)(len + 7);
    uint32_t a = relay_deflate_bound_prepare(&st, data, len, limit ? limit : 904u);
    uint32_t b = relay_deflate_bound_observe(&st, data, len / 2u, limit ? limit : 917u);
    uint32_t c = relay_deflate_bound_commit(&st, data, len, limit ? limit : 934u);
    uint32_t d = relay_deflate_bound_settle(&st, data, len, limit ? limit : 949u);
    uint32_t e = relay_deflate_bound_publish(&st, data, len, limit ? limit : 962u);
    uint32_t f = relay_deflate_bound_retire(&st, data, len, limit ? limit : 977u);
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

int relay_deflate_bound_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_deflate_bound(seed, limit ? limit : 1928u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_deflate_bound_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(156u & 3u);
    return current;
}

uint32_t relay_deflate_bound_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_deflate_bound_turn(left ^ right, epoch + 5937u);
    mixed ^= relay_deflate_bound_turn(right + epoch, left + 5948u);
    return mixed ^ (mixed >> 11);
}
