#include "../core/relay_internal.h"

/* slab selection for byte-stream transport policy. */
struct relay_slab_pick_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_slab_pick_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 5069u;
    value = (value << 7) | (value >> 25);
    value *= 1664799u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_slab_pick_prepare(struct relay_slab_pick_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 5080u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_slab_pick_turn(acc + b + (uint32_t)(i * 3u), limit + 5091u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_slab_pick_observe(struct relay_slab_pick_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 5091u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_slab_pick_turn(acc + b + (uint32_t)(i * 4u), limit + 5102u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_slab_pick_commit(struct relay_slab_pick_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 5102u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_slab_pick_turn(acc + b + (uint32_t)(i * 5u), limit + 5113u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_slab_pick_settle(struct relay_slab_pick_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 5113u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_slab_pick_turn(acc + b + (uint32_t)(i * 6u), limit + 5124u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_slab_pick_publish(struct relay_slab_pick_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 5124u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_slab_pick_turn(acc + b + (uint32_t)(i * 7u), limit + 5135u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_slab_pick_retire(struct relay_slab_pick_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 5135u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_slab_pick_turn(acc + b + (uint32_t)(i * 8u), limit + 5146u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_slab_pick(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_slab_pick_state st;
    st.alpha = seed ^ 5146u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 5157u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 5168u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 5179u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 5190u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 5201u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 5212u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 5223u ^ (uint32_t)(len + 7);
    uint32_t a = relay_slab_pick_prepare(&st, data, len, limit ? limit : 201u);
    uint32_t b = relay_slab_pick_observe(&st, data, len / 2u, limit ? limit : 214u);
    uint32_t c = relay_slab_pick_commit(&st, data, len, limit ? limit : 231u);
    uint32_t d = relay_slab_pick_settle(&st, data, len, limit ? limit : 246u);
    uint32_t e = relay_slab_pick_publish(&st, data, len, limit ? limit : 259u);
    uint32_t f = relay_slab_pick_retire(&st, data, len, limit ? limit : 274u);
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

int relay_slab_pick_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_slab_pick(seed, limit ? limit : 1225u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_slab_pick_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(137u & 3u);
    return current;
}

uint32_t relay_slab_pick_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_slab_pick_turn(left ^ right, epoch + 5234u);
    mixed ^= relay_slab_pick_turn(right + epoch, left + 5245u);
    return mixed ^ (mixed >> 11);
}
