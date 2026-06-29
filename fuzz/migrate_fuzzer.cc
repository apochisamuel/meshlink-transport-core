#include "../src/api/relaymesh.h"
#include <stdint.h>
#include <stddef.h>
#include "fuzzer_driver.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    relay_conn *conn = relay_conn_create();
    if (!conn) return 0;
    relay_conn_set_window(conn, 32);
    relay_conn_feed(conn, data, size);
    relay_route_migrate(conn, 1, 7);
    relay_conn_destroy(conn);
    return 0;
}

int main(int argc, char **argv) {
    return relaymesh_run_fuzzer_input(argc, argv, LLVMFuzzerTestOneInput);
}
