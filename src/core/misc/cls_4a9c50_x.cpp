// Unit cls_4a9c50_x: the four node-level helpers of IndexModifiable (TS3W.exe vtable
// 0x00FB2910) whose per-function rewrites live here. The class's vtable slots are
// reconstructed in src/core/resource/index_modifiable.cpp (verified earlier); see the
// header for why this file exists and what is and is not duplicated.
#include "ts3/misc/cls_4a9c50_x.h"

namespace ts3::misc {

namespace {

using ts3::resource::IndexMapLayout;
using ts3::resource::IndexNodeLayout;

// The node size the game's map allocator is called with: 0x28 (a 0x20-byte key+RecordInfo
// pair and the chain pointer), hard-coded in the original as a constant argument.
constexpr size_t kIndexNodeBytes = 0x28;
#if defined(_M_IX86)
static_assert(sizeof(IndexNodeLayout) == kIndexNodeBytes, "the game's map nodes are 0x28 bytes");
#endif

// What both sort passes compare: the node's RecordInfo.offset, at +0x10, unsigned (the
// original's jae/jb).
inline uint32_t NodeOffset(const void* node)
{
    return reinterpret_cast<const IndexNodeLayout*>(node)->value.offset;
}

inline void* NodeNext(const void* node)
{
    return reinterpret_cast<const IndexNodeLayout*>(node)->next;
}

inline void SetNodeNext(void* node, void* next)
{
    reinterpret_cast<IndexNodeLayout*>(node)->next = next;
}

}  // namespace

// TS3W.exe 0x004A9C50.
void SortIndexNodeRange(void** first, void** last)
{
    if (first == last)
        return;
    for (void** it = first + 1; it != last; ++it) {
        void* const current = *it;
        void** hole = it;
        while (hole != first) {
            void* const previous = *(hole - 1);
            if (NodeOffset(current) >= NodeOffset(previous))
                break;
            *hole = previous;
            --hole;
        }
        *hole = current;
    }
}

// TS3W.exe 0x004A9CA0. The original's do/while reads *(first - 1) before the first
// comparison; the guarded pass before it has left that slot small enough not to move.
void SortIndexNodeRangeUnguarded(void** first, void** last)
{
    for (void** it = first; it != last; ++it) {
        void* const current = *it;
        void** hole = it;
        while (NodeOffset(current) < NodeOffset(*(hole - 1))) {
            *hole = *(hole - 1);
            --hole;
        }
        *hole = current;
    }
}

// TS3W.exe 0x004AA280.
void FreeIndexMapNodes(IndexMapLayout* map, void** bucketArray, uint32_t bucketCount)
{
    for (uint32_t bucket = 0; bucket < bucketCount; ++bucket) {
        void* node = bucketArray[bucket];
        while (node) {
            void* const next = NodeNext(node);
            map->allocator.core_allocator()->Free(node, kIndexNodeBytes);
            node = next;
        }
        bucketArray[bucket] = nullptr;
    }
}

// TS3W.exe 0x004AA570.
void** EraseIndexMapNode(IndexMapLayout* map, void** outIterator, void* node, void** bucketSlot)
{
    void* next = NodeNext(node);
    outIterator[1] = bucketSlot;
    outIterator[0] = next;
    if (!next) {
        do {
            outIterator[1] = static_cast<void**>(outIterator[1]) + 1;
            next = *static_cast<void**>(outIterator[1]);
            outIterator[0] = next;
        } while (!next);
    }

    void* const head = *bucketSlot;
    if (head == node) {
        *bucketSlot = NodeNext(head);
    } else {
        void* previous = head;
        while (NodeNext(previous) != node)
            previous = NodeNext(previous);
        SetNodeNext(previous, NodeNext(node));
    }

    map->allocator.core_allocator()->Free(node, kIndexNodeBytes);
    --map->elementCount;
    return outIterator;
}

}  // namespace ts3::misc