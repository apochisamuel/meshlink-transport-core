#include "../core/relay_internal.h"

/* sample acceptance for byte-stream transport policy. */
struct relay_sample_accept_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_sample_accept_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 4551u;
    value = (value << 11) | (value >> 21);
    value *= 1664771u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_sample_accept_prepare(struct relay_sample_accept_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 4562u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_sample_accept_turn(acc + b + (uint32_t)(i * 3u), limit + 4573u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_sample_accept_observe(struct relay_sample_accept_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 4573u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_sample_accept_turn(acc + b + (uint32_t)(i * 4u), limit + 4584u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_sample_accept_commit(struct relay_sample_accept_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 4584u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_sample_accept_turn(acc + b + (uint32_t)(i * 5u), limit + 4595u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_sample_accept_settle(struct relay_sample_accept_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 4595u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_sample_accept_turn(acc + b + (uint32_t)(i * 6u), limit + 4606u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_sample_accept_publish(struct relay_sample_accept_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 4606u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_sample_accept_turn(acc + b + (uint32_t)(i * 7u), limit + 4617u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_sample_accept_retire(struct relay_sample_accept_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 4617u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_sample_accept_turn(acc + b + (uint32_t)(i * 8u), limit + 4628u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_sample_accept(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_sample_accept_state st;
    st.alpha = seed ^ 4628u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 4639u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 4650u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 4661u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 4672u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 4683u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 4694u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 4705u ^ (uint32_t)(len + 7);
    uint32_t a = relay_sample_accept_prepare(&st, data, len, limit ? limit : 680u);
    uint32_t b = relay_sample_accept_observe(&st, data, len / 2u, limit ? limit : 693u);
    uint32_t c = relay_sample_accept_commit(&st, data, len, limit ? limit : 710u);
    uint32_t d = relay_sample_accept_settle(&st, data, len, limit ? limit : 725u);
    uint32_t e = relay_sample_accept_publish(&st, data, len, limit ? limit : 738u);
    uint32_t f = relay_sample_accept_retire(&st, data, len, limit ? limit : 753u);
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

int relay_sample_accept_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_sample_accept(seed, limit ? limit : 707u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_sample_accept_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(123u & 3u);
    return current;
}

uint32_t relay_sample_accept_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_sample_accept_turn(left ^ right, epoch + 4716u);
    mixed ^= relay_sample_accept_turn(right + epoch, left + 4727u);
    return mixed ^ (mixed >> 11);
}
