#include "../core/relay_internal.h"

int relay_diagnostics_capture(relay_conn *conn) {
    if (!conn || !conn->transcript.events) return RELAY_EINVAL;
    conn->diag.events = conn->transcript.events;
    conn->diag.count = conn->transcript.count;
    conn->diag.epoch++;
    return RELAY_OK;
}

int relay_diagnostics_export(relay_conn *conn, char *out, size_t cap) {
    if (!conn || !out || cap == 0) return RELAY_EINVAL;
    size_t used = 0;
    for (size_t i = 0; i < conn->diag.count && used + 16 < cap; ++i) {
        relay_event *ev = &conn->diag.events[i];
        used += (size_t)snprintf(out + used, cap - used, "%u:%u;", ev->code, ev->channel);
    }
    return RELAY_OK;
}
