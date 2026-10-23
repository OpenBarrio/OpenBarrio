// Terrain cluster unit clu_6c9a00_Terrain: TS3W.exe 0x006C9A00 (the per-node update
// walk), 0x006C9B60 (name-to-entry lookup) and 0x006CF0E0 (the per-node update the
// walk performs). Layout evidence below; differential tests in
// src/exehost/clu_6c9a00_Terrain_test.cpp.
#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::terrain {

// The cluster object as these three functions see it (x86 layout, reconstructed):
//   +0x10  next: the list link. The container's head field (+0x4C) holds a pointer
//          to the FIRST node's +0x10; each node's +0x10 points at the next node's
//          +0x10; the last node's +0x10 points back at the container's +0x4C field
//          itself, which is the walk's sentinel.
//   +0x24/+0x2C/+0x30  the three field ADDRESSES 0x6CF0E0 hands to the manager
//          singleton's vtable slot 16.
//   +0x42  flags; bit 0 selects the 0x6079C0/0x6CE970 path inside 0x6CF0E0.
//   +0x44  the refcount the tail target 0x6CF0A0 (unit clu_6cca90_CAS) consumes.
//   +0x4C  head of the node list (0x6C9A00).
//   +0x64/+0x68  begin/end of the 32-byte entry array 0x6C9B60 scans.
struct TerrainCluster {
    // TS3W.exe 0x006C9A00, thiscall ret 0. Walks the +0x4C list calling UpdateNode
    // on every node and returns the last call's result. EA quirks kept: a null list
    // pointer still calls UpdateNode (with this == nullptr) and then dereferences
    // null; empty-list EAX is scratch and ignored by every known caller. The
    // rewrite returns zero when empty.
    int32_t VisitNodes();

    // TS3W.exe 0x006C9B60, thiscall ret 4. Returns the first 32-byte entry in
    // [+0x64, +0x68) whose first dword equals the 0x007CFE70 hash of `name`, or
    // null when no entry matches. Null names hash to zero and can match a key.
    void* FindEntry(const char* name);

    // TS3W.exe 0x006CF0E0, thiscall ret 0. Private helper of VisitNodes (its only
    // caller); the tail target 0x6CF0A0 belongs to unit clu_6cca90_CAS.
    int32_t UpdateNode();

    uint8_t head[0x10];       // +0x00..+0x0F opaque
    void* next;               // +0x10 list link (see above)
    uint8_t body14[0x10];     // +0x14..+0x23 opaque
    void* field24;            // +0x24 manager vcall arg 1 (its ADDRESS is passed)
    uint8_t body28[4];        // +0x28..+0x2B opaque
    void* field2C;            // +0x2C manager vcall arg 2 / 0x6CE970 value arg
    void* field30;            // +0x30 manager vcall arg 3 (its ADDRESS is passed)
    uint8_t body34[0xE];      // +0x34..+0x41 opaque
    uint8_t flags42;          // +0x42 bit 0 = the 0x6CE970 path
    uint8_t pad43[1];         // +0x43 opaque
    uint32_t refcount44;      // +0x44 consumed by 0x6CF0A0
    uint8_t tail48[4];        // +0x48..+0x4B opaque
    void* head4C;             // +0x4C list head
    uint8_t body50[0x14];     // +0x50..+0x63 opaque
    uint8_t* entries64;       // +0x64 entry array begin
    uint8_t* entries68;       // +0x68 entry array end
};
TS3_X86_OFFSET(TerrainCluster, next, 0x10);
TS3_X86_OFFSET(TerrainCluster, field24, 0x24);
TS3_X86_OFFSET(TerrainCluster, field2C, 0x2C);
TS3_X86_OFFSET(TerrainCluster, field30, 0x30);
TS3_X86_OFFSET(TerrainCluster, flags42, 0x42);
TS3_X86_OFFSET(TerrainCluster, refcount44, 0x44);
TS3_X86_OFFSET(TerrainCluster, head4C, 0x4C);
TS3_X86_OFFSET(TerrainCluster, entries64, 0x64);
TS3_X86_OFFSET(TerrainCluster, entries68, 0x68);
TS3_X86_SIZE(TerrainCluster, 0x6C);

}  // namespace ts3::terrain
