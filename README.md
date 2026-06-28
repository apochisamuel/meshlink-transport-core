# librelaymesh

librelaymesh is an embeddable C library and small command-line utility for reliable multiplexed binary message transport over byte streams. It is intended for local services that need session negotiation, logical channels, fragmentation, receipts, route migration, lightweight adapters, and readable diagnostics without bringing in a large runtime.

## Build

```sh
make all
make test
```

The build is fully local. It does not download dependencies or contact remote services.

## API example

```c
#include "src/api/relaymesh.h"

relay_conn *conn = relay_conn_create();
relay_conn_set_window(conn, 256);
relay_channel_open(conn, 1);
relay_conn_feed(conn, bytes, length);
relay_conn_destroy(conn);
```

Applications normally use `relay_build_frame` to encode frames, `relay_conn_feed` to process a byte stream, and `relay_channel_read` to receive completed channel data.

## CLI example

```sh
./relaymesh inspect sample.rm
./relaymesh feed sample.rm
```

`inspect` prints the first frame header. `feed` runs the stream through a normal connection state machine.

## Frame format

| Offset | Size | Field |
|---:|---:|---|
| 0 | 2 | magic bytes `RM` |
| 2 | 1 | frame type |
| 3 | 1 | flags |
| 4 | 2 | channel id, little endian |
| 6 | 4 | sequence number, little endian |
| 10 | 2 | payload length |
| 12 | 4 | CRC-derived checksum |
| 16 | n | payload |

The checksum covers the first twelve header bytes and the payload.

## Channel lifecycle

| State | Meaning |
|---|---|
| empty | slot has no active channel |
| open | data, fragments, receipts, and migration may be processed |
| draining | close was observed and queued data may still be inspected |
| closed | terminal state after error or explicit cleanup |

## Delivery model

A session starts with HELLO and HELLO_ACK. Channels are opened independently over the same byte stream. Data may arrive as complete frames or fragments. Receipt frames record retry metadata. Route migration moves a channel to a different route entry while preserving the application-visible channel id.

## Diagnostics

The transcript layer records compact frame events. Diagnostics can capture a view for export or replay inspection. These reports are intended for state-machine validation and production debugging.

## Robustness validation

The `fuzz/` directory contains byte-stream test harnesses and seed corpora for continuous validation of parser and state-machine behavior. The harnesses exercise channel, session, stream, and migration paths through the public API.

## License

MIT.
