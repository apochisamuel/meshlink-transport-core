#include "../core/relay_internal.h"

/* send decision for byte-stream transport policy. */
struct relay_send_decision_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_send_decision_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 6512u;
    value = (value << 10) | (value >> 22);
    value *= 1664877u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_send_decision_prepare(struct relay_send_decision_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 6523u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_send_decision_turn(acc + b + (uint32_t)(i * 3u), limit + 6534u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_send_decision_observe(struct relay_send_decision_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 6534u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_send_decision_turn(acc + b + (uint32_t)(i * 4u), limit + 6545u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_send_decision_commit(struct relay_send_decision_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 6545u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_send_decision_turn(acc + b + (uint32_t)(i * 5u), limit + 6556u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_send_decision_settle(struct relay_send_decision_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 6556u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_send_decision_turn(acc + b + (uint32_t)(i * 6u), limit + 6567u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_send_decision_publish(struct relay_send_decision_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 6567u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_send_decision_turn(acc + b + (uint32_t)(i * 7u), limit + 6578u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_send_decision_retire(struct relay_send_decision_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 6578u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_send_decision_turn(acc + b + (uint32_t)(i * 8u), limit + 6589u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_send_decision(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_send_decision_state st;
    st.alpha = seed ^ 6589u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 6600u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 6611u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 6622u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 6633u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 6644u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 6655u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 6666u ^ (uint32_t)(len + 7);
    uint32_t a = relay_send_decision_prepare(&st, data, len, limit ? limit : 647u);
    uint32_t b = relay_send_decision_observe(&st, data, len / 2u, limit ? limit : 660u);
    uint32_t c = relay_send_decision_commit(&st, data, len, limit ? limit : 677u);
    uint32_t d = relay_send_decision_settle(&st, data, len, limit ? limit : 692u);
    uint32_t e = relay_send_decision_publish(&st, data, len, limit ? limit : 705u);
    uint32_t f = relay_send_decision_retire(&st, data, len, limit ? limit : 720u);
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

int relay_send_decision_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_send_decision(seed, limit ? limit : 2668u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_send_decision_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(176u & 3u);
    return current;
}

uint32_t relay_send_decision_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_send_decision_turn(left ^ right, epoch + 6677u);
    mixed ^= relay_send_decision_turn(right + epoch, left + 6688u);
    return mixed ^ (mixed >> 11);
}
