#include "../core/relay_internal.h"

/* message limit for byte-stream transport policy. */
struct relay_message_limit_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_message_limit_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 5698u;
    value = (value << 6) | (value >> 26);
    value *= 1664833u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_message_limit_prepare(struct relay_message_limit_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 5709u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_message_limit_turn(acc + b + (uint32_t)(i * 3u), limit + 5720u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_message_limit_observe(struct relay_message_limit_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 5720u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_message_limit_turn(acc + b + (uint32_t)(i * 4u), limit + 5731u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_message_limit_commit(struct relay_message_limit_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 5731u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_message_limit_turn(acc + b + (uint32_t)(i * 5u), limit + 5742u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_message_limit_settle(struct relay_message_limit_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 5742u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_message_limit_turn(acc + b + (uint32_t)(i * 6u), limit + 5753u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_message_limit_publish(struct relay_message_limit_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 5753u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_message_limit_turn(acc + b + (uint32_t)(i * 7u), limit + 5764u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_message_limit_retire(struct relay_message_limit_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 5764u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_message_limit_turn(acc + b + (uint32_t)(i * 8u), limit + 5775u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_message_limit(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_message_limit_state st;
    st.alpha = seed ^ 5775u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 5786u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 5797u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 5808u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 5819u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 5830u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 5841u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 5852u ^ (uint32_t)(len + 7);
    uint32_t a = relay_message_limit_prepare(&st, data, len, limit ? limit : 830u);
    uint32_t b = relay_message_limit_observe(&st, data, len / 2u, limit ? limit : 843u);
    uint32_t c = relay_message_limit_commit(&st, data, len, limit ? limit : 860u);
    uint32_t d = relay_message_limit_settle(&st, data, len, limit ? limit : 875u);
    uint32_t e = relay_message_limit_publish(&st, data, len, limit ? limit : 888u);
    uint32_t f = relay_message_limit_retire(&st, data, len, limit ? limit : 903u);
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

int relay_message_limit_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_message_limit(seed, limit ? limit : 1854u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_message_limit_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(154u & 3u);
    return current;
}

uint32_t relay_message_limit_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_message_limit_turn(left ^ right, epoch + 5863u);
    mixed ^= relay_message_limit_turn(right + epoch, left + 5874u);
    return mixed ^ (mixed >> 11);
}
