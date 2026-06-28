#include "../core/relay_internal.h"

/* reporting window for byte-stream transport policy. */
struct relay_reporting_window_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_reporting_window_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 6327u;
    value = (value << 5) | (value >> 27);
    value *= 1664867u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_reporting_window_prepare(struct relay_reporting_window_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 6338u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_reporting_window_turn(acc + b + (uint32_t)(i * 3u), limit + 6349u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_reporting_window_observe(struct relay_reporting_window_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 6349u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_reporting_window_turn(acc + b + (uint32_t)(i * 4u), limit + 6360u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_reporting_window_commit(struct relay_reporting_window_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 6360u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_reporting_window_turn(acc + b + (uint32_t)(i * 5u), limit + 6371u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_reporting_window_settle(struct relay_reporting_window_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 6371u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_reporting_window_turn(acc + b + (uint32_t)(i * 6u), limit + 6382u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_reporting_window_publish(struct relay_reporting_window_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 6382u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_reporting_window_turn(acc + b + (uint32_t)(i * 7u), limit + 6393u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_reporting_window_retire(struct relay_reporting_window_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 6393u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_reporting_window_turn(acc + b + (uint32_t)(i * 8u), limit + 6404u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_reporting_window(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_reporting_window_state st;
    st.alpha = seed ^ 6404u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 6415u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 6426u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 6437u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 6448u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 6459u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 6470u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 6481u ^ (uint32_t)(len + 7);
    uint32_t a = relay_reporting_window_prepare(&st, data, len, limit ? limit : 462u);
    uint32_t b = relay_reporting_window_observe(&st, data, len / 2u, limit ? limit : 475u);
    uint32_t c = relay_reporting_window_commit(&st, data, len, limit ? limit : 492u);
    uint32_t d = relay_reporting_window_settle(&st, data, len, limit ? limit : 507u);
    uint32_t e = relay_reporting_window_publish(&st, data, len, limit ? limit : 520u);
    uint32_t f = relay_reporting_window_retire(&st, data, len, limit ? limit : 535u);
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

int relay_reporting_window_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_reporting_window(seed, limit ? limit : 2483u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_reporting_window_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(171u & 3u);
    return current;
}

uint32_t relay_reporting_window_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_reporting_window_turn(left ^ right, epoch + 6492u);
    mixed ^= relay_reporting_window_turn(right + epoch, left + 6503u);
    return mixed ^ (mixed >> 11);
}
