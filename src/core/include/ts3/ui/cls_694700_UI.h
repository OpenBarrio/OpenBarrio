// Unit cls_694700_UI (the project notes, subsystem UI): the ghost-effect settings
// object and the parameter record it builds. Two TS3W.exe functions:
//
//   0x00694700  Construct694700  thiscall, plain ret, returns this: the constructor.
//                                Writes the vtable 0x00FF28B0 into +0x00 (the original
//                                first stores 0x00FEFF6C there and immediately overwrites
//                                it), atomically clears the dword at +0x04 (xchg, old
//                                value discarded), sets byte +0x08 to 1 and dword +0x0C
//                                to 0. Bytes +0x09..+0x0B (padding) are never touched.
//   0x00694960  Build694960      thiscall, ret 8, returns the record argument: fills a
//                                0x118-byte descriptor with the seven Ghost* parameter
//                                ids and pointers into the 96-byte float parameter block,
//                                min-clamps params[8] when byte +0x14 is set, and updates
//                                the shared vec4 at 0x11CF6B0 from the mode dword +0x10.
//
// The seven ids are string hashes of the game's 0x007CFE70 (a clu_5c87a0_Renderer entry,
// not rewritten yet) cached behind the init-flag byte at 0x11CF6DC (bits 0x01..0x40 one
// per id, bit 0x80 = vec4 zero-initialized). On x86 the unit runs against those live
// cells - the same memory the original uses, so both see identical state; the record's
// +0xE8 then holds the real 0x11CF6B0 address. On x64 there is no game: an internal
// static (flags pre-set to 0xFF, so the hash call is never reached) fails closed.
//
// EA quirks kept (they change outputs): the params[8] clamp is a bit-select on the sign
// bit of (a - 0.6f), NOT a float compare - for NaN inputs it keeps the original bits when
// the sign is clear and stores 0.6f when it is set; the id init flags are read as a byte
// but set with a plain non-atomic `or` dword; vec4[2]/vec4[3] persist between calls
// (only the bit-0x80 first call zeroes them); the gain table keys on
// (mode & 0xCF00) in {0x300,0x400,0x500,0x700} plus the unsigned compare
// (mode & 0x7F) >= 0x20. All four float constants are read from their live cells
// (0xF9D500 0.6f, 0x107A538 1.0f, 0x1045CCC 10.0f, 0xFB2F68 6.0f, 0x1074010 3.0f).
#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::ui {

// Defined below (the constructor's methods reference it as an opaque pointer).
struct GhostParamRecord;

// The object both entries operate on (vtable 0x00FF28B0). Only the fields this unit
// touches are named; the game's class is larger.
struct GhostParams {
    void* vtable_;        // +0x00 set to vt 0x00FF28B0 by Construct694700
    uint32_t field04;     // +0x04 atomically cleared by Construct694700
    uint8_t field08;      // +0x08 set to 1 by Construct694700
    uint8_t pad09[3];     // +0x09..+0x0B never touched
    uint32_t field0C;     // +0x0C set to 0 by Construct694700
    uint32_t mode;        // +0x10 read by Build694960 (gain/vec4 selector)
    uint8_t clampParams;  // +0x14 nonzero: Build694960 clamps params[8] to >= 0.6f
    // +0x15.. rest of the game's class, unknown and untouched by this unit

    // 0x00694700, __thiscall, plain ret. Constructor: sets the fields above, returns this.
    GhostParams* Construct694700();
    // 0x00694960, __thiscall, ret 8. Fills `out`, updates the shared statics, min-clamps
    // params[8] (params is 24 floats / 96 bytes; the game passes its global parameter
    // blob + 96*index). Returns `out`.
    GhostParamRecord* Build694960(GhostParamRecord* out, float* params);
};

