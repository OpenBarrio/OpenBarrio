// The writable package index (IndexModifiable, TS3W.exe vtable 0x00FB2910, object 0x30
// bytes; DatabasePackedFile's index slot 25 builds it at 0x004A68E0). It holds one
// eastl::hash_map<ResourceKey, RecordInfo> ("PackedFile/IndexMap"): the game's map is the
// same EASTL as third_party/ea_EASTL, so the container is used directly on the game's
// objects (x86 offsets checked below). The read-only sibling is compressed_index.h.
//
// The vtable (each slot: game address, size, what it does):
//   0 +0x00 0x004ABDF0  88 deleting destructor (flag bit 0 frees the object)
//   1 +0x04 0x007EAEA0   3 IsModifiable: always true
//   2 +0x08 0x004ABDC0   9 IsEmpty
//   3 +0x0C 0x00F20C20   4 Count
//   4 +0x10 0x004ABDD0  30 Clear (drops the nodes, keeps the buckets)
//   5 +0x14 0x004AAB60  80 sum of the records' stored sizes
//   6 +0x18 0x004AAB00  83 highest offset + size
//   7 +0x1C 0x004AC180 187 MatchingKeys: keys in bucket order through a filter, into a
//                        game vector (ret 8; the count is returned even for a null vector)
//   8 +0x20 0x004AC240 251 SortedEntries: keys ordered by the records' file offsets
//                        (node pointers sorted by 0x004ABF30; ret 4)
//   9 +0x24 0x004ACE80 138 MarkRecordsUsed
//  10 +0x28 0x004ABE50  57 GetRecord
//  11 +0x2C 0x004AC990  42 SetRecord (insert-or-find at 0x004AC7B0)
//  12 +0x30 0x004ABE90 110 RemoveRecord (erase at 0x004AA570)
//  13 +0x34 0x004AC340 404 Load: DBPF index bytes into the map (always returns true)
//  14 +0x38 0x004AABB0 650 Save: the map to DBPF index bytes ("…/IndexImage")
//  15 +0x3C 0x004AAE40 137 Validate
//  16 +0x40 0x004AAED0 125 NoRecordOverlaps
// Shared map code: find 0x005E5E20 (108), insert 0x004AB670 (245), clear-nodes
// 0x004AA280 (78), erase 0x004AA570 (108), rehash 0x004AA730, prime policy 0x004E09B0.
#pragma once

#include <cstdint>
#include <cstring>

#include <EASTL/hash_map.h>

#include "ts3/eastl_game.h"
#include "ts3/layout.h"
#include "ts3/resource/compressed_index.h"
#include "ts3/resource/package_index.h"

