// Unit cls_4a9480_ResourceLoad (TS3W.exe 0x004A9480..0x004AE2E0, subsystem ResourceLoad).
// Of the unit's 14 functions, nine were already rewritten in the pilot packed-file module
// (src/core/resource/packed_file.cpp and packed_record.cpp, hooked in the harness as
// resource.RecordCtor*/WriteRecord*/FreeSpaceReset): 0x004AB070, 0x004AD810, 0x004AD920,
// 0x004ADCE0, 0x004ADE60, 0x004ADF30, 0x004ADFD0, 0x004AE0F0, 0x004AE2E0. This header owns
// the five that were still missing:
//   0x004A9480  an arena/buffer constructed through an allocator context, zeroed
//   0x004AA4E0  the sum of a FreeSpaceMap's block sizes
//   0x004AB060  a FreeSpaceMap's block-tree destructor helper
//   0x004AB5C0  the FreeSpaceMap constructor
//   0x004AE080  the read record (PackedFileRecord) destructor
#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::resource {
struct PackedFileRecord;
}  // namespace ts3::resource

namespace ts3::resourceload {

// The 0x24-byte free-space map as the game lays it out (x86 offsets match
// ts3::resource::FreeSpaceMap in ts3/resource/packed_file.h; the source asserts the two
// sizes and the block offset). This view exists because packed_file.* is another unit's
// source, and 0x004AB5C0/0x004AA4E0/0x004AB060 operate on the object's bytes.
struct FreeSpaceMapObject {
    int32_t total;              // 0x00 free space ends here
    uint32_t limit;             // 0x04 0x7FFFFFFF while unlimited
    uint32_t comparator;        // 0x08 the block map's comparator; untouched by the ctor
    void* anchorLeft;           // 0x0C the map's sentinel node, pointing at itself
    void* firstNode;            // 0x10 first block node, or the sentinel when empty
    void* anchorParent;         // 0x14
    uint32_t anchorColor;       // 0x18 written as a whole dword by the ctor (color byte + padding)
    uint32_t nodeCount;         // 0x1C
    const char* allocatorName;  // 0x20 "ResourceSystem/ResourceMan/PackedFile/HoleTable"

    // TS3W.exe 0x004AB5C0: total = -1, limit = 0x7FFFFFFF, an empty block map (anchor at
    // +0x0C holds its own address, first node points at it) whose allocator name is the
    // game's HoleTable string. Returns this. EA quirk kept: the comparator dword at
    // +0x08..+0x0B is not written (old bytes stay); +0x18 is zeroed as a dword, then its
    // low byte is zeroed again.
    FreeSpaceMapObject* Construct4AB5C0();

    // TS3W.exe 0x004AA4E0: the sum of every block's size (node +0x14), walking the block
    // tree from +0x10 with the game's successor 0x004E0B20 until the sentinel at +0x0C.
    // Read-only; 0 for an empty map. unsigned 32-bit accumulation (EA quirk kept).
    int32_t SumBlockSizes4AA4E0();
};
TS3_X86_OFFSET(FreeSpaceMapObject, comparator, 0x08);
TS3_X86_OFFSET(FreeSpaceMapObject, anchorLeft, 0x0C);
TS3_X86_OFFSET(FreeSpaceMapObject, firstNode, 0x10);
TS3_X86_OFFSET(FreeSpaceMapObject, anchorParent, 0x14);
TS3_X86_OFFSET(FreeSpaceMapObject, anchorColor, 0x18);
TS3_X86_OFFSET(FreeSpaceMapObject, nodeCount, 0x1C);
TS3_X86_OFFSET(FreeSpaceMapObject, allocatorName, 0x20);
TS3_X86_SIZE(FreeSpaceMapObject, 0x24);

// TS3W.exe 0x004AB060: drops a FreeSpaceMap's block tree through 0x006A0850 (still game
// code: __thiscall with one stack argument, called with ECX = the map at +0x08 and the
// node at +0x14; the original's return value is whatever that call leaves in EAX and is
// passed through). Side-effecting.
uint32_t FreeSpaceMapDestruct4AB060(FreeSpaceMapObject* self);

// The 12-byte arena/buffer 0x004A9480 fills (x86 offsets; pointer-sized fields so the x64
// build keeps them whole).
struct ZeroedBufferObject {
    void* begin;    // 0x00
    void* current;  // 0x04
    void* end;      // 0x08

    // TS3W.exe 0x004A9480: allocates `size` bytes through 0x004A8BE0 (this, size,
    // allocationContext; still game code), zeroes them, sets current = begin + size and
    // returns this. With size 0 nothing is zeroed but current still becomes begin. ret 8.
    ZeroedBufferObject* Construct4A9480(uint32_t size, void* allocationContext);
};
TS3_X86_OFFSET(ZeroedBufferObject, current, 0x04);
TS3_X86_OFFSET(ZeroedBufferObject, end, 0x08);
TS3_X86_SIZE(ZeroedBufferObject, 12);

// TS3W.exe 0x004AE080: the read record (ts3::resource::PackedFileRecord, vtable
// 0x00FB2A70, IStream part 0x00FB2A34) destructor. Releases the object at +0x2C through
// its vtable slot 6 and (reloading +0x2C) its deleting destructor slot 0 with flag 1,
// returns an open record to its package (owner slot 15 = CloseRecord, only when +0x24 is
// non-zero), releases the memory stream's shared buffer (0x004DEA80, only when +0x34 is
// non-null) and leaves the base vtables in place (stream +0x28 and MemoryStream +0x30
// 0x00FA99FC, record 0x00FB84D0). Returns the base vtable address the original leaves in
// EAX (0x00FA99FC; x64 builds return null). EA quirks kept: owner and the +0x2C object
// are used without null checks beyond the ones the assembly has.
#if defined(_M_IX86)
void* DestructPackedFileRecord4AE080(ts3::resource::PackedFileRecord* self);
#else
// x64 builds never run the game's record destructor: there is no game code to call and
// no game object to walk, so the width-independent rewrite is an empty stub.
inline void* DestructPackedFileRecord4AE080(ts3::resource::PackedFileRecord*) { return nullptr; }
#endif

}  // namespace ts3::resourceload