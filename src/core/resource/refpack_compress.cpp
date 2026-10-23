// RefPack compressor, rewritten from TS3W.exe:
//   0x004EC200  RefPackCodec::Compress (mode from the flags, size estimate)
//   0x004EC0A0  RefPackCompress (header, allocations, encoder dispatch)
//   0x004EB750  small-input encoder (8-bit hash, 0x400 scratch)
//   0x004EBB90  large-input encoder (16-bit hash, 0x40000 scratch)
//   0x004EB340  WriteBigEndian (the size in the header)
// The two encoders are the same state machine with a different hash, so they share one
// implementation here. Output is byte-identical to the original for every input.
//
// Quirks kept because they change the bytes (the project notes):
//  - dstCap is never read (confirmed: 0x004EC0A0 has no [ebp+0xC] access): no bounds
//    check, no give-up. The caller sizes the buffer via the flags&1 estimate or a
//    measure pass (dst == nullptr).
//  - The "hash table" is a masked prev[] array: hashTable[pos & window] holds the
//    previous head of that position's hash, but two positions sharing a slot overwrite
//    each other, so a chain step can jump to an unrelated position. Candidates are
//    validated by a byte compare, but the walk order — and with it the emitted
//    commands — follows those cross-links.
//  - A match is only emitted while at least 8 bytes remain (left >= 4 after the
//    left = remaining - 4 bookkeeping) and never covers the last 4 bytes of the input
//    (maxLen = remaining - 4).
#include "ts3/resource/refpack_compress.h"

#include <cstring>
#include <vector>

namespace ts3::refpack {
namespace {

// 0x004EB750 / 0x004EBB90. dst == nullptr counts the bytes that would be written (the
// measure path of 0x004EC0A0). Returns the encoded body size.
uint32_t Encode(uint8_t* dst, const uint8_t* src, uint32_t srcLen, uint32_t windowLimit,
                uint32_t* scratch, uint32_t* hashTable, bool big, bool wideHash)
{
    std::memset(scratch, 0xFF, wideHash ? 0x40000u : 0x400u);
    uint8_t* out = dst;                    // output cursor (esi)
    const uint8_t* litBase = src;          // start of the pending literal run
    uint32_t pending = 0;                  // pending literal bytes
    int32_t left = static_cast<int32_t>(srcLen) - 4;

    const auto hashOf = [&](const uint8_t* p) -> uint32_t {
        // small: b0 ^ b1 ^ b2; large: ((b0 << 8) | b2) ^ (b1 << 4)
        if (wideHash)
            return (uint32_t{p[0]} << 8 | p[2]) ^ (uint32_t{p[1]} << 4);
        return uint32_t{p[0]} ^ p[1] ^ p[2];
    };

    const uint8_t* cur = src;
    while (left >= 0) {
        uint32_t bestLen = 2;              // v35: best match length found for this position
        uint32_t bestCost = 2;             // v39: encoded size class (2/3/4) of that match
        uint32_t bestDist = 0;             // v42: distance - 1 of that match
        uint32_t maxLen = static_cast<uint32_t>(left);
        if (maxLen >= 1028)
            maxLen = 1028;
        const uint32_t hash = hashOf(cur);
        const uint32_t pos = static_cast<uint32_t>(cur - src);
        const uint32_t head = scratch[hash];   // most recent position with this hash
        const int32_t minPosI = static_cast<int32_t>(pos - windowLimit);
        const uint32_t minPos = minPosI <= 0 ? 0u : static_cast<uint32_t>(minPosI);

        bool searched = false;
        uint32_t cand = head;
        // Signed, as in the original: the -1 fill of the tables ends the walk here.
        if (static_cast<int32_t>(cand) >= static_cast<int32_t>(minPos)) {
            for (;;) {
                searched = true;
                // Prefilter on the byte that would extend the current best.
                if (cur[bestLen] == src[cand + bestLen]) {
                    uint32_t len = 0;
                    while (len < maxLen && cur[len] == src[cand + len])
                        ++len;
                    if (len > bestLen) {
                        const uint32_t dist1 = pos - cand - 1;
                        uint32_t cost;
                        if (dist1 >= 0x400 || len > 10)
                            cost = (dist1 >= 0x4000 || len > 0x43) ? 4u : 3u;
                        else
                            cost = 2u;
                        // Keep the candidate only when len - cost beats the current best
                        // (uint32 wrap exactly as the original's add/sub pair).
                        if (len - cost + 4 > bestLen - bestCost + 4) {
                            bestLen = len;
                            bestCost = cost;
                            bestDist = dist1;
                            if (len >= 1028)
                                break;  // 0x004EB750
                        }
                    }
                }
                cand = hashTable[windowLimit & cand];
                if (static_cast<int32_t>(cand) < static_cast<int32_t>(minPos))
                    break;
            }
        }

        const bool emitMatch = searched && bestCost < bestLen && left >= 4;
        if (emitMatch) {
            // Flush pending literals in 4-aligned runs of at most 112 bytes.
            while (pending > 3) {
                uint32_t n = pending & 0xFFFFFFFCu;
                if (static_cast<int32_t>(n) > 112)
                    n = 112;
                pending -= n;
                if (dst)
                    *out++ = static_cast<uint8_t>((n >> 2) - 33);
                else
                    ++out;
                if (dst)
                    std::memcpy(out, litBase, n);
                out += n;
                litBase += n;
            }
            // The match command, then the 0..3 literals it leaves pending.
            if (bestCost == 2) {
                if (dst) {
                    *out++ = static_cast<uint8_t>(pending + 4 * (bestLen - 3) + ((bestDist >> 3) & 0xE0));
                    *out++ = static_cast<uint8_t>(bestDist);
                } else {
                    out += 2;
                }
            } else if (bestCost == 3) {
                if (dst) {
                    *out++ = static_cast<uint8_t>(bestLen + 124);
                    *out++ = static_cast<uint8_t>((bestDist >> 8) + (pending << 6));
                    *out++ = static_cast<uint8_t>(bestDist);
                } else {
                    out += 3;
                }
            } else {
                if (dst) {
                    *out++ = static_cast<uint8_t>(pending + ((bestDist >> 12) & 0xF0) +
                                                  (((bestLen - 5) >> 6) & 0xFC) - 64);
                    *out++ = static_cast<uint8_t>(bestDist >> 8);
                    *out++ = static_cast<uint8_t>(bestDist);
                    *out++ = static_cast<uint8_t>(bestLen - 5);
                } else {
                    out += 4;
                }
            }
            if (pending) {
                if (dst)
                    std::memcpy(out, litBase, pending);
                out += pending;
                pending = 0;
            }
            // Advance over the match, inserting the covered positions into the tables:
            // only the match start when `big`, every position otherwise.
            if (big) {
                hashTable[windowLimit & pos] = scratch[hash];
                scratch[hash] = pos;
            } else {
                for (uint32_t k = 0; k < bestLen; ++k) {
                    const uint32_t p = pos + k;
                    const uint32_t h = hashOf(cur + k);
                    hashTable[windowLimit & p] = scratch[h];
                    scratch[h] = p;
                }
            }
            cur += bestLen;
            left -= static_cast<int32_t>(bestLen);
            litBase = cur;  // the pending run starts fresh after the match
        } else {
            // No worthwhile match: emit position `pos` as a literal and move on.
            // litBase keeps pointing at the start of the pending run.
            ++pending;
            hashTable[windowLimit & pos] = head;
            scratch[hash] = pos;
            ++cur;
            --left;
        }
    }

    // Tail: everything still pending (plus the remaining bytes) as literal runs, closed
    // by a stop command 0xFC..0xFF carrying 0..3 trailing literals.
    uint32_t rest = static_cast<uint32_t>(left) + pending + 4;
    while (rest > 3) {
        uint32_t n = rest & 0xFFFFFFFCu;
        if (static_cast<int32_t>(n) > 112)
            n = 112;
        rest -= n;
        if (dst)
            *out++ = static_cast<uint8_t>((n >> 2) - 33);
        else
            ++out;
        if (dst)
            std::memcpy(out, litBase, n);
        out += n;
        litBase += n;
    }
    if (dst)
        *out++ = static_cast<uint8_t>(rest - 4);
    else
        ++out;
    if (rest) {
        if (dst)
            std::memcpy(out, litBase, rest);
        out += rest;
    }
    return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(out) - reinterpret_cast<uintptr_t>(dst));
}

}  // namespace

