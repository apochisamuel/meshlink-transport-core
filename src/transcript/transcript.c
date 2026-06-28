#include "../core/relay_internal.h"

int relay_transcript_record(relay_conn *conn, uint32_t code, uint16_t channel, uint16_t detail) {
    if (!conn) return RELAY_EINVAL;
    if (conn->transcript.count == conn->transcript.cap) {
        size_t next_cap = conn->transcript.cap ? conn->transcript.cap * 2 : 16;
        relay_event *next = (relay_event *)realloc(conn->transcript.events, next_cap * sizeof(*next));
        if (!next) return RELAY_ENOMEM;
        conn->transcript.events = next;
        conn->transcript.cap = next_cap;
    }
    relay_event *ev = &conn->transcript.events[conn->transcript.count++];
    ev->code = code;
    ev->seq = conn->next_seq++;
    ev->channel = channel;
    ev->detail = detail;
    snprintf(ev->label, sizeof(ev->label), "frame-%u", code);
    return RELAY_OK;
}

int relay_transcript_compact(relay_conn *conn) {
    if (!conn) return RELAY_EINVAL;
    size_t keep = conn->transcript.count < 4 ? conn->transcript.count : 4;
    relay_event *next = (relay_event *)calloc(8, sizeof(*next));
    if (!next) return RELAY_ENOMEM;
    for (size_t i = 0; i < keep; ++i) next[i] = conn->transcript.events[conn->transcript.count - keep + i];
    free(conn->transcript.events);
    conn->transcript.events = next;
    conn->transcript.count = keep;
    conn->transcript.cap = 8;
    return RELAY_OK;
}
