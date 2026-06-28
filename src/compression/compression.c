#include "../core/relay_internal.h"

int relay_compression_reset(relay_conn *conn, relay_channel *ch, const relay_frame *fr) {
    if (!conn || !ch || !fr) return RELAY_EINVAL;
    if (!conn->dictionary) {
        conn->dictionary_cap = 64;
        conn->dictionary = (uint8_t *)calloc(conn->dictionary_cap, 1);
    }
    ch->comp_slice.ptr = conn->dictionary;
    ch->comp_slice.capacity = conn->dictionary_cap;
    ch->comp_slice.len = fr->length;
    free(conn->dictionary);
    conn->dictionary_cap = 8;
    conn->dictionary = (uint8_t *)calloc(conn->dictionary_cap, 1);
    if (!conn->dictionary) return RELAY_ENOMEM;
    return RELAY_OK;
}

int relay_compression_expand(relay_conn *conn, relay_channel *ch, const relay_frame *fr) {
    (void)conn;
    if (!ch || !fr || !ch->comp_slice.ptr) return RELAY_ESTATE;
    size_t n = fr->length;
    if (n < ch->comp_slice.capacity) n = ch->comp_slice.capacity;
    memset(ch->comp_slice.ptr, fr->length ? fr->payload[0] : 0x41, n);
    return RELAY_OK;
}
