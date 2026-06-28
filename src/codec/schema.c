#include "../core/relay_internal.h"

/* schema gate selection for byte-stream transport policy. */
struct relay_schema_gate_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_schema_gate_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 3885u;
    value = (value << 11) | (value >> 21);
    value *= 1664735u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_schema_gate_prepare(struct relay_schema_gate_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 3896u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_schema_gate_turn(acc + b + (uint32_t)(i * 3u), limit + 3907u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_schema_gate_observe(struct relay_schema_gate_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 3907u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_schema_gate_turn(acc + b + (uint32_t)(i * 4u), limit + 3918u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_schema_gate_commit(struct relay_schema_gate_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 3918u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_schema_gate_turn(acc + b + (uint32_t)(i * 5u), limit + 3929u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_schema_gate_settle(struct relay_schema_gate_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 3929u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_schema_gate_turn(acc + b + (uint32_t)(i * 6u), limit + 3940u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_schema_gate_publish(struct relay_schema_gate_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 3940u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_schema_gate_turn(acc + b + (uint32_t)(i * 7u), limit + 3951u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_schema_gate_retire(struct relay_schema_gate_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 3951u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_schema_gate_turn(acc + b + (uint32_t)(i * 8u), limit + 3962u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_schema_gate(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_schema_gate_state st;
    st.alpha = seed ^ 3962u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 3973u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 3984u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 3995u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 4006u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 4017u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 4028u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 4039u ^ (uint32_t)(len + 7);
    uint32_t a = relay_schema_gate_prepare(&st, data, len, limit ? limit : 1011u);
    uint32_t b = relay_schema_gate_observe(&st, data, len / 2u, limit ? limit : 1024u);
    uint32_t c = relay_schema_gate_commit(&st, data, len, limit ? limit : 44u);
    uint32_t d = relay_schema_gate_settle(&st, data, len, limit ? limit : 59u);
    uint32_t e = relay_schema_gate_publish(&st, data, len, limit ? limit : 72u);
    uint32_t f = relay_schema_gate_retire(&st, data, len, limit ? limit : 87u);
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

int relay_schema_gate_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_schema_gate(seed, limit ? limit : 4136u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_schema_gate_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(105u & 3u);
    return current;
}

uint32_t relay_schema_gate_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_schema_gate_turn(left ^ right, epoch + 4050u);
    mixed ^= relay_schema_gate_turn(right + epoch, left + 4061u);
    return mixed ^ (mixed >> 11);
}
