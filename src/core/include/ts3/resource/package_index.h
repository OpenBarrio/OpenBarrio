// A package's index as the rest of the engine sees it (IPackageIndex), and the game's
// IndexCompressed class laid out like the original (constructor 0x004AAF50, vtable
// 0x00FB2840), so its methods can run on the game's own objects. The index-building
// algorithm itself is in compressed_index.h.
#pragma once

#include <cstdint>

#include "ts3/eastl_game.h"
#include "ts3/layout.h"
#include "ts3/resource/compressed_index.h"

namespace ts3::resource {

// What an index reports for one record. IndexCompressed keeps one in the object and returns
// it; there `compression` is 0xFFFF exactly when size != memSize.
struct RecordInfo {
    uint32_t offset;
    uint32_t size;  // stored
    uint32_t memSize;
    uint16_t compression;
    uint8_t valid;  // IndexCompressed sets 1
};

// The package index interface (IndexCompressed or the modifiable index). Only the slot the
// read path calls is recovered.
struct FreeSpaceMap;

class IPackageIndex {
public:
#define TS3_UNRECOVERED(n) virtual void Unrecovered##n() = 0;
    TS3_UNRECOVERED(0)
    // Slot 1 (+0x04): true for the modifiable index (IndexModifiable), false for the
    // read-only IndexCompressed.
    virtual bool IsModifiable() = 0;
    TS3_UNRECOVERED(2)
    // Slot 3 (+0x0C; IndexCompressed 0x004AAFE0): number of records.
    virtual uint32_t Count() = 0;
    // Slot 4 (+0x10): probably empties the index (Open calls it when the index is bad).
    virtual void Clear() = 0;
    TS3_UNRECOVERED(5) TS3_UNRECOVERED(6) TS3_UNRECOVERED(7) TS3_UNRECOVERED(8)
    // Slot 9 (+0x24): takes every record's region out of `freeSpace` (offsets from the
    // package start; the header's `headerSize` bytes are already taken). False when a
    // record doesn't fit the file.
    virtual bool MarkRecordsUsed(FreeSpaceMap* freeSpace, uint32_t headerSize, uint32_t fileSize) = 0;
    // Slot 10 (+0x28). nullptr when the key is not in the index; the result lives in the
    // index and is only valid until the next call.
    virtual const RecordInfo* GetRecord(const ResourceKey* key) = 0;
    // Slot 11 (+0x2C): adds or updates a record's entry (ReadRecord registers new records
    // with an all-zero info).
    virtual void SetRecord(const ResourceKey* key, const RecordInfo* info) = 0;
    // Slot 12 (+0x30; IndexModifiable 0x004ABE90): drops a record's entry and copies it to
    // `removed`. False when the key isn't in the index.
    virtual bool RemoveRecord(const ResourceKey* key, RecordInfo* removed) = 0;
#undef TS3_UNRECOVERED
    // Slot 13 (+0x34; IndexCompressed 0x004AD020): builds the index from the DBPF index bytes.
    virtual bool Load(const void* data, uint32_t size, uint32_t unknown) = 0;
    // Slot 14 (+0x38): serializes the index (DBPF index bytes) into a block from the
    // package's allocator; the caller frees it.
    virtual bool Save(void** data, uint32_t* size, uint32_t unknown) = 0;
    // Slot 15 (+0x3C): checks the records against the file (offset `start`, `fileSize` bytes).
    virtual bool Validate(uint32_t start, uint32_t fileSize) = 0;
};

// The game's IndexCompressed. Differential tests use the game's constructor (vector names
// "IndexCompressed/IndexTable", "SizeOverflowTable", "TypeGroupTable"); the standalone x64
// build value-initializes this layout and supplies a portable allocator for its three vectors.
struct IndexCompressed {
    void* const* vtable;
    allocator::ICoreAllocator* allocator;  // 0x04
    GameVector<IndexRecord> records;       // 0x08
    GameVector<IndexSizeOverflow> overflow;  // 0x20
    GameVector<IndexBucket> buckets;       // 0x38: groupCount + 1 (sentinel)
    uint32_t groupCount;                   // 0x50
    RecordInfo lastRecord;                 // 0x54: what GetRecord returns

    // TS3W.exe 0x004AA1B0.
    const IndexRecord* Find(const ResourceKey& key) const { return FindRecordIn(*this, key); }
    // Virtual slot 10, TS3W.exe 0x004AA470.
    const RecordInfo* GetRecord(const ResourceKey* key);
    // Virtual slot 13, TS3W.exe 0x004AD020. Rebuilds the tables from the DBPF index bytes,
    // reusing the vectors' storage the way the original does, so the final vectors match
    // it byte for byte, capacities included. Divergence: the temporary tables of the build
    // live on the CRT heap, not in the game's allocator.
    bool Load(const void* index, uint32_t bytes, uint32_t unused);
    // Virtual slots 2 and 3, TS3W.exe 0x004AAFD0 and 0x004AAFE0.
    bool IsEmpty() const { return records.empty(); }
    uint32_t Count() const { return static_cast<uint32_t>(records.size()); }
};
TS3_X86_OFFSET(IndexCompressed, records, 0x08);
TS3_X86_OFFSET(IndexCompressed, overflow, 0x20);
TS3_X86_OFFSET(IndexCompressed, buckets, 0x38);
TS3_X86_OFFSET(IndexCompressed, groupCount, 0x50);
TS3_X86_OFFSET(IndexCompressed, lastRecord, 0x54);
TS3_X86_SIZE(IndexCompressed, 0x64);

}  // namespace ts3::resource
