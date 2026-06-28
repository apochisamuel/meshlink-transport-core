#include "../core/relay_internal.h"

static int relay_dispatch_session(relay_conn *conn, const relay_frame *fr) {
    if (fr->type == RELAY_F_HELLO) {
        conn->state = RELAY_CONN_HELLO;
        return relay_transcript_record(conn, fr->type, fr->channel_id, fr->length);
    }
    if (fr->type == RELAY_F_HELLO_ACK && conn->state == RELAY_CONN_HELLO) {
        conn->state = RELAY_CONN_READY;
        return relay_transcript_record(conn, fr->type, fr->channel_id, fr->length);
    }
    if (fr->type == RELAY_F_PING && conn->state == RELAY_CONN_READY) {
        return relay_transcript_record(conn, fr->type, fr->channel_id, fr->length);
    }
    return RELAY_ESTATE;
}

int relay_dispatch_frame(relay_conn *conn, const relay_frame *fr) {
    if (!conn || !fr) return RELAY_EINVAL;
    if (fr->type == RELAY_F_HELLO || fr->type == RELAY_F_HELLO_ACK || fr->type == RELAY_F_PING) {
        return relay_dispatch_session(conn, fr);
    }
    if (conn->state != RELAY_CONN_READY) return RELAY_ESTATE;
    relay_channel *ch = relay_channel_lookup(conn, fr->channel_id, fr->type == RELAY_F_OPEN);
    switch (fr->type) {
        case RELAY_F_OPEN: return relay_channel_handle_open(conn, fr);
        case RELAY_F_DATA:
            if (!ch || ch->state != RELAY_CHANNEL_OPEN) return RELAY_ESTATE;
            return relay_channel_deliver(conn, ch, fr->payload, fr->length);
        case RELAY_F_FRAG_BEGIN:
            if (!ch || ch->state != RELAY_CHANNEL_OPEN) return RELAY_ESTATE;
            if (ch->reassembly) relay_reassembly_release(ch->reassembly);
            ch->reassembly = relay_reassembly_begin(conn->window);
            if (!ch->reassembly) return RELAY_ENOMEM;
            relay_crypto_bind_fragment(conn, ch);
            return relay_reassembly_append(ch->reassembly, fr->payload, fr->length);
        case RELAY_F_FRAG_MORE:
        case RELAY_F_RETRANS:
            if (!ch || (ch->state != RELAY_CHANNEL_OPEN && ch->state != RELAY_CHANNEL_DRAINING)) return RELAY_ESTATE;
            if (!ch->reassembly) return RELAY_ESTATE;
            if (relay_reassembly_append(ch->reassembly, fr->payload, fr->length) != RELAY_OK) return RELAY_EINVAL;
            if (fr->type == RELAY_F_RETRANS || (fr->flags & 1u)) return relay_reassembly_finish(conn, ch);
            return RELAY_OK;
        case RELAY_F_CLOSE: return relay_channel_handle_close(conn, fr);
        case RELAY_F_ERROR: return relay_channel_handle_error(conn, fr);
        case RELAY_F_WINDOW:
            if (!ch) return RELAY_ESTATE;
            return relay_flow_apply_window(conn, ch, fr->length ? fr->payload[0] : 8);
        case RELAY_F_RECEIPT:
            if (!ch) return RELAY_ESTATE;
            if (fr->length >= 4 && fr->payload[0] == 'R' && fr->payload[1] == 'O' && fr->payload[2] == 'L' && fr->payload[3] == 'L') return relay_receipt_rollback(conn, ch, fr);
            return relay_receipt_track(conn, ch, fr);
        case RELAY_F_TIMER:
            if (!ch) return RELAY_ESTATE;
            return relay_scheduler_run(conn, ch);
        case RELAY_F_MIGRATE:
            if (!ch) return RELAY_ESTATE;
            return relay_route_migrate_internal(conn, ch, fr->length ? fr->payload[0] : 1);
        case RELAY_F_TRANSCRIPT_VIEW: return relay_diagnostics_capture(conn);
        case RELAY_F_TRANSCRIPT_COMPACT: return relay_transcript_compact(conn);
        case RELAY_F_EXPORT: {
            char buf[128];
            return relay_diagnostics_export(conn, buf, sizeof(buf));
        }
        case RELAY_F_COMP_RESET:
            if (!ch) return RELAY_ESTATE;
            return relay_compression_reset(conn, ch, fr);
        case RELAY_F_COMP_EXPAND:
            if (!ch) return RELAY_ESTATE;
            return relay_compression_expand(conn, ch, fr);
        case RELAY_F_KEY_ROTATE: return relay_crypto_rotate(conn, fr);
        case RELAY_F_KEY_COMPLETE:
            if (!ch) return RELAY_ESTATE;
            return relay_crypto_complete(conn, ch, fr);
        case RELAY_F_PRIORITY:
            if (!ch) return RELAY_ESTATE;
            if (!ch->priority_token) ch->priority_token = malloc(32);
            return ch->priority_token ? RELAY_OK : RELAY_ENOMEM;
        case RELAY_F_CANCEL:
            if (!ch) return RELAY_ESTATE;
            return relay_scheduler_cancel(conn, ch);
        case RELAY_F_REQUEUE:
            if (!ch) return RELAY_ESTATE;
            return relay_scheduler_requeue(conn, ch);
        case RELAY_F_HANDOFF:
            if (!ch) return RELAY_ESTATE;
            return relay_route_handoff(conn, ch, fr->length ? fr->payload[0] : 2);
        case RELAY_F_ACK_CLEAN: return relay_receipt_ack_cleanup(conn, fr);
        case RELAY_F_STREAM_VIEW:
            if (!ch) return RELAY_ESTATE;
            return relay_stream_make_view(conn, ch, fr);
        case RELAY_F_EVICT:
            if (!ch) return RELAY_ESTATE;
            return relay_stream_evict(conn, ch);
        case RELAY_F_STREAM_READ: {
            uint8_t tmp[64]; size_t got=0;
            if (!ch) return RELAY_ESTATE;
            return relay_stream_resume(conn, ch, tmp, sizeof(tmp), &got);
        }
        case RELAY_F_HEADER_EXT: return relay_header_extension_apply(conn, fr);
        default: return RELAY_EINVAL;
    }
}
