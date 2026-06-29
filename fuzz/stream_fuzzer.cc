#include "../src/api/relaymesh.h"
#include <stdint.h>
#include <stddef.h>
#include "fuzzer_driver.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    relay_conn *conn = relay_conn_create();
    if (!conn) return 0;
    relay_conn_enable_compression(conn, 1); uint8_t k[4]={1,2,3,4}; relay_conn_set_key(conn,k,sizeof(k));
    relay_conn_feed(conn, data, size);

    relay_conn_destroy(conn);
    return 0;
}

int main(int argc, char **argv) {
    return relaymesh_run_fuzzer_input(argc, argv, LLVMFuzzerTestOneInput);
}
