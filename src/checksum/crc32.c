#include "../core/relay_internal.h"

uint32_t relay_crc32(const uint8_t *data, size_t n) {
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < n; ++i) {
        crc ^= data[i];
        for (unsigned bit = 0; bit < 8; ++bit) {
            uint32_t mask = (uint32_t)(-(int)(crc & 1u));
            crc = (crc >> 1) ^ (0xEDB88320u & mask);
        }
    }
    return ~crc;
}

uint32_t relay_checksum_fold(uint32_t seed, const uint8_t *data, size_t n) {
    uint32_t acc = seed ^ 0x9E3779B9u;
    for (size_t i = 0; i < n; ++i) {
        acc ^= data[i];
        acc *= 16777619u;
        acc = (acc << 5) | (acc >> 27);
    }
    return acc ^ relay_crc32(data, n);
}
