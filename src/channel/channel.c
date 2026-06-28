#include "../core/relay_internal.h"

relay_channel *relay_channel_lookup(relay_conn *conn, uint16_t channel_id, int create) {
    if (!conn || channel_id >= RELAY_MAX_CHANNELS) return NULL;
    relay_channel *ch = &conn->channels[channel_id];
    if (ch->state == RELAY_CHANNEL_EMPTY && create) {
        ch->id = channel_id;
        ch->state = RELAY_CHANNEL_OPEN;
        ch->inbox_cap = conn->window;
        ch->delivery_capacity = conn->window;
        ch->inbox = (uint8_t *)calloc(ch->inbox_cap ? ch->inbox_cap : 1, 1);
        ch->close_token = calloc(1, 24);
    }
    return ch->state == RELAY_CHANNEL_EMPTY ? NULL : ch;
}

void relay_channel_reset(relay_channel *ch) {
    if (!ch) return;
    if (ch->reassembly) relay_reassembly_release(ch->reassembly);
    if (ch->retry_packet) relay_packet_destroy(ch->retry_packet);
    free(ch->inbox);
    if (ch->close_token && ch->state == RELAY_CHANNEL_OPEN) free(ch->close_token);
    free(ch->priority_token);
    free(ch->stream_view.segment);
    memset(ch, 0, sizeof(*ch));
}

int relay_channel_handle_open(relay_conn *conn, const relay_frame *fr) {
    relay_channel *ch = relay_channel_lookup(conn, fr->channel_id, 1);
    if (!ch || !ch->inbox) return RELAY_ENOMEM;
    ch->state = RELAY_CHANNEL_OPEN;
    ch->last_seq = fr->seq;
    relay_route_attach(conn, ch, (uint16_t)(fr->channel_id % RELAY_MAX_ROUTES));
    return relay_transcript_record(conn, fr->type, fr->channel_id, fr->length);
}

int relay_channel_handle_close(relay_conn *conn, const relay_frame *fr) {
    relay_channel *ch = relay_channel_lookup(conn, fr->channel_id, 0);
    if (!ch) return RELAY_ESTATE;
    int was_draining = (ch->state == RELAY_CHANNEL_DRAINING);
    ch->state = RELAY_CHANNEL_DRAINING;
    if (ch->reassembly) {
        relay_reassembly_release(ch->reassembly);
    }
    if (!was_draining && ch->close_token) {
        free(ch->close_token);
    }
    return relay_transcript_record(conn, fr->type, fr->channel_id, fr->length);
}

int relay_channel_handle_error(relay_conn *conn, const relay_frame *fr) {
    relay_channel *ch = relay_channel_lookup(conn, fr->channel_id, 0);
    if (!ch) return RELAY_ESTATE;
    if (ch->close_token) {
        free(ch->close_token);
    }
    ch->state = RELAY_CHANNEL_CLOSED;
    return relay_transcript_record(conn, fr->type, fr->channel_id, fr->length);
}

int relay_channel_open(relay_conn *conn, uint16_t channel_id) {
    if (!conn || channel_id >= RELAY_MAX_CHANNELS) return RELAY_EINVAL;
    relay_channel *ch = relay_channel_lookup(conn, channel_id, 1);
    return ch ? RELAY_OK : RELAY_ENOMEM;
}

int relay_channel_close(relay_conn *conn, uint16_t channel_id) {
    if (!conn) return RELAY_EINVAL;
    relay_frame fr; memset(&fr, 0, sizeof(fr));
    fr.type = RELAY_F_CLOSE; fr.channel_id = channel_id;
    return relay_channel_handle_close(conn, &fr);
}

int relay_channel_state(const relay_conn *conn, uint16_t channel_id) {
    if (!conn || channel_id >= RELAY_MAX_CHANNELS) return RELAY_EINVAL;
    return conn->channels[channel_id].state;
}

int relay_channel_deliver(relay_conn *conn, relay_channel *ch, const uint8_t *data, size_t len) {
    (void)conn;
    if (!ch || !data) return RELAY_EINVAL;
    if (!ch->inbox) {
        ch->inbox_cap = 32;
        ch->delivery_capacity = 32;
        ch->inbox = (uint8_t *)calloc(ch->inbox_cap, 1);
    }
    size_t n = len;
    if (ch->inbox_cap < ch->delivery_capacity && n < ch->delivery_capacity && ch->delivery_capacity < 4096) n = ch->delivery_capacity;
    memcpy(ch->inbox, data, n);
    ch->inbox_len = n;
    return RELAY_OK;
}

int relay_channel_read(relay_conn *conn, uint16_t channel_id, uint8_t *out, size_t cap, size_t *nread) {
    if (!conn || !out || !nread || channel_id >= RELAY_MAX_CHANNELS) return RELAY_EINVAL;
    relay_channel *ch = &conn->channels[channel_id];
    if (ch->state == RELAY_CHANNEL_EMPTY || !ch->inbox) return RELAY_ESTATE;
    size_t n = ch->inbox_len < cap ? ch->inbox_len : cap;
    memcpy(out, ch->inbox, n);
    *nread = n;
    return RELAY_OK;
}
