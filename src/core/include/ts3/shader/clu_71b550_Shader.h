// Cluster unit clu_71b550_Shader (subsystem Shader): three TS3W.exe leaves over one
// 120-byte record type (a shader parameter block held in a std::vector-like container —
// the callers stride the container by 120 and grow it with the usual insert/reallocate
// idiom).
//
//   0x0071B550  Assign71B550   __thiscall copy of the used fields (ret 4, returns this)
//   0x0071C240  Assign71C240   the same copy (identical field set; only the compiler's
//                              float-store order differs, which does not change memory)
//   0x00722F20  GetIndexIfAligned  returns the +0x10 dword when its low 2 bits are clear,
//                              else 0
//
// The two copies are the compiler-generated element copy of the record: they move every
// member but skip the padding holes (0x06, 0x1C, 0x45, 0x5E, 0x64 and the 0x75..0x77 tail),
// which stay with the destination. The twelve float fields go through the x87 (fld/fstp),
// which quiets a signaling NaN; that observable behaviour is kept (see the .cpp).
//
// In-unit calls go directly to the rewrites below; everything else runs through the ts3
// layout.h bridge (x64 builds abort there instead).
#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::shader {

// One 120-byte shader parameter record. Field names are offset-based: the unit is the
// only consumer and the payload semantics are not needed to reproduce the copies. The
// `gap*` members are struct padding the generated copy never touches.
struct ShaderParamRecord {
    uint32_t d00;       // +0x00 copied
    uint8_t b04;        // +0x04 copied
    uint8_t b05;        // +0x05 copied
    uint8_t gap06[2];   // +0x06 padding, never copied
    uint32_t d08;       // +0x08 copied
    uint32_t d0C;       // +0x0C copied
    uint32_t index;     // +0x10 copied; read by GetIndexIfAligned
    uint32_t d14;       // +0x14 copied
    uint32_t d18;       // +0x18 copied
    uint32_t gap1C;     // +0x1C padding, never copied
    uint32_t d20;       // +0x20 copied
    uint32_t d24;       // +0x24 copied
    uint32_t d28;       // +0x28 copied
    float f2C;          // +0x2C copied (x87)
    float f30;          // +0x30 copied (x87)
    float f34;          // +0x34 copied (x87)
    float f38;          // +0x38 copied (x87)
    float f3C;          // +0x3C copied (x87)
    float f40;          // +0x40 copied (x87)
    uint8_t b44;        // +0x44 copied
    uint8_t gap45[3];   // +0x45 padding, never copied
    float f48;          // +0x48 copied (x87)
    float f4C;          // +0x4C copied (x87)
    float f50;          // +0x50 copied (x87)
    float f54;          // +0x54 copied (x87)
    float f58;          // +0x58 copied (x87)
    uint16_t w5C;       // +0x5C copied
    uint8_t gap5E[2];   // +0x5E padding, never copied
    float f60;          // +0x60 copied (x87)
    uint32_t gap64;     // +0x64 padding, never copied
    uint32_t d68;       // +0x68 copied
    uint32_t d6C;       // +0x6C copied
    uint8_t b70;        // +0x70 copied
    uint8_t b71;        // +0x71 copied
    uint8_t b72;        // +0x72 copied
    uint8_t b73;        // +0x73 copied
    uint8_t b74;        // +0x74 copied
    uint8_t gap75[3];   // +0x75 padding (tail to the 120-byte stride), never copied

    // 0x0071B550: __thiscall copy of the used fields into `this` (ret 4, returns this).
    // The twelve floats go through the x87 (fld/fstp quiets a signaling NaN); the padding
    // holes stay with the destination.
    ShaderParamRecord* Assign71B550(const ShaderParamRecord& src);

    // 0x0071C240: identical copy (same field set as 0x0071B550).
    ShaderParamRecord* Assign71C240(const ShaderParamRecord& src);

    // 0x00722F20: returns the +0x10 dword when (dword & 3) == 0, else 0.
    uint32_t GetIndexIfAligned() const;
};
TS3_X86_OFFSET(ShaderParamRecord, b04, 0x04);
TS3_X86_OFFSET(ShaderParamRecord, d08, 0x08);
TS3_X86_OFFSET(ShaderParamRecord, index, 0x10);
TS3_X86_OFFSET(ShaderParamRecord, gap1C, 0x1C);
TS3_X86_OFFSET(ShaderParamRecord, f2C, 0x2C);
TS3_X86_OFFSET(ShaderParamRecord, f40, 0x40);
TS3_X86_OFFSET(ShaderParamRecord, b44, 0x44);
TS3_X86_OFFSET(ShaderParamRecord, f48, 0x48);
TS3_X86_OFFSET(ShaderParamRecord, w5C, 0x5C);
TS3_X86_OFFSET(ShaderParamRecord, f60, 0x60);
TS3_X86_OFFSET(ShaderParamRecord, gap64, 0x64);
TS3_X86_OFFSET(ShaderParamRecord, d68, 0x68);
TS3_X86_OFFSET(ShaderParamRecord, b70, 0x70);
TS3_X86_OFFSET(ShaderParamRecord, b74, 0x74);
TS3_X86_SIZE(ShaderParamRecord, 0x78);

}  // namespace ts3::shader
