// cls_86c000_Scripting — TargetSurfaceTypeMedatorModifier_8 and EventModifierQuality_2.
// Two SimData modifier classes; entry points are vtable slot 4 (Set...) and constructors.
// Dependencies NOT rewritten: call through ts3/layout.h bridge.

#ifndef TS3_SCRIPTING_CLS_86C000_SCRIPTING_H
#define TS3_SCRIPTING_CLS_86C000_SCRIPTING_H

#include <cstdint>

namespace ts3 {
namespace scripting {

// TargetSurfaceTypeMedatorModifier_8 — 16 bytes: +0 vtable 0x1029654, +4=0, +8=arg.
struct TargetSurfaceTypeMedatorModifier_8 {
    void*    vftable;   // +0
    uint32_t field4;    // +4 (always 0 after construct)
    int32_t  field8;    // +8  (constructor's a2, the "medator target type" id)
    uint32_t field12;   // +12 (uninitialised in original)

    // 0x0086C4E0 __thiscall(this, a2): +0=vftable, +4=0, +8=a2; ret this; ret 4.
    void* Construct86C4E0(int32_t a2);

    // 0x0086C000 vtable slot 4 (thiscall, 4 stack args, ret 0x10):
    // Sets a value on this via 0x0089F980/0x00939560/0x008693E0 and then appends
    // the "_" and Src strings into a1 (a char* dst buffer of max a2 bytes).
    int SetMedatorValue86C000(int a0, void* a1, uint32_t a2, int a3);
};

// EventModifierQuality_2 — 16 bytes: +0 vtable 0x1029690, +4=0, +8=arg.
struct EventModifierQuality_2 {
    void*    vftable;
    uint32_t field4;
    int32_t  field8;
    uint32_t field12;

    // 0x0086C6F0 __thiscall(this, a2): +0=vftable, +4=0, +8=a2; ret this; ret 4.
    void* Construct86C6F0(int32_t a2);

    // 0x0086C710 vtable slot 4 (thiscall, 4 stack args, ret 0x10):
    // Large quality-modifier dispatch (1047 bytes, EA string locals, vtable+0x134).
    // NOT rewritten: the source defines an AbortNotRewritten stub; the original
    // game code stays live (no hook). See notes.md / reimplemented.csv.
    int SetQualityModifier86C710(void* a2, void* a3, void* a4, void* a5);
};

// 0x0086C6C0 __thiscall(self): returns 0x11D9500 for type!=13&&type!=16;
// if (byte[self+8] & 0x30) returns *(void**)(self+16); else returns (self+16).
// Modelled as __fastcall(self, dummy) so the ECX register (this) layout matches
// the original __thiscall (a __thiscall target with no stack args is exactly
// __fastcall's first register slot). The test invokes both sides identically.
void* __fastcall GetMedatorValue86C6C0(void* self_, void* /*unused edx*/);

// 0x0089FB90 __thiscall(this, hash, val): looks up via 0x0089F7D0;
// if found and out_key==0 writes val to *(int*)found; returns 1 else 0.
// (Called from SetQualityModifier86C710 only; not an entry point.)
// __fastcall(this, dummy, hash, val) matches the original __thiscall register
// layout: ECX=this, hash/val on the stack.
char __fastcall SetValueByHash89FB90(void* container, void* /*unused edx*/,
                                    int hash, int val);

} // namespace scripting
} // namespace ts3

#endif // TS3_SCRIPTING_CLS_86C000_SCRIPTING_H