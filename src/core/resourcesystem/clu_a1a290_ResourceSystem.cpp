// Unit clu_a1a290_ResourceSystem (the project notes): the six SIGS object constructors
// of TS3W.exe 1.67.2 (Steam). Every one is __thiscall, takes only `this` and returns it;
// the field writes are unconditional constants, so the object's post-ctor bytes do not
// depend on the memory it starts from. The only callees are:
//   - 0x004E17E0, the game's lock constructor (unit clu_4e17e0_ResourceSystem, not
//     rewritten yet) - called through the bridge, like every unit so far;
//   - 0x00A43680, which builds the 0x84-byte object inside ProfileRooms. It belongs to no
//     unit in the project notes - through the bridge as well;
//   - the release of a ref-counted pointer, whose body (a locked decrement of the count
//     at +4, and on zero the exchange back to 1 and the virtual slot 0 with flag 1) runs
//     on an object of an unknown class - the virtual call through the bridge too.
// The three "dead" releases (0x58/0xE4/0xE8 in the profile store, the +0x30 slot of
// ProfileRooms' embedded object) read slots zeroed a few instructions earlier, so they
// can never fire during construction; they are kept because the original has them.
// Each Construct below follows its original's write order (the original disassembly/
// clu_a1a290_ResourceSystem, confirmed with a disassembler); writes that the compiler
// only scheduled apart do not overlap, so grouping them does not change the result.
#include "ts3/resourcesystem/clu_a1a290_ResourceSystem.h"

#include <cstring>

#include "ts3/layout.h"

#ifdef _MSC_VER
#include <intrin.h>
#endif

