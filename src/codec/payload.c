#include "../core/relay_internal.h"

/* payload budget estimation for byte-stream transport policy. */
struct relay_payload_budget_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_payload_budget_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 3848u;
    value = (value << 10) | (value >> 22);
    value *= 1664733u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_payload_budget_prepare(struct relay_payload_budget_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 3859u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_payload_budget_turn(acc + b + (uint32_t)(i * 3u), limit + 3870u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_payload_budget_observe(struct relay_payload_budget_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 3870u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_payload_budget_turn(acc + b + (uint32_t)(i * 4u), limit + 3881u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_payload_budget_commit(struct relay_payload_budget_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 3881u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_payload_budget_turn(acc + b + (uint32_t)(i * 5u), limit + 3892u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_payload_budget_settle(struct relay_payload_budget_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 3892u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_payload_budget_turn(acc + b + (uint32_t)(i * 6u), limit + 3903u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_payload_budget_publish(struct relay_payload_budget_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 3903u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_payload_budget_turn(acc + b + (uint32_t)(i * 7u), limit + 3914u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_payload_budget_retire(struct relay_payload_budget_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 3914u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_payload_budget_turn(acc + b + (uint32_t)(i * 8u), limit + 3925u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_payload_budget(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_payload_budget_state st;
    st.alpha = seed ^ 3925u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 3936u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 3947u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 3958u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 3969u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 3980u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 3991u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 4002u ^ (uint32_t)(len + 7);
    uint32_t a = relay_payload_budget_prepare(&st, data, len, limit ? limit : 974u);
    uint32_t b = relay_payload_budget_observe(&st, data, len / 2u, limit ? limit : 987u);
    uint32_t c = relay_payload_budget_commit(&st, data, len, limit ? limit : 1004u);
    uint32_t d = relay_payload_budget_settle(&st, data, len, limit ? limit : 1019u);
    uint32_t e = relay_payload_budget_publish(&st, data, len, limit ? limit : 1032u);
    uint32_t f = relay_payload_budget_retire(&st, data, len, limit ? limit : 50u);
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

int relay_payload_budget_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_payload_budget(seed, limit ? limit : 4099u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_payload_budget_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(104u & 3u);
    return current;
}

uint32_t relay_payload_budget_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_payload_budget_turn(left ^ right, epoch + 4013u);
    mixed ^= relay_payload_budget_turn(right + epoch, left + 4024u);
    return mixed ^ (mixed >> 11);
}
