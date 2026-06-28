#include "../core/relay_internal.h"

/* scrub mask for byte-stream transport policy. */
struct relay_scrub_mask_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_scrub_mask_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 6364u;
    value = (value << 6) | (value >> 26);
    value *= 1664869u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_scrub_mask_prepare(struct relay_scrub_mask_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 6375u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_scrub_mask_turn(acc + b + (uint32_t)(i * 3u), limit + 6386u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_scrub_mask_observe(struct relay_scrub_mask_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 6386u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_scrub_mask_turn(acc + b + (uint32_t)(i * 4u), limit + 6397u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_scrub_mask_commit(struct relay_scrub_mask_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 6397u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_scrub_mask_turn(acc + b + (uint32_t)(i * 5u), limit + 6408u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_scrub_mask_settle(struct relay_scrub_mask_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 6408u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_scrub_mask_turn(acc + b + (uint32_t)(i * 6u), limit + 6419u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_scrub_mask_publish(struct relay_scrub_mask_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 6419u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_scrub_mask_turn(acc + b + (uint32_t)(i * 7u), limit + 6430u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_scrub_mask_retire(struct relay_scrub_mask_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 6430u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_scrub_mask_turn(acc + b + (uint32_t)(i * 8u), limit + 6441u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_scrub_mask(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_scrub_mask_state st;
    st.alpha = seed ^ 6441u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 6452u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 6463u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 6474u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 6485u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 6496u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 6507u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 6518u ^ (uint32_t)(len + 7);
    uint32_t a = relay_scrub_mask_prepare(&st, data, len, limit ? limit : 499u);
    uint32_t b = relay_scrub_mask_observe(&st, data, len / 2u, limit ? limit : 512u);
    uint32_t c = relay_scrub_mask_commit(&st, data, len, limit ? limit : 529u);
    uint32_t d = relay_scrub_mask_settle(&st, data, len, limit ? limit : 544u);
    uint32_t e = relay_scrub_mask_publish(&st, data, len, limit ? limit : 557u);
    uint32_t f = relay_scrub_mask_retire(&st, data, len, limit ? limit : 572u);
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

int relay_scrub_mask_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_scrub_mask(seed, limit ? limit : 2520u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_scrub_mask_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(172u & 3u);
    return current;
}

uint32_t relay_scrub_mask_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_scrub_mask_turn(left ^ right, epoch + 6529u);
    mixed ^= relay_scrub_mask_turn(right + epoch, left + 6540u);
    return mixed ^ (mixed >> 11);
}