// The 0x118-byte record 0x00694960 fills. Grouped like the original's writes (one
// descriptor per 0x28-byte block at +0x20/+0x48/+0x70/+0x98/+0xC0/+0xE8); every byte not
// listed as a named field is left exactly as the caller had it - the original never
// touches it and neither does the rewrite.
struct GhostParamRecord {
    uint32_t count00;         // +0x00 = 1
    uint32_t idColor1;        // +0x04 hash("GhostColor1")
    uint8_t pad08[0x18];      // +0x08..+0x1F untouched
    float* params0;           // +0x20 = params + 0
    uint32_t n24;             // +0x24 = 3
    uint32_t flag28;          // +0x28 = 1
    uint32_t idColor2;        // +0x2C hash("GhostColor2")
    uint8_t pad30[0x18];      // +0x30..+0x47 untouched
    float* params4;           // +0x48 = params + 4 floats
    uint32_t n4C;             // +0x4C = 3
    uint32_t flag50;          // +0x50 = 1
    uint32_t idFilterParams;  // +0x54 hash("GhostFilterParams")
    uint8_t pad58[0x18];      // +0x58..+0x6F untouched
    float* params8;           // +0x70 = params + 8 floats
    uint32_t n74;             // +0x74 = 4
    uint32_t flag78;          // +0x78 = 1
    uint32_t idNoiseParams;   // +0x7C hash("GhostNoiseParams")
    uint8_t pad80[0x18];      // +0x80..+0x97 untouched
    float* params12;          // +0x98 = params + 12 floats
    uint32_t n9C;             // +0x9C = 4
    uint32_t nA0;             // +0xA0 = 4
    uint32_t idNoiseTexture;  // +0xA4 hash("GhostNoiseTexture")
    uint8_t padA8[8];         // +0xA8..+0xAF untouched
    uint32_t noiseTex[4];     // +0xB0..+0xBC raw dwords of params[20..23]
    uint8_t padC0[8];         // +0xC0..+0xC7 untouched
    uint32_t flagC8;          // +0xC8 = 1
    uint32_t idHeightParams;  // +0xCC hash("GhostHeightParams")
    uint8_t padD0[0x18];      // +0xD0..+0xE7 untouched
    void* sharedVec4;         // +0xE8 = &the vec4 static (0x11CF6B0 on x86)
    uint32_t nEC;             // +0xEC = 4
    uint32_t flagF0;          // +0xF0 = 1
    uint32_t idFilterColor;   // +0xF4 hash("GhostFilterColor")
    uint8_t padF8[0x18];      // +0xF8..+0x10F untouched
    float* params16;          // +0x110 = params + 16 floats
    uint32_t n114;            // +0x114 = 3
};
TS3_X86_OFFSET(GhostParamRecord, count00, 0x00);
TS3_X86_OFFSET(GhostParamRecord, idColor1, 0x04);
TS3_X86_OFFSET(GhostParamRecord, params0, 0x20);
TS3_X86_OFFSET(GhostParamRecord, n24, 0x24);
TS3_X86_OFFSET(GhostParamRecord, flag28, 0x28);
TS3_X86_OFFSET(GhostParamRecord, idColor2, 0x2C);
TS3_X86_OFFSET(GhostParamRecord, params4, 0x48);
TS3_X86_OFFSET(GhostParamRecord, n4C, 0x4C);
TS3_X86_OFFSET(GhostParamRecord, flag50, 0x50);
TS3_X86_OFFSET(GhostParamRecord, idFilterParams, 0x54);
TS3_X86_OFFSET(GhostParamRecord, params8, 0x70);
TS3_X86_OFFSET(GhostParamRecord, n74, 0x74);
TS3_X86_OFFSET(GhostParamRecord, flag78, 0x78);
TS3_X86_OFFSET(GhostParamRecord, idNoiseParams, 0x7C);
TS3_X86_OFFSET(GhostParamRecord, params12, 0x98);
TS3_X86_OFFSET(GhostParamRecord, n9C, 0x9C);
TS3_X86_OFFSET(GhostParamRecord, nA0, 0xA0);
TS3_X86_OFFSET(GhostParamRecord, idNoiseTexture, 0xA4);
TS3_X86_OFFSET(GhostParamRecord, noiseTex, 0xB0);
TS3_X86_OFFSET(GhostParamRecord, flagC8, 0xC8);
TS3_X86_OFFSET(GhostParamRecord, idHeightParams, 0xCC);
TS3_X86_OFFSET(GhostParamRecord, sharedVec4, 0xE8);
TS3_X86_OFFSET(GhostParamRecord, nEC, 0xEC);
TS3_X86_OFFSET(GhostParamRecord, flagF0, 0xF0);
TS3_X86_OFFSET(GhostParamRecord, idFilterColor, 0xF4);
TS3_X86_OFFSET(GhostParamRecord, params16, 0x110);
TS3_X86_OFFSET(GhostParamRecord, n114, 0x114);
TS3_X86_SIZE(GhostParamRecord, 0x118);

// The shared statics block at 0x11CF6B0 of the x86 game image: the vec4, the seven cached
// ids (one dword each) and the init-flag byte in the low byte of the last dword. The
// whole struct IS that memory on x86; on x64 the unit owns a private copy instead.
struct GhostStatics {
    alignas(16) float vec4[4];   // +0x00 @ 0x11CF6B0
    uint32_t idFilterColor;      // +0x10 @ 0x11CF6C0
    uint32_t idHeightParams;     // +0x14 @ 0x11CF6C4
    uint32_t idNoiseTexture;     // +0x18 @ 0x11CF6C8
    uint32_t idNoiseParams;      // +0x1C @ 0x11CF6CC
    uint32_t idFilterParams;     // +0x20 @ 0x11CF6D0
    uint32_t idColor2;           // +0x24 @ 0x11CF6D4
    uint32_t idColor1;           // +0x28 @ 0x11CF6D8
    uint32_t flags;              // +0x2C @ 0x11CF6DC (low byte used: 0x7F | 0x80)
};
TS3_X86_OFFSET(GhostStatics, vec4, 0x00);
TS3_X86_OFFSET(GhostStatics, idFilterColor, 0x10);
TS3_X86_OFFSET(GhostStatics, idHeightParams, 0x14);
TS3_X86_OFFSET(GhostStatics, idNoiseTexture, 0x18);
TS3_X86_OFFSET(GhostStatics, idNoiseParams, 0x1C);
TS3_X86_OFFSET(GhostStatics, idFilterParams, 0x20);
TS3_X86_OFFSET(GhostStatics, idColor2, 0x24);
TS3_X86_OFFSET(GhostStatics, idColor1, 0x28);
TS3_X86_OFFSET(GhostStatics, flags, 0x2C);
TS3_X86_SIZE(GhostStatics, 0x30);

// The x86 address of that block (the record's +0xE8 holds it; exposed for the tests).
constexpr uintptr_t kGhostStaticsAddress = 0x011CF6B0;

}  // namespace ts3::ui
