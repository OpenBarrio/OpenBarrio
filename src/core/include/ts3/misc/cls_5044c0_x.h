// src/core/include/ts3/misc/cls_5044c0_x.h - declared here.
//
// Unit cls_5044c0_x (TS3W.exe, subsystem misc/): 40 small, unrelated entry points that the
// game reaches through vtables, message handlers and adjustor thunks.  Every object layout
// below follows the accesses in the original disassembly; the original ABI is
// __thiscall with the receiver in ECX, so each entry point is a member function.  Members
// whose `this` is *not* the object base (adjustor thunks: the original instruction sequence) say so in
// their comment; the body subtracts N and forwards to the target's method.
//
// Methods that only call unrewritten game code carry it through ts3::CallGameMethod /
// CallGameFunction / CallGameVirtual (ts3/layout.h), never a local reimplementation.
#pragma once

#include <cstddef>
#include <cstdint>

#include "ts3/layout.h"

namespace ts3 {
namespace misc {

// ---------------------------------------------------------------------------------------
// 0x005044C0: intrusive refcount at +4 (ResourceBase sub-object).
struct ResourceBaseSec {
    void* f_00;    // +0x00
    int32_t f_04;  // +0x04: refcount, `lock xadd` +1 returns the new value
    int32_t AddRef();
    // 0x0078E8D0: adjustor thunk (the original instruction sequence); `this` points 8 bytes past
    // the object base, so the counter at base+4 is at `this-4`.
    int32_t AddRefAdjust8();
};
TS3_X86_OFFSET(ResourceBaseSec, f_04, 0x4);
TS3_X86_SIZE(ResourceBaseSec, 0x8);

// ---------------------------------------------------------------------------------------
// 0x00570080..0x00570850: ProceduralMotionAccumulationController message/refcount surface.
// The constructor (0x00570120, not in this unit) installs vtables 0x00FC21E0/+8 0x00FC21D4/
// +0xC 0x00FC21D0/+0x28 0x00FC21BC; +0x2C is a separate refcounted object.
struct ProceduralMotionController {
    void* f_00;             // +0x00: vtable
    int32_t f_04;           // +0x04: refcount
    void* f_08;             // +0x08: base vtable
    void* f_0C;             // +0x0C: base vtable
    uint8_t pad_10[0xC];    // +0x10
    void* f_1C;             // +0x1C: queried object
    uint8_t pad_20[0x8];    // +0x20
    void* f_28;             // +0x28: embedded sub-object head (vtable 0x00FC21BC)
    void* f_2C;             // +0x2C: refcounted object with a vtable
    alignas(16) float f_30[4];  // +0x30: rotation quaternion (movaps)
    float f_40;             // +0x40: remaining turn angle
    uint8_t pad_44[0xC];    // +0x44

    // 0x00570080 (ret 4): message handler.  Passes a0 and `this` to the game's motion
    // helpers; EAX keeps the last helper's whole result with AL replaced by a0[0x14].
    int32_t HandleMessage570080(void* a0);
    // 0x005700E0: returns the class id 0x0BA185DF (a full the original instruction sequence).
    int32_t ClassId5700E0();
    // 0x005700F0 (ret 4): the two accepted ids map to `this-8`, anything else to null.
    void* QueryInterface5700F0(int32_t id);
    // 0x00570110: adjustor thunk (the original instruction sequence); `this` points 0xC past
    // the object.
    void* DestroyingDtorAdjust570110(uint32_t flags);
    // 0x00570200 (ret 4): SSE turn-delta update through three game vtables.
    int32_t HandleMessage570200(float arg);
    // 0x005704B0 (ret 4): message handler gated on the game's 4DD700 result (<= 1).
    int32_t HandleMessage5704B0(void* a0);
    // 0x00570850 (ret 4, vtable slot 0): vector deleting destructor; returns `this`.
    void* DestroyingDtor570850(uint32_t flags);
};
TS3_X86_OFFSET(ProceduralMotionController, f_04, 0x4);
TS3_X86_OFFSET(ProceduralMotionController, f_1C, 0x1C);
TS3_X86_OFFSET(ProceduralMotionController, f_28, 0x28);
TS3_X86_OFFSET(ProceduralMotionController, f_2C, 0x2C);
TS3_X86_OFFSET(ProceduralMotionController, f_30, 0x30);
TS3_X86_OFFSET(ProceduralMotionController, f_40, 0x40);
TS3_X86_SIZE(ProceduralMotionController, 0x50);

// ---------------------------------------------------------------------------------------
// 0x00581EA0/0x00581FC0: SceneEntity destructor/handler pair.  The handlers 0x00581EA0 and
// the adjustor thunk 0x005818D0 run with `this` = base+8, the second vtable sub-object
// (vtable 0x00FCB7DC), so they are members of SceneEntitySub; the destructor 0x00581FC0
// runs on the base.
struct SceneEntitySub {
    void* f_00;     // +0x00 (entity +0x08): vtable 0x00FCB7DC
    uint32_t f_04;  // +0x04 (entity +0x0C): bit 1 gates the 0x005815F0 helper
    uint32_t f_08;  // +0x08 (entity +0x10): "holds a buffer?" qword
    uint32_t f_0C;  // +0x0C (entity +0x14)
    void* f_10;     // +0x10 (entity +0x18): freed by the base destructor

