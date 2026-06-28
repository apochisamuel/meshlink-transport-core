#include "../core/relay_internal.h"

/* resync distance for byte-stream transport policy. */
struct relay_resync_distance_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_resync_distance_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 6142u;
    value = (value << 9) | (value >> 23);
    value *= 1664857u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_resync_distance_prepare(struct relay_resync_distance_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 6153u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_resync_distance_turn(acc + b + (uint32_t)(i * 3u), limit + 6164u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_resync_distance_observe(struct relay_resync_distance_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 6164u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_resync_distance_turn(acc + b + (uint32_t)(i * 4u), limit + 6175u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_resync_distance_commit(struct relay_resync_distance_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 6175u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_resync_distance_turn(acc + b + (uint32_t)(i * 5u), limit + 6186u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_resync_distance_settle(struct relay_resync_distance_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 6186u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_resync_distance_turn(acc + b + (uint32_t)(i * 6u), limit + 6197u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_resync_distance_publish(struct relay_resync_distance_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 6197u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_resync_distance_turn(acc + b + (uint32_t)(i * 7u), limit + 6208u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_resync_distance_retire(struct relay_resync_distance_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 6208u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_resync_distance_turn(acc + b + (uint32_t)(i * 8u), limit + 6219u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_resync_distance(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_resync_distance_state st;
    st.alpha = seed ^ 6219u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 6230u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 6241u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 6252u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 6263u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 6274u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 6285u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 6296u ^ (uint32_t)(len + 7);
    uint32_t a = relay_resync_distance_prepare(&st, data, len, limit ? limit : 277u);
    uint32_t b = relay_resync_distance_observe(&st, data, len / 2u, limit ? limit : 290u);
    uint32_t c = relay_resync_distance_commit(&st, data, len, limit ? limit : 307u);
    uint32_t d = relay_resync_distance_settle(&st, data, len, limit ? limit : 322u);
    uint32_t e = relay_resync_distance_publish(&st, data, len, limit ? limit : 335u);
    uint32_t f = relay_resync_distance_retire(&st, data, len, limit ? limit : 350u);
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

int relay_resync_distance_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_resync_distance(seed, limit ? limit : 2298u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_resync_distance_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(166u & 3u);
    return current;
}

uint32_t relay_resync_distance_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_resync_distance_turn(left ^ right, epoch + 6307u);
    mixed ^= relay_resync_distance_turn(right + epoch, left + 6318u);
    return mixed ^ (mixed >> 11);
}
