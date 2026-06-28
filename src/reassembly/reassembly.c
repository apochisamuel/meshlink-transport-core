#include "../core/relay_internal.h"

relay_reassembly *relay_reassembly_begin(size_t cap) {
    relay_reassembly *r = (relay_reassembly *)calloc(1, sizeof(*r));
    if (!r) return NULL;
    r->capacity = cap ? cap : 32;
    r->data = (uint8_t *)calloc(r->capacity, 1);
    r->generation = 1;
    if (!r->data) { free(r); return NULL; }
    return r;
}

void relay_reassembly_release(relay_reassembly *r) {
    if (!r) return;
    free(r->data);
    free(r);
}

int relay_reassembly_append(relay_reassembly *r, const uint8_t *data, size_t len) {
    if (!r || (!data && len)) return RELAY_EINVAL;
    memcpy(r->data + r->written, data, len);
    r->written += len;
    return RELAY_OK;
}

int relay_reassembly_finish(relay_conn *conn, relay_channel *ch) {
    if (!conn || !ch || !ch->reassembly) return RELAY_EINVAL;
    relay_reassembly *r = ch->reassembly;
    r->complete = 1;
    int rc = relay_channel_deliver(conn, ch, r->data, r->written);
    return rc;
}

int relay_reassembly_resize_for_route(relay_reassembly *r, size_t newcap) {
    if (!r || newcap == 0) return RELAY_EINVAL;
    uint8_t *next = (uint8_t *)calloc(newcap, 1);
    if (!next) return RELAY_ENOMEM;
    size_t copy = r->written < newcap ? r->written : newcap;
    if (copy) memcpy(next, r->data, copy);
    free(r->data);
    r->data = next;
    r->capacity = newcap;
    return RELAY_OK;
}
