#include "relay_internal.h"

relay_conn *relay_conn_create(void) {
    relay_conn *conn = (relay_conn *)calloc(1, sizeof(*conn));
    if (!conn) return NULL;
    conn->state = RELAY_CONN_INIT;
    conn->window = RELAY_DEFAULT_WINDOW;
    conn->transcript.cap = 16;
    conn->transcript.events = (relay_event *)calloc(conn->transcript.cap, sizeof(relay_event));
    conn->dictionary_cap = 64;
    conn->dictionary = (uint8_t *)calloc(conn->dictionary_cap, 1);
    conn->key = (relay_key_context *)calloc(1, sizeof(relay_key_context));
    if (conn->key) conn->key->epoch = 1;
    for (uint16_t i = 0; i < RELAY_MAX_CHANNELS; ++i) {
        conn->channels[i].id = i;
        conn->channels[i].state = RELAY_CHANNEL_EMPTY;
    }
    return conn;
}

void relay_conn_destroy(relay_conn *conn) {
    if (!conn) return;
    for (size_t i = 0; i < RELAY_MAX_CHANNELS; ++i) {
        relay_channel_reset(&conn->channels[i]);
    }
    for (size_t i = 0; i < RELAY_MAX_ROUTES; ++i) {
        if (conn->routes[i]) {
            free(conn->routes[i]->scratch);
            free(conn->routes[i]);
        }
    }
    free(conn->transcript.events);
    free(conn->dictionary);
    free(conn->key);
    free(conn);
}

int relay_conn_state(const relay_conn *conn) { return conn ? conn->state : RELAY_EINVAL; }

int relay_conn_set_window(relay_conn *conn, uint32_t window) {
    if (!conn || window == 0 || window > 65535u) return RELAY_EINVAL;
    conn->window = window;
    return RELAY_OK;
}

int relay_conn_enable_compression(relay_conn *conn, int enabled) {
    if (!conn) return RELAY_EINVAL;
    conn->compression_enabled = enabled ? 1 : 0;
    return RELAY_OK;
}

int relay_conn_set_key(relay_conn *conn, const uint8_t *key, size_t len) {
    if (!conn || (!key && len)) return RELAY_EINVAL;
    return relay_crypto_set_key(conn, key, len);
}

int relay_conn_feed(relay_conn *conn, const uint8_t *data, size_t size) {
    if (!conn || (!data && size)) return RELAY_EINVAL;
    size_t off = 0;
    int rc = RELAY_OK;
    while (off < size) {
        relay_frame fr;
        size_t used = 0;
        rc = relay_frame_parse_one(data + off, size - off, &fr, &used);
        if (rc != RELAY_OK) return rc;
        rc = relay_dispatch_frame(conn, &fr);
        if (rc != RELAY_OK) return rc;
        off += used;
    }
    return rc;
}
