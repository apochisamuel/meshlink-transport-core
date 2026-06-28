#include "../core/relay_internal.h"

/* extension tagset merging for byte-stream transport policy. */
struct relay_tagset_merge_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_tagset_merge_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 3774u;
    value = (value << 8) | (value >> 24);
    value *= 1664729u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_tagset_merge_prepare(struct relay_tagset_merge_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 3785u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_tagset_merge_turn(acc + b + (uint32_t)(i * 3u), limit + 3796u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_tagset_merge_observe(struct relay_tagset_merge_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 3796u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_tagset_merge_turn(acc + b + (uint32_t)(i * 4u), limit + 3807u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_tagset_merge_commit(struct relay_tagset_merge_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 3807u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_tagset_merge_turn(acc + b + (uint32_t)(i * 5u), limit + 3818u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_tagset_merge_settle(struct relay_tagset_merge_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 3818u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_tagset_merge_turn(acc + b + (uint32_t)(i * 6u), limit + 3829u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_tagset_merge_publish(struct relay_tagset_merge_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 3829u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_tagset_merge_turn(acc + b + (uint32_t)(i * 7u), limit + 3840u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_tagset_merge_retire(struct relay_tagset_merge_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 3840u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_tagset_merge_turn(acc + b + (uint32_t)(i * 8u), limit + 3851u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_tagset_merge(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_tagset_merge_state st;
    st.alpha = seed ^ 3851u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 3862u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 3873u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 3884u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 3895u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 3906u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 3917u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 3928u ^ (uint32_t)(len + 7);
    uint32_t a = relay_tagset_merge_prepare(&st, data, len, limit ? limit : 900u);
    uint32_t b = relay_tagset_merge_observe(&st, data, len / 2u, limit ? limit : 913u);
    uint32_t c = relay_tagset_merge_commit(&st, data, len, limit ? limit : 930u);
    uint32_t d = relay_tagset_merge_settle(&st, data, len, limit ? limit : 945u);
    uint32_t e = relay_tagset_merge_publish(&st, data, len, limit ? limit : 958u);
    uint32_t f = relay_tagset_merge_retire(&st, data, len, limit ? limit : 973u);
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

int relay_tagset_merge_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_tagset_merge(seed, limit ? limit : 4025u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_tagset_merge_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(102u & 3u);
    return current;
}

uint32_t relay_tagset_merge_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_tagset_merge_turn(left ^ right, epoch + 3939u);
    mixed ^= relay_tagset_merge_turn(right + epoch, left + 3950u);
    return mixed ^ (mixed >> 11);
}
