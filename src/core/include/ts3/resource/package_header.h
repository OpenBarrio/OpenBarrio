// DBPF package header validation, bug-for-bug compatible with TS3W.exe 1.67.2.
// Portable: builds unchanged for x86 and x64.
#pragma once

#include <cstddef>
#include <cstdint>

namespace ts3::resource {

// The 96-byte DBPF 2.0 header, as the engine reads it (fields this code checks).
struct PackageHeader {
    uint32_t magic;            // 0x00 'DBPF'
    uint32_t majorVersion;     // 0x04
    uint32_t minorVersion;     // 0x08
    uint32_t unknown0C[6];     // 0x0C
    uint32_t indexCount;       // 0x24
    uint32_t indexOffsetV1;    // 0x28 (DBPF 1.x index offset)
    uint32_t indexSize;        // 0x2C
    uint32_t unknown30[3];     // 0x30
    uint32_t indexVersion;     // 0x3C (must be non-zero)
    uint32_t indexOffsetLow;   // 0x40
    uint32_t indexOffsetHigh;  // 0x44
    uint32_t unknown48[6];     // 0x48
};
static_assert(sizeof(PackageHeader) == 96, "DBPF header is 96 bytes");
static_assert(offsetof(PackageHeader, indexCount) == 0x24 && offsetof(PackageHeader, indexOffsetV1) == 0x28 &&
                  offsetof(PackageHeader, indexSize) == 0x2C && offsetof(PackageHeader, indexVersion) == 0x3C &&
                  offsetof(PackageHeader, indexOffsetLow) == 0x40 && offsetof(PackageHeader, indexOffsetHigh) == 0x44,
              "field offsets must match the file format");

inline constexpr uint32_t kDbpfMagic = 0x46504244;  // "DBPF"

// Pure part of DatabasePackedFile::ValidateHeader (TS3W.exe 0x004A7AD0, a virtual
// __thiscall method that also locks the package and asks its stream for the size).
// Quirk kept on purpose: when both index-offset fields are set, their agreement alone
// decides the result, overriding the magic/version/bounds checks.
bool IsValidPackageHeader(const PackageHeader& h, uint32_t fileSize);

}  // namespace ts3::resource
