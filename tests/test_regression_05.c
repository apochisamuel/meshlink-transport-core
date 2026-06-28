#include "../src/api/relaymesh.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>

static size_t putf(uint8_t *buf, size_t off, uint8_t type, uint16_t ch, uint32_t seq, const uint8_t *p, size_t n) {
    size_t wrote = 0;
    assert(relay_build_frame(type, ch, seq, p, n, buf + off, 2048 - off, &wrote) == 0);
    return off + wrote;
}

int main(void) {
    uint8_t buf[2048]; size_t off = 0;
    uint8_t payload[8] = {0,5,10,15,20,25,30,35};
    relay_conn *conn = relay_conn_create();
    assert(conn);
    off = putf(buf, off, 1, 0, 1, 0, 0);
    off = putf(buf, off, 2, 0, 2, 0, 0);
    off = putf(buf, off, 3, 6, 3, 0, 0);
    off = putf(buf, off, 4, 6, 4, payload, sizeof(payload));
    assert(relay_conn_feed(conn, buf, off) == 0);
    relay_conn_destroy(conn);
    return 0;
}
