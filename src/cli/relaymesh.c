#include "../api/relaymesh.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_file(const char *path, uint8_t **out, size_t *len) {
    FILE *fp = fopen(path, "rb");
    if (!fp) return 1;
    fseek(fp, 0, SEEK_END);
    long n = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (n < 0) { fclose(fp); return 1; }
    uint8_t *buf = (uint8_t *)malloc((size_t)n ? (size_t)n : 1);
    if (!buf) { fclose(fp); return 1; }
    if ((size_t)n && fread(buf, 1, (size_t)n, fp) != (size_t)n) { free(buf); fclose(fp); return 1; }
    fclose(fp);
    *out = buf; *len = (size_t)n;
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "usage: relaymesh inspect|feed <file>\n");
        return 2;
    }
    uint8_t *buf = NULL; size_t len = 0;
    if (read_file(argv[2], &buf, &len)) return 1;
    if (strcmp(argv[1], "inspect") == 0) {
        char line[128];
        int rc = relay_inspect_frame(buf, len, line, sizeof(line));
        if (rc == 0) puts(line);
        free(buf);
        return rc == 0 ? 0 : 1;
    }
    relay_conn *conn = relay_conn_create();
    int rc = relay_conn_feed(conn, buf, len);
    relay_conn_destroy(conn);
    free(buf);
    return rc == 0 ? 0 : 1;
}
