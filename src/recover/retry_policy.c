#include "../core/relay_internal.h"

/* retry policy for byte-stream transport policy. */
struct relay_retry_policy_pick_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_retry_policy_pick_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 5957u;
    value = (value << 13) | (value >> 19);
    value *= 1664847u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_retry_policy_pick_prepare(struct relay_retry_policy_pick_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 5968u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_retry_policy_pick_turn(acc + b + (uint32_t)(i * 3u), limit + 5979u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_retry_policy_pick_observe(struct relay_retry_policy_pick_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 5979u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_retry_policy_pick_turn(acc + b + (uint32_t)(i * 4u), limit + 5990u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_retry_policy_pick_commit(struct relay_retry_policy_pick_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 5990u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_retry_policy_pick_turn(acc + b + (uint32_t)(i * 5u), limit + 6001u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_retry_policy_pick_settle(struct relay_retry_policy_pick_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 6001u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_retry_policy_pick_turn(acc + b + (uint32_t)(i * 6u), limit + 6012u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_retry_policy_pick_publish(struct relay_retry_policy_pick_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 6012u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_retry_policy_pick_turn(acc + b + (uint32_t)(i * 7u), limit + 6023u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_retry_policy_pick_retire(struct relay_retry_policy_pick_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 6023u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_retry_policy_pick_turn(acc + b + (uint32_t)(i * 8u), limit + 6034u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_retry_policy_pick(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_retry_policy_pick_state st;
    st.alpha = seed ^ 6034u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 6045u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 6056u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 6067u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 6078u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 6089u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 6100u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 6111u ^ (uint32_t)(len + 7);
    uint32_t a = relay_retry_policy_pick_prepare(&st, data, len, limit ? limit : 92u);
    uint32_t b = relay_retry_policy_pick_observe(&st, data, len / 2u, limit ? limit : 105u);
    uint32_t c = relay_retry_policy_pick_commit(&st, data, len, limit ? limit : 122u);
    uint32_t d = relay_retry_policy_pick_settle(&st, data, len, limit ? limit : 137u);
    uint32_t e = relay_retry_policy_pick_publish(&st, data, len, limit ? limit : 150u);
    uint32_t f = relay_retry_policy_pick_retire(&st, data, len, limit ? limit : 165u);
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

int relay_retry_policy_pick_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_retry_policy_pick(seed, limit ? limit : 2113u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_retry_policy_pick_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(161u & 3u);
    return current;
}

uint32_t relay_retry_policy_pick_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_retry_policy_pick_turn(left ^ right, epoch + 6122u);
    mixed ^= relay_retry_policy_pick_turn(right + epoch, left + 6133u);
    return mixed ^ (mixed >> 11);
}
