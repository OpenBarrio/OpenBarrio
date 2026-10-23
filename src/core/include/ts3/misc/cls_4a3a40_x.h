// cls_4a3a40_x: the TS3W.exe functions the analysis pipeline grouped under this unit
// (misc/). They are not one C++ class: the bundle holds a global setter (0x004A3A40),
// the resource key-list container `ResourceMapList` (0x0059AEB0..0x0059D340), the
// key-field serializer 0x005A3830, a `DatabaseShadowWatchers` object with its debug
// key maps (0x00733E40..0x007368F0), the 16-byte resource key predicates
// (0x00996880..0x00AD9700) and two small indexed-vector helpers (0x00C64FB0/FE0).
//
// Every function is rewritten at its original ABI: thiscall functions are struct
// members, the ecx-ignoring ret-4 predicates are members too (their `this` is simply
// never read), so the harness can substitute them at the original addresses.
//
// TS3W.exe 0x00C64FE0 has no IDA function at the requested address (its first bytes
// are shared with another function), so its body was recovered from the disassembly.
#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3 {
namespace misc {

// ---------------------------------------------------------------------------
// Resource keys: four 32-bit words (type, group, instance, extra). The game
// spells the same 16 bytes as ResourceSortKey in clu_4a6c30_x.h.
// ---------------------------------------------------------------------------
struct ResourceMapKey {
    uint32_t type;      // +0x0
    uint32_t group;     // +0x4
    uint32_t instance;  // +0x8
    uint32_t high;      // +0xC

    // TS3W.exe 0x00996880 (thiscall ret 4): key->type == this->instance and
    // key->group == this->high. The original compares this+8/this+0xC, i.e. the same
    // two words this record spells as instance/high.
    bool MatchesTypePair(const void* key) const;
    // TS3W.exe 0x009A7490 (thiscall ret 4, `this` unused): key->instance is not one of
    // 0x05ED1226, 0xB4DD716B, 0x34E5247C.
    bool IsTypeExcluded3(const void* key) const;
    // TS3W.exe 0x009A74C0 (thiscall ret 4, `this` unused): key->instance is not one of
    // 0x05ED1226, 0x06B981ED, 0x220557DA, 0xD063545B, 0x022B756C, 0x35A33E29,
    // 0xB4DD716B, 0x34E5247C.
    bool IsTypeExcluded8(const void* key) const;
    // TS3W.exe 0x009A7C50 (stdcall ret 4, `this` unused): key->instance != 0x0679B9B8.
    bool IsTypeNotExcluded1(const void* key) const;
    // TS3W.exe 0x009A7C70 (stdcall ret 4, `this` unused): key->instance is not one of
    // 0x05ED1226, 0x0679B9B8, 0xB4DD716B, 0x34E5247C.
    bool IsTypeNotExcluded4(const void* key) const;
    // TS3W.exe 0x00AD4DF0 (thiscall ret 4): MatchesTypePair plus key->instance is not
    // 0x05ED1226
    bool MatchesTypePairEx2(const void* key) const;
    // TS3W.exe 0x00AD4ED0 (thiscall ret 8): out->instance/high = src->instance/high,
    // out->type/group = this->instance/high (the original copies in that order).
    // Returns `out` in EAX.
    ResourceMapKey* CopyPairWith(void* out, const void* src) const;
    // TS3W.exe 0x00AD96B0 (thiscall ret 4): MatchesTypePair plus key->high is one of
    // 0x00B0C507, 0x001064CA, 0x001DA7E9, 0x007E7555, 0x00BC1E4C, 0x00BC1E54 (the
    // original compares unsigned, then subtracts 0x00BC1E4C and 8).
    bool MatchesTypePair6(const void* key) const;
    // TS3W.exe 0x00AD9700 (thiscall ret 4): MatchesTypePair plus key->high == 0x001064CA.
    bool MatchesTypePairFire(const void* key) const;
};
TS3_X86_SIZE(ResourceMapKey, 0x10);
TS3_X86_OFFSET(ResourceMapKey, high, 0xC);

// A {first, last} view over an array of 16-byte keys (0x0059ADF0's argument).
struct KeySpan {
    ResourceMapKey* first;  // +0x0
    ResourceMapKey* last;   // +0x4
};
TS3_X86_SIZE(KeySpan, 0x8);
TS3_X86_OFFSET(KeySpan, last, 0x4);

// TS3W.exe 0x004A3A40 (cdecl, 1 arg): swap the dword global at 0x011C78D0 with the
// argument and return the previous value (the game stores a resource-system pointer
// there). Keeps no null checks - the original stores unconditionally.
uint32_t SwapKeyGlobal(uint32_t value);

// TS3W.exe 0x0059ADF0 (cdecl, ret 0): scan [first,last) for a 16-byte key whose
// `group` word has the same top byte as `mask`. EA quirk: `mode` is tested first and
// any non-zero mode makes every element miss, so the mask is only ever checked for
// mode == 0. Returns 1 on a hit, 0 otherwise.
uint8_t MaskScanKeyRange(const KeySpan* span, int32_t mode, uint32_t mask);

struct MapListNode;

// ---------------------------------------------------------------------------
// The name->guid map list container (map list vtables 0x00FD9A6C / 0x00FDDb14 /
// 0x00FDDb4c, always ending with the base vtable 0x00FB2CBC):
//   +0x10 a {begin, cursor, capacity} vector of 0x20-byte records,
//   +0x48 the sentinel of a circular list of MapListNode.
// ---------------------------------------------------------------------------
struct ResourceMapList {
    void* vtable;          // +0x00
    uint8_t pad04[0xC];    // +0x04
    void* vecBegin;        // +0x10
    void* vecCursor;       // +0x14
    void* vecCapacity;     // +0x18
    uint8_t pad1C[0x18];   // +0x1C
    void* baseVtable;      // +0x34 = 0x00FB2CBC after destruction
    uint8_t pad38[0x10];   // +0x38
    MapListNode* listNext; // +0x48 (circular; sentinel is &listNext)
    MapListNode* listPrev; // +0x4C

