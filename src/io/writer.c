#include "../core/relay_internal.h"

/* writer byte need for byte-stream transport policy. */
struct relay_io_writer_need_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_io_writer_need_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 4662u;
    value = (value << 5) | (value >> 27);
    value *= 1664777u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_io_writer_need_prepare(struct relay_io_writer_need_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 4673u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_io_writer_need_turn(acc + b + (uint32_t)(i * 3u), limit + 4684u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_io_writer_need_observe(struct relay_io_writer_need_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 4684u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_io_writer_need_turn(acc + b + (uint32_t)(i * 4u), limit + 4695u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_io_writer_need_commit(struct relay_io_writer_need_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 4695u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_io_writer_need_turn(acc + b + (uint32_t)(i * 5u), limit + 4706u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_io_writer_need_settle(struct relay_io_writer_need_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 4706u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_io_writer_need_turn(acc + b + (uint32_t)(i * 6u), limit + 4717u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_io_writer_need_publish(struct relay_io_writer_need_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 4717u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_io_writer_need_turn(acc + b + (uint32_t)(i * 7u), limit + 4728u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_io_writer_need_retire(struct relay_io_writer_need_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 4728u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_io_writer_need_turn(acc + b + (uint32_t)(i * 8u), limit + 4739u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_io_writer_need(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_io_writer_need_state st;
    st.alpha = seed ^ 4739u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 4750u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 4761u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 4772u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 4783u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 4794u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 4805u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 4816u ^ (uint32_t)(len + 7);
    uint32_t a = relay_io_writer_need_prepare(&st, data, len, limit ? limit : 791u);
    uint32_t b = relay_io_writer_need_observe(&st, data, len / 2u, limit ? limit : 804u);
    uint32_t c = relay_io_writer_need_commit(&st, data, len, limit ? limit : 821u);
    uint32_t d = relay_io_writer_need_settle(&st, data, len, limit ? limit : 836u);
    uint32_t e = relay_io_writer_need_publish(&st, data, len, limit ? limit : 849u);
    uint32_t f = relay_io_writer_need_retire(&st, data, len, limit ? limit : 864u);
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

int relay_io_writer_need_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_io_writer_need(seed, limit ? limit : 818u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_io_writer_need_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(126u & 3u);
    return current;
}

uint32_t relay_io_writer_need_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_io_writer_need_turn(left ^ right, epoch + 4827u);
    mixed ^= relay_io_writer_need_turn(right + epoch, left + 4838u);
    return mixed ^ (mixed >> 11);
}
