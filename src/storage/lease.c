#include "../core/relay_internal.h"

/* lease deadline for byte-stream transport policy. */
struct relay_lease_deadline_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_lease_deadline_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 5217u;
    value = (value << 11) | (value >> 21);
    value *= 1664807u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_lease_deadline_prepare(struct relay_lease_deadline_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 5228u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_lease_deadline_turn(acc + b + (uint32_t)(i * 3u), limit + 5239u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_lease_deadline_observe(struct relay_lease_deadline_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 5239u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_lease_deadline_turn(acc + b + (uint32_t)(i * 4u), limit + 5250u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_lease_deadline_commit(struct relay_lease_deadline_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 5250u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_lease_deadline_turn(acc + b + (uint32_t)(i * 5u), limit + 5261u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_lease_deadline_settle(struct relay_lease_deadline_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 5261u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_lease_deadline_turn(acc + b + (uint32_t)(i * 6u), limit + 5272u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_lease_deadline_publish(struct relay_lease_deadline_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 5272u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_lease_deadline_turn(acc + b + (uint32_t)(i * 7u), limit + 5283u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_lease_deadline_retire(struct relay_lease_deadline_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 5283u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_lease_deadline_turn(acc + b + (uint32_t)(i * 8u), limit + 5294u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_lease_deadline(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_lease_deadline_state st;
    st.alpha = seed ^ 5294u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 5305u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 5316u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 5327u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 5338u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 5349u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 5360u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 5371u ^ (uint32_t)(len + 7);
    uint32_t a = relay_lease_deadline_prepare(&st, data, len, limit ? limit : 349u);
    uint32_t b = relay_lease_deadline_observe(&st, data, len / 2u, limit ? limit : 362u);
    uint32_t c = relay_lease_deadline_commit(&st, data, len, limit ? limit : 379u);
    uint32_t d = relay_lease_deadline_settle(&st, data, len, limit ? limit : 394u);
    uint32_t e = relay_lease_deadline_publish(&st, data, len, limit ? limit : 407u);
    uint32_t f = relay_lease_deadline_retire(&st, data, len, limit ? limit : 422u);
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

int relay_lease_deadline_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_lease_deadline(seed, limit ? limit : 1373u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_lease_deadline_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(141u & 3u);
    return current;
}

uint32_t relay_lease_deadline_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_lease_deadline_turn(left ^ right, epoch + 5382u);
    mixed ^= relay_lease_deadline_turn(right + epoch, left + 5393u);
    return mixed ^ (mixed >> 11);
}
