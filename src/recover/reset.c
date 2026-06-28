#include "../core/relay_internal.h"

/* reset budget for byte-stream transport policy. */
struct relay_reset_budget_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_reset_budget_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 6068u;
    value = (value << 7) | (value >> 25);
    value *= 1664853u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_reset_budget_prepare(struct relay_reset_budget_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 6079u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_reset_budget_turn(acc + b + (uint32_t)(i * 3u), limit + 6090u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_reset_budget_observe(struct relay_reset_budget_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 6090u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_reset_budget_turn(acc + b + (uint32_t)(i * 4u), limit + 6101u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_reset_budget_commit(struct relay_reset_budget_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 6101u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_reset_budget_turn(acc + b + (uint32_t)(i * 5u), limit + 6112u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_reset_budget_settle(struct relay_reset_budget_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 6112u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_reset_budget_turn(acc + b + (uint32_t)(i * 6u), limit + 6123u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_reset_budget_publish(struct relay_reset_budget_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 6123u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_reset_budget_turn(acc + b + (uint32_t)(i * 7u), limit + 6134u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_reset_budget_retire(struct relay_reset_budget_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 6134u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_reset_budget_turn(acc + b + (uint32_t)(i * 8u), limit + 6145u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_reset_budget(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_reset_budget_state st;
    st.alpha = seed ^ 6145u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 6156u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 6167u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 6178u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 6189u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 6200u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 6211u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 6222u ^ (uint32_t)(len + 7);
    uint32_t a = relay_reset_budget_prepare(&st, data, len, limit ? limit : 203u);
    uint32_t b = relay_reset_budget_observe(&st, data, len / 2u, limit ? limit : 216u);
    uint32_t c = relay_reset_budget_commit(&st, data, len, limit ? limit : 233u);
    uint32_t d = relay_reset_budget_settle(&st, data, len, limit ? limit : 248u);
    uint32_t e = relay_reset_budget_publish(&st, data, len, limit ? limit : 261u);
    uint32_t f = relay_reset_budget_retire(&st, data, len, limit ? limit : 276u);
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

int relay_reset_budget_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_reset_budget(seed, limit ? limit : 2224u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_reset_budget_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(164u & 3u);
    return current;
}

uint32_t relay_reset_budget_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_reset_budget_turn(left ^ right, epoch + 6233u);
    mixed ^= relay_reset_budget_turn(right + epoch, left + 6244u);
    return mixed ^ (mixed >> 11);
}
