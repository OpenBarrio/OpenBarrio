// RouteManager unit cls_857220_RouteManager (TS3W.exe 0x00857220..0x00953710): the
// vehicle-body route/locomotion manager object plus the axis-angle quaternion helper its
// route code uses. Layout evidence in the header. Dependencies still in the game (reached
// through ts3/layout.h): the vehicle-body service setup 0x0085A100 and the route finalizer
// 0x00859890 (unit cls_839750_x), the route-object teardown 0x00856F10 (unit
// clu_83ba40_Scratch) and operator delete 0x004E3F70 (unit clu_4e3f70_Scratch).
//
// The object (at least 0x130 bytes) has three vtable subobjects, all written by the
// constructor and restored by the destructor:
//   +0x00 vtable 0x01027690 (the class: slot 0 returns the type id 0x0F612D91, slot 1 is a
//        QueryInterface against 0x0754F112, slot 8 is 0x008572E0)
//   +0x04 vtable 0x0102768C (one slot: deleting-dtor thunk 0x008572C0, the original instruction sequence)
//   +0x18 vtable 0x0102767C (4 slots; the first two are thunks the original instruction sequence into the
//        class's id/QI slots, then 0x00859770 and 0x00859840)
#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::routemanager {

// The refcounted child the manager keeps at +0xD0: vtable at +0x00, refcount at +0x04.
// Destruct (0x008583D0) decrements and, on hitting 0, resets the count to 1 and calls
// vtable slot 0 with the argument 1 (the game's release convention).
struct RouteManagerChild {
    void* vtable;      // +0x00
    uint32_t refcount; // +0x04
};
TS3_X86_OFFSET(RouteManagerChild, refcount, 0x04);

struct RouteManagerObject {
    // TS3W.exe 0x00857220, thiscall ret 0x8. Forwards (paramA, paramB) to the vehicle-body
    // service setup 0x0085A100 (its return is discarded), then writes the three vtables and
    // zeroes +0xD0/+0x128/+0x12C. Returns `this`.
    RouteManagerObject* Construct(uint32_t paramA, uint32_t paramB);
    // TS3W.exe 0x008583D0, thiscall ret 0x4. Restores the three vtables, runs the teardown
    // 0x00856F10, releases the +0xD0 child (refcount 1 -> 0 calls its vtable slot 0 with 1),
    // runs 0x00859890, and frees `this` through operator delete 0x004E3F70 when flags & 1.
    // Returns `this`.
    RouteManagerObject* Destruct(uint32_t flags);
    // TS3W.exe 0x008591F0, thiscall ret 0. When +0xC8 is set, asks the object at +0x14
    // (vtable slot 30) to drop it with argument 0 and clears +0xC8. Returns the game's eax:
    // 0 when the handle was null, otherwise the slot-30 call's return.
    uint32_t ReleaseHandle();
    // TS3W.exe 0x00953710, thiscall ret 0x8. Stores the byte at +0x106 and the float at
    // +0x108. Returns the byte (the game leaves it in AX).
    uint16_t SetWordFloat(uint8_t value, float speed);

    void* vtableMain;      // +0x00 (0x01027690)
    void* vtableSub04;     // +0x04 (0x0102768C)
    uint8_t body08[0x0C];  // +0x08..+0x13, never written by this unit
    void* request14;       // +0x14, object whose vtable slot 30 releases handleC8
    void* vtableSub18;     // +0x18 (0x0102767C)
    uint8_t body1C[0xAC];  // +0x1C..+0xC7, never written by this unit
    void* handleC8;        // +0xC8, released by ReleaseHandle
    uint8_t bodyCC[4];     // +0xCC..+0xCF, never written by this unit
    void* childD0;         // +0xD0, refcounted child released by Destruct
    uint8_t bodyD4[0x32];  // +0xD4..+0x105, never written by this unit
    uint16_t word106;      // +0x106, set by SetWordFloat
    float float108;        // +0x108, set by SetWordFloat
    uint8_t body10C[0x1C]; // +0x10C..+0x127, never written by this unit
    uint32_t field128;     // +0x128, zeroed by Construct
    uint32_t field12C;     // +0x12C, zeroed by Construct
};
TS3_X86_OFFSET(RouteManagerObject, request14, 0x14);
TS3_X86_OFFSET(RouteManagerObject, vtableSub18, 0x18);
TS3_X86_OFFSET(RouteManagerObject, handleC8, 0xC8);
TS3_X86_OFFSET(RouteManagerObject, childD0, 0xD0);
TS3_X86_OFFSET(RouteManagerObject, word106, 0x106);
TS3_X86_OFFSET(RouteManagerObject, float108, 0x108);
TS3_X86_OFFSET(RouteManagerObject, field128, 0x128);
TS3_X86_OFFSET(RouteManagerObject, field12C, 0x12C);
TS3_X86_SIZE(RouteManagerObject, 0x130);

// TS3W.exe 0x008607C0, cdecl ret 0. Builds the rotation matrix Rot(u, theta) with
// u = normalize(a1) (x87 rsqrt with two Newton-Raphson refinements; the dot product uses
// only the x/y/z lanes, so a1's w is ignored) and theta = a2[2] (x87 fsin/fcos), then
// extracts the quaternion of R*M (M's columns (u.z,0,-u.x), (-u.x*u.y,0,-u.z*u.y), u) with
// the game's four-way branch. Writes 16 bytes to *outQuat (movaps: a1/a2/out must be
// 16-byte aligned, like the original) and returns 1; returns 0 without touching outQuat
// when a1's x/y/z squared length is <= 0 or NaN.
int8_t QuaternionFromAxisAngle(const void* a1, const void* a2, void* out);

}  // namespace ts3::routemanager
