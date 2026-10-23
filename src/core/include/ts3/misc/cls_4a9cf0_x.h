#pragma once
#include "ts3/resource/package_index.h"

namespace ts3::misc::compressed {
using resource::IndexRecord;
using resource::IndexBucket;
using resource::IndexSizeOverflow;
using resource::ResourceKey;
using resource::RecordInfo;
using allocator::ICoreAllocator;
struct Allocation { ICoreAllocator* core; uint32_t flags; const char* name; };
template<class T> struct Vector {
    T *begin, *end, *capacity;
    Allocation allocation;
    size_t Size() const { return (reinterpret_cast<uintptr_t>(end)-reinterpret_cast<uintptr_t>(begin))/sizeof(T); }
    size_t Capacity() const { return (reinterpret_cast<uintptr_t>(capacity)-reinterpret_cast<uintptr_t>(begin))/sizeof(T); }
};
struct KeyVector {
    ResourceKey *begin, *end, *capacity;
    const char* name;
    size_t Size() const { return (reinterpret_cast<uintptr_t>(end)-reinterpret_cast<uintptr_t>(begin))/sizeof(ResourceKey); }
    size_t Capacity() const { return (reinterpret_cast<uintptr_t>(capacity)-reinterpret_cast<uintptr_t>(begin))/sizeof(ResourceKey); }
};
TS3_X86_SIZE(KeyVector, 16);
struct SortedEntry { const IndexRecord* record; const IndexBucket* bucket; };
// TS3W.exe 0x004A9CF0: guarded insertion sort by unsigned file offset.
void InsertSorted(SortedEntry* first, SortedEntry* last);
// TS3W.exe 0x004A9D50: unguarded insertion sort; preceding range supplies its sentinel.
void InsertSortedTail(SortedEntry* first, SortedEntry* last);
// TS3W.exe 0x004A9DB0: guarded insertion sort by unsigned 64-bit instance.
IndexRecord* InsertRecords(IndexRecord* first, IndexRecord* last);
// TS3W.exe 0x004A9E20: unguarded insertion sort with a preceding sentinel.
void InsertRecordsTail(IndexRecord* first, IndexRecord* last);
// TS3W.exe 0x004ABFB0: introsort of record/bucket pairs (offset ties are unstable).
void SortEntries(SortedEntry* first, SortedEntry* last, uint32_t predicate);
// TS3W.exe 0x004AC100: introsort of records by unsigned instance.
void SortRecords(IndexRecord* first, IndexRecord* last, uint32_t predicate);
struct PairVector : Vector<SortedEntry> {
    // TS3W.exe 0x004AB140: allocate a new pair buffer and copy the given range.
    SortedEntry* AllocateCopy(uint32_t count, const SortedEntry* first, const SortedEntry* last);
    // TS3W.exe 0x004AB600: reserve exactly the requested pair capacity.
    uintptr_t Reserve(uint32_t count);
};
struct RecordVector : Vector<IndexRecord> {
    // TS3W.exe 0x004ABB30: insert count copies, doubling capacity when necessary.
    uintptr_t Insert(IndexRecord* where, uint32_t count, const IndexRecord* value);
    // TS3W.exe 0x004AC840: resize the record vector, zero filling new records.
    uintptr_t Resize(uint32_t count);
};
struct GroupNode { uint32_t type, group; RecordVector records; GroupNode* next; };
struct InsertResult { GroupNode* node; GroupNode** bucket; uint8_t inserted; };
struct GroupMap {
    void* unused;
    GroupNode** buckets;
    uint32_t bucketCount, size;
    float maxLoad, growth;
    uint32_t nextResize;
    Allocation allocation;
    // TS3W.exe 0x004AA620: rehash by moving old chain heads to new chain heads.
    void Rehash(uint32_t count);
    // TS3W.exe 0x004ABAD0: allocate a node; original faults on allocation failure.
    GroupNode* NewNode(const uint32_t* key);
    // TS3W.exe 0x004AC030: insert a type/group pair using the game's prime rehash policy.
    InsertResult* Insert(InsertResult* out, const uint32_t* key, uint32_t unused);
    // TS3W.exe 0x004AC8B0: free every node and its record buffer, zeroing bucket heads.
    void ClearBuckets(GroupNode** table, uint32_t count);
};
struct Link { Link *next, *prev; GroupNode* value; };
struct Chain { Link head; /* head.value overlaps allocation.core in the sentinel */ };
struct ChainList {
    Link *next, *prev;
    Allocation allocation;
    // TS3W.exe 0x004AB540: copy a list range before a destination sentinel.
    void CopyBefore(Link* destination, const Link* first, const Link* last, uint32_t unused);
};
struct Chains : Vector<ChainList> {
    // TS3W.exe 0x004AA6D0: allocate an uninitialized chain array.
    Chains* Allocate(uint32_t count, const Allocation* source);
    // TS3W.exe 0x004ACF80: construct an array of empty circular lists.
    Chains* Construct(uint32_t count, const Allocation* source);
};
// TS3W.exe 0x004AC930: construct an array of lists by copying a prototype list.
void ConstructLists(ChainList* first, uint32_t count, const ChainList* prototype);
struct Index {
    void* const* vtable;
    ICoreAllocator* allocator;
    RecordVector records;
    Vector<IndexSizeOverflow> overflow;
    Vector<IndexBucket> buckets;
    uint32_t groupCount;
    RecordInfo lastRecord;
    // TS3W.exe 0x004AA050: sum stored sizes, including all overflow rows even if unused.
    uint32_t TotalSize() const;
    // TS3W.exe 0x004AA090: maximum wrapped unsigned offset plus stored size.
    uint32_t MaxEnd() const;
    // TS3W.exe 0x004AA0D0: test containment in an unsigned half-open range.
    bool Contains(uint32_t start, uint32_t end) const;
    // TS3W.exe 0x004AA140: true if a region intersects no nonempty record.
    bool Disjoint(uint32_t start, uint32_t size) const;
    // TS3W.exe 0x004AA1B0: coalesced hash lookup followed by instance lower_bound.
    const IndexRecord* Find(const ResourceKey* key) const;
    // TS3W.exe 0x004AA470: populate and return the object's cached record info.
    RecordInfo* GetRecord(const void* key);
    // TS3W.exe 0x004AAF50: initialize all tables, leaving the trailing padding untouched.
    Index* Initialize(void* core);
    // TS3W.exe 0x004AAFD0: return emptiness in AL, upper EAX is unspecified.
    bool Empty() const;
    // TS3W.exe 0x004AAFE0: signed byte-span divided by record stride.
    int32_t Count() const;
    // TS3W.exe 0x004AAFF0: destroy buffers in buckets/overflow/records order.
    void Destroy();
    // TS3W.exe 0x004AB5A0: scalar deleting destructor; only flags bit zero matters.
    Index* Delete(uint32_t flags);
    // TS3W.exe 0x004AC9C0: append matching keys in bucket order; filter returns AL.
    uint32_t Matches(KeyVector* out, void* filter);
    // TS3W.exe 0x004ACBB0: append all keys in unstable unsigned offset order.
    uint32_t SortedEntries(KeyVector* out);
    // TS3W.exe 0x004ACF10: validate bounds and mark nonempty records in a free-space map.
    bool MarkUsed(void* map, uint32_t start, uint32_t end) const;
    // TS3W.exe 0x004AD020: parse DBPF index and build coalesced type/group chains.
    bool Load(const uint32_t* data, uint32_t bytes, uint32_t unused);
};
TS3_X86_SIZE(Allocation, 12);
TS3_X86_SIZE(SortedEntry, 8);
TS3_X86_SIZE(RecordVector, 24);
TS3_X86_SIZE(GroupNode, 36);
TS3_X86_OFFSET(GroupNode, next, 32);
TS3_X86_SIZE(GroupMap, 40);
TS3_X86_OFFSET(GroupMap, allocation, 28);
TS3_X86_SIZE(ChainList, 20);
TS3_X86_SIZE(Link, 12);
TS3_X86_OFFSET(Index, records, 8);
TS3_X86_OFFSET(Index, overflow, 32);
TS3_X86_OFFSET(Index, buckets, 56);
TS3_X86_OFFSET(Index, groupCount, 80);
TS3_X86_OFFSET(Index, lastRecord, 84);
TS3_X86_SIZE(Index, 100);
}