uint32_t Compress(const void* srcV, uint32_t srcLen, void* dstV, uint32_t /*dstCap*/, uint32_t flags)
{
    const auto* src = static_cast<const uint8_t*>(srcV);
    auto* dst = static_cast<uint8_t*>(dstV);

    // 0x004EC200: mode from the flags (bit 1 first, then bit 16).
    const uint32_t mode = (flags & 2) ? 1u : (flags & 0x10000) ? 2u : 0u;

    if (!dst && (flags & 1)) {
        // Size estimate: lea/add/shr chain, kept as-is so it wraps exactly like the game.
        uint32_t e = srcLen;
        e += e * 4;
        e += e;
        e += e;
        e >>= 4;
        return e + 0x20;
    }

    // 0x004EC0A0: signed compares, as in the original (setle / jl).
    const bool small = static_cast<int32_t>(srcLen) <= 0x4000;
    const bool bigHeader = static_cast<int32_t>(srcLen) < 0x1000000;
    uint32_t headerWord = bigHeader ? 0x10FBu : 0x90FBu;
    uint32_t windowLimit = 0x1FFFFu;
    if (!(mode & 1)) {
        headerWord |= 0x4000;
        windowLimit = 0x3FFF;
    }
    const uint32_t sizeBytes = bigHeader ? 3u : 4u;

    // Scratch: 0x400 on the stack for small inputs, 0x40000 from the heap otherwise;
    // the hash/prev table is windowLimit * 4 + 4 bytes. The game's allocator is not
    // involved in the rewrite; neither table's initial contents are ever read.
    std::vector<uint32_t> scratch(small ? 0x400u / 4u : 0x40000u / 4u);
    std::vector<uint32_t> hashTable(windowLimit + 1);

    uint32_t body = 0;
    if (dst) {
        // Header: the flag word big-endian, then srcLen big-endian in 3 or 4 bytes
        // (0x004EB340 WriteBigEndian).
        dst[0] = static_cast<uint8_t>(headerWord >> 8);
        dst[1] = static_cast<uint8_t>(headerWord);
        for (uint32_t i = 0; i < sizeBytes; ++i)
            dst[2 + i] = static_cast<uint8_t>(srcLen >> (8 * (sizeBytes - 1 - i)));
        body = Encode(dst + 2 + sizeBytes, src, srcLen, windowLimit, scratch.data(), hashTable.data(),
                      (mode >> 1) & 1, !small);
    } else {
        body = Encode(nullptr, src, srcLen, windowLimit, scratch.data(), hashTable.data(), (mode >> 1) & 1, !small);
    }
    return body + sizeBytes + 2;
}

}  // namespace ts3::refpack
