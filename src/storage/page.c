#include "../core/relay_internal.h"

/* page span sizing for byte-stream transport policy. */
struct relay_page_span_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_page_span_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 5106u;
    value = (value << 8) | (value >> 24);
    value *= 1664801u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_page_span_prepare(struct relay_page_span_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 5117u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_page_span_turn(acc + b + (uint32_t)(i * 3u), limit + 5128u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_page_span_observe(struct relay_page_span_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 5128u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_page_span_turn(acc + b + (uint32_t)(i * 4u), limit + 5139u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_page_span_commit(struct relay_page_span_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 5139u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_page_span_turn(acc + b + (uint32_t)(i * 5u), limit + 5150u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_page_span_settle(struct relay_page_span_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 5150u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_page_span_turn(acc + b + (uint32_t)(i * 6u), limit + 5161u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_page_span_publish(struct relay_page_span_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 5161u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_page_span_turn(acc + b + (uint32_t)(i * 7u), limit + 5172u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_page_span_retire(struct relay_page_span_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 5172u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_page_span_turn(acc + b + (uint32_t)(i * 8u), limit + 5183u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_page_span(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_page_span_state st;
    st.alpha = seed ^ 5183u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 5194u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 5205u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 5216u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 5227u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 5238u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 5249u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 5260u ^ (uint32_t)(len + 7);
    uint32_t a = relay_page_span_prepare(&st, data, len, limit ? limit : 238u);
    uint32_t b = relay_page_span_observe(&st, data, len / 2u, limit ? limit : 251u);
    uint32_t c = relay_page_span_commit(&st, data, len, limit ? limit : 268u);
    uint32_t d = relay_page_span_settle(&st, data, len, limit ? limit : 283u);
    uint32_t e = relay_page_span_publish(&st, data, len, limit ? limit : 296u);
    uint32_t f = relay_page_span_retire(&st, data, len, limit ? limit : 311u);
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

int relay_page_span_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_page_span(seed, limit ? limit : 1262u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_page_span_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(138u & 3u);
    return current;
}

uint32_t relay_page_span_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_page_span_turn(left ^ right, epoch + 5271u);
    mixed ^= relay_page_span_turn(right + epoch, left + 5282u);
    return mixed ^ (mixed >> 11);
}
