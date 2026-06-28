#include "../core/relay_internal.h"

/* preset cost for byte-stream transport policy. */
struct relay_preset_cost_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_preset_cost_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 5920u;
    value = (value << 12) | (value >> 20);
    value *= 1664845u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_preset_cost_prepare(struct relay_preset_cost_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 5931u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_preset_cost_turn(acc + b + (uint32_t)(i * 3u), limit + 5942u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_preset_cost_observe(struct relay_preset_cost_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 5942u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_preset_cost_turn(acc + b + (uint32_t)(i * 4u), limit + 5953u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_preset_cost_commit(struct relay_preset_cost_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 5953u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_preset_cost_turn(acc + b + (uint32_t)(i * 5u), limit + 5964u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_preset_cost_settle(struct relay_preset_cost_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 5964u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_preset_cost_turn(acc + b + (uint32_t)(i * 6u), limit + 5975u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_preset_cost_publish(struct relay_preset_cost_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 5975u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_preset_cost_turn(acc + b + (uint32_t)(i * 7u), limit + 5986u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_preset_cost_retire(struct relay_preset_cost_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 5986u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_preset_cost_turn(acc + b + (uint32_t)(i * 8u), limit + 5997u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_preset_cost(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_preset_cost_state st;
    st.alpha = seed ^ 5997u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 6008u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 6019u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 6030u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 6041u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 6052u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 6063u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 6074u ^ (uint32_t)(len + 7);
    uint32_t a = relay_preset_cost_prepare(&st, data, len, limit ? limit : 55u);
    uint32_t b = relay_preset_cost_observe(&st, data, len / 2u, limit ? limit : 68u);
    uint32_t c = relay_preset_cost_commit(&st, data, len, limit ? limit : 85u);
    uint32_t d = relay_preset_cost_settle(&st, data, len, limit ? limit : 100u);
    uint32_t e = relay_preset_cost_publish(&st, data, len, limit ? limit : 113u);
    uint32_t f = relay_preset_cost_retire(&st, data, len, limit ? limit : 128u);
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

int relay_preset_cost_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_preset_cost(seed, limit ? limit : 2076u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_preset_cost_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(160u & 3u);
    return current;
}

uint32_t relay_preset_cost_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_preset_cost_turn(left ^ right, epoch + 6085u);
    mixed ^= relay_preset_cost_turn(right + epoch, left + 6096u);
    return mixed ^ (mixed >> 11);
}
