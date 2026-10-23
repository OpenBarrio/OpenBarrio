#include "ts3/resource/index_modifiable.h"

#include <memory>
#include <new>

#include "ts3/io/memory_stream.h"
#include "ts3/resource/packed_file.h"

namespace ts3::resource {

// TS3W.exe 0x004ABD70. The game's constructor leaves the map's four functor bytes alone
// (they are empty); this one lets EASTL touch them, which it doesn't.
void IndexModifiable::Construct(allocator::ICoreAllocator* coreAllocator)
{
#if defined(_M_IX86)
    vtable = reinterpret_cast<void* const*>(static_cast<uintptr_t>(0x00FB2910));
#else
    vtable = nullptr;
#endif
    allocator = coreAllocator;
    new (&map) IndexMap(GameAllocator("ResourceSystem/ResourceMan/PackedFile/IndexMap", coreAllocator, 0));
}

// TS3W.exe 0x004ABDF0 (the body of the deleting destructor): free the nodes and the bucket
// array, then the destroyed-object vtable the original leaves behind.
void IndexModifiable::Destruct()
{
    std::destroy_at(&map);
#if defined(_M_IX86)
    vtable = reinterpret_cast<void* const*>(static_cast<uintptr_t>(0x00FB84D0));
#endif
}

void IndexModifiable::DeletingDestruct(uint32_t flags)
{
    Destruct();
    if (flags & 1)
        io::EaDelete(this);
}

// TS3W.exe 0x004A68E0 (DatabasePackedFile slot 25): allocate the 0x30-byte object through
// EaNew (0x004E3DB0: 16-byte header holding the allocator, which DeletingDestruct's EaDelete
// reads back) and run the ctor's body on it. Fails to nullptr on a failed Alloc, as the
// original does.
IndexModifiable* CreateIndexModifiable(allocator::ICoreAllocator* allocator)
{
    auto* obj = static_cast<IndexModifiable*>(
        io::EaNew(sizeof(IndexModifiable), allocator, "ResourceSystem/ResourceMan/PackedFile/IndexModifiable"));
    if (!obj)
        return nullptr;
    obj->Construct(allocator);
    return obj;
}

// Virtual slot 2 (0x004ABDC0) and slot 3 (0x00F20C20).
bool IndexModifiable::IsEmpty() const
{
    return map.empty();
}

uint32_t IndexModifiable::Count() const
{
    return static_cast<uint32_t>(map.size());
}

// Virtual slot 4 (0x004ABDD0): the nodes go, the buckets stay.
bool IndexModifiable::Clear()
{
    map.clear();
    return true;
}

// Virtual slot 5 (0x004AAB60): the records' stored sizes, added up.
uint32_t IndexModifiable::TotalStoredSize()
{
    uint32_t total = 0;
    for (auto it = map.begin(); it != map.end(); ++it)
        total += it->second.size;
    return total;
}

// Virtual slot 6 (0x004AAB00): the highest offset + size (end of the topmost region).
uint32_t IndexModifiable::HighestRecordEnd()
{
    uint32_t highest = 0;
    for (auto it = map.begin(); it != map.end(); ++it) {
        const uint32_t end = it->second.offset + it->second.size;
        if (highest < end)
            highest = end;
    }
    return highest;
}

// Virtual slot 7 (0x004AC180): every key the filter accepts, in bucket order. The count is
// returned even for a null vector; a non-null `out` gets the keys appended (the original
// grows it in place, carrying whatever the caller already stored).
uint32_t IndexModifiable::MatchingKeys(IndexKeyVector* out, const IndexKeyFilter* filter)
{
    uint32_t count = 0;
    for (auto it = map.begin(); it != map.end(); ++it) {
        const ResourceKey& key = it->first;
        if (filter && !CallGameVirtual<bool, 1>(filter, &key))
            continue;
        if (out)
            out->push_back(key);
        ++count;
    }
    return count;
}

// ---- the sort behind slot 8 (TS3W.exe 0x004ABF30 and its private helpers) -------------
// The game collects the map's node pointers into a scratch and sorts those slots by the
// node's RecordInfo offset (node + 0x10); a map node's address is its pair's address
// (IndexNodeLayout: the pair comes first), so the map's iterators hand the same pointers
// over. The scratch is heap-local, not on-disk format: 4-byte slots on x86 (byte-identical
// to the original), pointer-sized here — the comparisons, the recursion and every tie
// behave exactly as the binary does (third_party's ea_EASTL introsort is a later
// generation and does NOT match it).
namespace {

using SortNode = const IndexMap::value_type*;

// 0x004A9C50: insertion sort bounded by `first` — stable, the game's `jae` stops ties.
void InsertionSortNodes(SortNode* first, SortNode* last)
{
    if (first == last)
        return;
    for (SortNode* next = first + 1; next != last; ++next) {
        SortNode cur = *next;
        SortNode* it = next;
        while (it != first) {
            const SortNode prev = *(it - 1);
            if (!(cur->second.offset < prev->second.offset))
                break;
            *it = prev;
            --it;
        }
        *it = cur;
    }
}

// 0x004A9CA0: the same without the `first` bound; the caller guarantees the element below
// `first` is already <= every element of the range, so the underflow read is in-bounds.
void UnguardedInsertionSortNodes(SortNode* first, SortNode* last)
{
    for (SortNode* it = first; it != last; ++it) {
        SortNode cur = *it;
        for (;;) {
            const SortNode prev = *(it - 1);
            if (!(cur->second.offset < prev->second.offset))
                break;
            *it = prev;
            --it;
        }
        *it = cur;
    }
}

// 0x004A9F10: the children have been bubbled into the hole; walk the held (top) value back
// up while a parent is strictly smaller (ties keep the parent where it is, the game's jae).
void PlaceHeldNode(SortNode* first, size_t limit, size_t hole, const SortNode* top)
{
    if (hole <= limit) {
        first[hole] = *top;
        return;
    }
    for (;;) {
        const size_t parent = (hole - 1) / 2;
        const SortNode parentNode = first[parent];
        if (!(parentNode->second.offset < (*top)->second.offset))
            break;
        first[hole] = parentNode;
        hole = parent;
        if (hole <= limit) {
            first[hole] = *top;
            return;
        }
    }
    first[hole] = *top;
}

// 0x004AA2D0: bubble the larger of the two children up into the hole (a max-heap by the
// record offset; a tie keeps the right child, the game's `jae`), then PlaceHeldNode.
void AdjustHeapNodes(SortNode* first, size_t holeIndex, size_t length, const SortNode* top)
{
    size_t hole = holeIndex;
    size_t child = 2 * hole + 2;
    while (child < length) {
        if (first[child]->second.offset < first[child - 1]->second.offset)
            --child;
        first[hole] = first[child];
        hole = child;
        child = 2 * hole + 2;
    }
    if (child == length) {  // exactly one child: the left one at child - 1
        first[hole] = first[child - 1];
        hole = child - 1;
    }
    PlaceHeldNode(first, holeIndex, hole, top);
}

// 0x004AA860: heapify every parent, the last one first down to the root.
void MakeHeapNodes(SortNode* first, SortNode* last)
{
    const auto n = static_cast<size_t>(last - first);
    if (n < 2)
        return;
    for (size_t i = ((n - 2) >> 1) + 1; i > 0;) {
        --i;
        const SortNode top = first[i];
        AdjustHeapNodes(first, i, n, &top);
    }
}

// 0x004AA8B0: move the root to the end and sift the value that took its place back up,
// until one element is left at `first` (the original's second round on the last pair).
void SortHeapNodes(SortNode* first, SortNode* last)
{
    if (last - first <= 1)
        return;
    for (SortNode* end = last - 1;; --end) {
        const SortNode top = *end;
        *end = *first;
        AdjustHeapNodes(first, 0, static_cast<size_t>(end - first), &top);
        if (end == first)
            return;
    }
}

// 0x004AB280: make_heap + sort_heap (its middle loop is dead — only this caller exists).
void HeapSortNodes(SortNode* first, SortNode* last)
{
    MakeHeapNodes(first, last);
    SortHeapNodes(first, last);
}

// 0x004AB230: the median-of-three partition around the node in *medianSlot; returns the
// cut position (the game's eax): everything before it has offset <= the pivot's, and
// everything from it on offset >= the pivot's.
SortNode* PartitionNodes(SortNode* first, SortNode* last, SortNode* medianSlot)
{
    const SortNode pivot = *medianSlot;
    const uint32_t pivotOff = pivot->second.offset;
    SortNode* left = first;
    SortNode* right = last;
    for (;;) {
        while ((*left)->second.offset < pivotOff)
            ++left;
        do {
            --right;
        } while (pivotOff < (*right)->second.offset);
        if (left >= right)
            return left;
        const SortNode tmp = *left;
        *left = *right;
        *right = tmp;
        ++left;
    }
}

// 0x004AB950: the introsort driver: partition, recurse into the right of the cut, keep
// going on the left while it is larger than 28 elements; a spent depth budget (or a short
// tail left at depth 0) heapsorts the range instead.
void IntroLoopNodes(SortNode* first, SortNode* last, int32_t depthLimit)
{
    while (last - first > 28) {  // the original compares the byte length to 0x70 (28 slots)
        if (depthLimit <= 0) {
            HeapSortNodes(first, last);
            return;
        }
        --depthLimit;
        // median of first, first + n/2 (floor) and last - 1, the original's jae ladder
        const auto n = static_cast<size_t>(last - first);
        SortNode* const mid = first + (n / 2);
        SortNode* const last1 = last - 1;
        const uint32_t firstOff = (*first)->second.offset;
        const uint32_t midOff = (*mid)->second.offset;
        const uint32_t lastOff = (*last1)->second.offset;
        SortNode median;  // the pivot NODE, in a slot the partition takes by pointer
        if (firstOff >= midOff) {
            if (firstOff < lastOff)
                median = *first;
            else
                median = midOff >= lastOff ? *mid : *last1;
        } else {
            if (midOff < lastOff)
                median = *mid;
            else
                median = firstOff >= lastOff ? *first : *last1;
        }
        SortNode* const cut = PartitionNodes(first, last, &median);
        IntroLoopNodes(cut, last, depthLimit);
        last = cut;
    }
    if (depthLimit == 0)
        HeapSortNodes(first, last);
}

// 0x004ABF30: the entry. The depth budget is 2 * (bit width of the count) - 2; then the
// first 28 elements get a guarded insertion sort and the tail an unguarded one (safe: the
// guarded pass has left first[27..0] non-increasing).
void IntroSortNodes(SortNode* first, SortNode* last)
{
    if (first == last)
        return;
    size_t width = 0;
    for (size_t x = static_cast<size_t>(last - first); x != 0; x >>= 1)
        ++width;
    IntroLoopNodes(first, last, static_cast<int32_t>(2 * width - 2));
    if (last - first <= 28) {
        InsertionSortNodes(first, last);
    } else {
        InsertionSortNodes(first, first + 28);
        UnguardedInsertionSortNodes(first + 28, last);
    }
}

}  // namespace

// Virtual slot 8 (0x004AC240): every key ordered by its record's file offset, through the
// sort stack ported above. The scratch of node pointers comes from the object's allocator
// ("ResourceSystem/ResourceMan/PackedFile/IndexPair") and is freed the way the original
// frees it — with the size 0 (unknown).
uint32_t IndexModifiable::SortedEntries(IndexKeyVector* out)
{
    const auto count = static_cast<uint32_t>(map.size());
    auto** nodes = static_cast<SortNode*>(
        allocator->Alloc(count * sizeof(SortNode), "ResourceSystem/ResourceMan/PackedFile/IndexPair", 0));
    uint32_t filled = 0;
    for (auto it = map.begin(); it != map.end(); ++it)
        nodes[filled++] = &*it;
    IntroSortNodes(nodes, nodes + filled);
    for (uint32_t i = 0; i < filled; ++i) {
        if (out)
            out->push_back(nodes[i]->first);
    }
    allocator->Free(nodes, 0);
    return count;
}

// Virtual slot 9 (0x004ACE80): every sized record must fit [headerSize, fileSize) and be
// taken out of the free space; size-0 records are skipped.
bool IndexModifiable::MarkRecordsUsed(FreeSpaceMap* freeSpace, uint32_t headerSize, uint32_t fileSize)
{
    for (auto it = map.begin(); it != map.end(); ++it) {
        const RecordInfo& info = it->second;
        if (!info.size)
            continue;
        if (info.offset < headerSize || info.offset >= fileSize || info.size > fileSize - info.offset)
            return false;
        if (!freeSpace->MarkUsed(static_cast<int32_t>(info.offset), static_cast<int32_t>(info.size)))
            return false;
    }
    return true;
}

// Virtual slot 10 (0x004ABE50): the entry itself, at node + 0x10.
const RecordInfo* IndexModifiable::GetRecord(const ResourceKey* key)
{
    const auto it = map.find(*key);
    if (it == map.end())
        return nullptr;
    return &it->second;
}

// Virtual slot 11 (0x004AC990): find or insert (zero RecordInfo, like the original's
// 0x004AC7B0), then copy the caller's four dwords over it, padding included.
void IndexModifiable::SetRecord(const ResourceKey* key, const RecordInfo* info)
{
    RecordInfo& slot = map[*key];
    std::memcpy(&slot, info, sizeof(RecordInfo));
}

// Virtual slot 12 (0x004ABE90): the entry's four dwords out, then the node away.
bool IndexModifiable::RemoveRecord(const ResourceKey* key, RecordInfo* removed)
{
    const auto it = map.find(*key);
    if (it == map.end())
        return false;
    std::memcpy(removed, &it->second, sizeof(RecordInfo));
    map.erase(it);
    return true;
}

// Virtual slot 13 (0x004AC340): a DBPF index image into the map. (sic) The original has
// no bounds checks: a truncated image keeps reading past its end, so this walks the same
// bytes the same way (the tests feed both sides identical buffers). Records that are
// already in the map stay as they are (the insert at 0x004AB670 keeps the first).
bool IndexModifiable::Load(const void* data, uint32_t size, uint32_t unknown)
{
    auto* words = static_cast<const uint32_t*>(data);
    const uint32_t* const end = words + (size >> 2);
    const uint32_t flags = *words++;
    const bool hasType = (flags & 1) != 0;    // every record's type (key dword 3) is shared
    const bool hasGroup = (flags & 2) != 0;   // group (key dword 4)
    const bool hasHigh = (flags & 4) != 0;    // instanceHigh (key dword 2)
    uint32_t defaultType = 0xFFFFFFFFu, defaultGroup = 0xFFFFFFFFu, defaultHigh = 0xFFFFFFFFu;
    if (hasType)
        defaultType = *words++;
    if (hasGroup)
        defaultGroup = *words++;
    if (hasHigh)
        defaultHigh = *words++;
    while (words < end) {
        ResourceKey key{};
        RecordInfo info{};
        key.type = hasType ? defaultType : *words++;
        key.group = hasGroup ? defaultGroup : *words++;
        key.instanceHigh = hasHigh ? defaultHigh : *words++;
        key.instanceLow = *words++;
        info.offset = *words++;
        uint32_t storedSize = *words++;
        info.memSize = *words++;
        if (storedSize >= 0x80000000u) {
            const uint32_t extra = *words++;
            info.compression = static_cast<uint16_t>(extra);
            info.valid = static_cast<uint8_t>((extra >> 16) & 1);
            storedSize &= 0x7FFFFFFFu;
        } else {
            // (sic) a legacy image without the compression word: infer it, valid stays 0.
            info.compression = static_cast<uint16_t>(info.memSize != storedSize ? 0xFFFF : 0);
            info.valid = 0;
        }
        info.size = storedSize;
        if (unknown)
            info.valid = 1;
        map.insert(IndexMap::value_type(key, info));
    }
    return true;
}

// Virtual slot 14 (0x004AABB0): the map as a DBPF index image — Load's inverse. The
// record dwords are [type][group][instanceHigh] only where the flag bit is clear, then
// instanceLow, offset, size | 0x80000000, memSize and the compression/valid word.
bool IndexModifiable::Save(void** dataOut, uint32_t* sizeOut, uint32_t unknown)
{
    // The defaults are the FIRST record's fields; the flag bits say which fields every
    // record shares (an empty map shares nothing, the original's flags all false).
    bool allSameType = false, allSameGroup = false, allSameHigh = false;
    uint32_t defaultType = 0, defaultGroup = 0, defaultHigh = 0;
    auto it = map.begin();
    if (it != map.end()) {
        allSameType = allSameGroup = allSameHigh = true;
        defaultType = it->first.type;
        defaultGroup = it->first.group;
        defaultHigh = it->first.instanceHigh;
        for (; it != map.end(); ++it) {
            if (allSameType && it->first.type != defaultType)
                allSameType = false;
            if (allSameGroup && it->first.group != defaultGroup)
                allSameGroup = false;
            if (allSameHigh && it->first.instanceHigh != defaultHigh)
                allSameHigh = false;
        }
    }
    uint32_t recordBytes = 32;
    uint32_t imageSize = 4;
    if (allSameType) {
        imageSize = 8;
        recordBytes = 28;
    }
    if (allSameGroup) {
        imageSize += 4;
        recordBytes -= 4;
    }
    if (allSameHigh) {
        imageSize += 4;
        recordBytes -= 4;
    }
    imageSize += static_cast<uint32_t>(map.size()) * recordBytes;
    *sizeOut = imageSize;
    auto* image = static_cast<uint8_t*>(allocator->Alloc(imageSize, "ResourceLoad/PackedFile/IndexImage", 0));
    *dataOut = image;
    if (!image)
        return false;
    auto* out = reinterpret_cast<uint32_t*>(image);
    *out++ = (allSameType ? 1u : 0u) | (allSameGroup ? 2u : 0u) | (allSameHigh ? 4u : 0u);
    if (allSameType)
        *out++ = defaultType;
    if (allSameGroup)
        *out++ = defaultGroup;
    if (allSameHigh)
        *out++ = defaultHigh;
    for (it = map.begin(); it != map.end(); ++it) {
        const ResourceKey& key = it->first;
        const RecordInfo& info = it->second;
        if (!allSameType)
            *out++ = key.type;
        if (!allSameGroup)
            *out++ = key.group;
        if (!allSameHigh)
            *out++ = key.instanceHigh;
        *out++ = key.instanceLow;
        *out++ = info.offset;
        *out++ = info.size | 0x80000000u;
        *out++ = info.memSize;
        *out++ = static_cast<uint32_t>(info.compression) | ((info.valid || unknown) ? 0x10000u : 0u);
    }
    return true;
}

// Virtual slot 15 (0x004AAE40): sized records must lie in [start, fileSize).
bool IndexModifiable::Validate(uint32_t start, uint32_t fileSize)
{
    for (auto it = map.begin(); it != map.end(); ++it) {
        const RecordInfo& info = it->second;
        if (!info.size)
            continue;
        if (info.offset - start >= fileSize - start || info.size > fileSize - info.offset)
            return false;
    }
    return true;
}

// Virtual slot 16 (0x004AAED0): no record may overlap [start, start + length).
bool IndexModifiable::NoRecordOverlaps(uint32_t start, uint32_t length)
{
    if (!length)
        return true;
    for (auto it = map.begin(); it != map.end(); ++it) {
        const RecordInfo& info = it->second;
        if (!info.size)
            continue;
        if (info.offset < start + length && start < info.size + info.offset)
            return false;
    }
    return true;
}

}  // namespace ts3::resource
