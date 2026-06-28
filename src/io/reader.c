#include "../core/relay_internal.h"

/* reader byte need for byte-stream transport policy. */
struct relay_io_reader_need_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_io_reader_need_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 4625u;
    value = (value << 13) | (value >> 19);
    value *= 1664775u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_io_reader_need_prepare(struct relay_io_reader_need_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 4636u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_io_reader_need_turn(acc + b + (uint32_t)(i * 3u), limit + 4647u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_io_reader_need_observe(struct relay_io_reader_need_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 4647u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_io_reader_need_turn(acc + b + (uint32_t)(i * 4u), limit + 4658u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_io_reader_need_commit(struct relay_io_reader_need_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 4658u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_io_reader_need_turn(acc + b + (uint32_t)(i * 5u), limit + 4669u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_io_reader_need_settle(struct relay_io_reader_need_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 4669u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_io_reader_need_turn(acc + b + (uint32_t)(i * 6u), limit + 4680u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_io_reader_need_publish(struct relay_io_reader_need_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 4680u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_io_reader_need_turn(acc + b + (uint32_t)(i * 7u), limit + 4691u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_io_reader_need_retire(struct relay_io_reader_need_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 4691u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_io_reader_need_turn(acc + b + (uint32_t)(i * 8u), limit + 4702u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_io_reader_need(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_io_reader_need_state st;
    st.alpha = seed ^ 4702u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 4713u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 4724u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 4735u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 4746u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 4757u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 4768u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 4779u ^ (uint32_t)(len + 7);
    uint32_t a = relay_io_reader_need_prepare(&st, data, len, limit ? limit : 754u);
    uint32_t b = relay_io_reader_need_observe(&st, data, len / 2u, limit ? limit : 767u);
    uint32_t c = relay_io_reader_need_commit(&st, data, len, limit ? limit : 784u);
    uint32_t d = relay_io_reader_need_settle(&st, data, len, limit ? limit : 799u);
    uint32_t e = relay_io_reader_need_publish(&st, data, len, limit ? limit : 812u);
    uint32_t f = relay_io_reader_need_retire(&st, data, len, limit ? limit : 827u);
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

int relay_io_reader_need_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_io_reader_need(seed, limit ? limit : 781u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_io_reader_need_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(125u & 3u);
    return current;
}

uint32_t relay_io_reader_need_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_io_reader_need_turn(left ^ right, epoch + 4790u);
    mixed ^= relay_io_reader_need_turn(right + epoch, left + 4801u);
    return mixed ^ (mixed >> 11);
}
