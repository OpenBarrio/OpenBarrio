// Cluster unit clu_aa83d0_Terrain (subsystem Terrain): two TS3W.exe leaves over the
// terrain height-field object (the uint16 heightmap the terrain builder and the cutter
// read):
//
//   0x00AA83D0  SampleHeight    thiscall, ret 8: one height sample as double
//   0x00AA8550  RowStrideBytes  thiscall, plain ret: 2 * width (row stride in bytes)
//
// Layout (the x86 game's; this view covers exactly the bytes the two leaves touch):
// the height scale (float) sits at +0x08, the map dimensions - width in samples and
// height in samples - at +0x14/+0x18, and the sample array (width*height uint16,
// row-major) at +0x38. Callers confirm the reading: 0x00C87420 clamps its coordinates
// to [0, width)/[0, height) and treats -100.0 as "no height"; 0x00C24380 strides the
// array as `base + RowStrideBytes() * row + 2 * col`.
//
// EA quirks kept: both bounds checks of SampleHeight are UNSIGNED (jae), the row index
// is a wrapping 32-bit imul, and the out-of-range result is the float constant
// 0xC2C80000 (-100.0f) loaded with fld and returned as double (exact - the in-range
// product of a 16-bit sample and a 24-bit float is exact in double as well).
#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::terrain {

// The height-field object, field-of-view style: only the bytes SampleHeight and
// RowStrideBytes read have names; the rest is payload this unit never touches.
struct TerrainHeightField {
    uint8_t pad00[0x08];
    float scale;                 // +0x08 height scale the sample is multiplied by
    uint8_t pad0C[0x08];
    uint32_t width;              // +0x14 samples per row
    uint32_t height;             // +0x18 rows
    uint8_t pad1C[0x1C];
    const uint16_t* samples;     // +0x38 width*height uint16 samples, row-major

    // 0x00AA83D0 (ret 8): -100.0 when x >= width or y >= height (unsigned compares),
    // else (double)samples[x + y * width] * (double)scale.
    double SampleHeight(uint32_t x, uint32_t y) const;

    // 0x00AA8550: 2 * width, wrapping at 2^32 - the row stride in bytes.
    uint32_t RowStrideBytes() const;
};
TS3_X86_OFFSET(TerrainHeightField, scale, 0x08);
TS3_X86_OFFSET(TerrainHeightField, width, 0x14);
TS3_X86_OFFSET(TerrainHeightField, height, 0x18);
TS3_X86_OFFSET(TerrainHeightField, samples, 0x38);
TS3_X86_SIZE(TerrainHeightField, 0x3C);

}  // namespace ts3::terrain
