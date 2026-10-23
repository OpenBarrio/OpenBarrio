// CAS resource objects behind TS3W.exe 0x005F7540..0x005FB510 (unit cls_5f7540_CAS).
//
// Four sibling classes share one 12-byte reference-counted header (vtable 0x00FE6058 while
// only the base is constructed; slots 0/1 are AddRef/Release over the u16 at +0x08):
//   A  vtable 0x00FE6278, 0x30 bytes, pool tag "CAS/TextureCompositorResource/SkinToneOp"
//      - constructors 0x005FA830, destructor 0x005FA9B0
//   B  vtable 0x00FE62D0, 0x50 bytes (a base subobject of the 0x00FE3588 class: its next
//      member sits at +0x50, see 0x005CA300)
//      - constructors 0x005FAA20 / 0x005FAC60, destructor 0x005FAEB0
//   C  vtable 0x00FE6390, 0x68 bytes, pool tag "CAS/TextureCompositorResource/ClearOp"
//      (PoolAlloc(0x68, 0x00FE40E0) at 0x005DCAFC)
//      - constructor 0x005FAFB0
//   D  vtable 0x00FE63E8 (the CAS texture-builder class of unit cls_5f7ab0_x; only its
//      +0x63 byte and vtable slot 20 are used here)
//      - methods 0x005F7540, 0x005F8170, 0x005FB510 (the latter is vtable slot 25)
// Dependencies still in the game (reached through ts3/layout.h): the texture-op manager
// singleton at 0x011D3C84 (vtable slots 13/15), the named-service registry at 0x011D753C
// (0x0079B840 / 0x0079A2D0), the color packer 0x0060CD10 (unit clu_5c9890_CAS), and the
// deletor thunk 0x00D4B150, whose address is passed through as an opaque callback.
#pragma once

#include "ts3/layout.h"

namespace ts3::cas {

// The shared header of all four classes. A freshly PoolAlloc'd (0x10 bytes) base object
// (0x005F7E20) leaves +0x0C..+0x0F for the deriving constructor to fill.
struct CasResourceHeader {
    // TS3W.exe 0x005F7E20, thiscall ret 0: the base constructor (also re-inits pooled
    // objects). Zeroes +0x04/+0x08/+0x0A, folds +0x0B to (old & 0xFA) | 2 and restores the
    // base vtable. Returns `this`.
    CasResourceHeader* Construct();

    void* vtable;        // +0x00
    uint32_t field04;    // +0x04
    uint16_t refCount;   // +0x08 (AddRef/Release, 0x005CA3B0/0x005CA3C0)
    uint8_t flags0A;     // +0x0A: bit 5 = visible(?), low nibble = a substate (see ctors)
    uint8_t state0B;     // +0x0B: every constructor sets (old & 0xFA) | 2
};
TS3_X86_OFFSET(CasResourceHeader, state0B, 0x0B);

// Class A. Every constructor writes +0x00..+0x2E and leaves +0x2F untouched.
struct CasSkinToneOp {
    // TS3W.exe 0x005FA830, thiscall ret 0x24 (9 stack args). Returns `this`.
    CasSkinToneOp* Construct(const float* rgba, uint32_t param, uint8_t flagA, uint8_t flagB,
                             uint8_t flagC, uint8_t flagD, float scale255, const float* packColor,
                             const uint32_t* sourceKeys);
    // TS3W.exe 0x005FA9B0, thiscall ret 0. Restores the base vtable and releases the
    // handle at +0x24 (unregisters it with the manager first). Returns the game's eax:
    // 0 when the handle was null, the Release call's return, or (tagged handles) the
    // manager call's return.
    uint32_t Destruct();

