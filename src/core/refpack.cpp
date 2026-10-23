#include "ts3/refpack.h"

namespace ts3::refpack {

bool IsCompressed(const void* src)
{
    const auto* p = static_cast<const uint8_t*>(src);
    const uint32_t header = (uint32_t{p[0]} << 8) | p[1];
    return (header & 0x1FFF) == 0x10FB;
}

uint32_t ReadBigEndian(const uint8_t* p, int bytes)
{
    switch (bytes) {
    case 1: return p[0];
    case 2: return (uint32_t{p[0]} << 8) | p[1];
    case 3: return (uint32_t{p[0]} << 16) | (uint32_t{p[1]} << 8) | p[2];
    case 4: return (uint32_t{p[0]} << 24) | (uint32_t{p[1]} << 16) | (uint32_t{p[2]} << 8) | p[3];
    default: return 0;
    }
}

uint32_t Decode(uint8_t* dst, uint32_t dstCap, const uint8_t* src, uint32_t srcLen)
{
    if (src == nullptr || srcLen < 2)
        return 0;

    // Header: flags, 0xFB, [compressed size if flags & 1], decompressed size.
    // Sizes are 3 bytes, or 4 when flags & 0x80.
    const uint8_t flags = src[0];
    const uint32_t sizeBytes = (flags & 0x80) ? 4 : 3;
    const uint8_t* in = src + 2;
    uint32_t inLeft = srcLen - 2;
    if (flags & 1) {
        if (inLeft < sizeBytes)
            return 0;
        in += sizeBytes;
        inLeft -= sizeBytes;
    }
    if (inLeft < sizeBytes)
        return 0;
    const uint32_t declaredSize = ReadBigEndian(in, static_cast<int>(sizeBytes));
    in += sizeBytes;
    inLeft -= sizeBytes;

    uint8_t* out = dst;
    // Copies `n` literal bytes from the input; false if input or output would overflow.
    auto literals = [&](uint32_t n, uint32_t consumedHeader) -> bool {
        if (dstCap < n)
            return false;
        dstCap -= n;
        if (inLeft - consumedHeader < n)
            return false;
        inLeft = inLeft - consumedHeader - n;
        for (; n; --n)
            *out++ = *in++;
        return true;
    };
    // Copies `n` bytes from `distance + 1` back in the output; overlap is intended.
    auto backref = [&](uint32_t distance, uint32_t n) -> bool {
        if (static_cast<size_t>(out - dst) < size_t{distance} + 1)
            return false;  // reference before the start of the output
        const uint8_t* ref = out - distance - 1;
        if (dstCap < n)
            return false;
        dstCap -= n;
        for (; n; --n)
            *out++ = *ref++;
        return true;
    };

    while (inLeft != 0) {
        const uint8_t b0 = in[0];
        const uint32_t after = inLeft - 1;  // bytes after the command byte
        if ((b0 & 0x80) == 0) {
            // 0xxxxxxx: 2-byte command; 0-3 literals, 3-10 bytes from up to 1024 back.
            if (after == 0)
                return 0;
            const uint8_t b1 = in[1];
            in += 2;
            if (!literals(b0 & 3, 2))
                return 0;
            if (!backref((uint32_t{b0} & 0x60) << 3 | b1, ((b0 >> 2) & 7) + 3))
                return 0;
        } else if ((b0 & 0x40) == 0) {
            // 10xxxxxx: 3-byte command; 0-3 literals, 4-67 bytes from up to 16 KiB back.
            if (after < 2)
                return 0;
            const uint8_t b1 = in[1], b2 = in[2];
            in += 3;
            if (!literals(b1 >> 6, 3))
                return 0;
            if (!backref((uint32_t{b1} & 0x3F) << 8 | b2, (b0 & 0x3Fu) + 4))
                return 0;
        } else if ((b0 & 0x20) == 0) {
            // 110xxxxx: 4-byte command; 0-3 literals, 5-1028 bytes from up to 128 KiB back.
            if (after < 3)
                return 0;
            const uint8_t b1 = in[1], b2 = in[2], b3 = in[3];
            in += 4;
            if (!literals(b0 & 3, 4))
                return 0;
            const uint32_t distance = (uint32_t{b0} & 0x10) << 12 | uint32_t{b1} << 8 | b2;
            if (!backref(distance, b3 + 5 + ((uint32_t{b0} & 0x0C) << 6)))
                return 0;
        } else {
            // 111xxxxx: literal run of 4-112 bytes, or (>= 0xFC) stop code with 0-3 literals.
            uint32_t n = (b0 & 0x1Fu) * 4 + 4;
            if (n > 0x70) {
                n = b0 & 3;
                if (dstCap < n || after < n)
                    return 0;
                ++in;
                for (; n; --n)
                    *out++ = *in++;
                return declaredSize;
            }
            ++in;
            if (!literals(n, 1))
                return 0;
        }
    }
    return 0;  // input ended without a stop code
}

uint32_t Decompress(const void* src, uint32_t srcLen, void* dst, uint32_t dstCap)
{
    const auto* p = static_cast<const uint8_t*>(src);
    if (!IsCompressed(p))
        return kError;
    const uint32_t size = ReadBigEndian(p + 2, (p[0] & 0x80) ? 4 : 3);
    if (dst == nullptr)
        return size;
    if (size > dstCap)
        return kError;
    return Decode(static_cast<uint8_t*>(dst), dstCap, p, srcLen);
}

}  // namespace ts3::refpack
