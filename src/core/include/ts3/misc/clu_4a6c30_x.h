#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::misc {
struct ResourceSortKey { uint32_t type, group, low, high; };
TS3_X86_SIZE(ResourceSortKey, 16);
TS3_X86_OFFSET(ResourceSortKey, high, 12);
struct ResourceKeyTarget {
    uint32_t prefix[2]; ResourceSortKey key;
    // TS3W.exe 0x004AF0E0: copy four words in order, returning the last word.
    uint32_t AssignKey(const void* source);
};
TS3_X86_OFFSET(ResourceKeyTarget, key, 8);
// TS3W.exe 0x004AF530: guarded insertion sort, returning the last insertion position.
ResourceSortKey* KeyInsertionSort(ResourceSortKey* first, ResourceSortKey* last);
// TS3W.exe 0x004AF5E0: unguarded insertion sort; first[-1] must be a minimum sentinel.
void KeyUnguardedInsertion(ResourceSortKey* first, ResourceSortKey* last);
// TS3W.exe 0x004AF670: find the first adjacent equal pair, or last.
ResourceSortKey* KeyAdjacentFind(ResourceSortKey* first, ResourceSortKey* last);
// TS3W.exe 0x004AF6B0: return the median of three keys, preserving tie selection.
ResourceSortKey* KeyMedian(ResourceSortKey* a, ResourceSortKey* b, ResourceSortKey* c);
// TS3W.exe 0x004AF7A0: sift a saved key upward through a max heap.
uint32_t KeyPushHeap(ResourceSortKey* base, int32_t top, int32_t hole, const ResourceSortKey* value);
// TS3W.exe 0x004AF840: compact adjacent duplicates and return the new end.
ResourceSortKey* KeyUnique(ResourceSortKey* first, ResourceSortKey* last);
// TS3W.exe 0x004AF8B0: move a heap hole down, then sift the saved key upward.
uint32_t KeyAdjustHeap(ResourceSortKey* base, int32_t top, int32_t count, int32_t hole, const ResourceSortKey* value);
// TS3W.exe 0x004AF970: unguarded partition around a copied pivot.
ResourceSortKey* KeyPartition(ResourceSortKey* first, ResourceSortKey* last, const ResourceSortKey* pivot);
// TS3W.exe 0x004AFA40: build a max heap (return register unspecified for fewer than two keys).
void KeyMakeHeap(ResourceSortKey* first, ResourceSortKey* last);
// TS3W.exe 0x004AFAB0: pop the max heap into ascending order, returning remaining count.
int32_t KeySortHeap(ResourceSortKey* first, ResourceSortKey* last);
// TS3W.exe 0x004AFB30: select and sort the smallest prefix using a max heap.
int32_t KeyPartialSort(ResourceSortKey* first, ResourceSortKey* middle, ResourceSortKey* last);
// TS3W.exe 0x004AFBE0: introsort partition loop, stopping at 28 keys unless depth is zero.
int32_t KeyIntroLoop(ResourceSortKey* first, ResourceSortKey* last, int32_t depth);
struct ResourceKeyVector { ResourceSortKey *first, *last, *capacity; };
TS3_X86_OFFSET(ResourceKeyVector, last, 4);
// TS3W.exe 0x004AFCD0: sort and deduplicate a key vector; dependencies use the game bridge.
int32_t KeySortUnique(ResourceKeyVector* vector);
// TS3W.exe 0x004B3740: interpolate packed bytes with truncation and wrapping, not saturation.
uint32_t InterpolatePackedColor(uint32_t a, uint32_t b, float fraction);
}

