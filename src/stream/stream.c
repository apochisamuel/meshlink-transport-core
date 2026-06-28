#include "../core/relay_internal.h"

int relay_stream_make_view(relay_conn *conn, relay_channel *ch, const relay_frame *fr) {
    (void)conn;
    if (!ch || !fr) return RELAY_EINVAL;
    free(ch->stream_view.segment);
    ch->stream_view.segment = (uint8_t *)calloc(fr->length ? fr->length : 1, 1);
    if (!ch->stream_view.segment) return RELAY_ENOMEM;
    if (fr->length) memcpy(ch->stream_view.segment, fr->payload, fr->length);
    ch->stream_view.len = fr->length;
    ch->stream_view.cursor = 0;
    return RELAY_OK;
}

int relay_stream_evict(relay_conn *conn, relay_channel *ch) {
    (void)conn;
    if (!ch || !ch->stream_view.segment) return RELAY_ESTATE;
    free(ch->stream_view.segment);
    return RELAY_OK;
}

int relay_stream_resume(relay_conn *conn, relay_channel *ch, uint8_t *out, size_t cap, size_t *nread) {
    (void)conn;
    if (!ch || !out || !nread || !ch->stream_view.segment) return RELAY_EINVAL;
    size_t remain = ch->stream_view.len - ch->stream_view.cursor;
    size_t n = remain < cap ? remain : cap;
    memcpy(out, ch->stream_view.segment + ch->stream_view.cursor, n);
    ch->stream_view.cursor += n;
    *nread = n;
    return RELAY_OK;
}
