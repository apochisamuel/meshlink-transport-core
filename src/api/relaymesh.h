#ifndef RELAYMESH_H
#define RELAYMESH_H
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct relay_conn relay_conn;

enum relay_status {
    RELAY_OK = 0,
    RELAY_EINVAL = -1,
    RELAY_ENOMEM = -2,
    RELAY_ECRC = -3,
    RELAY_ESTATE = -4,
    RELAY_EAGAIN = -5
};

enum relay_conn_state {
    RELAY_CONN_INIT = 0,
    RELAY_CONN_HELLO = 1,
    RELAY_CONN_READY = 2,
    RELAY_CONN_CLOSING = 3
};

enum relay_channel_state {
    RELAY_CHANNEL_EMPTY = 0,
    RELAY_CHANNEL_OPEN = 1,
    RELAY_CHANNEL_DRAINING = 2,
    RELAY_CHANNEL_CLOSED = 3
};

relay_conn *relay_conn_create(void);
void relay_conn_destroy(relay_conn *conn);
int relay_conn_feed(relay_conn *conn, const uint8_t *data, size_t size);
int relay_conn_state(const relay_conn *conn);
int relay_conn_set_window(relay_conn *conn, uint32_t window);
int relay_conn_set_key(relay_conn *conn, const uint8_t *key, size_t len);
int relay_conn_enable_compression(relay_conn *conn, int enabled);
int relay_route_migrate(relay_conn *conn, uint16_t channel_id, uint16_t route_id);
int relay_channel_open(relay_conn *conn, uint16_t channel_id);
int relay_channel_close(relay_conn *conn, uint16_t channel_id);
int relay_channel_state(const relay_conn *conn, uint16_t channel_id);
int relay_channel_read(relay_conn *conn, uint16_t channel_id, uint8_t *out, size_t cap, size_t *nread);
int relay_build_frame(uint8_t type, uint16_t channel_id, uint32_t seq, const uint8_t *payload, size_t payload_len, uint8_t *out, size_t cap, size_t *nwritten);
int relay_inspect_frame(const uint8_t *data, size_t size, char *out, size_t cap);

#ifdef __cplusplus
}
#endif
#endif
