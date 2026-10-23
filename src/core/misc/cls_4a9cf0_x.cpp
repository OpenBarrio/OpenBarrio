#include "ts3/misc/cls_4a9cf0_x.h"
#include "ts3/io/memory_stream.h"
#include <algorithm>
#include <cstring>
#include "ts3/misc/clu_4a9e90_x.h"
#include "ts3/resourcesystem/clu_4a0a80_ResourceSystem.h"
#include <vector>

namespace ts3::misc::compressed {
namespace {
const char* Name(unsigned address, const char* portable)
{
#if defined(_M_IX86)
    (void)portable;
    return reinterpret_cast<const char*>(static_cast<uintptr_t>(address));
#else
    (void)address;
    return portable;
#endif
}
void* const* Table(unsigned address)
{
#if defined(_M_IX86)
    return reinterpret_cast<void* const*>(static_cast<uintptr_t>(address));
#else
    (void)address;
    return nullptr; // No x86 game vtable in a standalone x64 process.
#endif
}
const char* TemporaryName() { return Name(0xFB2884, "ResourceLoad/PackedFile/IndexCompressed"); }
ICoreAllocator* DefaultAllocator() { return CallGameFunction<ICoreAllocator*, 0x931020>(); }
void* Allocate(const Allocation& a, size_t bytes) { return a.core->Alloc(bytes, a.name, a.flags); }
template<class T> void Release(Vector<T>& v)
{
    if (v.begin) v.allocation.core->Free(v.begin, v.Capacity() * sizeof(T));
}
uint32_t StoredSize(const Index& index, const IndexRecord& record)
{
    return record.size16 == 0xFFFF ? index.overflow.begin[record.memSize16].size : record.size16;
}
bool LessRecord(const IndexRecord& a, const IndexRecord& b)
{
    return a.instanceHigh < b.instanceHigh || (a.instanceHigh == b.instanceHigh && a.instanceLow < b.instanceLow);
}
// The dependencies are distinct units; leave their exact allocation/copy policies in the game.
void AppendPair(PairVector& v, SortedEntry value)
{
    if (v.end == v.capacity) {
#if defined(_M_IX86)
        reinterpret_cast<resourcesystem::PairVector*>(&v)->Insert(
            reinterpret_cast<resourcesystem::ResourcePair*>(v.end), reinterpret_cast<const resourcesystem::ResourcePair*>(&value));
#else
        const size_t size = v.Size();
        v.Reserve(static_cast<uint32_t>(size ? size * 2 : 1));
        *v.end++ = value;
#endif
    } else *v.end++ = value;
}
void AppendOverflow(Vector<IndexSizeOverflow>& v, IndexSizeOverflow value)
{
    if (v.end == v.capacity) {
#if defined(_M_IX86)
        reinterpret_cast<resourcesystem::PairVector*>(&v)->Insert(
            reinterpret_cast<resourcesystem::ResourcePair*>(v.end), reinterpret_cast<const resourcesystem::ResourcePair*>(&value));
#else
        const size_t size = v.Size(), capacity = size ? size * 2 : 1;
        auto storage = static_cast<IndexSizeOverflow*>(Allocate(v.allocation, capacity * sizeof(IndexSizeOverflow)));
        if (size) std::memcpy(storage, v.begin, size * sizeof(IndexSizeOverflow));
        Release(v); v.begin = storage; v.end = storage + size; v.capacity = storage + capacity;
        *v.end++ = value;
#endif
    }
    else *v.end++ = value;
}
void CopyRecords(IndexRecord** out, const IndexRecord* first, const IndexRecord* last, IndexRecord* destination)
{
    CallGameFunction<void, 0xAB5BA0>(out, first, last, destination);
}
void FillRecords(IndexRecord* first, uint32_t count, const IndexRecord* value)
{
    CallGameFunction<void, 0xAB5B60>(first, count, value);
}
void AppendKeys(KeyVector* out, const PairVector& pairs, uint32_t count)
{
    const size_t old = out->Size();
    CallGameMethod<void, 0x45A5A0>(out, static_cast<uint32_t>(old + count));
    ResourceKey* destination = out->begin + old;
    for (auto p = pairs.begin; p != pairs.end; ++p, ++destination)
        *destination = {p->record->instanceLow, p->record->instanceHigh, p->bucket->type, p->bucket->group};
}
void FreeList(ChainList& list)
{
    auto sentinel = reinterpret_cast<Link*>(&list);
    for (auto node = list.next; node != sentinel;) {
        auto old = node;
        node = node->next;
        list.allocation.core->Free(old, sizeof(Link));
    }
}
}

// 0x004A9CF0 (cdecl): stable insertion prefix, unsigned file offsets.
void InsertSorted(SortedEntry* first, SortedEntry* last)
{
    if (first == last) return;
    for (auto p = first + 1; p != last; ++p) {
        auto value = *p;
        auto hole = p;
        while (hole != first && value.record->offset < hole[-1].record->offset) {
            *hole = hole[-1]; --hole;
        }
        *hole = value;
    }
}
// 0x004A9D50 (cdecl, NOT usercall): EAX on empty input is unspecified.
void InsertSortedTail(SortedEntry* first, SortedEntry* last)
{
    for (auto p = first; p != last; ++p) {
        auto value = *p;
        auto hole = p;
        while (value.record->offset < hole[-1].record->offset) {
            *hole = hole[-1]; --hole;
        }
        *hole = value;
    }
}
// 0x004A9DB0 (cdecl): retain the final insertion-hole return, including end on n <= 1.
IndexRecord* InsertRecords(IndexRecord* first, IndexRecord* last)
{
    auto result = last;
    if (first != last) for (auto p = first + 1; p != last; ++p) {
        auto value = *p;
        result = p;
        while (result != first && LessRecord(value, result[-1])) {
            *result = result[-1]; --result;
        }
        *result = value;
    }
    return result;
}
// 0x004A9E20 (cdecl): no lower bound check, caller supplies a sorted prefix.
void InsertRecordsTail(IndexRecord* first, IndexRecord* last)
{
    for (auto p = first; p != last; ++p) {
        auto value = *p;
        auto hole = p;
        while (LessRecord(value, hole[-1])) { *hole = hole[-1]; --hole; }
        *hole = value;
    }
}
// 0x004ABFB0 (cdecl): quicksort dependency owns partition/tie order.
void SortEntries(SortedEntry* first, SortedEntry* last, uint32_t predicate)
{
    if (first == last) return;
    size_t n = last - first;
    int depth = 0;
    for (auto k = n; k; k >>= 1) ++depth;
    // The verified dependency stores a 32-bit payload. Adapt it instead of truncating
    // the bucket pointers on x64; payload is an index into the original pairs.
    std::vector<SortedEntry> original(first, last);
    std::vector<clu4a9e90::SortedEntry> adapted(n);
    for (size_t i = 0; i < n; ++i) adapted[i] = {
        reinterpret_cast<clu4a9e90::SortedKey*>(const_cast<IndexRecord*>(first[i].record)), static_cast<uint32_t>(i)};
    clu4a9e90::QuickSorted(adapted.data(), adapted.data() + n, 2 * depth - 2, predicate);
    for (size_t i = 0; i < n; ++i) first[i] = original[adapted[i].payload];
    if (n <= 28) InsertSorted(first, last);
    else { InsertSorted(first, first + 28); InsertSortedTail(first + 28, last); }
}
// 0x004AC100 (cdecl).
void SortRecords(IndexRecord* first, IndexRecord* last, uint32_t predicate)
{
    if (first == last) return;
    size_t n = last - first;
    int depth = 0;
    for (auto k = n; k; k >>= 1) ++depth;
    clu4a9e90::QuickRecord(reinterpret_cast<clu4a9e90::Record*>(first), reinterpret_cast<clu4a9e90::Record*>(last), 2 * depth - 2, predicate);
    if (n <= 28) InsertRecords(first, last);
    else { InsertRecords(first, first + 28); InsertRecordsTail(first + 28, last); }
}
// 0x004AA050 (thiscall).
uint32_t Index::TotalSize() const
{
    uint32_t sum = 0;
    for (auto p = records.begin; p != records.end; ++p) if (p->size16 != 0xFFFF) sum += p->size16;
    // EA counts every overflow row, not just the rows referenced by records.
    for (auto p = overflow.begin; p != overflow.end; ++p) sum += p->size;
    return sum;
}
// 0x004AA090 (thiscall).
uint32_t Index::MaxEnd() const
{
    uint32_t result = 0;
    for (auto p = records.begin; p != records.end; ++p) result = std::max(result, p->offset + StoredSize(*this, *p));
    return result;
}
// 0x004AA0D0 (thiscall, ret 8); all arithmetic wraps at 32 bits.
bool Index::Contains(uint32_t start, uint32_t end) const
{
    for (auto p = records.begin; p != records.end; ++p) {
        auto size = StoredSize(*this, *p);
        if (size && (p->offset - start >= end - start || size > end - p->offset)) return false;
    }
    return true;
}
// 0x004AA140 (thiscall, ret 8).
bool Index::Disjoint(uint32_t start, uint32_t size) const
{
    if (!size) return true;
    for (auto p = records.begin; p != records.end; ++p) {
        auto length = StoredSize(*this, *p);
        if (length && p->offset < start + size && start < length + p->offset) return false;
    }
    return true;
}
// 0x004AA1B0 (thiscall, ret 4).
const IndexRecord* Index::Find(const ResourceKey* key) const
{
    if (!groupCount) return nullptr;
    auto bucket = (key->type + key->group) % groupCount;
    if (static_cast<int32_t>(bucket) < 0) return nullptr;
    while (buckets.begin[bucket].type != key->type || buckets.begin[bucket].group != key->group) {
        bucket = buckets.begin[bucket].next;
        if (static_cast<int32_t>(bucket) < 0) return nullptr;
    }
    uint32_t lo = buckets.begin[bucket].firstRecord;
    uint32_t end = buckets.begin[bucket + 1].firstRecord;
    uint32_t hi = end - 1;
    while (lo < hi) {
        auto mid = (hi + lo) >> 1;
        const auto& r = records.begin[mid];
        if (r.instanceHigh > key->instanceHigh || (r.instanceHigh == key->instanceHigh && r.instanceLow >= key->instanceLow)) hi = mid;
        else lo = mid + 1;
    }
    if (hi >= end) return nullptr;
    const auto* r = records.begin + hi;
    return r->instanceLow == key->instanceLow && r->instanceHigh == key->instanceHigh ? r : nullptr;
}
// 0x004AA470 (thiscall, ret 4).
RecordInfo* Index::GetRecord(const void* key)
{
    auto r = Find(static_cast<const ResourceKey*>(key));
    if (!r) return nullptr;
    lastRecord.offset = r->offset;
    lastRecord.valid = 1;
    if (r->size16 == 0xFFFF) {
        lastRecord.size = overflow.begin[r->memSize16].size;
        lastRecord.memSize = overflow.begin[r->memSize16].memSize;
    } else {
        lastRecord.size = r->size16;
        lastRecord.memSize = r->memSize16;
    }
    lastRecord.compression = lastRecord.size != lastRecord.memSize ? 0xFFFF : 0;
    return &lastRecord;
}
// 0x004AAF50 (thiscall, ret 4): write only the original's fields, not padding.
Index* Index::Initialize(void* supplied)
{
    auto core = static_cast<ICoreAllocator*>(supplied);
    allocator = core;
    vtable = Table(0xFB2840);
    records.begin = records.end = records.capacity = nullptr;
    records.allocation = {core, 1, Name(0xFB27F8, "ResourceSystem/ResourceMan/PackedFile/IndexCompressed/IndexTable")};
    overflow.begin = overflow.end = overflow.capacity = nullptr;
    overflow.allocation = {core, 1, Name(0xFB27B0, "ResourceSystem/ResourceMan/PackedFile/IndexCompressed/SizeOverflowTable")};
    buckets.begin = buckets.end = buckets.capacity = nullptr;
    buckets.allocation = {core, 1, Name(0xFB2768, "ResourceSystem/ResourceMan/PackedFile/IndexCompressed/TypeGroupTable")};
    groupCount = 0;
    lastRecord.offset = lastRecord.size = lastRecord.memSize = 0;
    lastRecord.compression = 0;
    lastRecord.valid = 0;
    return this;
}
// 0x004AAFD0 (thiscall): only AL is part of the bool contract.
bool Index::Empty() const { return records.begin == records.end; }
// 0x004AAFE0 (thiscall): arithmetic shift, even for a malformed negative span.
int32_t Index::Count() const
{
    auto span = static_cast<int32_t>(reinterpret_cast<uintptr_t>(records.end) - reinterpret_cast<uintptr_t>(records.begin));
    return span >> 4;
}
// 0x004AAFF0 (thiscall): buffers remain dangling, just as in the original destructor.
void Index::Destroy()
{
    Release(buckets);
    Release(overflow);
    Release(records);
    vtable = Table(0xFB84D0);
}
// 0x004AB5A0 (thiscall, ret 4).
Index* Index::Delete(uint32_t flags)
{
    Destroy();
    if (flags & 1) io::EaDelete(this);
    return this;
}
// 0x004AB140 (thiscall, ret 12).
SortedEntry* PairVector::AllocateCopy(uint32_t count, const SortedEntry* first, const SortedEntry* last)
{
    auto result = count ? static_cast<SortedEntry*>(Allocate(allocation, sizeof(SortedEntry) * count)) : nullptr;
    auto destination = result;
    for (; first != last; ++first, ++destination) if (destination) *destination = *first;
    return result;
}
// 0x004AB600 (thiscall, ret 4): EAX is old begin if no growth, otherwise old size.
uintptr_t PairVector::Reserve(uint32_t count)
{
    auto result = reinterpret_cast<uintptr_t>(begin);
    if (count > Capacity()) {
        auto storage = AllocateCopy(count, begin, end);
        const size_t old = Size();
        Release(*this);
        result = old;
        begin = storage;
        end = storage + old;
        capacity = storage + count;
    }
    return result;
}
// 0x004AA620 (thiscall, ret 4); incidental allocator EAX is not a semantic return.
void GroupMap::Rehash(uint32_t count)
{
    auto table = static_cast<GroupNode**>(Allocate(allocation, sizeof(GroupNode*) * (size_t(count) + 1)));
    std::memset(table, 0, sizeof(GroupNode*) * count);
    table[count] = reinterpret_cast<GroupNode*>(~uintptr_t(0));
    for (uint32_t i = 0; i < bucketCount; ++i) while (auto node = buckets[i]) {
        auto hash = (node->type + node->group) % count;
        buckets[i] = node->next;
        node->next = table[hash];
        table[hash] = node;
    }
    if (bucketCount > 1) allocation.core->Free(buckets, sizeof(GroupNode*) * (size_t(bucketCount) + 1));
    buckets = table;
    bucketCount = count;
}
// 0x004ABAD0 (thiscall, ret 4).
GroupNode* GroupMap::NewNode(const uint32_t* key)
{
    auto node = static_cast<GroupNode*>(Allocate(allocation, sizeof(GroupNode)));
    if (node) {
        node->type = key[0]; node->group = key[1];
        node->records.begin = node->records.end = node->records.capacity = nullptr;
        node->records.allocation = {DefaultAllocator(), 0, TemporaryName()};
    }
    node->next = nullptr; // EA deliberately has no usable allocation-failure path.
    return node;
}
// 0x004AC030 (thiscall, ret 12).
InsertResult* GroupMap::Insert(InsertResult* out, const uint32_t* key, uint32_t)
{
    uint32_t hash = key[0] + key[1];
    uint32_t bucket = hash % bucketCount;
    for (auto node = buckets[bucket]; node; node = node->next) if (node->type == key[0] && node->group == key[1]) {
        out->node = node; out->bucket = buckets + bucket; out->inserted = 0;
        return out;
    }
    struct PolicyResult { uint8_t needed; uint8_t padding[3]; uint32_t count; } policy;
    CallGameMethod<void, 0x4E09B0>(&maxLoad, &policy, bucketCount, size, 1u);
    auto node = NewNode(key);
    if (policy.needed) { bucket = hash % policy.count; Rehash(policy.count); }
    node->next = buckets[bucket]; buckets[bucket] = node; ++size;
    out->node = node; out->bucket = buckets + bucket; out->inserted = 1;
    return out;
}
// 0x004AC8B0 (thiscall, ret 8).
void GroupMap::ClearBuckets(GroupNode** table, uint32_t count)
{
    for (uint32_t i = 0; i < count; ++i) {
        for (auto node = table[i]; node;) {
            auto old = node; node = node->next;
            Release(old->records);
            allocation.core->Free(old, sizeof(GroupNode));
        }
        table[i] = nullptr;
    }
}
// 0x004AA6D0 (thiscall, ret 8).
Chains* Chains::Allocate(uint32_t count, const Allocation* source)
{
    allocation = *source;
    begin = count ? static_cast<ChainList*>(compressed::Allocate(allocation, sizeof(ChainList) * count)) : nullptr;
    end = begin;
    capacity = begin + count;
    return this;
}
// 0x004AB540 (thiscall, ret 16).
void ChainList::CopyBefore(Link* destination, const Link* first, const Link* last, uint32_t)
{
    for (auto source = first; source != last; source = source->next) {
        auto node = static_cast<Link*>(Allocate(allocation, sizeof(Link)));
        // The original's placement-new check is node != -8 on x86, not node != null.
        if (reinterpret_cast<uintptr_t>(node) != uintptr_t(0) - offsetof(Link, value)) node->value = source->value;
        node->next = destination;
        node->prev = destination->prev;
        destination->prev->next = node;
        destination->prev = node;
    }
}
// 0x004AC930 (cdecl).
void ConstructLists(ChainList* first, uint32_t count, const ChainList* prototype)
{
    for (uint32_t i = 0; i < count; ++i) if (auto p = first + i) {
        p->allocation = prototype->allocation;
        auto sentinel = reinterpret_cast<Link*>(p);
        p->next = p->prev = sentinel;
        p->CopyBefore(sentinel, prototype->next, reinterpret_cast<const Link*>(prototype), 0);
    }
}
// 0x004ACF80 (thiscall, ret 8).
Chains* Chains::Construct(uint32_t count, const Allocation* source)
{
    Allocate(count, source);
    ChainList prototype;
    prototype.next = prototype.prev = reinterpret_cast<Link*>(&prototype);
    prototype.allocation = {DefaultAllocator(), 0, TemporaryName()};
    ConstructLists(begin, count, &prototype);
    FreeList(prototype);
    end = begin + count;
    return this;
}
// 0x004ABB30 (thiscall, ret 12): exact helper calls and growth policy.
uintptr_t RecordVector::Insert(IndexRecord* where, uint32_t count, const IndexRecord* value)
{
    auto result = reinterpret_cast<uintptr_t>(end);
    if (count > static_cast<size_t>(capacity - end)) {
        const size_t old = Size();
        const size_t grown = std::max(old ? old * 2 : size_t(1), old + count);
        auto storage = grown ? static_cast<IndexRecord*>(Allocate(allocation, grown * sizeof(IndexRecord))) : nullptr;
        auto saved = where;
        CopyRecords(&where, begin, saved, storage);
        FillRecords(where, count, value);
        CopyRecords(&where, saved, end, where + count);
        Release(*this);
        begin = storage; end = where; capacity = storage + grown;
        return reinterpret_cast<uintptr_t>(capacity);
    }
    if (!count) return result;
    auto oldEnd = end;
    const IndexRecord copy = *value; // value may alias this vector.
    size_t tail = oldEnd - where;
    if (count >= tail) {
        FillRecords(oldEnd, count - static_cast<uint32_t>(tail), &copy);
        end += count - tail;
        IndexRecord* ignored;
        CopyRecords(&ignored, where, oldEnd, end);
        end += tail;
        return CallGameFunction<uintptr_t, 0xAB5B30>(where, oldEnd, &copy);
    }
    IndexRecord* ignored;
    CopyRecords(&ignored, oldEnd - count, oldEnd, oldEnd);
    end += count;
    CallGameFunction<void, 0x5F23F0>(where, oldEnd - count, oldEnd);
    return CallGameFunction<uintptr_t, 0xAB5B30>(where, where + count, &copy);
}
// 0x004AC840 (thiscall, ret 4).
uintptr_t RecordVector::Resize(uint32_t count)
{
    size_t size = Size();
    if (count > size) { const IndexRecord zero{}; return Insert(end, count - static_cast<uint32_t>(size), &zero); }
    auto newEnd = begin + count;
    auto result = CallGameFunction<uintptr_t, 0xC60180>(end, end, newEnd);
    end = newEnd;
    return result;
}
// 0x004AC9C0 (thiscall, ret 8).
uint32_t Index::Matches(KeyVector* out, void* filter)
{
    PairVector selected{};
    selected.allocation = {allocator, 0, Name(0xFB2984, "ResourceSystem/ResourceMan/PackedFile/IndexCompressed/Matches")};
    if (out && !filter) selected.Reserve(static_cast<uint32_t>(Count()));
    uint32_t matched = 0;
    for (uint32_t i = 0; i < groupCount; ++i) {
        auto bucket = buckets.begin + i;
        for (auto p = records.begin + bucket->firstRecord; p != records.begin + bucket[1].firstRecord; ++p) {
            ResourceKey key{p->instanceLow, p->instanceHigh, bucket->type, bucket->group};
            if (!filter || CallGameVirtual<uint8_t, 1>(filter, &key)) {
                if (out) AppendPair(selected, {p, bucket});
                ++matched;
            }
        }
    }
    if (out && selected.begin != selected.end) AppendKeys(out, selected, static_cast<uint32_t>(selected.Size()));
    Release(selected);
    return matched;
}
// 0x004ACBB0 (thiscall, ret 4).
uint32_t Index::SortedEntries(KeyVector* out)
{
    uint32_t count = static_cast<uint32_t>(Count());
    if (out && count) {
        PairVector selected{};
        selected.allocation = {allocator, 0, Name(0xFB29C8, "ResourceSystem/ResourceMan/PackedFile/IndexCompressed/SortedEntries")};
        selected.Reserve(count);
        for (uint32_t i = 0; i < groupCount; ++i) {
            auto bucket = buckets.begin + i;
            for (auto p = records.begin + bucket->firstRecord; p != records.begin + bucket[1].firstRecord; ++p) AppendPair(selected, {p, bucket});
        }
        SortEntries(selected.begin, selected.end, 0);
        AppendKeys(out, selected, count);
        Release(selected);
    }
    return count;
}
// 0x004ACF10 (thiscall, ret 12).
bool Index::MarkUsed(void* map, uint32_t start, uint32_t end) const
{
    for (auto p = records.begin; p != records.end; ++p) {
        auto size = StoredSize(*this, *p);
        if (size && (p->offset < start || p->offset >= end || size > end - p->offset ||
                     !CallGameMethod<uint8_t, 0x4ACD70>(map, p->offset, size))) return false;
    }
    return true;
}
// 0x004AD020 (thiscall, ret 12): original temporary maps/lists and allocation order.
bool Index::Load(const uint32_t* data, uint32_t bytes, uint32_t)
{
    auto limit = data + (bytes >> 2);
    uint32_t flags = *data++;
    uint32_t type = flags & 1 ? *data++ : ~0u;
    uint32_t group = flags & 2 ? *data++ : ~0u;
    uint32_t high = flags & 4 ? *data++ : ~0u;
    overflow.end = overflow.begin;
    buckets.end = buckets.begin;
    GroupNode* emptyBuckets[2] = {nullptr, reinterpret_cast<GroupNode*>(~uintptr_t(0))};
    GroupMap map{nullptr, emptyBuckets, 1, 0, 1.0f, 2.0f, 0, {allocator, 0, TemporaryName()}};
    uint32_t total = 0;
    while (data < limit) {
        uint32_t key[2] = {flags & 1 ? type : *data++, flags & 2 ? group : *data++};
        uint32_t instanceHigh = flags & 4 ? high : *data++;
        uint32_t instanceLow = *data++;
        uint32_t offset = *data++;
        uint32_t size = *data++;
        uint32_t memory = *data++;
        if (size & 0x80000000u) { ++data; size &= 0x7FFFFFFF; }
        InsertResult inserted;
        map.Insert(&inserted, key, 0);
        auto& vector = inserted.node->records;
        if (inserted.inserted) vector.allocation = {allocator, 0, TemporaryName()};
        IndexRecord zero{};
        if (vector.end == vector.capacity) CallGameMethod<void, 0x4E8BA0>(&vector, vector.end, &zero);
        else *vector.end++ = zero;
        auto& record = vector.end[-1];
        record.instanceLow = instanceLow; record.instanceHigh = instanceHigh; record.offset = offset;
        if (size >= 0xFFFF || memory >= 0xFFFF) {
            record.size16 = 0xFFFF;
            record.memSize16 = static_cast<uint16_t>(overflow.Size());
            AppendOverflow(overflow, {size, memory});
        } else { record.size16 = static_cast<uint16_t>(size); record.memSize16 = static_cast<uint16_t>(memory); }
        ++total;
    }
    groupCount = map.size;
    // Bucket vector has the same stride as IndexRecord. Use the assigned insertion helper.
    IndexRecord sentinel{0, 0, 0, 0xFFFF, 0xFFFF};
    auto& bucketVector = reinterpret_cast<RecordVector&>(buckets);
    bucketVector.Insert(bucketVector.end, groupCount + 1, &sentinel);
    Allocation source{allocator, 0, TemporaryName()};
    Chains chains;
    chains.Construct(groupCount, &source);
    for (uint32_t i = 0; i < map.bucketCount; ++i) for (auto node = map.buckets[i]; node; node = node->next) {
        auto& list = chains.begin[(node->type + node->group) % groupCount];
        auto destination = reinterpret_cast<Link*>(&list);
        if (list.next == destination) list.allocation = source;
        auto link = static_cast<Link*>(Allocate(list.allocation, sizeof(Link)));
        link->value = node;
        link->next = destination; link->prev = list.prev;
        list.prev->next = link; list.prev = link;
    }
    records.Resize(total);
    auto destination = records.begin;
    uint32_t donor = 0;
    for (uint32_t i = 0; i < groupCount; ++i) {
        auto& list = chains.begin[i];
        auto& bucket = buckets.begin[i];
        bucket.firstRecord = static_cast<uint32_t>(destination - records.begin);
        GroupNode* node;
        if (list.next != reinterpret_cast<Link*>(&list)) node = list.next->value;
        else {
            while (chains.begin[donor].next == reinterpret_cast<Link*>(chains.begin + donor) || chains.begin[donor].next == chains.begin[donor].prev) ++donor;
            auto& from = chains.begin[donor];
            auto link = from.prev;
            node = link->value;
            link->prev->next = link->next;
            link->next->prev = link->prev;
            from.allocation.core->Free(link, sizeof(Link));
            bucket.next = buckets.begin[donor].next;
            buckets.begin[donor].next = i;
        }
        bucket.type = node->type; bucket.group = node->group;
        SortRecords(node->records.begin, node->records.end, 0);
        for (auto p = node->records.begin; p != node->records.end; ++p) *destination++ = *p;
    }
    buckets.end[-1].firstRecord = static_cast<uint32_t>(destination - records.begin);
    for (auto list = chains.begin; list != chains.end; ++list) FreeList(*list);
    Release(chains);
    map.ClearBuckets(map.buckets, map.bucketCount);
    if (map.bucketCount > 1) map.allocation.core->Free(map.buckets, sizeof(GroupNode*) * (size_t(map.bucketCount) + 1));
    return true;
}
}
