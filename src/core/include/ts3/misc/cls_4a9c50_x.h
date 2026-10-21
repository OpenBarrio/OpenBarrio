// Unit cls_4a9c50_x: the private map/sort helpers of IndexModifiable (TS3W.exe vtable
// 0x00FB2910, object 0x30 bytes). The class itself was reconstructed earlier in
// src/core/resource/index_modifiable.cpp on top of the game's own eastl::hash_map; these
// are the four node-level helpers of that same class that had no explicit rewrite yet
// (the other 19 functions of the unit are the vtable slots, verified there):
//   0x004A9C50 / 0x004A9CA0  the introsort's insertion-sort passes (slot 8 uses them)
//   0x004AA280 / 0x004AA570  the map's clear-nodes and erase-node internals (slots 4/12)
// They work on the game's map layout pinned by ts3::resource::IndexMapLayout (bucket
// array of pointer slots, node chains through +0x20, the owning ICoreAllocator at +0x1C)
// and free nodes with the game's hard-coded 40-byte node size. On x86 the layout mirrors
// the game object byte for byte; the helpers are only ever driven with game-built map
// objects.
#pragma once

#include <cstdint>

#include "ts3/resource/index_modifiable.h"

namespace ts3::misc {

// TS3W.exe 0x004A9C50: stable insertion sort of the node-pointer range [first, last) by
// the unsigned dword at node + 0x10 (RecordInfo.offset). The pass is guarded: it never
// reads below `first` (ties keep the earlier node, as the original's unsigned >= test does).
void SortIndexNodeRange(void** first, void** last);

// TS3W.exe 0x004A9CA0: the unguarded variant of the pass above. The caller guarantees the
// slot below `first` is <= every element of the range (the introsort entry sorts the first
// 28 slots guarded and the tail unguarded), so the read of *(first - 1) stays in bounds.
void SortIndexNodeRangeUnguarded(void** first, void** last);

// TS3W.exe 0x004AA280: free every node of every chain in `bucketArray` (bucketCount
// slots) through the map's allocator with the game's node size 40, then zero the slots.
// The original's return value is whatever the last allocator free left in EAX and every
// caller ignores it; nothing is returned here. (EA quirk kept: the frees are not
// null-checked; a chain's next pointer is read before the node is freed.)
void FreeIndexMapNodes(resource::IndexMapLayout* map, void** bucketArray, uint32_t bucketCount);

// TS3W.exe 0x004AA570: unlink `node` from the chain in `bucketSlot`, free it (size 40),
// decrement the map's element count and write the iterator to the next element into
// `outIterator` ({node, bucketSlot}). `bucketSlot` is where the caller's iterator came
// from (the game passes the iterator's own slot); when the erased node ends its chain the
// iterator advances to the next non-empty bucket, reading until a non-null slot (the
// game's map keeps a live end sentinel past the buckets). Returns outIterator (the
// original returns it in EAX).
void** EraseIndexMapNode(resource::IndexMapLayout* map, void** outIterator, void* node, void** bucketSlot);

}  // namespace ts3::misc