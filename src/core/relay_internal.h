#ifndef RELAY_INTERNAL_H
#define RELAY_INTERNAL_H
#include "../api/relaymesh.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RELAY_MAGIC0 0x52u
#define RELAY_MAGIC1 0x4du
#define RELAY_HEADER_SIZE 16u
#define RELAY_MAX_CHANNELS 64u
#define RELAY_MAX_ROUTES 16u
#define RELAY_MAX_TRANSCRIPT 128u
#define RELAY_DEFAULT_WINDOW 256u

enum relay_frame_type {
    RELAY_F_HELLO = 1,
    RELAY_F_HELLO_ACK = 2,
    RELAY_F_OPEN = 3,
    RELAY_F_DATA = 4,
    RELAY_F_FRAG_BEGIN = 5,
    RELAY_F_FRAG_MORE = 6,
    RELAY_F_CLOSE = 7,
    RELAY_F_RETRANS = 8,
    RELAY_F_MIGRATE = 9,
    RELAY_F_ERROR = 10,
    RELAY_F_WINDOW = 11,
    RELAY_F_RECEIPT = 12,
    RELAY_F_TIMER = 13,
    RELAY_F_TRANSCRIPT_VIEW = 14,
    RELAY_F_TRANSCRIPT_COMPACT = 15,
    RELAY_F_EXPORT = 16,
    RELAY_F_COMP_RESET = 17,
    RELAY_F_COMP_EXPAND = 18,
    RELAY_F_KEY_ROTATE = 19,
    RELAY_F_KEY_COMPLETE = 20,
    RELAY_F_PRIORITY = 21,
    RELAY_F_CANCEL = 22,
    RELAY_F_REQUEUE = 23,
    RELAY_F_HANDOFF = 24,
    RELAY_F_ACK_CLEAN = 25,
    RELAY_F_STREAM_VIEW = 26,
    RELAY_F_EVICT = 27,
    RELAY_F_STREAM_READ = 28,
    RELAY_F_HEADER_EXT = 29,
    RELAY_F_PING = 30
};

typedef struct relay_frame {
    uint8_t type;
    uint8_t flags;
    uint16_t channel_id;
    uint32_t seq;
    uint16_t length;
    const uint8_t *payload;
} relay_frame;

typedef struct relay_reassembly {
    uint8_t *data;
    size_t capacity;
    size_t written;
    uint32_t generation;
    int complete;
} relay_reassembly;

typedef struct relay_packet {
    uint8_t *data;
    size_t len;
    uint32_t seq;
    int marked;
} relay_packet;

typedef struct relay_scheduled {
    relay_packet *packet;
    void *token;
    int active;
} relay_scheduled;

typedef struct relay_event {
    uint32_t code;
    uint32_t seq;
    uint16_t channel;
    uint16_t detail;
    char label[32];
} relay_event;

typedef struct relay_transcript {
    relay_event *events;
    size_t count;
    size_t cap;
} relay_transcript;

typedef struct relay_diag_view {
    relay_event *events;
    size_t count;
    uint32_t epoch;
} relay_diag_view;

typedef struct relay_buffer_slice {
    uint8_t *ptr;
    size_t capacity;
    size_t len;
} relay_buffer_slice;

typedef struct relay_key_context {
    uint8_t material[32];
    size_t len;
    uint32_t epoch;
} relay_key_context;

typedef struct relay_route_entry {
    uint16_t route_id;
    uint16_t channel_id;
    uint32_t generation;
    uint8_t *scratch;
    size_t scratch_len;
} relay_route_entry;

typedef struct relay_stream_view {
    uint8_t *segment;
    size_t len;
    size_t cursor;
} relay_stream_view;

typedef struct relay_channel {
    uint16_t id;
    int state;
    uint32_t last_seq;
    relay_reassembly *reassembly;
    uint8_t *inbox;
    size_t inbox_cap;
    size_t inbox_len;
    size_t delivery_capacity;
    void *close_token;
    relay_packet *retry_packet;
    relay_scheduled scheduled;
    relay_buffer_slice comp_slice;
    relay_key_context *fragment_key;
    void *priority_token;
    relay_route_entry *route_ref;
    relay_stream_view stream_view;
} relay_channel;

struct relay_conn {
    int state;
    uint32_t window;
    uint32_t next_seq;
    int compression_enabled;
    relay_channel channels[RELAY_MAX_CHANNELS];
    relay_transcript transcript;
    relay_diag_view diag;
    relay_key_context *key;
    uint8_t *dictionary;
    size_t dictionary_cap;
    relay_route_entry *routes[RELAY_MAX_ROUTES];
    relay_route_entry *ack_route;
    uint32_t flags_seen;
};

