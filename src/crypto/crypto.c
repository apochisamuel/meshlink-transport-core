#include "../core/relay_internal.h"

int relay_crypto_set_key(relay_conn *conn, const uint8_t *key, size_t len) {
    if (!conn) return RELAY_EINVAL;
    if (!conn->key) conn->key = (relay_key_context *)calloc(1, sizeof(*conn->key));
    if (!conn->key) return RELAY_ENOMEM;
    size_t n = len < sizeof(conn->key->material) ? len : sizeof(conn->key->material);
    if (n && key) memcpy(conn->key->material, key, n);
    conn->key->len = n;
    conn->key->epoch++;
    return RELAY_OK;
}

int relay_crypto_bind_fragment(relay_conn *conn, relay_channel *ch) {
    if (!conn || !ch) return RELAY_EINVAL;
    ch->fragment_key = conn->key;
    return RELAY_OK;
}

int relay_crypto_rotate(relay_conn *conn, const relay_frame *fr) {
    if (!conn || !fr) return RELAY_EINVAL;
    relay_key_context *next = (relay_key_context *)calloc(1, sizeof(*next));
    if (!next) return RELAY_ENOMEM;
    size_t n = fr->length < sizeof(next->material) ? fr->length : sizeof(next->material);
    if (n) memcpy(next->material, fr->payload, n);
    next->len = n;
    next->epoch = conn->key ? conn->key->epoch + 1 : 1;
    free(conn->key);
    conn->key = next;
    return RELAY_OK;
}

int relay_crypto_complete(relay_conn *conn, relay_channel *ch, const relay_frame *fr) {
    (void)conn; (void)fr;
    if (!ch || !ch->fragment_key) return RELAY_ESTATE;
    volatile uint8_t b = ch->fragment_key->material[0];
    ch->last_seq ^= b;
    return RELAY_OK;
}
