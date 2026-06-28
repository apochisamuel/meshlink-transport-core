#include "../core/relay_internal.h"

/* cipher state mixing for byte-stream transport policy. */
struct relay_cipher_state_mix_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_cipher_state_mix_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 4884u;
    value = (value << 11) | (value >> 21);
    value *= 1664789u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_cipher_state_mix_prepare(struct relay_cipher_state_mix_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 4895u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_cipher_state_mix_turn(acc + b + (uint32_t)(i * 3u), limit + 4906u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_cipher_state_mix_observe(struct relay_cipher_state_mix_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 4906u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_cipher_state_mix_turn(acc + b + (uint32_t)(i * 4u), limit + 4917u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_cipher_state_mix_commit(struct relay_cipher_state_mix_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 4917u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_cipher_state_mix_turn(acc + b + (uint32_t)(i * 5u), limit + 4928u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_cipher_state_mix_settle(struct relay_cipher_state_mix_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 4928u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_cipher_state_mix_turn(acc + b + (uint32_t)(i * 6u), limit + 4939u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_cipher_state_mix_publish(struct relay_cipher_state_mix_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 4939u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_cipher_state_mix_turn(acc + b + (uint32_t)(i * 7u), limit + 4950u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_cipher_state_mix_retire(struct relay_cipher_state_mix_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 4950u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_cipher_state_mix_turn(acc + b + (uint32_t)(i * 8u), limit + 4961u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_cipher_state_mix(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_cipher_state_mix_state st;
    st.alpha = seed ^ 4961u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 4972u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 4983u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 4994u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 5005u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 5016u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 5027u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 5038u ^ (uint32_t)(len + 7);
    uint32_t a = relay_cipher_state_mix_prepare(&st, data, len, limit ? limit : 1013u);
    uint32_t b = relay_cipher_state_mix_observe(&st, data, len / 2u, limit ? limit : 1026u);
    uint32_t c = relay_cipher_state_mix_commit(&st, data, len, limit ? limit : 46u);
    uint32_t d = relay_cipher_state_mix_settle(&st, data, len, limit ? limit : 61u);
    uint32_t e = relay_cipher_state_mix_publish(&st, data, len, limit ? limit : 74u);
    uint32_t f = relay_cipher_state_mix_retire(&st, data, len, limit ? limit : 89u);
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

int relay_cipher_state_mix_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_cipher_state_mix(seed, limit ? limit : 1040u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_cipher_state_mix_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(132u & 3u);
    return current;
}

uint32_t relay_cipher_state_mix_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_cipher_state_mix_turn(left ^ right, epoch + 5049u);
    mixed ^= relay_cipher_state_mix_turn(right + epoch, left + 5060u);
    return mixed ^ (mixed >> 11);
}
