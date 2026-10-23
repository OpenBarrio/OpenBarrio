// Unit cls_c6f240_Debug (the project notes): the "World/CopyKeyFilter" debug filter - a
// 68-byte (0x44) object the game stack-constructs to hold two small hash maps of copy keys,
// and a predicate that says whether a key is present in either map. All three TS3W.exe
// functions of the unit:
//
//   CopyKeyFilter::IsFiltered   0x00C6F240  thiscall, ret 4   (vtable 0x01079C70 slot 1)
//   CopyKeyFilter::Initialize   0x00C6FB60  thiscall, no stack args (leaf constructor, 3 callers)
//   CopyKeyFilter::Destroy      0x00C6FBD0  thiscall, ret 4   (vtable slot 0: scalar deleting)
//
// Object layout (byte offsets; the two maps are subobjects at +0x04 and +0x24, so their
// bucket array / count live at +0x08/+0x0C and +0x28/+0x2C - read as [sub+4] / [sub+8]):
//
//   +0x00 vtable pointer          0x01079C70 (slot 0 Destroy, slot 1 IsFiltered)
//   +0x08 bucket array pointer    0x0114C900 (the static {0, ~0, ~0, ~1} single-bucket array)
//   +0x0C element index           1
//   +0x10 0                       (cleared by the C1 destructor 0x00AA9550)
//   +0x14 1.0f  (from 0x0107A538)   +0x18 2.0f  (from 0x00F98A58)
//   +0x1C 0
//   +0x20 name pointer            0x01079C5C "World/CopyKeyFilter"
//   +0x28 bucket array pointer    0x0114C900   +0x2C element index 1
//   +0x30 0                       +0x34 1.0f   +0x38 2.0f   +0x3C 0
//   +0x40 name pointer            0x01079C5C "World/CopyKeyFilter"
//
// +0x04 and +0x24 (the map subobjects' own first dword) are NOT written by the constructor;
// the original leaves whatever the stack held there, and nothing in this unit reads them.
//
// Call edges leaving the unit: IsFiltered -> 0x00592EC0 (clu_592ec0_Debug, map1 find) and
// 0x0072DAD0 (map2 find); Destroy -> 0x00AA9550 (the C1 destructor of this class, another
// unit) and 0x004E3F70 (clu_4e3f70_Scratch, operator delete). All four stay bridge calls.
//
// The vtable 0x01079C70 is only referenced by the constructor, and the constructor is only
// ever called on stack objects (0x00AA95C0, 0x00ACBF50 - the original instruction sequence), so IsFiltered
// (slot 1) has no code callers at all and Destroy (slot 0) is only reachable through a
// pointer to the object; see the unit report and the exehost test.
//
// Because the object stores *game data addresses* (vtable, name, bucket array) as 32-bit
// literals, the rewrite uses byte-offset Load32/Store32 accessors instead of a C++ struct -
// the same choice the other address-holding rewrites make (e.g. ts3::videorenderablecontext,
// src/core/videorenderablecontext/cls_618eb0_VideoRenderableC.cpp:12). On x64 those literals
// are meaningless, so the two paths that walk them stop with NotRewritten.
#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3 { namespace debug {

class CopyKeyFilter {
public:
    // TS3W.exe 0x00C6F240 (__thiscall, ret 4, vtable slot 1). True when `key` (a 16-byte key
    // object; the map1 lookup uses its +8 dword, the map2 lookup its first three dwords) is
    // found in map1 or in map2. `key` is only ever passed on to the two find helpers, so its
    // exact type is the game's own key record.
    //
    // EA contract: only AL is defined on return (the true path is the original instruction sequence and the second
    // comparison the original instruction sequence, both on top of the find helper's returned buffer address), so
    // callers must test the low byte only - exactly what a C++ bool return guarantees.
    bool IsFiltered(const void* key);

    // TS3W.exe 0x00C6FB60 (__thiscall, no stack args). Fills the object from constants and
    // returns `this` (EAX = the ECX it was called with). Idempotent: a second call writes the
    // same values, which is what makes it safe for a compare hook to run both sides.
    CopyKeyFilter* Initialize();

    // TS3W.exe 0x00C6FBD0 (__thiscall, ret 4, vtable slot 0). The scalar deleting destructor:
    // runs the C1 destructor 0x00AA9550, then frees the object through 0x004E3F70 when bit 0
    // of `deleteFlags` is set (the original tests only that bit), and returns `this`.
    CopyKeyFilter* Destroy(uint32_t deleteFlags);
};

// The object's 68-byte layout, as documentation and as a compile-time check (the rewrite
// reaches the fields by byte offset, because the fields hold 32-bit game *data* addresses).
// Every field is 32 bits wide, so this layout is the same on x64; the x64 build simply never
// dereferences the addresses it stores.
struct CopyKeyFilterLayout {
    uint32_t vtable;    // +0x00 game vtable 0x01079C70
    uint32_t mapSlot;   // +0x04 map1 subobject head, never written or read by this unit
    uint32_t buckets;   // +0x08 map1 bucket array (game address)
    uint32_t count;     // +0x0C map1 element index
    uint32_t cleared;   // +0x10 cleared by the C1 destructor
    uint32_t lo;        // +0x14 float 1.0
    uint32_t hi;        // +0x18 float 2.0
    uint32_t pad1C;     // +0x1C 0
    uint32_t name;      // +0x20 "World/CopyKeyFilter" (game address)
    uint32_t map2Slot;  // +0x24 map2 subobject head, never written or read by this unit
    uint32_t buckets2;  // +0x28 map2 bucket array (game address)
    uint32_t count2;    // +0x2C map2 element index
    uint32_t cleared2;  // +0x30 cleared by the C1 destructor
    uint32_t lo2;       // +0x34 float 1.0
    uint32_t hi2;       // +0x38 float 2.0
    uint32_t pad3C;     // +0x3C 0
    uint32_t name2;     // +0x40 "World/CopyKeyFilter" (game address)
};

TS3_X86_SIZE(CopyKeyFilterLayout, 0x44);
TS3_X86_OFFSET(CopyKeyFilterLayout, vtable, 0x00);
TS3_X86_OFFSET(CopyKeyFilterLayout, buckets, 0x08);
TS3_X86_OFFSET(CopyKeyFilterLayout, count, 0x0C);
TS3_X86_OFFSET(CopyKeyFilterLayout, cleared, 0x10);
TS3_X86_OFFSET(CopyKeyFilterLayout, lo, 0x14);
TS3_X86_OFFSET(CopyKeyFilterLayout, hi, 0x18);
TS3_X86_OFFSET(CopyKeyFilterLayout, pad1C, 0x1C);
TS3_X86_OFFSET(CopyKeyFilterLayout, name, 0x20);
TS3_X86_OFFSET(CopyKeyFilterLayout, buckets2, 0x28);
TS3_X86_OFFSET(CopyKeyFilterLayout, count2, 0x2C);
TS3_X86_OFFSET(CopyKeyFilterLayout, cleared2, 0x30);
TS3_X86_OFFSET(CopyKeyFilterLayout, lo2, 0x34);
TS3_X86_OFFSET(CopyKeyFilterLayout, hi2, 0x38);
TS3_X86_OFFSET(CopyKeyFilterLayout, pad3C, 0x3C);
TS3_X86_OFFSET(CopyKeyFilterLayout, name2, 0x40);

// The object size the constructor's writes cover (0x00..0x43) and the callers reserve.
constexpr unsigned kCopyKeyFilterSize = 0x44;

} }  // namespace ts3::debug
