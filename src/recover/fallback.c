#include "../core/relay_internal.h"

/* fallback weight for byte-stream transport policy. */
struct relay_fallback_weight_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_fallback_weight_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 5994u;
    value = (value << 5) | (value >> 27);
    value *= 1664849u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_fallback_weight_prepare(struct relay_fallback_weight_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 6005u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_fallback_weight_turn(acc + b + (uint32_t)(i * 3u), limit + 6016u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_fallback_weight_observe(struct relay_fallback_weight_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 6016u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_fallback_weight_turn(acc + b + (uint32_t)(i * 4u), limit + 6027u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_fallback_weight_commit(struct relay_fallback_weight_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 6027u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_fallback_weight_turn(acc + b + (uint32_t)(i * 5u), limit + 6038u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_fallback_weight_settle(struct relay_fallback_weight_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 6038u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_fallback_weight_turn(acc + b + (uint32_t)(i * 6u), limit + 6049u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_fallback_weight_publish(struct relay_fallback_weight_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 6049u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_fallback_weight_turn(acc + b + (uint32_t)(i * 7u), limit + 6060u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_fallback_weight_retire(struct relay_fallback_weight_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 6060u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_fallback_weight_turn(acc + b + (uint32_t)(i * 8u), limit + 6071u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_fallback_weight(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_fallback_weight_state st;
    st.alpha = seed ^ 6071u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 6082u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 6093u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 6104u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 6115u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 6126u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 6137u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 6148u ^ (uint32_t)(len + 7);
    uint32_t a = relay_fallback_weight_prepare(&st, data, len, limit ? limit : 129u);
    uint32_t b = relay_fallback_weight_observe(&st, data, len / 2u, limit ? limit : 142u);
    uint32_t c = relay_fallback_weight_commit(&st, data, len, limit ? limit : 159u);
    uint32_t d = relay_fallback_weight_settle(&st, data, len, limit ? limit : 174u);
    uint32_t e = relay_fallback_weight_publish(&st, data, len, limit ? limit : 187u);
    uint32_t f = relay_fallback_weight_retire(&st, data, len, limit ? limit : 202u);
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

int relay_fallback_weight_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_fallback_weight(seed, limit ? limit : 2150u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_fallback_weight_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(162u & 3u);
    return current;
}

uint32_t relay_fallback_weight_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_fallback_weight_turn(left ^ right, epoch + 6159u);
    mixed ^= relay_fallback_weight_turn(right + epoch, left + 6170u);
    return mixed ^ (mixed >> 11);
}