    // 0x00581EA0 (ret 8): id 0x7829873A clears the +0x10 qword of the entity.  The id
    // mismatch path's the original instruction sequence leaves the upper EAX bytes to the caller.
    int32_t HandleMessage581EA0(int32_t id, int32_t unused);
    // 0x005818D0 (ret 4): adjustor thunk (the original instruction sequence); `this` = base+8.
    void* DestroyingDtorAdjust5818D0(uint32_t flags);
};
TS3_X86_OFFSET(SceneEntitySub, f_04, 0x4);
TS3_X86_OFFSET(SceneEntitySub, f_10, 0x10);
TS3_X86_SIZE(SceneEntitySub, 0x14);

struct SceneEntity {
    void* f_00;            // +0x00: vtable 0x00FCB7EC
    void* f_04;            // +0x04
    SceneEntitySub sub;    // +0x08

    // 0x00581FC0 (ret 4, vtable slot 0): vector deleting destructor; returns `this`.
    void* DestroyingDtor581FC0(uint32_t flags);
};
TS3_X86_OFFSET(SceneEntity, sub, 0x8);
TS3_X86_SIZE(SceneEntity, 0x1C);

// ---------------------------------------------------------------------------------------
// 0x005A7120: std::fpos<int>::seekpos, the only non-class entry point of the unit.
struct StdFpos {
    uint32_t f_00;  // +0x00
    uint32_t f_04;  // +0x04
    int64_t f_08;   // +0x08: returned whole in EDX:EAX
    int64_t SeekPos();
};
TS3_X86_OFFSET(StdFpos, f_08, 0x8);
TS3_X86_SIZE(StdFpos, 0x10);

// ---------------------------------------------------------------------------------------
// 0x005A71D0 (vtable 0x00FDE6B0): releases the object at +0x18 through its vtable slot +4.
struct ManagedOwner5A71D0 {
    void* f_00;          // +0x00: vtable 0x00FDE6B0
    uint8_t pad_04[0x14];
    void* f_18;          // +0x18: optional object with a vtable; slot +4 is called
    void* DestroyingDtor(uint32_t flags);
};
TS3_X86_OFFSET(ManagedOwner5A71D0, f_18, 0x18);

// ---------------------------------------------------------------------------------------
// 0x005A8E00 (vtables 0x00FDE70C/+4, 0x00FDE6FC/+8) and its adjustor thunks 0x005A8DE0
// (the original instruction sequence) / 0x005A8DF0 (the original instruction sequence).
struct TripleVtable5A8E00 {
    void* f_00;        // +0x00: vtable 0x00FDE70C
    void* f_04;        // +0x04: vtable 0x00FDE6FC
    void* f_08;        // +0x08
    void* DestroyingDtor(uint32_t flags);
    void* DestroyingDtorAdjust4(uint32_t flags);
    void* DestroyingDtorAdjust8(uint32_t flags);
};
TS3_X86_OFFSET(TripleVtable5A8E00, f_08, 0x8);

// ---------------------------------------------------------------------------------------
// 0x005A9BE0 (vtable 0x00FDE830): destructor that releases a sub-object first.
struct Object5A9BE0 {
    uint8_t pad_00[4];
    void* DestroyingDtor(uint32_t flags);
};

// ---------------------------------------------------------------------------------------
// 0x005AD500 (vtable 0x00FDE710): id-gated cleanup of the pointer at +0xC.
struct CasBuilder5AD500 {
    void* f_00;         // +0x00: vtable 0x00FDE710
    uint8_t pad_04[0x8];
    void* f_0C;         // +0x0C: freed by 0x004E3F70 after 0x005E44C0
    int32_t HandleMessage5AD500(int32_t id, int32_t unused);
};
TS3_X86_OFFSET(CasBuilder5AD500, f_0C, 0xC);

// ---------------------------------------------------------------------------------------
// 0x005ADD20 (vtable 0x00FDE69C), 0x005FE250 (vtable 0x00FE659C) and 0x00743850
// (vtable 0x00FDE658): minimal vector deleting destructors; all return `this`.
struct SimpleDtor5ADD20 {
    void* f_00;
    void* DestroyingDtor(uint32_t flags);
};
struct SimpleDtor5FE250 {
    void* f_00;
    void* DestroyingDtor(uint32_t flags);
};
struct SimpleDtor743850 {
    void* f_00;
    void* DestroyingDtor(uint32_t flags);
};

// ---------------------------------------------------------------------------------------
// 0x00616C20..0x00616F70: alpha-texture sub-object (vtable 0x00FE8318 at +0, a second
// base vtable 0x00FE8328 at +8 deleted by 0x00616F70).
struct AlphaTexturePartial {
    void* f_00;           // +0x00: vtable 0x00FE8318
    uint32_t pad_04;      // +0x04
    void* f_08;           // +0x08: second base vtable 0x00FE8328
    uint32_t f_0C;        // +0x0C: zero forwards 0x00616EB0 to 0x00616C40
    uint32_t f_10;        // +0x10: shifted by 0x00616C30
    uint8_t f_14;         // +0x14: bit 0 gates the 0x00616D30 call
    uint8_t pad_15[3];    // +0x15
    int32_t f_18;         // +0x18: shift base
    uint8_t pad_1C[8];    // +0x1C

