#include "../core/relay_internal.h"

/* trace mask for byte-stream transport policy. */
struct relay_trace_mask_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_trace_mask_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 6290u;
    value = (value << 13) | (value >> 19);
    value *= 1664865u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_trace_mask_prepare(struct relay_trace_mask_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 6301u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_trace_mask_turn(acc + b + (uint32_t)(i * 3u), limit + 6312u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_trace_mask_observe(struct relay_trace_mask_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 6312u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_trace_mask_turn(acc + b + (uint32_t)(i * 4u), limit + 6323u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_trace_mask_commit(struct relay_trace_mask_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 6323u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_trace_mask_turn(acc + b + (uint32_t)(i * 5u), limit + 6334u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_trace_mask_settle(struct relay_trace_mask_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 6334u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_trace_mask_turn(acc + b + (uint32_t)(i * 6u), limit + 6345u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_trace_mask_publish(struct relay_trace_mask_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 6345u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_trace_mask_turn(acc + b + (uint32_t)(i * 7u), limit + 6356u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_trace_mask_retire(struct relay_trace_mask_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 6356u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_trace_mask_turn(acc + b + (uint32_t)(i * 8u), limit + 6367u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_trace_mask(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_trace_mask_state st;
    st.alpha = seed ^ 6367u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 6378u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 6389u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 6400u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 6411u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 6422u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 6433u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 6444u ^ (uint32_t)(len + 7);
    uint32_t a = relay_trace_mask_prepare(&st, data, len, limit ? limit : 425u);
    uint32_t b = relay_trace_mask_observe(&st, data, len / 2u, limit ? limit : 438u);
    uint32_t c = relay_trace_mask_commit(&st, data, len, limit ? limit : 455u);
    uint32_t d = relay_trace_mask_settle(&st, data, len, limit ? limit : 470u);
    uint32_t e = relay_trace_mask_publish(&st, data, len, limit ? limit : 483u);
    uint32_t f = relay_trace_mask_retire(&st, data, len, limit ? limit : 498u);
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

int relay_trace_mask_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_trace_mask(seed, limit ? limit : 2446u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_trace_mask_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(170u & 3u);
    return current;
}

uint32_t relay_trace_mask_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_trace_mask_turn(left ^ right, epoch + 6455u);
    mixed ^= relay_trace_mask_turn(right + epoch, left + 6466u);
    return mixed ^ (mixed >> 11);
}
