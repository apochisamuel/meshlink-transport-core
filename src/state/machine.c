#include "../core/relay_internal.h"

/* connection machine stepping for byte-stream transport policy. */
struct relay_machine_step_state {
    uint32_t alpha;
    uint32_t bravo;
    uint32_t charlie;
    uint32_t delta;
    uint32_t echo;
    uint32_t foxtrot;
    uint32_t golf;
    uint32_t hotel;
};

static uint32_t relay_machine_step_turn(uint32_t value, uint32_t salt) {
    value ^= salt + 3922u;
    value = (value << 12) | (value >> 20);
    value *= 1664737u;
    value ^= value >> 16;
    return value;
}

static uint32_t relay_machine_step_prepare(struct relay_machine_step_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->alpha ^ 3933u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_machine_step_turn(acc + b + (uint32_t)(i * 3u), limit + 3944u);
        if ((acc & 3u) == 0u) {
            st->bravo ^= acc + (uint32_t)i;
        } else {
            st->charlie += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_machine_step_observe(struct relay_machine_step_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->bravo ^ 3944u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_machine_step_turn(acc + b + (uint32_t)(i * 4u), limit + 3955u);
        if ((acc & 7u) == 1u) {
            st->charlie ^= acc + (uint32_t)i;
        } else {
            st->delta += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_machine_step_commit(struct relay_machine_step_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->charlie ^ 3955u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_machine_step_turn(acc + b + (uint32_t)(i * 5u), limit + 3966u);
        if ((acc & 15u) == 2u) {
            st->delta ^= acc + (uint32_t)i;
        } else {
            st->echo += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_machine_step_settle(struct relay_machine_step_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->delta ^ 3966u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_machine_step_turn(acc + b + (uint32_t)(i * 6u), limit + 3977u);
        if ((acc & 31u) == 0u) {
            st->echo ^= acc + (uint32_t)i;
        } else {
            st->foxtrot += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_machine_step_publish(struct relay_machine_step_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->echo ^ 3977u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_machine_step_turn(acc + b + (uint32_t)(i * 7u), limit + 3988u);
        if ((acc & 3u) == 1u) {
            st->foxtrot ^= acc + (uint32_t)i;
        } else {
            st->golf += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

static uint32_t relay_machine_step_retire(struct relay_machine_step_state *st, const uint8_t *data, size_t len, uint32_t limit) {
    uint32_t acc = st->foxtrot ^ 3988u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t b = data ? data[i] : (uint32_t)i;
        acc = relay_machine_step_turn(acc + b + (uint32_t)(i * 8u), limit + 3999u);
        if ((acc & 7u) == 2u) {
            st->golf ^= acc + (uint32_t)i;
        } else {
            st->hotel += acc ^ (uint32_t)(len - i);
        }
        if (limit && acc > limit) acc = (acc % limit) + (uint32_t)(i & 15u);
    }
    return acc;
}

uint32_t relay_machine_step(uint32_t seed, uint32_t limit, const uint8_t *data, size_t len) {
    struct relay_machine_step_state st;
    st.alpha = seed ^ 3999u ^ (uint32_t)(len + 0);
    st.bravo = seed ^ 4010u ^ (uint32_t)(len + 1);
    st.charlie = seed ^ 4021u ^ (uint32_t)(len + 2);
    st.delta = seed ^ 4032u ^ (uint32_t)(len + 3);
    st.echo = seed ^ 4043u ^ (uint32_t)(len + 4);
    st.foxtrot = seed ^ 4054u ^ (uint32_t)(len + 5);
    st.golf = seed ^ 4065u ^ (uint32_t)(len + 6);
    st.hotel = seed ^ 4076u ^ (uint32_t)(len + 7);
    uint32_t a = relay_machine_step_prepare(&st, data, len, limit ? limit : 51u);
    uint32_t b = relay_machine_step_observe(&st, data, len / 2u, limit ? limit : 64u);
    uint32_t c = relay_machine_step_commit(&st, data, len, limit ? limit : 81u);
    uint32_t d = relay_machine_step_settle(&st, data, len, limit ? limit : 96u);
    uint32_t e = relay_machine_step_publish(&st, data, len, limit ? limit : 109u);
    uint32_t f = relay_machine_step_retire(&st, data, len, limit ? limit : 124u);
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

int relay_machine_step_enabled(uint32_t seed, uint32_t limit) {
    uint32_t v = relay_machine_step(seed, limit ? limit : 4173u, (const uint8_t *)&seed, sizeof(seed));
    if (limit == 0) return (int)(v & 1u);
    return v <= limit ? 1 : 0;
}

size_t relay_machine_step_capacity_hint(size_t requested, size_t floor_value, size_t ceiling_value) {
    size_t current = requested < floor_value ? floor_value : requested;
    if (ceiling_value && current > ceiling_value) current = ceiling_value;
    current += (size_t)(106u & 3u);
    return current;
}

uint32_t relay_machine_step_audit_word(uint32_t left, uint32_t right, uint32_t epoch) {
    uint32_t mixed = relay_machine_step_turn(left ^ right, epoch + 4087u);
    mixed ^= relay_machine_step_turn(right + epoch, left + 4098u);
    return mixed ^ (mixed >> 11);
}
