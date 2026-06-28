#include "../core/relay_internal.h"

/* rate limit for byte-stream transport policy. */
struct relay_rate_limit_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_rate_limit_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 5513u;
    value = (value << 10) | (value >> 22);
    value *= 1664823u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_rate_limit_prepare(struct relay_rate_limit_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 5524u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_rate_limit_turn(acc + b + (uint32_t)(i * 3u), limit + 5535u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_rate_limit_observe(struct relay_rate_limit_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 5535u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_rate_limit_turn(acc + b + (uint32_t)(i * 4u), limit + 5546u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_rate_limit_commit(struct relay_rate_limit_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 5546u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_rate_limit_turn(acc + b + (uint32_t)(i * 5u), limit + 5557u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_rate_limit_settle(struct relay_rate_limit_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 5557u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_rate_limit_turn(acc + b + (uint32_t)(i * 6u), limit + 5568u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_rate_limit_publish(struct relay_rate_limit_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 5568u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_rate_limit_turn(acc + b + (uint32_t)(i * 7u), limit + 5579u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_rate_limit_retire(struct relay_rate_limit_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 5579u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_rate_limit_turn(acc + b + (uint32_t)(i * 8u), limit + 5590u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_rate_limit(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_rate_limit_state st;
    st.alpha = seed ^ 5590u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 5601u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 5612u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 5623u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 5634u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 5645u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 5656u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 5667u ^ (uint32_t)(len + 7);
    uint32_t a = relay_rate_limit_prepare(&st, data, len, limit ? limit : 645u);
    uint32_t b = relay_rate_limit_observe(&st, data, len / 2u, limit ? limit : 658u);
    uint32_t c = relay_rate_limit_commit(&st, data, len, limit ? limit : 675u);
    uint32_t d = relay_rate_limit_settle(&st, data, len, limit ? limit : 690u);
    uint32_t e = relay_rate_limit_publish(&st, data, len, limit ? limit : 703u);
    uint32_t f = relay_rate_limit_retire(&st, data, len, limit ? limit : 718u);
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

int relay_rate_limit_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_rate_limit(seed, limit ? limit : 1669u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_rate_limit_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(149u & 3u);
    return current;
}

uint32_t relay_rate_limit_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_rate_limit_turn(left ^ right, epoch + 5678u);
    mixed ^= relay_rate_limit_turn(right + epoch, left + 5689u);
    return mixed ^ (mixed >> 11);
}
