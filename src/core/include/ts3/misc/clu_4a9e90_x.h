#pragma once
#include "ts3/layout.h"
#include <cstdint>
#include "ts3/io/memory_stream.h"
#include "ts3/allocator/core_allocator.h"
namespace ts3::misc::clu4a9e90 {
struct PointerKey { uint32_t prefix[4]; uint32_t key; };
struct SortedKey { uint32_t prefix[2]; uint32_t key; };
struct SortedEntry { SortedKey* item; uint32_t payload; };
struct Record { uint64_t key; uint32_t payload[2]; };
TS3_X86_OFFSET(PointerKey, key, 16);
TS3_X86_OFFSET(SortedKey, key, 8);
TS3_X86_OFFSET(SortedEntry, payload, 4);
TS3_X86_SIZE(SortedEntry, 8);
TS3_X86_SIZE(Record, 16);
// TS3W.exe 0x004A9E90: Select the median key, preserving equal-key pointer selection.
const uint64_t* Median64(const uint64_t* a, const uint64_t* b, const uint64_t* c);
// TS3W.exe 0x004A9F10: Push a saved value up the max heap.
PointerKey* PushPointer(PointerKey** a, int top, int hole, PointerKey* const* value, uint32_t comparator);
// TS3W.exe 0x004AA2D0: Restore the heap after removing its root.
PointerKey* AdjustPointer(PointerKey** a, int top, int n, int hole, PointerKey* const* value, uint32_t comparator);
// TS3W.exe 0x004AA860: Build a max heap; empty-span EAX is unspecified.
void MakePointer(PointerKey** a, PointerKey** end, uint32_t comparator);
// TS3W.exe 0x004AA8B0: Sort a max heap, returning the final span count.
int SortPointer(PointerKey** a, PointerKey** end, uint32_t comparator);
// TS3W.exe 0x004AB230: Partition the range around a captured pivot key.
PointerKey** PartitionPointer(PointerKey** a, PointerKey** end, PointerKey* const* pivot, uint32_t comparator);
// TS3W.exe 0x004AB280: Heap-select and sort the prefix.
int PartialPointer(PointerKey** a, PointerKey** middle, PointerKey** end, uint32_t comparator);
// TS3W.exe 0x004AB950: Introsort partition phase; small spans are intentionally not insertion sorted.
int QuickPointer(PointerKey** a, PointerKey** end, int depth, uint32_t comparator);
// TS3W.exe 0x004A9F70: Push a saved value up the max heap.
uint32_t PushSorted(SortedEntry* a, int top, int hole, const SortedEntry* value, uint32_t comparator);
// TS3W.exe 0x004AA340: Restore the heap after removing its root.
uint32_t AdjustSorted(SortedEntry* a, int top, int n, int hole, const SortedEntry* value, uint32_t comparator);
// TS3W.exe 0x004AA910: Build a max heap; empty-span EAX is unspecified.
void MakeSorted(SortedEntry* a, SortedEntry* end, uint32_t comparator);
// TS3W.exe 0x004AA970: Sort a max heap, returning the final span count.
int SortSorted(SortedEntry* a, SortedEntry* end, uint32_t comparator);
// TS3W.exe 0x004AB2F0: Partition the range around a captured pivot key.
SortedEntry* PartitionSorted(SortedEntry* a, SortedEntry* end, const SortedEntry* pivot, uint32_t comparator);
// TS3W.exe 0x004AB360: Heap-select and sort the prefix.
int PartialSorted(SortedEntry* a, SortedEntry* middle, SortedEntry* end, uint32_t comparator);
// TS3W.exe 0x004ABA10: Introsort partition phase; small spans are intentionally not insertion sorted.
int QuickSorted(SortedEntry* a, SortedEntry* end, int depth, uint32_t comparator);
// TS3W.exe 0x004A9FD0: Push a saved value up the max heap.
Record* PushRecord(Record* a, int top, int hole, const Record* value, uint32_t comparator);
// TS3W.exe 0x004AA3C0: Restore the heap after removing its root.
Record* AdjustRecord(Record* a, int top, int n, int hole, const Record* value, uint32_t comparator);
// TS3W.exe 0x004AA9F0: Build a max heap; empty-span EAX is unspecified.
void MakeRecord(Record* a, Record* end, uint32_t comparator);
// TS3W.exe 0x004AAA70: Sort a max heap, returning the final span count.
int SortRecord(Record* a, Record* end, uint32_t comparator);
// TS3W.exe 0x004AB3F0: Partition the range around a captured pivot key.
Record* PartitionRecord(Record* a, Record* end, const Record* pivot, uint32_t comparator);
// TS3W.exe 0x004AB490: Heap-select and sort the prefix.
int PartialRecord(Record* a, Record* middle, Record* end, uint32_t comparator);
// TS3W.exe 0x004ABCE0: Introsort partition phase; small spans are intentionally not insertion sorted.
int QuickRecord(Record* a, Record* end, int depth, uint32_t comparator);
// TS3W.exe 0x007410D0: Push a saved value up the max heap.
uint32_t PushUint64(uint64_t* a, int top, int hole, const uint64_t* value);
// TS3W.exe 0x007411B0: Restore the heap after removing its root.
uint32_t AdjustUint64(uint64_t* a, int top, int n, int hole, const uint64_t* value);
// TS3W.exe 0x007413B0: Build a max heap; empty-span EAX is unspecified.
void MakeUint64(uint64_t* a, uint64_t* end);
// TS3W.exe 0x00741400: Sort a max heap, returning the final span count.
int SortUint64(uint64_t* a, uint64_t* end);
// TS3W.exe 0x00741470: Partition the range around a captured pivot key.
uint64_t* PartitionUint64(uint64_t* a, uint64_t* end, const uint64_t* pivot);
// TS3W.exe 0x007414E0: Heap-select and sort the prefix.
int PartialUint64(uint64_t* a, uint64_t* middle, uint64_t* end);
// TS3W.exe 0x00741560: Introsort partition phase; small spans are intentionally not insertion sorted.
int QuickUint64(uint64_t* a, uint64_t* end, int depth);
// TS3W.exe 0x005C8360: Return the exponent of the rounded unsigned-to-float conversion, including -127 for zero.
int FloatExponent(uint32_t value);

struct HashNode { uint32_t value[8]; HashNode* next; uint32_t unused; };
struct HashInsertResult { HashNode* node; HashNode** bucket; uint8_t inserted; };
TS3_X86_OFFSET(HashInsertResult,inserted,8);
struct HashTable {
    void* vtable; HashNode** buckets; uint32_t bucketCount, count;
    uint32_t policy[3]; ts3::allocator::ICoreAllocator* allocator;
    uint32_t flags; const char* name;
    // TS3W.exe 0x004AA730: Allocate buckets and prepend existing chains, reversing collisions.
    uintptr_t Rehash(uint32_t buckets);
    // TS3W.exe 0x004AB691 (interior +0x21 of 0x004AB670): Unique insertion with prime-policy rehash.
    HashInsertResult* Insert(HashInsertResult* out,const uint32_t* value,uint32_t comparator);
};
TS3_X86_OFFSET(HashNode,next,32); TS3_X86_SIZE(HashNode,40);
TS3_X86_OFFSET(HashTable,allocator,28); TS3_X86_OFFSET(HashTable,name,36);
TS3_X86_SIZE(HashTable,40);
struct TreeValue { int32_t key; uint32_t value; };
struct TreeNode { TreeNode* right; TreeNode* left; TreeNode* parent; uint32_t color; TreeValue value; };
struct InsertResult { TreeNode* node; uint8_t inserted; };
struct Tree {
    void* unused; struct Header { TreeNode* right; TreeNode* left; TreeNode* parent; uint32_t color; } header; uint32_t count; ts3::allocator::ICoreAllocator* allocator;
    // TS3W.exe 0x004AA7E0: Allocate and link a signed-key tree node.
    TreeNode** InsertNode(TreeNode** out,TreeNode* parent,const TreeValue* value,uint8_t forceLeft);
    // TS3W.exe 0x004AB1A0: Insert a unique signed key, returning the existing node on duplicates.
    InsertResult* InsertUnique(InsertResult* out,const TreeValue* value,uint32_t comparator);
    // TS3W.exe 0x004AB890: Insert a signed key using a predecessor hint when valid.
    TreeNode** InsertHint(TreeNode** out,TreeNode* hint,const TreeValue* value,uint32_t comparator);
    // TS3W.exe 0x004ABF00: Forward hinted insertion, returning the output iterator.
    TreeNode** Insert(TreeNode** out,TreeNode* hint,const TreeValue* value);
};
// The embedded header is only the 16-byte link base, NOT a full TreeNode.
TS3_X86_OFFSET(TreeNode,value,16); TS3_X86_SIZE(TreeNode,24);
TS3_X86_OFFSET(Tree,header,4); TS3_X86_OFFSET(Tree,count,20); TS3_X86_OFFSET(Tree,allocator,24);
TS3_X86_SIZE(Tree,28);
struct Stream : ts3::io::MemoryStream {
    // TS3W.exe 0x004ADBB0: Forward to the previously verified buffer-data getter.
    uint8_t* Data() const;
};
TS3_X86_SIZE(Stream,40);
// TS3W.exe 0x0060CF40: Remove leading mip levels, retaining the ignored width-exponent quirk.
uintptr_t TrimMips(void* format,uint32_t* width,uint32_t* height,uint32_t* levels,Stream* stream,const int32_t* limits);
// TS3W.exe 0x0060D140: Check the final mip's byte sum after validating the stream span.
bool LastMipZero(Stream* stream,uint32_t width,uint32_t height,uint32_t levels,uint32_t format);
} // namespace ts3::misc::clu4a9e90