    CasResourceHeader header;  // +0x00
    uint32_t key0C;            // +0x0C = sourceKeys[0]
    uint32_t key10;            // +0x10 = sourceKeys[1]
    uint32_t key14;            // +0x14 = sourceKeys[3] (EA: [2] is skipped)
    uint32_t param18;          // +0x18
    int16_t color1C[4];        // +0x1C..+0x23 = (int16)(int)(rgba[i] * 255)
    void* handle24;            // +0x24 resource handle released by Destruct
    uint32_t packedColor28;    // +0x28 dword packed by 0x0060CD10 from packColor
    uint16_t param2C;          // +0x2C = (uint16)(int)(scale255 * 255)
    uint8_t flags2E;           // +0x2E = (flagC << 4) | (flagB & 0xF)
    uint8_t pad2F;             // +0x2F never written
};
TS3_X86_OFFSET(CasSkinToneOp, key0C, 0x0C);
TS3_X86_OFFSET(CasSkinToneOp, handle24, 0x24);
TS3_X86_OFFSET(CasSkinToneOp, packedColor28, 0x28);
TS3_X86_OFFSET(CasSkinToneOp, flags2E, 0x2E);
TS3_X86_SIZE(CasSkinToneOp, 0x30);

// Class B: one CAS layer (two 0..255 colours, resource keys, two manager-tracked handles
// and one plain object slot). 0x50 bytes; nothing beyond +0x4F is touched.
struct CasLayer {
    // TS3W.exe 0x005FAA20, thiscall ret 0x38 (14 stack args). The three refs come from
    // refs[0], refs[1], refs[3]. Returns `this`.
    CasLayer* ConstructFromRefs(const uint32_t* refs, const float* colorA, const float* colorB,
                                float weight0, uint8_t flag0, uint8_t flag1, uint8_t flag2,
                                uint8_t flag3, uint8_t flag4, float weight1, float weight2,
                                const float* packColor, const uint32_t* keys, uint8_t id4F);
    // TS3W.exe 0x005FAC60, thiscall ret 0x38 (14 stack args). Same layout, but +0x28..+0x3C
    // are zeroed and the id lands at +0x40. Returns `this`.
    CasLayer* ConstructWithId(uint32_t id, const float* colorA, const float* colorB, float weight0,
                              uint8_t flag0, uint8_t flag1, uint8_t flag2, uint8_t flag3,
                              uint8_t flag4, float weight1, float weight2, const float* packColor,
                              const uint32_t* keys, uint8_t id4F);
    // TS3W.exe 0x005FAEB0, thiscall ret 0. Releases +0x34 (manager unregister FIRST - EA
    // quirk - then vtable slot 1), +0x3C (vtable slot 2), +0x38 (manager + slot 1), then
    // retries +0x34 - the second pass re-unregisters a still-set (tagged) +0x34 with the
    // manager. Returns the final pass's eax (0 after a normal release).
    uint32_t Destruct();

    CasResourceHeader header;  // +0x00
    int16_t color0C[4];        // +0x0C..+0x13 = (int16)(int)(colorA[i] * 255)
    int16_t color14[4];        // +0x14..+0x1B = (int16)(int)(colorB[i] * 255)
    uint32_t keys1C[3];        // +0x1C = keys[0], +0x20 = keys[1], +0x24 = keys[3]
    uint32_t ref28;            // +0x28 refs[0] (refs flavour) / 0 (id flavour)
    uint32_t ref2C;            // +0x2C refs[1]                          / 0
    uint32_t ref30;            // +0x30 refs[3]                          / 0
    void* handle34;            // +0x34 handle, manager-unregistered + vtable slot 1
    void* handle38;            // +0x38 handle, manager-unregistered + vtable slot 1
    void* object3C;            // +0x3C object released through vtable slot 2
    uint32_t id40;             // +0x40 (id flavour) / 0
    uint32_t packedColor44;    // +0x44 dword packed by 0x0060CD10 from packColor
    uint16_t weight48;         // +0x48 = (uint16)(int)(weight2 * 255)
    uint16_t weight4A;         // +0x4A = (uint16)(int)(weight0 * 255)
    uint16_t weight4C;         // +0x4C = (uint16)(int)(weight1 * 255)
    uint8_t flags4E;           // +0x4E = (flag2 << 4) | (flag1 & 0xF)
    uint8_t id4F;              // +0x4F
};
TS3_X86_OFFSET(CasLayer, color0C, 0x0C);
TS3_X86_OFFSET(CasLayer, keys1C, 0x1C);
TS3_X86_OFFSET(CasLayer, handle34, 0x34);
TS3_X86_OFFSET(CasLayer, object3C, 0x3C);
TS3_X86_OFFSET(CasLayer, packedColor44, 0x44);
TS3_X86_OFFSET(CasLayer, flags4E, 0x4E);
TS3_X86_SIZE(CasLayer, 0x50);

// Class C: one CAS texture-compositor clear op. 0x68 bytes; +0x67 is never written.
struct CasClearOp {
    // TS3W.exe 0x005FAFB0, thiscall ret 0x30 (12 stack args). `weight` is stored bit-exact
    // as a float at +0x5C (the game loads the raw dword). When keys[2] != 0 and the
    // manager exists, asks it (vtable slot 13) to fill +0x58. Returns `this`.
    CasClearOp* Construct(const uint32_t* keysA, const uint32_t* keysB, const uint32_t* keysC,
                          const uint32_t* keysD, float weight, uint8_t flag0, uint8_t flag1,
                          uint8_t flag2, uint8_t flag3, float scale255, const float* packColor,
                          const uint32_t* keys);

