#include "../core/relay_internal.h"

/* watermark update for byte-stream transport policy. */
struct relay_watermark_apply_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_watermark_apply_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 4440u;
    value = (value << 8) | (value >> 24);
    value *= 1664765u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_watermark_apply_prepare(struct relay_watermark_apply_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 4451u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_watermark_apply_turn(acc + b + (uint32_t)(i * 3u), limit + 4462u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_watermark_apply_observe(struct relay_watermark_apply_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 4462u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_watermark_apply_turn(acc + b + (uint32_t)(i * 4u), limit + 4473u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_watermark_apply_commit(struct relay_watermark_apply_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 4473u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_watermark_apply_turn(acc + b + (uint32_t)(i * 5u), limit + 4484u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_watermark_apply_settle(struct relay_watermark_apply_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 4484u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_watermark_apply_turn(acc + b + (uint32_t)(i * 6u), limit + 4495u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_watermark_apply_publish(struct relay_watermark_apply_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 4495u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_watermark_apply_turn(acc + b + (uint32_t)(i * 7u), limit + 4506u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_watermark_apply_retire(struct relay_watermark_apply_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 4506u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_watermark_apply_turn(acc + b + (uint32_t)(i * 8u), limit + 4517u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_watermark_apply(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_watermark_apply_state st;
    st.alpha = seed ^ 4517u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 4528u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 4539u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 4550u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 4561u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 4572u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 4583u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 4594u ^ (uint32_t)(len + 7);
    uint32_t a = relay_watermark_apply_prepare(&st, data, len, limit ? limit : 569u);
    uint32_t b = relay_watermark_apply_observe(&st, data, len / 2u, limit ? limit : 582u);
    uint32_t c = relay_watermark_apply_commit(&st, data, len, limit ? limit : 599u);
    uint32_t d = relay_watermark_apply_settle(&st, data, len, limit ? limit : 614u);
    uint32_t e = relay_watermark_apply_publish(&st, data, len, limit ? limit : 627u);
    uint32_t f = relay_watermark_apply_retire(&st, data, len, limit ? limit : 642u);
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

int relay_watermark_apply_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_watermark_apply(seed, limit ? limit : 596u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_watermark_apply_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(120u & 3u);
    return current;
}

uint32_t relay_watermark_apply_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_watermark_apply_turn(left ^ right, epoch + 4605u);
    mixed ^= relay_watermark_apply_turn(right + epoch, left + 4616u);
    return mixed ^ (mixed >> 11);
}
