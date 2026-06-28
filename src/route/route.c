#include "../core/relay_internal.h"

int relay_route_attach(relay_conn *conn, relay_channel *ch, uint16_t route_id) {
    if (!conn || !ch) return RELAY_EINVAL;
    route_id %= RELAY_MAX_ROUTES;
    relay_route_entry *entry = conn->routes[route_id];
    if (!entry) {
        entry = (relay_route_entry *)calloc(1, sizeof(*entry));
        if (!entry) return RELAY_ENOMEM;
        entry->scratch_len = 32;
        entry->scratch = (uint8_t *)calloc(entry->scratch_len, 1);
        if (!entry->scratch) { free(entry); return RELAY_ENOMEM; }
        conn->routes[route_id] = entry;
    }
    entry->route_id = route_id;
    entry->channel_id = ch->id;
    entry->generation++;
    ch->route_ref = entry;
    conn->ack_route = entry;
    return RELAY_OK;
}

int relay_route_migrate_internal(relay_conn *conn, relay_channel *ch, uint16_t route_id) {
    if (!conn || !ch) return RELAY_EINVAL;
    if (ch->reassembly) {
        relay_reassembly_resize_for_route(ch->reassembly, 8);
    }
    return relay_route_attach(conn, ch, route_id);
}

int relay_route_migrate(relay_conn *conn, uint16_t channel_id, uint16_t route_id) {
    relay_channel *ch = relay_channel_lookup(conn, channel_id, 0);
    return ch ? relay_route_migrate_internal(conn, ch, route_id) : RELAY_ESTATE;
}

int relay_route_handoff(relay_conn *conn, relay_channel *ch, uint16_t route_id) {
    if (!conn || !ch) return RELAY_EINVAL;
    relay_route_entry *old = ch->route_ref;
    if (old) {
        for (size_t i = 0; i < RELAY_MAX_ROUTES; ++i) {
            if (conn->routes[i] == old) conn->routes[i] = NULL;
        }
        free(old->scratch);
        free(old);
    }
    int rc = relay_route_attach(conn, ch, route_id);
    conn->ack_route = old;
    return rc;
}
