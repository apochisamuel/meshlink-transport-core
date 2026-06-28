#include "../src/api/relaymesh.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>

int main(void) {
    relay_conn *conn = relay_conn_create();
    assert(conn);
    assert(relay_conn_set_window(conn, 128) == 0);
    uint8_t key[4] = {9,8,7,6};
    assert(relay_conn_set_key(conn, key, sizeof(key)) == 0);
    assert(relay_conn_enable_compression(conn, 1) == 0);
    assert(relay_channel_open(conn, 2) == 0);
    assert(relay_route_migrate(conn, 2, 3) == 0);
    relay_conn_destroy(conn);
    return 0;
}
