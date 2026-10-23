// Unit cls_4e3580_Debug (the project notes): the debug TraceHelper - a small refcounted
// record a trace site constructs, hands to the per-thread trace server (register / release),
// and reads back through a vtable. All seven TS3W.exe functions of the unit:
//
//   TraceHelper::SetId        0x004E3580  thiscall, ret 4 (vtable slot 1: +0x24 + reset flags)
//   (private) SwapThreadSlot  0x004E37C0  thiscall, no stack args (the +0x10 slot cell)
//   TraceHelper::Construct    0x004E3910  thiscall, ret 0x10 (vtable set + register slot[6])
//   TraceHelper::SetEnabled   0x004E3A00  thiscall, ret 4 (vtable slot 7: +0x04 byte)
//   TraceHelper::Release      0x004E3A10  thiscall, no stack args (release slot[7]) -> void*
//   TraceHelper::Destruct     0x004E3B90  thiscall, ret 4 (vtable slot 0: release + delete)
//   TraceHelper::GetInfo      0x007E94F0  thiscall, no stack args (vtable slot 6: &info[0])
//
// Call edges inside the unit: Construct -> SwapThreadSlot, Release -> SwapThreadSlot,
// Destruct -> (Release's body) + SwapThreadSlot. Outside the unit the Construct/Release/
// Destruct paths reach the per-thread trace server through the global 0x011CB84C (a
// manager whose +0x10 field is the current thread's slot): they call the slot's vtable
// [6] (register) / [7] (release), and SwapThreadSlot builds the slot through the game's
// 0x00931020 (thread key) + 0x004E3760 (get-or-create slot) and AddRef/Releases via
// vtable [0]/[1]. The three leaves (SetId, SetEnabled, GetInfo) are pure field access.
//
// The three game-only paths (Construct / Release / Destruct) read the 0x011CB84C manager
// and make virtual calls, so they only run on x86; the field writes are portable.
#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::debug {

// The per-thread trace slot object the manager caches at +0x10. Opaque here (created by
// the game's 0x004E3760 in another unit); the unit only uses its vtable [0]=AddRef,
// [1]=Release, [6]=register a helper, [7]=release a helper.
struct TraceSlot;

// The +0x10 sub-object of the 0x011CB84C manager that 0x004E37C0 takes as its `this`:
// the cached slot and the thread key it was built for (0 = "fetch the current key").
struct TraceSlotRef {
    TraceSlot* slot;  // +0x00  (== manager->field_10) current thread's slot
    uint32_t key;     // +0x04  (== manager->field_14) key `slot` was built for
};

// The manager object the global 0x011CB84C points at. Only the +0x10 slot cell is read
// by this unit (the rest of the manager belongs to the trace runtime); the cell is the
// whole of SwapThreadSlot's `this`.
struct TraceHelperManager {
    uint8_t before10[0x10];  // not touched by this unit
    TraceSlotRef cell;       // +0x10  {slot, key}
};
TS3_X86_OFFSET(TraceHelperManager, cell, 0x10);

// The TraceHelper record (the class of 0x004E3910). 40 bytes on x86; the vtable pointer
// is the game's vt_TraceHelper at 0x00FB8ABC (slot[0]=Destruct, [1]=SetId, [6]=GetInfo,
// [7]=SetEnabled), which the game code always writes into +0x00.
struct TraceHelper {
    void* vtable_;      // +0x00  vt_TraceHelper (0x00FB8ABC)
    uint8_t registered; // +0x04  1 while the record is live with the server
    uint8_t flag05;     // +0x05
    uint8_t flag06;     // +0x06  set when registration failed (no server)
    // +0x07 padding
    int32_t type;       // +0x08  the category passed to Construct
    int32_t level;      // +0x0C  derived from `type` by Construct's switch
    int32_t interval;   // +0x10  timeout passed to Construct (or a type default)
    const char* name;   // +0x14  label passed to Construct (or "Assert" / "<Unknown>")
    int32_t info[3];    // +0x18  the 3-dword block copied from Construct's last argument
    int32_t field24;    // +0x24  written by SetId

    // 0x004E3910, __thiscall, ret 0x10, returns this. Fills the record and registers it
    // with the per-thread trace server (or marks registration failed when absent).
    TraceHelper* Construct(int32_t type, const char* name, int32_t interval,
                           const int32_t infoBlock[3]);
    // 0x004E3580, __thiscall, ret 4, returns a0. Vtable slot 1: stores the id and resets
    // the two flag bytes.
    int32_t SetId(int32_t id);
    // 0x004E3A00, __thiscall, ret 4, returns the low byte of the argument. Vtable slot 7.
    uint8_t SetEnabled(uint8_t enabled);
    // 0x007E94F0, __thiscall, returns this+0x18 (the info block). Vtable slot 6.
    int32_t* GetInfo();
    // 0x004E3A10, __thiscall, no stack args. Sets the vtable and releases the record
    // from the per-thread trace server (vtable slot [7]); no delete. Returns the slot's
    // release result (0 when there is no server) - caller 0x00935FA0 returns it on.
    void* Release();
    // 0x004E3B90, __thiscall, ret 4, returns this. Vtable slot 0: Release's body, then
    // the game's operator delete (0x004E3F70) when the low flag bit is set.
    TraceHelper* Destruct(uint32_t flags);

    // 0x004E37C0, __thiscall on the manager's +0x10 slot cell (`self` is that cell, not a
    // TraceHelper): builds the thread's slot (via the game's 0x00931020 key + 0x004E3760
    // get-or-create), AddRefs the new slot, stores it, and Releases the old one. Encoded
    // as __fastcall with a discarded EDX: the original takes `self` in ECX (thiscall)
    // and never reads EDX.
#if defined(_M_IX86)
    static TraceSlot* __fastcall SwapThreadSlot(TraceSlotRef* self, void* edxDiscard);
#else
    static TraceSlot* SwapThreadSlot(TraceSlotRef* self, void* edxDiscard);
#endif
};
TS3_X86_OFFSET(TraceHelper, registered, 0x04);
TS3_X86_OFFSET(TraceHelper, type, 0x08);
TS3_X86_OFFSET(TraceHelper, level, 0x0C);
TS3_X86_OFFSET(TraceHelper, interval, 0x10);
TS3_X86_OFFSET(TraceHelper, name, 0x14);
TS3_X86_OFFSET(TraceHelper, info, 0x18);
TS3_X86_OFFSET(TraceHelper, field24, 0x24);
TS3_X86_SIZE(TraceHelper, 0x28);

}  // namespace ts3::debug
