// The resource-system list host around TS3W.exe 0x00A23840 (subsystem ResourceSystem).
// Three functions of one 0x58-byte class (vtable 0x01059CA8): its constructor, its
// deleting destructor (slot 2) and the no-op stub shared by slots 5 and 6.
//
// Kept as the original does them (they are the observable behaviour):
//   - the constructor's vtable store lands BETWEEN the key constructor call and the lock
//     constructor call, and overwrites the 0x00FDDE44 the key constructor had just put at
//     +0x00 (a dead store the original still performs, kept so the byte sequence the
//     object goes through is the game's own);
//   - the key constructor 0x0059ECC0 zeroes +0x08 with an atomic exchange and throws the
//     old value away — a plain store would be enough for the game, but the exchange is
//     what runs, so it stays the game's own code;
//   - the deleting destructor tests only bit 0 of the LOW BYTE of its flag argument
//     (the original instruction sequence) and returns `this` on both paths;
//   - the no-op stub is a `jmp` to a bare `ret`: it never touches memory and its "return
//     value" is whatever EAX held on entry, which no caller reads.
// Dependencies 0x004E17E0 (lock constructor, unit clu_4e17e0_ResourceSystem),
// 0x0059ECC0 (key constructor, not a unit), 0x00A23880 (destructor body, not a unit) and
// 0x004E3F70 (operator delete, unit clu_4e3f70_Scratch) are all still original code and
// are reached through the bridge. Nothing here compiles into a call on x64: the bridge
// aborts first.
#include "ts3/resourcesystem/cls_a23840_ResourceSystem.h"

#include <cstdint>

namespace ts3::resourcesystem {

namespace {
// The game's key constructor: stores the vtable-ish pointer at +0x00 (overwritten by this
// unit right after), the type key at +0x04, an atomic 0 at +0x08 and the second argument at
// +0x0C. __thiscall, ret 8. Not a unit of its own.
constexpr unsigned kKeyConstruct = 0x0059ECC0;
// The game's lock constructor (unit clu_4e17e0_ResourceSystem, todo): __thiscall, ret 8,
// (lock, const void* name, bool useSection).
constexpr unsigned kLockConstruct = 0x004E17E0;
// The destructor body of this class (0x00A23880): __thiscall, plain ret, tail-jumps into
// the shared base teardown. Not a unit of its own.
constexpr unsigned kDestructBody = 0x00A23880;
// The game's operator delete (unit clu_4e3f70_Scratch, todo): __cdecl, one pointer.
constexpr unsigned kOperatorDelete = 0x004E3F70;
}  // namespace

// TS3W.exe 0x00A23840 (thiscall, plain ret). `this` is a 0x58-byte block from the game's
// operator new; the callers (0x00A239A0, 0x00A3FF00) do not construct it when the
// allocation failed, so `this` is never null here.
ResourceLockedList* ResourceLockedList::Construct()
{
    // The key constructor first: it writes +0x00..+0x0C, and its +0x00 value is the next
    // instruction's target.
    CallGameMethod<void, kKeyConstruct>(this, kResourceLockedListTypeKey, 0u);
    vtable = reinterpret_cast<void*>(kResourceLockedListVtable);
    // The lock at +0x28: no name, critical section wanted.
    CallGameMethod<void, kLockConstruct>(&lock, nullptr, 1u);
    // Both list heads and the word between them start empty.
    nodeList = nullptr;
    unused1C = 0u;
    spareList = nullptr;
    return this;
}

// TS3W.exe 0x00A23A00 (thiscall, ret 4), vtable slot 2. The destructor body 0x00A23880
// drains the two node lists through the game's operator delete and tears the lock down;
// it is not part of this unit. `flags` bit 0 (as tested on the low byte only, like the
// original) adds the free of the object itself. Returns `this` on both paths.
ResourceLockedList* ResourceLockedList::DeleteLockedList(uint32_t flags)
{
    CallGameMethod<void, kDestructBody>(this);
    if (flags & 1u)
        CallGameFunction<void, kOperatorDelete>(this);
    return this;
}

// TS3W.exe 0x00A379C0 (thiscall): the no-op stub in vtable slots 5 and 6 (slot 5 of
// 0x0105DEF8 too). The original jumps to a bare `ret`, so the game's own callers
// (0x009E0660, 0x00F20A70) read no result from it.
void ResourceLockedList::VtableNoOp() {}

}  // namespace ts3::resourcesystem
