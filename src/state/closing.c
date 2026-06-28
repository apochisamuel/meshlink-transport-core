#include "../core/relay_internal.h"

/* closing phase weighting for byte-stream transport policy. */
struct relay_closing_weight_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_closing_weight_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 4070u;
    value = (value << 7) | (value >> 25);
    value *= 1664745u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_closing_weight_prepare(struct relay_closing_weight_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 4081u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_closing_weight_turn(acc + b + (uint32_t)(i * 3u), limit + 4092u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_closing_weight_observe(struct relay_closing_weight_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 4092u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_closing_weight_turn(acc + b + (uint32_t)(i * 4u), limit + 4103u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_closing_weight_commit(struct relay_closing_weight_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 4103u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_closing_weight_turn(acc + b + (uint32_t)(i * 5u), limit + 4114u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_closing_weight_settle(struct relay_closing_weight_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 4114u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_closing_weight_turn(acc + b + (uint32_t)(i * 6u), limit + 4125u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_closing_weight_publish(struct relay_closing_weight_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 4125u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_closing_weight_turn(acc + b + (uint32_t)(i * 7u), limit + 4136u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_closing_weight_retire(struct relay_closing_weight_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 4136u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_closing_weight_turn(acc + b + (uint32_t)(i * 8u), limit + 4147u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_closing_weight(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_closing_weight_state st;
    st.alpha = seed ^ 4147u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 4158u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 4169u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 4180u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 4191u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 4202u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 4213u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 4224u ^ (uint32_t)(len + 7);
    uint32_t a = relay_closing_weight_prepare(&st, data, len, limit ? limit : 199u);
    uint32_t b = relay_closing_weight_observe(&st, data, len / 2u, limit ? limit : 212u);
    uint32_t c = relay_closing_weight_commit(&st, data, len, limit ? limit : 229u);
    uint32_t d = relay_closing_weight_settle(&st, data, len, limit ? limit : 244u);
    uint32_t e = relay_closing_weight_publish(&st, data, len, limit ? limit : 257u);
    uint32_t f = relay_closing_weight_retire(&st, data, len, limit ? limit : 272u);
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

int relay_closing_weight_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_closing_weight(seed, limit ? limit : 226u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_closing_weight_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(110u & 3u);
    return current;
}

uint32_t relay_closing_weight_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_closing_weight_turn(left ^ right, epoch + 4235u);
    mixed ^= relay_closing_weight_turn(right + epoch, left + 4246u);
    return mixed ^ (mixed >> 11);
}
