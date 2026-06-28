#include "../core/relay_internal.h"

/* prefix classification for byte-stream transport policy. */
struct relay_prefix_classify_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_prefix_classify_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 3811u;
    value = (value << 9) | (value >> 23);
    value *= 1664731u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_prefix_classify_prepare(struct relay_prefix_classify_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 3822u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_prefix_classify_turn(acc + b + (uint32_t)(i * 3u), limit + 3833u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_prefix_classify_observe(struct relay_prefix_classify_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 3833u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_prefix_classify_turn(acc + b + (uint32_t)(i * 4u), limit + 3844u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_prefix_classify_commit(struct relay_prefix_classify_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 3844u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_prefix_classify_turn(acc + b + (uint32_t)(i * 5u), limit + 3855u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_prefix_classify_settle(struct relay_prefix_classify_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 3855u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_prefix_classify_turn(acc + b + (uint32_t)(i * 6u), limit + 3866u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_prefix_classify_publish(struct relay_prefix_classify_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 3866u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_prefix_classify_turn(acc + b + (uint32_t)(i * 7u), limit + 3877u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_prefix_classify_retire(struct relay_prefix_classify_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 3877u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_prefix_classify_turn(acc + b + (uint32_t)(i * 8u), limit + 3888u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_prefix_classify(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_prefix_classify_state st;
    st.alpha = seed ^ 3888u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 3899u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 3910u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 3921u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 3932u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 3943u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 3954u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 3965u ^ (uint32_t)(len + 7);
    uint32_t a = relay_prefix_classify_prepare(&st, data, len, limit ? limit : 937u);
    uint32_t b = relay_prefix_classify_observe(&st, data, len / 2u, limit ? limit : 950u);
    uint32_t c = relay_prefix_classify_commit(&st, data, len, limit ? limit : 967u);
    uint32_t d = relay_prefix_classify_settle(&st, data, len, limit ? limit : 982u);
    uint32_t e = relay_prefix_classify_publish(&st, data, len, limit ? limit : 995u);
    uint32_t f = relay_prefix_classify_retire(&st, data, len, limit ? limit : 1010u);
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

int relay_prefix_classify_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_prefix_classify(seed, limit ? limit : 4062u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_prefix_classify_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(103u & 3u);
    return current;
}

uint32_t relay_prefix_classify_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_prefix_classify_turn(left ^ right, epoch + 3976u);
    mixed ^= relay_prefix_classify_turn(right + epoch, left + 3987u);
    return mixed ^ (mixed >> 11);
}