    CasResourceHeader header;  // +0x00
    uint32_t key0C[15];        // +0x0C..+0x44: keysA/keysB/keysC/keysD/keys, 3 dwords each
                               // ([0], [1], [3])
    uint32_t reserved48[4];    // +0x48..+0x54, zeroed
    uint32_t managerOut58;     // +0x58, zeroed; filled by the manager request
    float weight5C;            // +0x5C, bit-exact copy of `weight`
    uint32_t packedColor60;    // +0x60 dword packed by 0x0060CD10 from packColor
    uint16_t param64;          // +0x64 = (uint16)(int)(scale255 * 255)
    uint8_t flags66;           // +0x66 = (flag2 << 4) | (flag1 & 0xF)
    uint8_t pad67;             // +0x67 never written
};
TS3_X86_OFFSET(CasClearOp, key0C, 0x0C);
TS3_X86_OFFSET(CasClearOp, managerOut58, 0x58);
TS3_X86_OFFSET(CasClearOp, weight5C, 0x5C);
TS3_X86_OFFSET(CasClearOp, packedColor60, 0x60);
TS3_X86_OFFSET(CasClearOp, flags66, 0x66);
TS3_X86_SIZE(CasClearOp, 0x68);

// Class D (owned by unit cls_5f7ab0_x): only the header shape and the byte this unit's
// method writes are modelled here.
struct CasTextureBuilder {
    // TS3W.exe 0x005F7540, thiscall ret 0. Looks up the "tbops" service in the registry at
    // 0x011D753C; when found, feeds vtable slot 20's value to 0x0079A2D0 and returns the
    // service. Returns null when the registry has no such service.
    void* FetchTextureOpService();
    // TS3W.exe 0x005F8170, thiscall ret 0x10 (4 stack args). Asks the manager (vtable
    // slot 13) to queue op 0x03420F34; returns the game's answer reduced to 0/1 by its
    // low byte (the original instruction sequence), and 0 when there is no manager.
    uint32_t RegisterTextureOps(uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5);
    // TS3W.exe 0x005FB510, thiscall ret 4 (vtable 0x00FE63E8 slot 25). Replaces the low
    // nibble of the byte at +0x63 and clamps that nibble to 3. Returns the byte's new
    // full value.
    uint8_t SetPatternLevel(uint8_t level);

    void* vtable;           // +0x00 (0x00FE63E8 when constructed)
    uint8_t body04[4];      // +0x04
    uint16_t refCount;      // +0x08
    uint8_t flags0A;        // +0x0A
    uint8_t state0B;        // +0x0B
    uint8_t body0C[0x57];   // +0x0C..+0x62
    uint8_t patternByte;    // +0x63
};
TS3_X86_OFFSET(CasTextureBuilder, patternByte, 0x63);

}  // namespace ts3::cas
