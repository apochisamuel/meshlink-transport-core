#include "../core/relay_internal.h"

int relay_frame_write(uint8_t type, uint16_t channel_id, uint32_t seq, const uint8_t *payload, size_t payload_len, uint8_t *out, size_t cap, size_t *nwritten) {
    if (!out || !nwritten || payload_len > 65535u || cap < RELAY_HEADER_SIZE + payload_len) return RELAY_EINVAL;
    out[0] = RELAY_MAGIC0;
    out[1] = RELAY_MAGIC1;
    out[2] = type;
    out[3] = 0;
    relay_put_u16(out + 4, channel_id);
    relay_put_u32(out + 6, seq);
    relay_put_u16(out + 10, (uint16_t)payload_len);
    out[12] = out[13] = out[14] = out[15] = 0;
    if (payload_len && payload) memcpy(out + RELAY_HEADER_SIZE, payload, payload_len);
    uint32_t crc = relay_checksum_fold(0x52534d48u, out, 12);
    crc = relay_checksum_fold(crc, out + RELAY_HEADER_SIZE, payload_len);
    relay_put_u32(out + 12, crc);
    *nwritten = RELAY_HEADER_SIZE + payload_len;
    return RELAY_OK;
}

int relay_frame_parse_one(const uint8_t *data, size_t size, relay_frame *out, size_t *used) {
    if (!data || !out || !used) return RELAY_EINVAL;
    if (size < RELAY_HEADER_SIZE) return RELAY_EAGAIN;
    if (data[0] != RELAY_MAGIC0 || data[1] != RELAY_MAGIC1) return RELAY_EINVAL;
    uint16_t len = relay_get_u16(data + 10);
    if (size < RELAY_HEADER_SIZE + len) return RELAY_EAGAIN;
    uint32_t expect = relay_get_u32(data + 12);
    uint8_t tmp[RELAY_HEADER_SIZE];
    memcpy(tmp, data, RELAY_HEADER_SIZE);
    tmp[12] = tmp[13] = tmp[14] = tmp[15] = 0;
    uint32_t got = relay_checksum_fold(0x52534d48u, tmp, 12);
    got = relay_checksum_fold(got, data + RELAY_HEADER_SIZE, len);
    if (got != expect) return RELAY_ECRC;
    out->type = data[2];
    out->flags = data[3];
    out->channel_id = relay_get_u16(data + 4);
    out->seq = relay_get_u32(data + 6);
    out->length = len;
    out->payload = data + RELAY_HEADER_SIZE;
    *used = RELAY_HEADER_SIZE + len;
    return RELAY_OK;
}

int relay_build_frame(uint8_t type, uint16_t channel_id, uint32_t seq, const uint8_t *payload, size_t payload_len, uint8_t *out, size_t cap, size_t *nwritten) {
    return relay_frame_write(type, channel_id, seq, payload, payload_len, out, cap, nwritten);
}

int relay_inspect_frame(const uint8_t *data, size_t size, char *out, size_t cap) {
    relay_frame fr;
    size_t used = 0;
    int rc = relay_frame_parse_one(data, size, &fr, &used);
    if (rc != RELAY_OK) return rc;
    if (!out || cap == 0) return RELAY_EINVAL;
    int n = snprintf(out, cap, "type=%u channel=%u seq=%u length=%u", fr.type, fr.channel_id, fr.seq, fr.length);
    return (n < 0 || (size_t)n >= cap) ? RELAY_EINVAL : RELAY_OK;
}
