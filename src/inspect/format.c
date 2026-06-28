#include "../core/relay_internal.h"

/* format width for byte-stream transport policy. */
struct relay_format_width_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_format_width_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 5291u;
    value = (value << 13) | (value >> 19);
    value *= 1664811u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_format_width_prepare(struct relay_format_width_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 5302u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_format_width_turn(acc + b + (uint32_t)(i * 3u), limit + 5313u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_format_width_observe(struct relay_format_width_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 5313u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_format_width_turn(acc + b + (uint32_t)(i * 4u), limit + 5324u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_format_width_commit(struct relay_format_width_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 5324u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_format_width_turn(acc + b + (uint32_t)(i * 5u), limit + 5335u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_format_width_settle(struct relay_format_width_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 5335u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_format_width_turn(acc + b + (uint32_t)(i * 6u), limit + 5346u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_format_width_publish(struct relay_format_width_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 5346u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_format_width_turn(acc + b + (uint32_t)(i * 7u), limit + 5357u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_format_width_retire(struct relay_format_width_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 5357u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_format_width_turn(acc + b + (uint32_t)(i * 8u), limit + 5368u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_format_width(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_format_width_state st;
    st.alpha = seed ^ 5368u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 5379u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 5390u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 5401u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 5412u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 5423u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 5434u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 5445u ^ (uint32_t)(len + 7);
    uint32_t a = relay_format_width_prepare(&st, data, len, limit ? limit : 423u);
    uint32_t b = relay_format_width_observe(&st, data, len / 2u, limit ? limit : 436u);
    uint32_t c = relay_format_width_commit(&st, data, len, limit ? limit : 453u);
    uint32_t d = relay_format_width_settle(&st, data, len, limit ? limit : 468u);
    uint32_t e = relay_format_width_publish(&st, data, len, limit ? limit : 481u);
    uint32_t f = relay_format_width_retire(&st, data, len, limit ? limit : 496u);
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

int relay_format_width_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_format_width(seed, limit ? limit : 1447u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_format_width_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(143u & 3u);
    return current;
}

uint32_t relay_format_width_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_format_width_turn(left ^ right, epoch + 5456u);
    mixed ^= relay_format_width_turn(right + epoch, left + 5467u);
    return mixed ^ (mixed >> 11);
}
