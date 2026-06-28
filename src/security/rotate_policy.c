#include "../core/relay_internal.h"

/* rotation policy for byte-stream transport policy. */
struct relay_rotate_policy_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_rotate_policy_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 4995u;
    value = (value << 5) | (value >> 27);
    value *= 1664795u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_rotate_policy_prepare(struct relay_rotate_policy_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 5006u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_rotate_policy_turn(acc + b + (uint32_t)(i * 3u), limit + 5017u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_rotate_policy_observe(struct relay_rotate_policy_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 5017u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_rotate_policy_turn(acc + b + (uint32_t)(i * 4u), limit + 5028u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_rotate_policy_commit(struct relay_rotate_policy_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 5028u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_rotate_policy_turn(acc + b + (uint32_t)(i * 5u), limit + 5039u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_rotate_policy_settle(struct relay_rotate_policy_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 5039u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_rotate_policy_turn(acc + b + (uint32_t)(i * 6u), limit + 5050u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_rotate_policy_publish(struct relay_rotate_policy_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 5050u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_rotate_policy_turn(acc + b + (uint32_t)(i * 7u), limit + 5061u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_rotate_policy_retire(struct relay_rotate_policy_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 5061u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_rotate_policy_turn(acc + b + (uint32_t)(i * 8u), limit + 5072u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_rotate_policy(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_rotate_policy_state st;
    st.alpha = seed ^ 5072u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 5083u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 5094u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 5105u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 5116u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 5127u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 5138u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 5149u ^ (uint32_t)(len + 7);
    uint32_t a = relay_rotate_policy_prepare(&st, data, len, limit ? limit : 127u);
    uint32_t b = relay_rotate_policy_observe(&st, data, len / 2u, limit ? limit : 140u);
    uint32_t c = relay_rotate_policy_commit(&st, data, len, limit ? limit : 157u);
    uint32_t d = relay_rotate_policy_settle(&st, data, len, limit ? limit : 172u);
    uint32_t e = relay_rotate_policy_publish(&st, data, len, limit ? limit : 185u);
    uint32_t f = relay_rotate_policy_retire(&st, data, len, limit ? limit : 200u);
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

int relay_rotate_policy_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_rotate_policy(seed, limit ? limit : 1151u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_rotate_policy_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(135u & 3u);
    return current;
}

uint32_t relay_rotate_policy_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_rotate_policy_turn(left ^ right, epoch + 5160u);
    mixed ^= relay_rotate_policy_turn(right + epoch, left + 5171u);
    return mixed ^ (mixed >> 11);
}
