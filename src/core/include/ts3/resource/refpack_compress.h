// RefPack compressor: the compression half of ts3/refpack.h, rewritten from TS3W.exe
// 0x004EC200 (RefPackCodec::Compress).
#pragma once

#include <cstdint>

namespace ts3::refpack {

// TS3W.exe 0x004EC200. Compresses src into dst and returns the total size (header
// included), byte-identical to the game's codec for every input.
//
//   flags & 1        with dst == nullptr: return the size estimate (srcLen * 5 / 4 + 32)
//                    instead of compressing.
//   flags & 2        128 KiB window, header without the 0x4000 bit (mode 1).
//   flags & 0x10000  insert only the match start into the hash tables (mode 2).
//   otherwise        16 KiB window with the 0x4000 header bit (mode 0).
//   dst == nullptr without flags & 1: measure pass, returns the size a real call produces.
//
// dstCap is part of the codec's signature but the original never reads it: there is no
// buffer-full check and no "output bigger than input" fallback, so the caller must size
// the buffer (that is what the estimate and measure paths are for).
uint32_t Compress(const void* src, uint32_t srcLen, void* dst, uint32_t dstCap, uint32_t flags);

}  // namespace ts3::refpack
