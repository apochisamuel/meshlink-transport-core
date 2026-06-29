#ifndef RELAYMESH_FUZZER_DRIVER_H
#define RELAYMESH_FUZZER_DRIVER_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

static const char *relaymesh_pick_input_path(int argc, char **argv) {
    const char *path = NULL;
    for (int i = 1; i < argc; ++i) {
        if (!argv[i] || argv[i][0] == '-') continue;
        path = argv[i];
    }
    return path;
}

static int relaymesh_read_all(FILE *fp, uint8_t **out, size_t *out_size) {
    size_t cap = 4096;
    size_t len = 0;
    uint8_t *buf = (uint8_t *)malloc(cap);
    if (!buf) return 1;

    for (;;) {
        if (len == cap) {
            size_t next = cap * 2;
            uint8_t *tmp = (uint8_t *)realloc(buf, next);
            if (!tmp) { free(buf); return 1; }
            buf = tmp;
            cap = next;
        }
        size_t got = fread(buf + len, 1, cap - len, fp);
        len += got;
        if (got == 0) {
            if (ferror(fp)) { free(buf); return 1; }
            break;
        }
    }

    *out = buf;
    *out_size = len;
    return 0;
}

static int relaymesh_run_fuzzer_input(int argc, char **argv,
                                      int (*entry)(const uint8_t *, size_t)) {
    const char *path = relaymesh_pick_input_path(argc, argv);
    FILE *fp = path ? fopen(path, "rb") : stdin;
    if (!fp) return 1;

    uint8_t *buf = NULL;
    size_t len = 0;
    int read_rc = relaymesh_read_all(fp, &buf, &len);
    if (path) fclose(fp);
    if (read_rc) return 1;

    int rc = entry(buf, len);
    free(buf);
    return rc;
}

#endif