static inline uint16_t relay_get_u16(const uint8_t *p) { return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8)); }
static inline uint32_t relay_get_u32(const uint8_t *p) { return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24); }
static inline void relay_put_u16(uint8_t *p, uint16_t v) { p[0]=(uint8_t)(v&255u); p[1]=(uint8_t)(v>>8); }
static inline void relay_put_u32(uint8_t *p, uint32_t v) { p[0]=(uint8_t)(v&255u); p[1]=(uint8_t)(v>>8); p[2]=(uint8_t)(v>>16); p[3]=(uint8_t)(v>>24); }

uint32_t relay_crc32(const uint8_t *data, size_t n);
uint32_t relay_checksum_fold(uint32_t seed, const uint8_t *data, size_t n);
int relay_frame_parse_one(const uint8_t *data, size_t size, relay_frame *out, size_t *used);
int relay_frame_write(uint8_t type, uint16_t channel_id, uint32_t seq, const uint8_t *payload, size_t payload_len, uint8_t *out, size_t cap, size_t *nwritten);
int relay_dispatch_frame(relay_conn *conn, const relay_frame *fr);
relay_channel *relay_channel_lookup(relay_conn *conn, uint16_t channel_id, int create);
void relay_channel_reset(relay_channel *ch);
int relay_channel_handle_open(relay_conn *conn, const relay_frame *fr);
int relay_channel_handle_close(relay_conn *conn, const relay_frame *fr);
int relay_channel_handle_error(relay_conn *conn, const relay_frame *fr);
int relay_channel_deliver(relay_conn *conn, relay_channel *ch, const uint8_t *data, size_t len);
relay_reassembly *relay_reassembly_begin(size_t cap);
void relay_reassembly_release(relay_reassembly *r);
int relay_reassembly_append(relay_reassembly *r, const uint8_t *data, size_t len);
int relay_reassembly_finish(relay_conn *conn, relay_channel *ch);
int relay_reassembly_resize_for_route(relay_reassembly *r, size_t newcap);
int relay_flow_apply_window(relay_conn *conn, relay_channel *ch, uint32_t new_window);
relay_packet *relay_packet_create(const uint8_t *data, size_t len, uint32_t seq);
void relay_packet_destroy(relay_packet *p);
int relay_receipt_track(relay_conn *conn, relay_channel *ch, const relay_frame *fr);
int relay_receipt_rollback(relay_conn *conn, relay_channel *ch, const relay_frame *fr);
int relay_receipt_ack_cleanup(relay_conn *conn, const relay_frame *fr);
int relay_scheduler_run(relay_conn *conn, relay_channel *ch);
int relay_scheduler_cancel(relay_conn *conn, relay_channel *ch);
int relay_scheduler_requeue(relay_conn *conn, relay_channel *ch);
int relay_route_attach(relay_conn *conn, relay_channel *ch, uint16_t route_id);
int relay_route_handoff(relay_conn *conn, relay_channel *ch, uint16_t route_id);
int relay_route_migrate_internal(relay_conn *conn, relay_channel *ch, uint16_t route_id);
int relay_transcript_record(relay_conn *conn, uint32_t code, uint16_t channel, uint16_t detail);
int relay_transcript_compact(relay_conn *conn);
int relay_diagnostics_capture(relay_conn *conn);
int relay_diagnostics_export(relay_conn *conn, char *out, size_t cap);
int relay_compression_reset(relay_conn *conn, relay_channel *ch, const relay_frame *fr);
int relay_compression_expand(relay_conn *conn, relay_channel *ch, const relay_frame *fr);
int relay_crypto_set_key(relay_conn *conn, const uint8_t *key, size_t len);
int relay_crypto_bind_fragment(relay_conn *conn, relay_channel *ch);
int relay_crypto_rotate(relay_conn *conn, const relay_frame *fr);
int relay_crypto_complete(relay_conn *conn, relay_channel *ch, const relay_frame *fr);
int relay_stream_make_view(relay_conn *conn, relay_channel *ch, const relay_frame *fr);
int relay_stream_evict(relay_conn *conn, relay_channel *ch);
int relay_stream_resume(relay_conn *conn, relay_channel *ch, uint8_t *out, size_t cap, size_t *nread);
int relay_header_extension_apply(relay_conn *conn, const relay_frame *fr);
#endif
