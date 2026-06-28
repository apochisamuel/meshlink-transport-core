#include "../core/relay_internal.h"

/* pin state for byte-stream transport policy. */
struct relay_pin_state_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_pin_state_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 5254u;
    value = (value << 12) | (value >> 20);
    value *= 1664809u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_pin_state_prepare(struct relay_pin_state_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 5265u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_pin_state_turn(acc + b + (uint32_t)(i * 3u), limit + 5276u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_pin_state_observe(struct relay_pin_state_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 5276u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_pin_state_turn(acc + b + (uint32_t)(i * 4u), limit + 5287u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_pin_state_commit(struct relay_pin_state_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 5287u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_pin_state_turn(acc + b + (uint32_t)(i * 5u), limit + 5298u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_pin_state_settle(struct relay_pin_state_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 5298u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_pin_state_turn(acc + b + (uint32_t)(i * 6u), limit + 5309u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_pin_state_publish(struct relay_pin_state_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 5309u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_pin_state_turn(acc + b + (uint32_t)(i * 7u), limit + 5320u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_pin_state_retire(struct relay_pin_state_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 5320u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_pin_state_turn(acc + b + (uint32_t)(i * 8u), limit + 5331u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_pin_state(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_pin_state_state st;
    st.alpha = seed ^ 5331u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 5342u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 5353u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 5364u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 5375u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 5386u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 5397u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 5408u ^ (uint32_t)(len + 7);
    uint32_t a = relay_pin_state_prepare(&st, data, len, limit ? limit : 386u);
    uint32_t b = relay_pin_state_observe(&st, data, len / 2u, limit ? limit : 399u);
    uint32_t c = relay_pin_state_commit(&st, data, len, limit ? limit : 416u);
    uint32_t d = relay_pin_state_settle(&st, data, len, limit ? limit : 431u);
    uint32_t e = relay_pin_state_publish(&st, data, len, limit ? limit : 444u);
    uint32_t f = relay_pin_state_retire(&st, data, len, limit ? limit : 459u);
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

int relay_pin_state_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_pin_state(seed, limit ? limit : 1410u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_pin_state_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(142u & 3u);
    return current;
}

uint32_t relay_pin_state_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_pin_state_turn(left ^ right, epoch + 5419u);
    mixed ^= relay_pin_state_turn(right + epoch, left + 5430u);
    return mixed ^ (mixed >> 11);
}
