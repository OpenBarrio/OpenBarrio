// EA RefPack (QFS) decompression, bug-for-bug compatible with TS3W.exe 1.67.2.
// Portable: builds unchanged for x86 and x64.
#pragma once

#include <cstddef>
#include <cstdint>

namespace ts3::refpack {

// TS3W.exe 0x004EC080 (__stdcall, 2 args; the length is ignored).
// True when the first two bytes are a RefPack header: (b0 << 8 | b1) & 0x1FFF == 0x10FB.
bool IsCompressed(const void* src);

// TS3W.exe 0x004EB2F0 (__cdecl). Big-endian read of 1-4 bytes; 0 for other widths.
uint32_t ReadBigEndian(const uint8_t* p, int bytes);

// TS3W.exe 0x004EB3B0 (__cdecl). Decodes `src` into `dst`.
// Returns the decompressed size stored in the header on success (not the number of
// bytes actually written), and 0 on any error, including input that ends without a
// stop code.
uint32_t Decode(uint8_t* dst, uint32_t dstCap, const uint8_t* src, uint32_t srcLen);

// TS3W.exe 0x004EC010 (__stdcall, 5 args; the 5th is ignored).
// - not RefPack:                     0xFFFFFFFF
// - dst == nullptr:                  decompressed size from the header
// - decompressed size > dstCap:      0xFFFFFFFF
// - otherwise:                       Decode(dst, dstCap, src, srcLen)
uint32_t Decompress(const void* src, uint32_t srcLen, void* dst, uint32_t dstCap);

inline constexpr uint32_t kError = 0xFFFFFFFFu;

}  // namespace ts3::refpack
