#include "../core/relay_internal.h"

/* cache scorecard for byte-stream transport policy. */
struct relay_cache_scorecard_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_cache_scorecard_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 5143u;
    value = (value << 9) | (value >> 23);
    value *= 1664803u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_cache_scorecard_prepare(struct relay_cache_scorecard_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 5154u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_cache_scorecard_turn(acc + b + (uint32_t)(i * 3u), limit + 5165u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_cache_scorecard_observe(struct relay_cache_scorecard_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 5165u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_cache_scorecard_turn(acc + b + (uint32_t)(i * 4u), limit + 5176u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_cache_scorecard_commit(struct relay_cache_scorecard_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 5176u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_cache_scorecard_turn(acc + b + (uint32_t)(i * 5u), limit + 5187u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_cache_scorecard_settle(struct relay_cache_scorecard_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 5187u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_cache_scorecard_turn(acc + b + (uint32_t)(i * 6u), limit + 5198u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_cache_scorecard_publish(struct relay_cache_scorecard_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 5198u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_cache_scorecard_turn(acc + b + (uint32_t)(i * 7u), limit + 5209u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_cache_scorecard_retire(struct relay_cache_scorecard_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 5209u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_cache_scorecard_turn(acc + b + (uint32_t)(i * 8u), limit + 5220u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_cache_scorecard(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_cache_scorecard_state st;
    st.alpha = seed ^ 5220u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 5231u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 5242u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 5253u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 5264u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 5275u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 5286u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 5297u ^ (uint32_t)(len + 7);
    uint32_t a = relay_cache_scorecard_prepare(&st, data, len, limit ? limit : 275u);
    uint32_t b = relay_cache_scorecard_observe(&st, data, len / 2u, limit ? limit : 288u);
    uint32_t c = relay_cache_scorecard_commit(&st, data, len, limit ? limit : 305u);
    uint32_t d = relay_cache_scorecard_settle(&st, data, len, limit ? limit : 320u);
    uint32_t e = relay_cache_scorecard_publish(&st, data, len, limit ? limit : 333u);
    uint32_t f = relay_cache_scorecard_retire(&st, data, len, limit ? limit : 348u);
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

int relay_cache_scorecard_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_cache_scorecard(seed, limit ? limit : 1299u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_cache_scorecard_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(139u & 3u);
    return current;
}

uint32_t relay_cache_scorecard_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_cache_scorecard_turn(left ^ right, epoch + 5308u);
    mixed ^= relay_cache_scorecard_turn(right + epoch, left + 5319u);
    return mixed ^ (mixed >> 11);
}
