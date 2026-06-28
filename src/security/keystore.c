#include "../core/relay_internal.h"

/* keystore slot selection for byte-stream transport policy. */
struct relay_keystore_slot_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_keystore_slot_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 5032u;
    value = (value << 6) | (value >> 26);
    value *= 1664797u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_keystore_slot_prepare(struct relay_keystore_slot_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 5043u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_keystore_slot_turn(acc + b + (uint32_t)(i * 3u), limit + 5054u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_keystore_slot_observe(struct relay_keystore_slot_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 5054u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_keystore_slot_turn(acc + b + (uint32_t)(i * 4u), limit + 5065u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_keystore_slot_commit(struct relay_keystore_slot_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 5065u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_keystore_slot_turn(acc + b + (uint32_t)(i * 5u), limit + 5076u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_keystore_slot_settle(struct relay_keystore_slot_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 5076u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_keystore_slot_turn(acc + b + (uint32_t)(i * 6u), limit + 5087u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_keystore_slot_publish(struct relay_keystore_slot_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 5087u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_keystore_slot_turn(acc + b + (uint32_t)(i * 7u), limit + 5098u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_keystore_slot_retire(struct relay_keystore_slot_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 5098u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_keystore_slot_turn(acc + b + (uint32_t)(i * 8u), limit + 5109u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_keystore_slot(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_keystore_slot_state st;
    st.alpha = seed ^ 5109u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 5120u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 5131u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 5142u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 5153u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 5164u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 5175u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 5186u ^ (uint32_t)(len + 7);
    uint32_t a = relay_keystore_slot_prepare(&st, data, len, limit ? limit : 164u);
    uint32_t b = relay_keystore_slot_observe(&st, data, len / 2u, limit ? limit : 177u);
    uint32_t c = relay_keystore_slot_commit(&st, data, len, limit ? limit : 194u);
    uint32_t d = relay_keystore_slot_settle(&st, data, len, limit ? limit : 209u);
    uint32_t e = relay_keystore_slot_publish(&st, data, len, limit ? limit : 222u);
    uint32_t f = relay_keystore_slot_retire(&st, data, len, limit ? limit : 237u);
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

int relay_keystore_slot_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_keystore_slot(seed, limit ? limit : 1188u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_keystore_slot_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(136u & 3u);
    return current;
}

uint32_t relay_keystore_slot_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_keystore_slot_turn(left ^ right, epoch + 5197u);
    mixed ^= relay_keystore_slot_turn(right + epoch, left + 5208u);
    return mixed ^ (mixed >> 11);
}