    // TS3W.exe 0x0059CD60: return the derived-vtable constant 0x00FDDb14.
    uint32_t VtableConstant();
    // TS3W.exe 0x0059AEB0: install the 0x00FD9A6C vtable, run the game teardown
    // 0x0044C390, clear the global 0x011CE2D0, free every list node through 0x004E3F70,
    // install the base vtable 0x00FB2CBC and tail-call the base destructor 0x00731CF0.
    // Returns the base destructor's EAX.
    int32_t DestructRefList();
    // TS3W.exe 0x0059B050: find the list node whose key matches (type, group, instance,
    // high) == key->{type, group, instance, high} in the order the original compares
    // them (type, instance, high, group), release its +0x18 object through vtable slot 1,
    // unlink and free the node. Returns the key pointer when nothing matched, else the
    // 0x004E3F70 return.
    void* EraseKey(const ResourceMapKey* key);
    // TS3W.exe 0x0059CFC0: release the +0x10 vector through 0x00C1C390 and reset the
    // cursor to the begin pointer. Returns the 0x00C1C390 EAX.
    int32_t ClearVector();
    // TS3W.exe 0x0059D060: install the 0x00FDDb4c vtable, clear the global 0x011CE308,
    // release the vector through 0x00C1C390, free the vector begin through 0x004E3F70,
    // call the base destructor 0x00731CF0 and free `this` when flags bit 0 is set.
    // Always returns `this`.
    ResourceMapList* Destroy(int32_t flags);
    // TS3W.exe 0x0059D1E0: ask the resource manager (0x004AFD20) for the key list
    // tagged 0xF3A38370 through the database object's vtable slot 0x30, then for every
    // key query the manager's slot 0x0C and append the resulting 0x28-byte record to
    // the +0x10 vector (growing through 0x0059D0C0, constructing through 0x0093C460).
    // Returns the 0x004E3F70 free of the collected list, or 0.
    int32_t AppendDatabase(void* database);
    // TS3W.exe 0x0059D340: if the manager exists, make sure the database type filter is
    // registered (0x00935430 when the query at manager slot 0x2C fails), collect the
    // database list (manager slot 0x48) through the 0x00931020 allocator's list, call
    // AppendDatabase for every entry (node+8), then release the list through the
    // allocator's vtable slot 3. Returns 1 on the manager path, 0 without a manager.
    uint8_t LoadDatabaseKeys();
};
TS3_X86_OFFSET(ResourceMapList, vecBegin, 0x10);
TS3_X86_OFFSET(ResourceMapList, vecCursor, 0x14);
TS3_X86_OFFSET(ResourceMapList, vecCapacity, 0x18);
TS3_X86_OFFSET(ResourceMapList, baseVtable, 0x34);
TS3_X86_OFFSET(ResourceMapList, listNext, 0x48);
TS3_X86_OFFSET(ResourceMapList, listPrev, 0x4C);
TS3_X86_SIZE(ResourceMapList, 0x50);

// A node of the +0x48 list: intrusive links, then the 16-byte key and the refcounted
// object whose vtable slot 1 is the release slot.
struct MapListNode {
    MapListNode* next;  // +0x00
    MapListNode* prev;  // +0x04
    ResourceMapKey key;    // +0x08
    void* object;       // +0x18
};
TS3_X86_OFFSET(MapListNode, key, 0x08);
TS3_X86_OFFSET(MapListNode, object, 0x18);
TS3_X86_SIZE(MapListNode, 0x1C);

// ---------------------------------------------------------------------------
// 0x005A3830's object: six dwords the serializer reads/writes.
// ---------------------------------------------------------------------------
struct KeyFieldOwner {
    uint32_t f00;         // +0x00
    uint32_t f04;         // +0x04
    uint32_t f08;         // +0x08
    uint32_t f0C;         // +0x0C
    uint32_t f10;         // +0x10
    uint8_t pad14[0xC];   // +0x14
    uint32_t f20;         // +0x20