namespace ts3::resource {

struct FreeSpaceMap;

// The map's key hash: instanceLow XOR group (TS3W.exe 0x005E5E20 and 0x004AB670). The
// hash is only used to pick a bucket; nodes carry no cached hash code (Find compares the
// key's four dwords), so these functors must stay empty (they are the four bytes at the
// start of the game's map object).
struct IndexKeyHash {
    size_t operator()(const ResourceKey& key) const
    {
        return static_cast<size_t>(key.instanceLow ^ key.group);
    }
};
struct IndexKeyEqual {
    bool operator()(const ResourceKey& a, const ResourceKey& b) const
    {
        return a.instanceLow == b.instanceLow && a.instanceHigh == b.instanceHigh && a.type == b.type &&
               a.group == b.group;
    }
};
static_assert(sizeof(IndexKeyHash) == 1 && sizeof(IndexKeyEqual) == 1, "empty functors, as in the game's map");

// The game's hash_map: prime rehash policy (max load 1.0, growth 2.0, the shared prime
// table), nodes of the pair plus a next pointer, and the game allocator with the name
// "ResourceSystem/ResourceMan/PackedFile/IndexMap". Hash codes are not cached in nodes.
using IndexMap = eastl::hash_map<ResourceKey, RecordInfo, IndexKeyHash, IndexKeyEqual, GameAllocator>;

// x86 mirror of IndexMap, so the layout the game's code reads and writes is pinned down:
// four empty functor bytes, the bucket array (bucketCount + 1 slots, the last a non-null
// end sentinel, the shared {null, ~0} array while bucketCount is 1), the counts, the prime
// rehash policy, then the allocator.
struct IndexMapLayout {
    uint8_t functors[4];
    void** bucketArray;
    uint32_t bucketCount;
    uint32_t elementCount;
    float maxLoadFactor;
    float growthFactor;
    uint32_t nextResize;
    GameAllocator allocator;
};
TS3_X86_OFFSET(IndexMapLayout, bucketArray, 0x04);
TS3_X86_OFFSET(IndexMapLayout, bucketCount, 0x08);
TS3_X86_OFFSET(IndexMapLayout, elementCount, 0x0C);
TS3_X86_OFFSET(IndexMapLayout, maxLoadFactor, 0x10);
TS3_X86_OFFSET(IndexMapLayout, growthFactor, 0x14);
TS3_X86_OFFSET(IndexMapLayout, nextResize, 0x18);
TS3_X86_OFFSET(IndexMapLayout, allocator, 0x1C);
TS3_X86_SIZE(IndexMapLayout, 0x28);

// A map node: the key and RecordInfo of a pair (RecordInfo at node+0x10, what GetRecord
// hands out), the chain link at +0x20, then alignment padding to the pair's 8.
struct IndexNodeLayout {
    ResourceKey key;
    RecordInfo value;
    void* next;
};
TS3_X86_OFFSET(IndexNodeLayout, value, 0x10);
TS3_X86_OFFSET(IndexNodeLayout, next, 0x20);
TS3_X86_SIZE(IndexNodeLayout, 0x28);

static_assert(sizeof(void*) != 4 || sizeof(IndexMap) == sizeof(IndexMapLayout), "the game's map object is 0x28 bytes on x86");
static_assert(sizeof(void*) != 4 || sizeof(IndexMap::value_type) == 32, "a key + RecordInfo pair is 32 bytes");
static_assert(sizeof(void*) != 4 || sizeof(IndexMap::node_type) == sizeof(IndexNodeLayout),
              "nodes are 0x28 bytes, as the game frees them");

// The output of MatchingKeys and SortedEntries: the game's eastl::vector<ResourceKey>
// (0x18 bytes on x86; slots 7 and 8 push onto whatever the caller already holds, growing
// through the vector's GameAllocator).
using IndexKeyVector = GameVector<ResourceKey>;

// A caller-side key filter for MatchingKeys (slot 7): only vtable slot 1 (+0x04, thiscall
// bool(const ResourceKey*)) is ever called.
struct IndexKeyFilter {
    void* const* vtable;
};

// The index object. Never built by the game's code here: tests and the harness hand in
// objects built by the game's constructor (0x004ABD70). `Construct` reproduces it.
struct IndexModifiable {
    void* const* vtable;              // 0x00 the game's (0x00FB2910) on x86
    allocator::ICoreAllocator* allocator;  // 0x04 what Save allocates its index image with
    IndexMap map;                     // 0x08 the records, keyed by ResourceKey

    // TS3W.exe 0x004ABD70, on uninitialised memory: the game's vtable, the allocator, and
    // an empty map (shared empty bucket array, counts 0/1, policy 1.0/2.0/0).
    void Construct(allocator::ICoreAllocator* coreAllocator);
    // TS3W.exe 0x004ABDF0, without the delete (the game's slot 0 is a deleting destructor
    // whose flag selects it): frees every node and, past the shared empty array, the
    // buckets, and leaves the object with the destroyed-object vtable.
    void Destruct();
    // The game's slot 0 in full: Destruct, and with flags & 1 the object's memory goes too
    // (EA's operator delete, 0x004E4230 — only ever called on the game's own heap).
    void DeletingDestruct(uint32_t flags);

    // Virtual slot 1 (+0x04; the shared stub at 0x007EAEA0): always true here.
    bool IsModifiable() const { return true; }
    // Virtual slot 2 (+0x08, 0x004ABDC0): no records.
    bool IsEmpty() const;
    // Virtual slot 3 (+0x0C, 0x00F20C20): the number of records.
    uint32_t Count() const;
    // Virtual slot 4 (+0x10, 0x004ABDD0): drops every record, keeps the buckets. True.
    bool Clear();

