// In-memory package index built by DatabasePackedFile's IndexCompressed::Load
// (TS3W.exe 0x004AD020), reproduced byte for byte. Portable: x86 and x64.
#pragma once

#include <cstdint>
#include <functional>
#include <vector>

namespace ts3::resource {

// 16-byte record; sizes >= 0xFFFF live in the overflow table (size16 == 0xFFFF and
// memSize16 = overflow index).
struct IndexRecord {
    uint32_t instanceLow;
    uint32_t instanceHigh;
    uint32_t offset;
    uint16_t size16;
    uint16_t memSize16;
};
static_assert(sizeof(IndexRecord) == 16, "matches the engine's record layout");

// One per (type, group) group, coalesced hashing by (type + group) % groupCount;
// `next` links collisions (0xFFFFFFFF = end). A sentinel bucket follows the last one,
// its firstRecord being the total record count.
struct IndexBucket {
    uint32_t type;
    uint32_t group;
    uint32_t firstRecord;
    uint32_t next;
};
static_assert(sizeof(IndexBucket) == 16, "matches the engine's bucket layout");

struct IndexSizeOverflow {
    uint32_t size;
    uint32_t memSize;
};
static_assert(sizeof(IndexSizeOverflow) == 8, "matches the engine's overflow entry layout");

// The engine's resource key layout (16 bytes): 64-bit instance as two dwords, type, group.
// 8-byte aligned like EA's (the instance is a 64-bit field there): a key + pointer pair is
// 24 bytes, as in the game's containers.
struct alignas(8) ResourceKey {
    uint32_t instanceLow;
    uint32_t instanceHigh;
    uint32_t type;
    uint32_t group;
};
static_assert(sizeof(ResourceKey) == 16, "matches the engine's ResourceKey layout");

struct CompressedIndex {
    std::vector<IndexRecord> records;
    std::vector<IndexSizeOverflow> overflow;
    std::vector<IndexBucket> buckets;  // groupCount + 1 (sentinel)
    uint32_t groupCount = 0;
};

// Builds the index from the raw DBPF index bytes (flags word first). Returns false if
// the data is truncated (where the original would read past its buffer).
// Divergence: records with duplicate (type, group, instance) keys may end up in a
// different order than the original's unstable introsort gives them; no installed
// package contains such duplicates.
bool BuildCompressedIndex(const uint8_t* index, uint32_t bytes, CompressedIndex& out);

// IndexCompressed::Find (TS3W.exe 0x004AA1B0): the record for `key`, or nullptr.
// Coalesced-hash bucket walk by (type + group) % groupCount, then a lower-bound binary
// search on the 64-bit instance within the group. `Index` is CompressedIndex or the game's
// IndexCompressed (package_index.h): anything with records, buckets and groupCount.
template <class Index>
const IndexRecord* FindRecordIn(const Index& index, const ResourceKey& key)
{
    const uint32_t count = index.groupCount;
    if (count == 0)
        return nullptr;
    uint32_t b = (key.type + key.group) % count;
    if (static_cast<int32_t>(b) < 0)
        return nullptr;
    while (index.buckets[b].type != key.type || index.buckets[b].group != key.group) {
        b = index.buckets[b].next;
        if (static_cast<int32_t>(b) < 0)
            return nullptr;
    }
    uint32_t lo = index.buckets[b].firstRecord;
    const uint32_t end = index.buckets[b + 1].firstRecord;
    uint32_t hi = end - 1;
    while (lo < hi) {
        const uint32_t mid = (hi + lo) >> 1;
        const IndexRecord& r = index.records[mid];
        if (r.instanceHigh < key.instanceHigh || (r.instanceHigh == key.instanceHigh && r.instanceLow < key.instanceLow))
            lo = mid + 1;
        else
            hi = mid;
    }
    if (hi < end && index.records[hi].instanceLow == key.instanceLow && index.records[hi].instanceHigh == key.instanceHigh)
        return &index.records[hi];
    return nullptr;
}

inline const IndexRecord* FindRecord(const CompressedIndex& index, const ResourceKey& key)
{
    return FindRecordIn(index, key);
}

// IndexCompressed::Matches (TS3W.exe 0x004AC9C0): keys of all records in bucket order that
// pass `filter` (all when null), appended to `out` when given; returns how many matched.
uint32_t MatchingKeys(const CompressedIndex& index, const std::function<bool(const ResourceKey&)>* filter,
                      std::vector<ResourceKey>* out);

// IndexCompressed::SortedEntries (TS3W.exe 0x004ACBB0): keys of all records ordered by file
// offset (EASTL sort, same tie order as the game), appended to `out` when given; returns
// the total number of records.
uint32_t SortedKeys(const CompressedIndex& index, std::vector<ResourceKey>* out);

}  // namespace ts3::resource
