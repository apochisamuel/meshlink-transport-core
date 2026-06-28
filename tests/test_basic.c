#include "../src/api/relaymesh.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>

static size_t add(uint8_t *buf, size_t off, uint8_t type, uint16_t ch, uint32_t seq, const uint8_t *p, size_t n) {
    size_t wrote = 0;
    int rc = relay_build_frame(type, ch, seq, p, n, buf + off, 4096 - off, &wrote);
    assert(rc == 0);
    return off + wrote;
}

int main(void) {
    uint8_t buf[4096]; size_t off = 0;
    const uint8_t data[] = {1,2,3,4,5};
    off = add(buf, off, 1, 0, 1, NULL, 0);
    off = add(buf, off, 2, 0, 2, NULL, 0);
    off = add(buf, off, 3, 1, 3, NULL, 0);
    off = add(buf, off, 4, 1, 4, data, sizeof(data));
    relay_conn *conn = relay_conn_create();
    assert(conn);
    assert(relay_conn_feed(conn, buf, off) == 0);
    assert(relay_conn_state(conn) == 2);
    assert(relay_channel_state(conn, 1) == 1);
    uint8_t out[16]; size_t got = 0;
    assert(relay_channel_read(conn, 1, out, sizeof(out), &got) == 0);
    assert(got >= sizeof(data));
    relay_conn_destroy(conn);
    return 0;
}
