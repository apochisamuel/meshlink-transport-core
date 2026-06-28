#include "../src/api/relaymesh.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    relay_conn *conn = relay_conn_create();
    if (!conn) return 0;

    relay_conn_feed(conn, data, size);
    relay_conn_feed(conn, data, size);
    relay_conn_destroy(conn);
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) return 0;
    FILE *fp = fopen(argv[1], "rb");
    if (!fp) return 1;
    fseek(fp, 0, SEEK_END); long n = ftell(fp); fseek(fp, 0, SEEK_SET);
    if (n < 0) { fclose(fp); return 1; }
    uint8_t *buf = (uint8_t *)malloc((size_t)n ? (size_t)n : 1);
    if (!buf) { fclose(fp); return 1; }
    if ((size_t)n && fread(buf, 1, (size_t)n, fp) != (size_t)n) { free(buf); fclose(fp); return 1; }
    fclose(fp);
    int rc = LLVMFuzzerTestOneInput(buf, (size_t)n);
    free(buf);
    return rc;
}
