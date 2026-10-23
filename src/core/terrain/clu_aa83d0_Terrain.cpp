// Unit clu_aa83d0_Terrain (Terrain): the two height-field leaves. Semantics from the
// TS3W.exe 1.67.2 listings (the original addresses are noted on each function); both
// are self-contained reads of the object - no game calls, no state touched.
#include "ts3/terrain/clu_aa83d0_Terrain.h"

namespace ts3::terrain {

double TerrainHeightField::SampleHeight(uint32_t x, uint32_t y) const
{
    // TS3W.exe 0x00AA83D0 (ret 8): the width is loaded first and both bounds checks
    // are unsigned (`cmp ...; jae`), so an inverted/huge dimension lets coordinates
    // through exactly as the original would - and a coordinate equal to the dimension
    // is out of range. The -100.0 path is the fld of the float constant 0xC2C80000;
    // -100.0f converts to -100.0 exactly.
    const uint32_t w = width;
    if (x >= w || y >= height)
        return -100.0;

    // the original instruction sequence / the original instruction sequence wrap the index at 2^32 (uint32_t arithmetic
    // does the same), the word load is zero-extended (movzx), and the x87
    // fild dword of that zero-extended value is (double)(uint32)sample - exact for a
    // uint16. Product: 16-bit sample x 24-bit float = at most 40 significand bits, so
    // the extended-precision fmul and a plain double multiply agree bit for bit.
    const uint32_t index = w * y + x;
    return static_cast<double>(samples[index]) * static_cast<double>(scale);
}

uint32_t TerrainHeightField::RowStrideBytes() const
{
    // TS3W.exe 0x00AA8550: the original instruction sequence - 2 * width,
    // wrapping at 2^32 (uint32_t addition wraps the same way).
    return width + width;
}

}  // namespace ts3::terrain