namespace ts3::resourcesystem {

namespace {

// Dependency addresses in TS3W.exe 1.67.2 (Steam).
constexpr unsigned kLockCtor = 0x004E17E0;    // __thiscall (lock, const void* name, bool useSection), ret 8
constexpr unsigned kDepCtorA43680 = 0x00A43680;  // __thiscall (dep), ret 0

// The shared ref-count release the ctors run on their pointer slots: `p` points at an
// object whose refcount sits at +4; dropping the last reference (old count 1) resets the
// count to 1 and calls the object's virtual slot 0 with the flag 1 (its destructor, on a
// class only the slot's owner knows).
void ReleaseRefCounted(void*& slot)
{
    void* p = slot;
    if (p == nullptr)
        return;
    slot = nullptr;
    auto* refs = static_cast<volatile long*>(static_cast<void*>(static_cast<uint8_t*>(p) + 4));
    if (_InterlockedDecrement(refs) == 0) {
        _InterlockedExchange(refs, 1);
        CallGameVirtual<void, 0>(p, 1);
    }
}

// Starts one of the game's locks exactly as the original ctors do: the unnamed
// critical-section flavour (name = none, useSection = true).
void ConstructLock(GameLock& lock)
{
    CallGameMethod<void, kLockCtor>(&lock, static_cast<const void*>(nullptr), true);
}

}  // namespace

// 0x00A3FAE0: two cleared dwords, one lock at +0x08.
SigsServiceLock* SigsServiceLock::Construct()
{
    field00 = 0;
    field04 = 0;
    ConstructLock(lock);
    return this;
}

// 0x00A1A290: four locks (0x28, 0x78, 0xA8, 0x108) and the zeroed fields around them;
// fieldE8 ends at 2.
SigsServices* SigsServices::Construct()
{
    field00 = 0;
    field08 = 0;
    field0C = 0;
    field10 = 0;
    field14 = 0;
    field18 = 0;
    field1C = 0;
    field20 = 0;
    ConstructLock(lockA);
    field58 = 0;
    ConstructLock(lockB);
    ConstructLock(lockC);
    fieldD8 = 0;
    fieldDC = 0;
    fieldE0 = 0;
    fieldE4 = 0;
    ConstructLock(lockD);
    fieldEC = 0;
    fieldF0 = 0;
    fieldF8 = 0;
    fieldFC = 0;
    field100 = 0;
    field104 = 0;
    field64 = 0;
    field68 = 0;
    field6C = 0;
    field60 = 0;
    field70 = 0;
    field5C = 0;
    fieldE8 = 2;
    return this;
}

// 0x00A35DF0: one lock at +0x40, everything before it zeroed.
ProfileConnections* ProfileConnections::Construct()
{
    field00 = 0;
    field04 = 0;
    field08 = 0;
    bytes2C[0] = 0;
    bytes2C[1] = 0;
    bytes2C[2] = 0;
    bytes2C[3] = 0;
    ConstructLock(lock);
    field30 = 0;
    field34 = 0;
    field38 = 0;
    field0C = 0;
    field10 = 0;
    field14 = 0;
    field18 = 0;
    field1C = 0;
    field20 = 0;
    field24 = 0;
    field28 = 0;
    owner = nullptr;
    return this;
}

// 0x00A43970: the dependency object at +0x20 (through the bridge), a lock at +0xB0, the
// re-zeroed first dword of the dependency object, and the dead release of its +0x10 slot.
ProfileRooms* ProfileRooms::Construct()
{
    field0C = 0;
    field10 = 0;
    field14 = 0;
    field18 = 0;
    bytes1C[0] = 0;
    bytes1C[1] = 0;
    bytes1C[2] = 0;
    CallGameMethod<void, kDepCtorA43680>(&embedded);
    ConstructLock(lock);
    *reinterpret_cast<uint32_t*>(embedded.head) = 0;
    ReleaseRefCounted(embedded.released);
    field00 = 0;
    field04 = 0;
    field08 = 0;
    owner = nullptr;  // 0xA8
    fieldAC = 15;
    return this;
}

// 0x00A400A0: one lock at +0x20; the only non-zero field is the limit 31 at +0x14.
ProfileQueue* ProfileQueue::Construct()
{
    field00 = 0;
    field04 = 0;
    field08 = 0;
    field0C = 0;
    bytes10[0] = 0;
    bytes10[1] = 0;
    ConstructLock(lock);
    owner = nullptr;
    limit = 31;
    return this;
}

// 0x00A3B540: the three sub-constructors at 0xF0/0x160/0x240, the constant fields, the
// two memsets (0x00..0x30 and 0x30..0x9C), the three dead releases and the self-pointers
// the store writes into its sub-objects (+0x3C / +0xA8 / +0x18 of each).
SigsProfileStore* SigsProfileStore::Construct()
{
    field58 = nullptr;
    fieldE4 = nullptr;
    fieldE8 = nullptr;
    connections.Construct();
    rooms.Construct();
    queue.Construct();
    fieldCC = 0;
    fieldD0 = 0;
    fieldD4 = 0;
    fieldD8 = 0;
    limitA0 = 0x0FFFFFFF;
    fieldA4 = 0;
    limitA8 = 0x0FFFFFFF;
    fieldAC = 0;
    ReleaseRefCounted(fieldE8);  // dead: zeroed above
    ReleaseRefCounted(field58);  // dead: zeroed above
    fieldB4 = 0;
    fieldB8 = 0;
    fieldBC = 0;
    fieldC0 = 0;
    queue.tail[0x28] = 0;  // byte 0x2B4
    fieldB0 = 0;
    fieldDC = 0;
    fieldE0 = 0;
    ReleaseRefCounted(fieldE4);  // dead: zeroed above
    std::memset(this, 0, 0x30);
    std::memset(reinterpret_cast<uint8_t*>(this) + 0x30, 0, 0x6C);
    queue.tail[0x29] = 0;  // byte 0x2B5
    queue.tail[0x2A] = 0;  // byte 0x2B6
    fieldC8 = 0xFFFFFFFF;
    fieldC4 = 0;
    field24 = 0xFFFFFFFF;
    rooms.owner = this;       // 0x208
    queue.owner = this;       // 0x258
    connections.owner = this; // 0x12C
    return this;
}

}  // namespace ts3::resourcesystem
