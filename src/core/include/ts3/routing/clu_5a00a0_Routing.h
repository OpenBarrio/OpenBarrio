// Cluster unit clu_5a00a0_Routing (subsystem Routing): every function registers or
// unregisters something with the one object the game keeps in the global at 0x11CE3F8.
// 0x005A00A0 is that global's getter (71 callers, a leaf); the other five fetch it,
// skip everything when it is null, and otherwise make a virtual call on it:
//   0x006E7B60 installs the Model LOD radii, Model LOD Adjust and Object Size Hide
//              Factor tuning entries (thiscall, plain ret),
//   0x00731ED0 installs three cache menu entries (cdecl, plain ret),
//   0x007377C0 hands this+0x14 to the object's slot 19 and continues at 0x00737060 with
//              this+0x18 as the new this (thiscall, tail jmp - so a plain ret),
//   0x00AE0380 one-shot initialiser for the "Lot Renderer" tweak (guarded by the byte at
//              0x11EB610, cdecl, returns AL=1),
//   0x00AE0400 its counterpart: clears the same byte and unregisters (cdecl, plain ret).
// Dependencies are all inside the unit (0x005A00A0) or bridged (the virtual slots and
// 0x00737060, which is not rewritten here).
#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::routing {

// What 0x006E7B60's `this` must provide. The function only ever takes this+0x2C and
// hands its ADDRESS to the tuning object (the game stores a float there - the static
// initialiser at 0x00F828E0 writes the same shape of cell at 0x11D1850 - but this unit
// never reads or writes it, so the field's type is not assumed here).
struct LodSettingsHost {
    uint8_t prefix[0x2C];
    uint8_t lodAdjustTarget[4];  // +0x2C, address only
};
TS3_X86_OFFSET(LodSettingsHost, lodAdjustTarget, 0x2C);

// What 0x007377C0's `this` must provide: +0x14 goes to the tuning object's slot 19,
// +0x18 becomes 0x00737060's `this`.
struct RoutingHandoffTarget {
    uint8_t prefix[0x14];
    uint8_t tuningTarget[4];  // +0x14, address only
    uint8_t child[4];         // +0x18, address only
};
TS3_X86_OFFSET(RoutingHandoffTarget, tuningTarget, 0x14);
TS3_X86_OFFSET(RoutingHandoffTarget, child, 0x18);

// 0x005A00A0 (ENTRY, leaf, 71 callers; cdecl, plain `ret`): the object every other
// function of this unit talks to, read from the game global at 0x11CE3F8 (null until
// the game has built it - every caller tests it).
void* GetTuningObject();

// 0x006E7B60 (ENTRY; thiscall, plain `ret`, 1 caller 0x006EB610): writes the five
// {owner, index} binding cells at 0x11D1814..0x11D184B (stride 12, this + 0..4), then
// makes nine virtual calls (slots 6, 17 x6, 12). Returns the last call's result, or 0
// when the object is null.
int32_t RegisterLodSettings(LodSettingsHost* self);

// 0x00731ED0 (ENTRY; cdecl, plain `ret`, 1 caller 0x00737060): three slot-18 calls with
// L"Shrink cache", L"Cache overflow size" and L"Cache size". Returns the last one's
// result, or 0 when the object is null.
int32_t RegisterCacheMenu();

// 0x007377C0 (ENTRY, vtable; thiscall, tail jmp to 0x00737060 so a plain `ret`): hands
// this+0x14 to slot 19 when the object exists, then continues at 0x00737060 (bridged -
// not part of this unit) with this+0x18 as `this`. The return value is that call's.
int32_t RoutingHandoff(RoutingHandoffTarget* self);

// 0x00AE0380 (ENTRY; cdecl, 2 callers 0x00AE6C20 / 0x00C7D430): when the byte at
// 0x11EB610 is 0, sets it to 1 and makes three virtual calls (slots 6, 7, 6) - the
// "Lot Renderer" budget, the tweak bound to 0x11EB650, and the (0, INT_MAX) range.
// EA quirk kept: the original writes only AL on the way out, so the returned dword's
// upper 24 bits are the last virtual call's result (0 when the object is null), and on
// the already-initialised early-out they are whatever the caller left in EAX - the
// rewrite returns 0 there, which is why the differential test only compares the low byte
// on that path. The low byte is always 1.
uint32_t InitLotRenderer();

// 0x00AE0400 (ENTRY; cdecl, plain `ret`, 1 caller 0x00C7D670): the counterpart of
// 0x00AE0380 - when 0x11EB610 is 1, clears it and unregisters 0x1068BC8 through slot 18.
// Returns the slot-18 result (0 when the object is null); on the no-op path the original
// leaves EAX completely untouched, so the rewrite returns 0 there and the test does not
// compare the return value on that path.
int32_t ReleaseLotRenderer();

}  // namespace ts3::routing