namespace ts3::misc {
// TS3W.exe 0x004AEC10: recognize a terminal 8/16-digit hexadecimal name suffix.
const char16_t* FindHexSuffix(const char16_t* first, const char16_t* last, uint8_t underscore, uint8_t allow64);
// TS3W.exe 0x004AEB70: fold the high byte of the game's wide hash into its low 24 bits.
uint32_t FoldedWideHash(const char16_t* text, uint32_t length);
// TS3W.exe 0x004AECC0: query the interface at object + one pointer, or return null.
void* QueryResourceInterface(void* object);
// TS3W.exe 0x004AF200: parse type:group:instance, including the legacy apostrophe syntax.
uint8_t ParseResourceKey(const char16_t* text, ResourceSortKey* output);
struct ResourceVisitorTarget {
    // TS3W.exe 0x004B40B0: dispatch visitor slot 9 with this as the argument.
    int32_t VisitSlot9(void* visitor);
    // TS3W.exe 0x004B4110: dispatch visitor slot 10 with this as the argument.
    int32_t VisitSlot10(void* visitor);
};
struct ResourceVisitLink { ResourceVisitLink *next, *previous; void* item; };
struct ResourceVisitContainer {
    uint8_t prefix[36]; ResourceVisitLink sentinel;
    // TS3W.exe 0x004B41E0: begin, visit children until false, then always end.
    int32_t VisitChildren(void* visitor);
};
TS3_X86_OFFSET(ResourceVisitContainer, sentinel, 36);
TS3_X86_OFFSET(ResourceVisitLink, item, 8);
struct ResourceTreeNode {
    ResourceTreeNode *right, *left, *parent;
    uint8_t black; uint8_t padding[3];
};
TS3_X86_SIZE(ResourceTreeNode, 16);
TS3_X86_OFFSET(ResourceTreeNode, black, 12);
// TS3W.exe 0x004E0BA0: rotate a right child upward and return the possibly changed root.
ResourceTreeNode* RotateResourceRight(ResourceTreeNode* node, ResourceTreeNode* root);
// TS3W.exe 0x004E0BF0: rotate a left child upward and return the possibly changed root.
ResourceTreeNode* RotateResourceLeft(ResourceTreeNode* node, ResourceTreeNode* root);
}

#include "ts3/resource/packed_file.h"
namespace ts3::misc {
struct PackedFileQueries : resource::DatabasePackedFile {
    // TS3W.exe 0x004A6C30: query index slot 8 under the record mutex, or zero.
    uint32_t QueryIndex(uint32_t argument);
    // TS3W.exe 0x004A6EE0: return the largest allocatable free block under the record mutex.
    uint32_t LargestFreeBlock();
};
struct FreeSpaceQueries : resource::FreeSpaceMap {
    // TS3W.exe 0x004AA510: largest clipped free block or signed-total tail; wrapping addition kept.
    uint32_t LargestBlock();
};
struct ResourceMapValue { ResourceSortKey key; void* record; uint32_t padding; };
struct ResourceMapNode { ResourceTreeNode links; ResourceMapValue value; };
TS3_X86_OFFSET(ResourceMapNode, value, 16);
TS3_X86_SIZE(ResourceMapNode, 40);
struct ResourceNodeMap {
    uint32_t comparator; ResourceTreeNode anchor; uint32_t size;
    void* allocator; const char* name; uint32_t flags;
    // TS3W.exe 0x004A76A0: recursively destroy right subtrees and iterate left links (40-byte nodes).
    void Destroy40(ResourceTreeNode* node);
    // TS3W.exe 0x004B0B00: same destruction traversal for 32-byte nodes.
    void Destroy32(ResourceTreeNode* node);
    // TS3W.exe 0x004A8400: erase and free a node, returning its successor through the iterator.
    ResourceTreeNode** Erase(ResourceTreeNode** output, ResourceTreeNode* node);
    // TS3W.exe 0x004A8C40: allocate, copy value, and link one node, preserving allocator argument order.
    ResourceTreeNode** InsertAt(ResourceTreeNode** output, ResourceTreeNode* parent, const ResourceMapValue* value, uint8_t forceLeft);
    // TS3W.exe 0x004A8F10: find an upper-bound insertion position and insert; final stack argument unused.
    ResourceTreeNode** Insert(ResourceTreeNode** output, const ResourceMapValue* value, uint32_t unused);
};
TS3_X86_OFFSET(ResourceNodeMap, anchor, 4);
TS3_X86_OFFSET(ResourceNodeMap, allocator, 24);
TS3_X86_SIZE(ResourceNodeMap, 36);
// TS3W.exe 0x004E0E20: unlink one red-black node and repair colors and extrema.
ResourceTreeNode* EraseResourceTree(ResourceTreeNode* node, ResourceTreeNode* anchor);
// TS3W.exe 0x004AECE0: construct a resource key from group!name.extension with resolver callbacks.
uint8_t ParseResourceName(ResourceSortKey* output, const char16_t* text, uint32_t type, uint32_t group, void* manager, void* context);
struct ResourceKeyRange { uint32_t untouched0, minType, maxType, minGroup, maxGroup, untouched20; uint64_t minInstance, maxInstance; };
TS3_X86_SIZE(ResourceKeyRange, 40);
// TS3W.exe 0x004AF2E0: parse a range from a pointer-to-string; the caller's pointer is never advanced.
uint8_t ParseResourceRange(const char16_t* const* text, ResourceKeyRange* output);
}
