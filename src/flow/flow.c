#include "../core/relay_internal.h"

int relay_flow_apply_window(relay_conn *conn, relay_channel *ch, uint32_t new_window) {
    if (!conn || !ch || new_window == 0 || new_window > 4096) return RELAY_EINVAL;
    conn->window = new_window;
    uint8_t *next = (uint8_t *)calloc(new_window, 1);
    if (!next) return RELAY_ENOMEM;
    size_t copy = ch->inbox_len < new_window ? ch->inbox_len : new_window;
    if (copy && ch->inbox) memcpy(next, ch->inbox, copy);
    free(ch->inbox);
    ch->inbox = next;
    ch->inbox_cap = new_window;
    return RELAY_OK;
}
