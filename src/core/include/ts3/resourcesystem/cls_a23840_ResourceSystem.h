// The resource-system list host around TS3W.exe 0x00A23840 (subsystem ResourceSystem).
// One 0x58-byte object (vtable 0x01059CA8) with a fourcc-ish type key in its prefix, two
// node-list heads and the lock the game's lock constructor 0x004E17E0 builds at +0x28.
// Its callers allocate it through the game's operator new (0x004E3F90, size 0x58) and
// register it with 0x0059F500; the key constructor 0x0059ECC0, the destructor body
// 0x00A23880
// through the layout.h bridge.
//
//   0x00A23840  Construct            __thiscall, plain ret, returns this
//   0x00A23A00  DeleteLockedList     __thiscall, ret 4, vtable slot 2 (deleting destructor)
//   0x00A379C0  VtableNoOp           __thiscall, vtable slots 5 and 6 (also slot 5 of the
//                                   vtable at 0x0105DEF8); a 5-byte jmp to a bare `ret`
#pragma once

#include <cstdint>

#include "ts3/layout.h"
#include "ts3/resourcesystem/clu_a1a290_ResourceSystem.h"  // GameLock

namespace ts3::resourcesystem {

// The node the destructor body chains off +0x18 and +0x20. Only the link field at +0x18
// matters here (the list walk and the free of each node happen inside the game's
// destructor body 0x00A23880); the rest of the node belongs to other units.
struct LockedListNode {
    uint8_t unused0[0x18];
    LockedListNode* next;  // +0x18
};
TS3_X86_OFFSET(LockedListNode, next, 0x18);
TS3_X86_SIZE(LockedListNode, 0x1C);

// The object this unit owns. Callers (0x00A239A0, SocialFeaturesManager::Init 0x00A3FF00)
// allocate exactly 0x58 bytes for it, so the struct below is the whole object, not just the
// touched extent. The lock's 0x2C-byte shape comes from the sibling unit
// clu_a1a290_ResourceSystem.h (same 0x004E17E0 constructor); +0x54 is the allocation's
// unused last dword.
struct ResourceLockedList {
    void* vtable;        // +0x00 the game's 0x01059CA8 after Construct
    uint32_t typeKey;    // +0x04 0x0C34FE06, written by the key constructor 0x0059ECC0
    uint32_t keyState8;  // +0x08 the key constructor exchanges a 0 in (its old value is
                         //       discarded; the xchg keeps the write atomic)
    uint32_t keyStateC;  // +0x0C 0, the key constructor's second argument
    uint32_t unused10;   // +0x10 untouched by this unit
    uint32_t unused14;   // +0x14 untouched by this unit
    LockedListNode* nodeList;    // +0x18 list head, cleared by Construct, drained by the
                                 //              game's destructor body 0x00A23880
    uint32_t unused1C;           // +0x1C cleared by Construct
    LockedListNode* spareList;   // +0x20 second list head, same treatment
    uint8_t gap24[4];            // +0x24: not written by this unit or its callees
    GameLock lock;               // +0x28 built by 0x004E17E0(this, name 0, useSection 1)
    uint8_t tail[4];             // +0x54: the allocation's last dword, never touched

    // TS3W.exe 0x00A23840 (thiscall, plain ret). Runs the game's key constructor on the
    // object prefix, installs this class's vtable, builds the lock at +0x28 and clears the
    // two list heads and +0x1C. Returns `this` (the callers store the result).
    ResourceLockedList* Construct();

    // TS3W.exe 0x00A23A00 (thiscall, ret 4), vtable slot 2: the deleting destructor. Runs
    // the game's destructor body 0x00A23880, then frees the object through the game's
    // operator delete 0x004E3F70 when the low byte of `flags` has bit 0 set. Returns `this`.
    ResourceLockedList* DeleteLockedList(uint32_t flags);

    // TS3W.exe 0x00A379C0 (thiscall): the no-op vtable stub shared by slots 5 and 6 of this
    // class's vtable (and slot 5 of 0x0105DEF8). It reads and writes nothing; the original
    // is a `jmp` to a bare `ret`, so its "return value" is whatever EAX already held.
    void VtableNoOp();
};
TS3_X86_OFFSET(ResourceLockedList, vtable, 0x00);
TS3_X86_OFFSET(ResourceLockedList, typeKey, 0x04);
TS3_X86_OFFSET(ResourceLockedList, keyState8, 0x08);
TS3_X86_OFFSET(ResourceLockedList, keyStateC, 0x0C);
TS3_X86_OFFSET(ResourceLockedList, nodeList, 0x18);
TS3_X86_OFFSET(ResourceLockedList, unused1C, 0x1C);
TS3_X86_OFFSET(ResourceLockedList, spareList, 0x20);
TS3_X86_OFFSET(ResourceLockedList, lock, 0x28);
TS3_X86_SIZE(ResourceLockedList, 0x58);

// The vtable Construct installs. x86-only meaning (the object never exists in an x64
// build); the constant compiles everywhere.
inline constexpr uintptr_t kResourceLockedListVtable = 0x01059CA8u;

// The type key the game's key constructor 0x0059ECC0 stores at +0x04.
inline constexpr uint32_t kResourceLockedListTypeKey = 0x0C34FE06u;

}  // namespace ts3::resourcesystem