    // TS3W.exe 0x005A3830: construct the game's 0x18-byte field-reader (0x007D91B0),
    // read the six dwords through 0x004DD700 / 0x007D7530, accumulate the success flag
    // through 0x007D77C0, finish through 0x007D7760 and return the reader's flag byte.
    uint8_t SerializeFields(void* stream);
};
TS3_X86_OFFSET(KeyFieldOwner, f04, 0x04);
TS3_X86_OFFSET(KeyFieldOwner, f08, 0x08);
TS3_X86_OFFSET(KeyFieldOwner, f0C, 0x0C);
TS3_X86_OFFSET(KeyFieldOwner, f10, 0x10);
TS3_X86_OFFSET(KeyFieldOwner, f20, 0x20);
TS3_X86_SIZE(KeyFieldOwner, 0x24);

// ---------------------------------------------------------------------------
// The DatabaseShadowWatchers object (2060+ bytes on x86): the two cache pointers at
// +0x1E0/+0x1E4, a flag byte at +0x1F0, the debug key map at +0x758 (circular list)
// and the "map built" byte at +0x764.
// ---------------------------------------------------------------------------
struct DatabaseShadowWatchers {
    uint8_t pad00[0x8];    // +0x00
    void* keySource;       // +0x08: vtable pointer of the object embedded at +8; the
                           // 0x007368F0 slot-0x64 call passes &this->keySource as `this`
                           // (the original loads the vtable from +8 but passes +8 itself)
    uint8_t pad0C[0x1D4];  // +0x0C .. +0x1DF
    void* cacheRefA;       // +0x1E0
    void* cacheRefB;       // +0x1E4
    uint8_t pad1E8[0x8];
    uint8_t flag;          // +0x1F0
    uint8_t pad1F1[0x567]; // +0x1F1 .. +0x757
    void* tempMapNext;     // +0x758 (circular list sentinel)
    void* tempMapPrev;     // +0x75C
    uint32_t pad760;       // +0x760
    uint8_t mapBuilt;      // +0x764

