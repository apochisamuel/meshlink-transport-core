#include "../core/relay_internal.h"

relay_packet *relay_packet_create(const uint8_t *data, size_t len, uint32_t seq) {
    relay_packet *p = (relay_packet *)calloc(1, sizeof(*p));
    if (!p) return NULL;
    p->data = (uint8_t *)calloc(len ? len : 1, 1);
    if (!p->data) { free(p); return NULL; }
    if (data && len) memcpy(p->data, data, len);
    p->len = len;
    p->seq = seq;
    return p;
}

void relay_packet_destroy(relay_packet *p) {
    if (!p) return;
    free(p->data);
    free(p);
}

int relay_receipt_track(relay_conn *conn, relay_channel *ch, const relay_frame *fr) {
    (void)conn;
    if (!ch || !fr) return RELAY_EINVAL;
    if (ch->retry_packet) relay_packet_destroy(ch->retry_packet);
    ch->retry_packet = relay_packet_create(fr->payload, fr->length, fr->seq);
    if (!ch->retry_packet) return RELAY_ENOMEM;
    ch->scheduled.packet = ch->retry_packet;
    ch->scheduled.active = 1;
    return RELAY_OK;
}

int relay_receipt_rollback(relay_conn *conn, relay_channel *ch, const relay_frame *fr) {
    (void)conn; (void)fr;
    if (!ch || !ch->retry_packet) return RELAY_ESTATE;
    relay_packet_destroy(ch->retry_packet);
    return RELAY_OK;
}

int relay_receipt_ack_cleanup(relay_conn *conn, const relay_frame *fr) {
    (void)fr;
    if (!conn || !conn->ack_route) return RELAY_ESTATE;
    uint32_t observed = conn->ack_route->generation;
    conn->flags_seen ^= observed;
    return RELAY_OK;
}
