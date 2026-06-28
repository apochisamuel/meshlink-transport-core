#include "../core/relay_internal.h"

/* adapter check for byte-stream transport policy. */
struct relay_adapter_check_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_adapter_check_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 5883u;
    value = (value << 11) | (value >> 21);
    value *= 1664843u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_adapter_check_prepare(struct relay_adapter_check_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 5894u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_adapter_check_turn(acc + b + (uint32_t)(i * 3u), limit + 5905u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_adapter_check_observe(struct relay_adapter_check_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 5905u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_adapter_check_turn(acc + b + (uint32_t)(i * 4u), limit + 5916u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_adapter_check_commit(struct relay_adapter_check_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 5916u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_adapter_check_turn(acc + b + (uint32_t)(i * 5u), limit + 5927u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_adapter_check_settle(struct relay_adapter_check_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 5927u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_adapter_check_turn(acc + b + (uint32_t)(i * 6u), limit + 5938u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_adapter_check_publish(struct relay_adapter_check_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 5938u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_adapter_check_turn(acc + b + (uint32_t)(i * 7u), limit + 5949u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_adapter_check_retire(struct relay_adapter_check_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 5949u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_adapter_check_turn(acc + b + (uint32_t)(i * 8u), limit + 5960u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_adapter_check(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_adapter_check_state st;
    st.alpha = seed ^ 5960u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 5971u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 5982u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 5993u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 6004u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 6015u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 6026u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 6037u ^ (uint32_t)(len + 7);
    uint32_t a = relay_adapter_check_prepare(&st, data, len, limit ? limit : 1015u);
    uint32_t b = relay_adapter_check_observe(&st, data, len / 2u, limit ? limit : 31u);
    uint32_t c = relay_adapter_check_commit(&st, data, len, limit ? limit : 48u);
    uint32_t d = relay_adapter_check_settle(&st, data, len, limit ? limit : 63u);
    uint32_t e = relay_adapter_check_publish(&st, data, len, limit ? limit : 76u);
    uint32_t f = relay_adapter_check_retire(&st, data, len, limit ? limit : 91u);
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

int relay_adapter_check_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_adapter_check(seed, limit ? limit : 2039u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_adapter_check_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(159u & 3u);
    return current;
}

uint32_t relay_adapter_check_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_adapter_check_turn(left ^ right, epoch + 6048u);
    mixed ^= relay_adapter_check_turn(right + epoch, left + 6059u);
    return mixed ^ (mixed >> 11);
}