    // Virtual slot 5 (+0x14, 0x004AAB60): the sum of every record's stored size.
    uint32_t TotalStoredSize();
    // Virtual slot 6 (+0x18, 0x004AAB00): the highest offset + size over the records (the
    // end of the topmost region).
    uint32_t HighestRecordEnd();
    // Virtual slot 7 (+0x1C, 0x004AC180): the keys whose filter check passes (all of them
    // with a null filter), in bucket order, appended to `out` (a null vector counts
    // without collecting). Returns how many keys matched, counted either way.
    uint32_t MatchingKeys(IndexKeyVector* out, const IndexKeyFilter* filter);
    // Virtual slot 8 (+0x20, 0x004AC240): every key ordered by its record's file offset,
    // through the game's introsort (0x004ABF30), appended to `out`. The node-pointer
    // scratch comes from the object's allocator ("…/IndexPair") and is freed with the
    // size the original passes (0 — unknown). Returns the number of keys.
    uint32_t SortedEntries(IndexKeyVector* out);
    // Virtual slot 9 (+0x24, 0x004ACE80): takes every record's region out of `freeSpace`
    // (FreeSpaceMap::MarkUsed, offsets from the package start; the header's `headerSize`
    // bytes are already taken). False when a record doesn't fit [headerSize, fileSize).
    bool MarkRecordsUsed(FreeSpaceMap* freeSpace, uint32_t headerSize, uint32_t fileSize);
    // Virtual slot 10 (+0x28, 0x004ABE50): the record for `key`, or nullptr. The result is
    // the entry itself: valid until that record is changed or removed.
    const RecordInfo* GetRecord(const ResourceKey* key);
    // Virtual slot 11 (+0x2C, 0x004AC990): adds or replaces a record's entry (the whole
    // RecordInfo, padding included).
    void SetRecord(const ResourceKey* key, const RecordInfo* info);
    // Virtual slot 12 (+0x30, 0x004ABE90): drops a record's entry and copies it to
    // `removed`. False when the key isn't in the index.
    bool RemoveRecord(const ResourceKey* key, RecordInfo* removed);
    // Virtual slot 13 (+0x34, 0x004AC340): reads a DBPF index image (the bytes at the
    // package's indexOffset) into the map — insert-or-keep on duplicates, always true.
    // The format: flags dword (bit0 type, bit1 group, bit2 instanceHigh are shared by
    // every record), the shared values in that order, then per record [type][group]
    // [instanceHigh] — each only when its flag bit is clear — [instanceLow][offset]
    // [size][memSize] and, when the size's high bit is set, one more dword holding the
    // compression word and the valid flag. (sic) Without that high bit the record is a
    // legacy one: compression becomes 0xFFFF or 0 from memSize != size and valid stays 0,
    // and the image has no bounds checks — a truncated one reads past its end. `unknown`
    // (the caller's force-valid) sets every record's valid flag.
    bool Load(const void* data, uint32_t size, uint32_t unknown);
    // Virtual slot 14 (+0x38, 0x004AABB0): the map as a DBPF index image, allocated with
    // the object's allocator under "ResourceLoad/PackedFile/IndexImage" (0 on allocation
    // failure; the size is reported either way). Load's inverse: a record's size always
    // gets the high bit and the compression/valid dword, and `unknown` forces valid.
    bool Save(void** dataOut, uint32_t* sizeOut, uint32_t unknown);
    // Virtual slot 15 (+0x3C, 0x004AAE40): every record with a size must lie in
    // [start, fileSize). Records of size 0 are skipped.
    bool Validate(uint32_t start, uint32_t fileSize);
    // Virtual slot 16 (+0x40, 0x004AAED0): true when no record overlaps
    // [start, start + length); a zero length is always free.
    bool NoRecordOverlaps(uint32_t start, uint32_t length);
};
TS3_X86_OFFSET(IndexModifiable, allocator, 0x04);
TS3_X86_OFFSET(IndexModifiable, map, 0x08);
TS3_X86_SIZE(IndexModifiable, 0x30);

// TS3W.exe 0x004A68E0 (DatabasePackedFile slot 25): allocate an IndexModifiable through
// `allocator` and construct it. A 0x30-byte Alloc named
// "ResourceSystem/ResourceMan/PackedFile/IndexModifiable", then the same body as the ctor
// (0x004ABD70). Returns nullptr when the allocation fails, as the original does. (Not a
// member: in the game this is a free thunk that tail-jumps into the ctor.)
IndexModifiable* CreateIndexModifiable(allocator::ICoreAllocator* allocator);

}  // namespace ts3::resource
