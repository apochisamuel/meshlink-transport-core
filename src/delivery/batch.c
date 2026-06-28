#include "../core/relay_internal.h"

/* batch sizing for byte-stream transport policy. */
struct relay_batch_limit_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_batch_limit_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 4181u;
    value = (value << 10) | (value >> 22);
    value *= 1664751u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_batch_limit_prepare(struct relay_batch_limit_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 4192u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_batch_limit_turn(acc + b + (uint32_t)(i * 3u), limit + 4203u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_batch_limit_observe(struct relay_batch_limit_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 4203u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_batch_limit_turn(acc + b + (uint32_t)(i * 4u), limit + 4214u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_batch_limit_commit(struct relay_batch_limit_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 4214u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_batch_limit_turn(acc + b + (uint32_t)(i * 5u), limit + 4225u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_batch_limit_settle(struct relay_batch_limit_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 4225u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_batch_limit_turn(acc + b + (uint32_t)(i * 6u), limit + 4236u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_batch_limit_publish(struct relay_batch_limit_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 4236u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_batch_limit_turn(acc + b + (uint32_t)(i * 7u), limit + 4247u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_batch_limit_retire(struct relay_batch_limit_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 4247u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_batch_limit_turn(acc + b + (uint32_t)(i * 8u), limit + 4258u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_batch_limit(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_batch_limit_state st;
    st.alpha = seed ^ 4258u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 4269u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 4280u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 4291u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 4302u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 4313u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 4324u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 4335u ^ (uint32_t)(len + 7);
    uint32_t a = relay_batch_limit_prepare(&st, data, len, limit ? limit : 310u);
    uint32_t b = relay_batch_limit_observe(&st, data, len / 2u, limit ? limit : 323u);
    uint32_t c = relay_batch_limit_commit(&st, data, len, limit ? limit : 340u);
    uint32_t d = relay_batch_limit_settle(&st, data, len, limit ? limit : 355u);
    uint32_t e = relay_batch_limit_publish(&st, data, len, limit ? limit : 368u);
    uint32_t f = relay_batch_limit_retire(&st, data, len, limit ? limit : 383u);
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

int relay_batch_limit_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_batch_limit(seed, limit ? limit : 337u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_batch_limit_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(113u & 3u);
    return current;
}

uint32_t relay_batch_limit_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_batch_limit_turn(left ^ right, epoch + 4346u);
    mixed ^= relay_batch_limit_turn(right + epoch, left + 4357u);
    return mixed ^ (mixed >> 11);
}