    // TS3W.exe 0x00733E40: notify the object 0x00727280 returns (its vtable slot 0x24
    // with both arguments) and then call the caller object's slot 0x40. Returns the
    // last slot's EAX.
    int32_t NotifyShadow(void* target, int32_t value);
    // TS3W.exe 0x00733E70: trim each non-null cache (the verified
    // clu_727290_ResourceSystem 0x00732E90) to 0 bytes with a forced trim. Returns the
    // second trim's value when there is one, else the first, else stale EAX (0 here).
    int32_t TrimCacheRefs();
    // TS3W.exe 0x00733EE0: store the low byte of the argument in +0x1F0 and return it
    // in AL (the rest of EAX is the caller's).
    uint8_t SetFlag(int32_t value);
    // TS3W.exe 0x00733EF0: thiscall tail-jump to 0x00731CA0 with this+0x768.
    int32_t InitKeyMapMember();
    // TS3W.exe 0x00733F00: thiscall tail-jump to 0x00730270 with this+0x768.
    int32_t DestroyKeyMapMember();
    // TS3W.exe 0x00735C20: when +0x764 is set and the +0x758 list is empty, collect the
    // "Debug/KeyNameMap/TempKeyList" keys through the manager (0x004AFD20) and build one
    // 72-byte record per key. Returns &this->tempMapNext on every path.
    void* BuildTempKeyList();
    // TS3W.exe 0x00736600: return the byte at +0x1F0 in AL.
    uint8_t GetFlag() const;
    // TS3W.exe 0x007368F0: sort the "ResourceSystem/DatabaseKeyFilter" key list of the
    // object at +0x8 (slot 0x30), then call slot 0x28 of every entry of that object's
    // +0x64 list, releasing the list nodes and the key vector. Returns the vector free.
    int32_t FilterDatabases(void* target);
};
TS3_X86_OFFSET(DatabaseShadowWatchers, keySource, 0x08);
TS3_X86_OFFSET(DatabaseShadowWatchers, flag, 0x1F0);
TS3_X86_OFFSET(DatabaseShadowWatchers, tempMapNext, 0x758);
TS3_X86_OFFSET(DatabaseShadowWatchers, mapBuilt, 0x764);

// ---------------------------------------------------------------------------
// 0x00C64FB0/FE0's owner: a vtable, a sub-object at +4 handed to 0x006258F0, and an
// array of 32-bit ids at +8 indexed by +0xC.
// ---------------------------------------------------------------------------
struct IndexedVector {
    void* vtable;      // +0x00
    uint32_t pad04;    // +0x04 (0x006258F0's `this`)
    uint32_t* ids;     // +0x08
    uint32_t count;    // +0x0C
    uint32_t pad10;    // +0x10

    // TS3W.exe 0x00C64FB0: compare the id at index `count` with the dword 0x0072DAD0
    // returns for `key`; returns whether they differ.
    bool ContainsDifferent(void* key);
    // TS3W.exe 0x00C64FE0: run the +4 sub-object's teardown 0x006258F0, free the id
    // array when more than one id is stored, install the base vtable 0x00FB2CBC and
    // free `this` when flags bit 0 is set. Always returns `this`.
    IndexedVector* Destroy(int32_t flags);
};
TS3_X86_OFFSET(IndexedVector, ids, 0x08);
TS3_X86_OFFSET(IndexedVector, count, 0x0C);

}  // namespace misc
}  // namespace ts3