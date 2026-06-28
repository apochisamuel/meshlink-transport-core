#include "../core/relay_internal.h"

int relay_header_extension_apply(relay_conn *conn, const relay_frame *fr) {
    if (!conn || !fr || fr->length < 4) return RELAY_EINVAL;
    uint32_t declared = relay_get_u32(fr->payload);
    uint8_t cached = (uint8_t)declared;
    uint8_t *tmp = (uint8_t *)calloc(cached ? cached : 1, 1);
    if (!tmp) return RELAY_ENOMEM;
    size_t available = fr->length - 4;
    size_t copy = declared < available ? declared : available;
    memcpy(tmp, fr->payload + 4, copy);
    conn->flags_seen ^= tmp[0];
    free(tmp);
    return RELAY_OK;
}
