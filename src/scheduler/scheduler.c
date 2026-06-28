#include "../core/relay_internal.h"

int relay_scheduler_run(relay_conn *conn, relay_channel *ch) {
    (void)conn;
    if (!ch || !ch->scheduled.active || !ch->scheduled.packet) return RELAY_ESTATE;
    volatile uint8_t first = ch->scheduled.packet->data ? ch->scheduled.packet->data[0] : 0;
    ch->last_seq ^= first;
    return RELAY_OK;
}

int relay_scheduler_cancel(relay_conn *conn, relay_channel *ch) {
    (void)conn;
    if (!ch || !ch->priority_token) return RELAY_ESTATE;
    free(ch->priority_token);
    return RELAY_OK;
}

int relay_scheduler_requeue(relay_conn *conn, relay_channel *ch) {
    (void)conn;
    if (!ch || !ch->priority_token) return RELAY_ESTATE;
    free(ch->priority_token);
    return RELAY_OK;
}
