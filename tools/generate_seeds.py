#!/usr/bin/env python3
import struct
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
CORPUS = ROOT / "fuzz" / "corpus"

def crc32_c(data):
    crc = 0xffffffff
    for b in data:
        crc ^= b
        for _ in range(8):
            mask = 0xffffffff if (crc & 1) else 0
            crc = ((crc >> 1) ^ (0xEDB88320 & mask)) & 0xffffffff
    return (~crc) & 0xffffffff

def checksum_fold(seed, data):
    acc = (seed ^ 0x9E3779B9) & 0xffffffff
    for b in data:
        acc ^= b
        acc = (acc * 16777619) & 0xffffffff
        acc = ((acc << 5) | (acc >> 27)) & 0xffffffff
    return (acc ^ crc32_c(data)) & 0xffffffff

def frame(t, ch, seq, payload=b"", flags=0):
    h = bytearray(16)
    h[0:2] = b"RM"
    h[2] = t & 255
    h[3] = flags & 255
    h[4:6] = struct.pack("<H", ch)
    h[6:10] = struct.pack("<I", seq)
    h[10:12] = struct.pack("<H", len(payload))
    crc = checksum_fold(0x52534d48, bytes(h[:12]))
    crc = checksum_fold(crc, payload)
    h[12:16] = struct.pack("<I", crc)
    return bytes(h) + payload

def session(frames):
    return b"".join(frames)

def main():
    base = [frame(1,0,1), frame(2,0,2)]
    cases = [
        base,
        base+[frame(3,1,3), frame(4,1,4,b"hello")],
        base+[frame(3,1,3), frame(5,1,4,b"ab"), frame(6,1,5,b"cd")],
        base+[frame(3,1,3), frame(5,1,4,b"partial")],
        base+[frame(3,1,3), frame(4,1,4,b"data"), frame(7,1,5)],
        base+[frame(3,1,3), frame(3,2,4), frame(4,1,5,b"a"), frame(4,2,6,b"b")],
        base+[frame(3,1,3), frame(11,1,4,b"\x20")],
        base+[frame(3,1,3), frame(5,1,4,b"frag"), frame(9,1,5,b"\x03")],
        base+[frame(30,0,3)],
        base+[frame(3,1,3), frame(12,1,4,b"ack")],
        base+[frame(3,1,3), frame(17,1,4,b"dict")],
        base+[frame(3,1,3), frame(12,1,4,b"retry"), frame(13,1,5)]
    ]
    for h in ["channel_fuzzer", "session_fuzzer", "stream_fuzzer", "migrate_fuzzer"]:
        d = CORPUS / h
        d.mkdir(parents=True, exist_ok=True)
        for idx, c in enumerate(cases, 1):
            if h == "channel_fuzzer" and idx == 5:
                c = base + [frame(3,1,3), frame(4,1,4,b"data")]
            (d / f"seed_{idx:02d}.bin").write_bytes(session(c))
if __name__ == "__main__":
    main()
