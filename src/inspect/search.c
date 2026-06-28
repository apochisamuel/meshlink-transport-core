#include "../core/relay_internal.h"

/* search stride for byte-stream transport policy. */
struct relay_search_stride_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_search_stride_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 5476u;
    value = (value << 9) | (value >> 23);
    value *= 1664821u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_search_stride_prepare(struct relay_search_stride_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 5487u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_search_stride_turn(acc + b + (uint32_t)(i * 3u), limit + 5498u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_search_stride_observe(struct relay_search_stride_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 5498u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_search_stride_turn(acc + b + (uint32_t)(i * 4u), limit + 5509u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_search_stride_commit(struct relay_search_stride_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 5509u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_search_stride_turn(acc + b + (uint32_t)(i * 5u), limit + 5520u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_search_stride_settle(struct relay_search_stride_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 5520u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_search_stride_turn(acc + b + (uint32_t)(i * 6u), limit + 5531u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_search_stride_publish(struct relay_search_stride_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 5531u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_search_stride_turn(acc + b + (uint32_t)(i * 7u), limit + 5542u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_search_stride_retire(struct relay_search_stride_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 5542u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_search_stride_turn(acc + b + (uint32_t)(i * 8u), limit + 5553u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_search_stride(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_search_stride_state st;
    st.alpha = seed ^ 5553u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 5564u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 5575u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 5586u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 5597u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 5608u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 5619u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 5630u ^ (uint32_t)(len + 7);
    uint32_t a = relay_search_stride_prepare(&st, data, len, limit ? limit : 608u);
    uint32_t b = relay_search_stride_observe(&st, data, len / 2u, limit ? limit : 621u);
    uint32_t c = relay_search_stride_commit(&st, data, len, limit ? limit : 638u);
    uint32_t d = relay_search_stride_settle(&st, data, len, limit ? limit : 653u);
    uint32_t e = relay_search_stride_publish(&st, data, len, limit ? limit : 666u);
    uint32_t f = relay_search_stride_retire(&st, data, len, limit ? limit : 681u);
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

int relay_search_stride_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_search_stride(seed, limit ? limit : 1632u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_search_stride_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(148u & 3u);
    return current;
}

uint32_t relay_search_stride_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_search_stride_turn(left ^ right, epoch + 5641u);
    mixed ^= relay_search_stride_turn(right + epoch, left + 5652u);
    return mixed ^ (mixed >> 11);
}
