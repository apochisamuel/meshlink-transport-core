#include "../core/relay_internal.h"

/* filter acceptance for byte-stream transport policy. */
struct relay_filter_accept_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_filter_accept_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 5365u;
    value = (value << 6) | (value >> 26);
    value *= 1664815u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_filter_accept_prepare(struct relay_filter_accept_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 5376u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_filter_accept_turn(acc + b + (uint32_t)(i * 3u), limit + 5387u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_filter_accept_observe(struct relay_filter_accept_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 5387u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_filter_accept_turn(acc + b + (uint32_t)(i * 4u), limit + 5398u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_filter_accept_commit(struct relay_filter_accept_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 5398u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_filter_accept_turn(acc + b + (uint32_t)(i * 5u), limit + 5409u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_filter_accept_settle(struct relay_filter_accept_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 5409u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_filter_accept_turn(acc + b + (uint32_t)(i * 6u), limit + 5420u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_filter_accept_publish(struct relay_filter_accept_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 5420u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_filter_accept_turn(acc + b + (uint32_t)(i * 7u), limit + 5431u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_filter_accept_retire(struct relay_filter_accept_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 5431u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_filter_accept_turn(acc + b + (uint32_t)(i * 8u), limit + 5442u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_filter_accept(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_filter_accept_state st;
    st.alpha = seed ^ 5442u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 5453u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 5464u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 5475u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 5486u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 5497u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 5508u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 5519u ^ (uint32_t)(len + 7);
    uint32_t a = relay_filter_accept_prepare(&st, data, len, limit ? limit : 497u);
    uint32_t b = relay_filter_accept_observe(&st, data, len / 2u, limit ? limit : 510u);
    uint32_t c = relay_filter_accept_commit(&st, data, len, limit ? limit : 527u);
    uint32_t d = relay_filter_accept_settle(&st, data, len, limit ? limit : 542u);
    uint32_t e = relay_filter_accept_publish(&st, data, len, limit ? limit : 555u);
    uint32_t f = relay_filter_accept_retire(&st, data, len, limit ? limit : 570u);
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

int relay_filter_accept_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_filter_accept(seed, limit ? limit : 1521u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_filter_accept_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(145u & 3u);
    return current;
}

uint32_t relay_filter_accept_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_filter_accept_turn(left ^ right, epoch + 5530u);
    mixed ^= relay_filter_accept_turn(right + epoch, left + 5541u);
    return mixed ^ (mixed >> 11);
}