    // 0x00616C20: vtable slot +0xC result plus 36.
    int32_t GetOffset616C20();
    // 0x00616C30: f_10 << ((byte)f_18 + 1) with the x86 CL mask.
    int32_t ComputeShift616C30();
    // 0x00616EB0: 0x00616C40(this-8) when [this+0xC] is zero, else 1 (EAX residue).
    int32_t Poll616EB0();
    // 0x00616ED0: 0x00616D30(this-8) when byte [this+0x14] has bit 0, else nothing.
    int32_t Notify616ED0();
    // 0x00616F20 (ret 4): adjustor thunk (the original instruction sequence).
    void* DestroyingDtorAdjust616F20(uint32_t flags);
    // 0x00616F70 (ret 4): vector deleting destructor; returns `this`.
    void* DestroyingDtor616F70(uint32_t flags);
};
TS3_X86_OFFSET(AlphaTexturePartial, f_08, 0x8);
TS3_X86_OFFSET(AlphaTexturePartial, f_0C, 0xC);
TS3_X86_OFFSET(AlphaTexturePartial, f_10, 0x10);
TS3_X86_OFFSET(AlphaTexturePartial, f_14, 0x14);
TS3_X86_OFFSET(AlphaTexturePartial, f_18, 0x18);
TS3_X86_SIZE(AlphaTexturePartial, 0x24);

// ---------------------------------------------------------------------------------------
// 0x00617AF0 (vtable 0x00FE9454): constant getter.  0x00617CE0: adjustor thunk
// (the original instruction sequence), `this` = base+8.
struct VideoDownsampleMaterial {
    void* f_00;
    int32_t NumErrors617AF0();
    int32_t Process618020Adjust8();
};

// ---------------------------------------------------------------------------------------
// 0x00B742C0 (vtable 0x00FBA378): constant getter, always 2.
struct GDEBufferView {
    uint8_t pad_00[4];
    int32_t KindB742C0();
};

// ---------------------------------------------------------------------------------------
// 0x00C0FD60: empty 1-byte script-bridge stub (`ret`).
struct TextureResourceStub {
    void ScubaDivingModeStub();
};

// ---------------------------------------------------------------------------------------
// 0x008ABE50/0x008ABE60 (vtable sub-object at +8, refcount at +0xC) and the adjustor
// thunks 0x008711F0 (the original instruction sequence -> Release), 0x009C0440 (the original instruction sequence -> Release),
// 0x009C0450 (the original instruction sequence -> AddRef), 0x00EC2350 (the original instruction sequence -> AddRef) plus 0x009C0440's
// 0x009C0450-style siblings; see the comments.
struct RefCounted08 {
    uint8_t pad_00[8];  // +0x00
    void* f_08;         // +0x08: vtable; slot 0 is the deleting destructor
    int32_t f_0C;       // +0x0C: refcount
    int32_t AddRef();
    int32_t Release();
    int32_t ReleaseAdjust4();
    int32_t ReleaseAdjust8();
    int32_t AddRefAdjust4();
    int32_t AddRefAdjust8();
};
TS3_X86_OFFSET(RefCounted08, f_08, 0x8);
TS3_X86_OFFSET(RefCounted08, f_0C, 0xC);
TS3_X86_SIZE(RefCounted08, 0x10);

// 0x00EC17A0: same release protocol with the vtable at +0 and the refcount at +4, plus the
// adjustor thunk 0x0078E8E0 (the original instruction sequence).
struct RefCounted00 {
    void* f_00;    // +0x00: vtable; slot 0 is the deleting destructor
    int32_t f_04;  // +0x04: refcount
    int32_t Release();
    int32_t ReleaseAdjust8();
};
TS3_X86_OFFSET(RefCounted00, f_04, 0x4);
TS3_X86_SIZE(RefCounted00, 0x8);

}  // namespace misc
}  // namespace ts3