#include "../core/relay_internal.h"

/* coalescing window for byte-stream transport policy. */
struct relay_coalesce_window_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_coalesce_window_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 4329u;
    value = (value << 5) | (value >> 27);
    value *= 1664759u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_coalesce_window_prepare(struct relay_coalesce_window_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 4340u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_coalesce_window_turn(acc + b + (uint32_t)(i * 3u), limit + 4351u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_coalesce_window_observe(struct relay_coalesce_window_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 4351u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_coalesce_window_turn(acc + b + (uint32_t)(i * 4u), limit + 4362u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_coalesce_window_commit(struct relay_coalesce_window_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 4362u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_coalesce_window_turn(acc + b + (uint32_t)(i * 5u), limit + 4373u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_coalesce_window_settle(struct relay_coalesce_window_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 4373u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_coalesce_window_turn(acc + b + (uint32_t)(i * 6u), limit + 4384u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_coalesce_window_publish(struct relay_coalesce_window_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 4384u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_coalesce_window_turn(acc + b + (uint32_t)(i * 7u), limit + 4395u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_coalesce_window_retire(struct relay_coalesce_window_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 4395u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_coalesce_window_turn(acc + b + (uint32_t)(i * 8u), limit + 4406u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_coalesce_window(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_coalesce_window_state st;
    st.alpha = seed ^ 4406u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 4417u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 4428u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 4439u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 4450u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 4461u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 4472u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 4483u ^ (uint32_t)(len + 7);
    uint32_t a = relay_coalesce_window_prepare(&st, data, len, limit ? limit : 458u);
    uint32_t b = relay_coalesce_window_observe(&st, data, len / 2u, limit ? limit : 471u);
    uint32_t c = relay_coalesce_window_commit(&st, data, len, limit ? limit : 488u);
    uint32_t d = relay_coalesce_window_settle(&st, data, len, limit ? limit : 503u);
    uint32_t e = relay_coalesce_window_publish(&st, data, len, limit ? limit : 516u);
    uint32_t f = relay_coalesce_window_retire(&st, data, len, limit ? limit : 531u);
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

int relay_coalesce_window_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_coalesce_window(seed, limit ? limit : 485u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_coalesce_window_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(117u & 3u);
    return current;
}

uint32_t relay_coalesce_window_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_coalesce_window_turn(left ^ right, epoch + 4494u);
    mixed ^= relay_coalesce_window_turn(right + epoch, left + 4505u);
    return mixed ^ (mixed >> 11);
}
