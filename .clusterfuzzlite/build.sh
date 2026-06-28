#!/bin/sh
set -eu
: "${OUT:=/out}"
make clean
make SAN="-fsanitize=address,fuzzer-no-link" all
mkdir -p "$OUT"
cp fuzz/channel_fuzzer "$OUT/channel_fuzzer"
cp fuzz/session_fuzzer "$OUT/session_fuzzer"
cp fuzz/stream_fuzzer "$OUT/stream_fuzzer"
cp fuzz/migrate_fuzzer "$OUT/migrate_fuzzer"
