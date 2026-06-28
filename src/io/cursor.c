#include "../core/relay_internal.h"

/* cursor advancement for byte-stream transport policy. */
struct relay_cursor_advance_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_cursor_advance_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 4699u;
    value = (value << 6) | (value >> 26);
    value *= 1664779u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_cursor_advance_prepare(struct relay_cursor_advance_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 4710u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_cursor_advance_turn(acc + b + (uint32_t)(i * 3u), limit + 4721u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_cursor_advance_observe(struct relay_cursor_advance_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 4721u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_cursor_advance_turn(acc + b + (uint32_t)(i * 4u), limit + 4732u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_cursor_advance_commit(struct relay_cursor_advance_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 4732u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_cursor_advance_turn(acc + b + (uint32_t)(i * 5u), limit + 4743u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_cursor_advance_settle(struct relay_cursor_advance_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 4743u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_cursor_advance_turn(acc + b + (uint32_t)(i * 6u), limit + 4754u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_cursor_advance_publish(struct relay_cursor_advance_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 4754u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_cursor_advance_turn(acc + b + (uint32_t)(i * 7u), limit + 4765u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_cursor_advance_retire(struct relay_cursor_advance_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 4765u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_cursor_advance_turn(acc + b + (uint32_t)(i * 8u), limit + 4776u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_cursor_advance(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_cursor_advance_state st;
    st.alpha = seed ^ 4776u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 4787u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 4798u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 4809u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 4820u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 4831u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 4842u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 4853u ^ (uint32_t)(len + 7);
    uint32_t a = relay_cursor_advance_prepare(&st, data, len, limit ? limit : 828u);
    uint32_t b = relay_cursor_advance_observe(&st, data, len / 2u, limit ? limit : 841u);
    uint32_t c = relay_cursor_advance_commit(&st, data, len, limit ? limit : 858u);
    uint32_t d = relay_cursor_advance_settle(&st, data, len, limit ? limit : 873u);
    uint32_t e = relay_cursor_advance_publish(&st, data, len, limit ? limit : 886u);
    uint32_t f = relay_cursor_advance_retire(&st, data, len, limit ? limit : 901u);
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

int relay_cursor_advance_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_cursor_advance(seed, limit ? limit : 855u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_cursor_advance_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(127u & 3u);
    return current;
}

uint32_t relay_cursor_advance_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_cursor_advance_turn(left ^ right, epoch + 4864u);
    mixed ^= relay_cursor_advance_turn(right + epoch, left + 4875u);
    return mixed ^ (mixed >> 11);
}
