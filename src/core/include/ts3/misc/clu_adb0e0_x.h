#pragma once
// Cluster unit clu_adb0e0_x (misc subsystem): TS3W.exe 0x00ADB0E0, 0x00ADB0F0, 0x00ADB480,
// 0x00ADDAB0, 0x00ADE8E0, 0x00ADEA90, 0x00ADEB10, 0x00ADED70, 0x00ADEDE0, 0x00ADF210,
// 0x00AE0280, 0x00AE0370, 0x00AE0880, 0x00AE1750, 0x00AE1850, 0x00AE3970, 0x00AE4770,
// 0x00AE58B0, 0x00AE6510. Three groups: an SGI-STL-style heap/introsort family over
// 28-byte elements with the compare key in the dword at +16 (unsigned compares), a u16
// keyed tree find, and a chunked-range intrusive-pointer assign plus two SIMD transforms.
// In-unit calls go directly to the rewrites below; everything else runs through the ts3
// layout.h bridge (x64 builds abort there instead). EA bugs that affect outputs are kept
// and marked "EA:".
#include <cstdint>

#include "ts3/layout.h"

namespace ts3::misc {

// ---- the heap / sort family -------------------------------------------------
//
// Elements are 7 dwords (28 bytes); the sort key is element dword 4 (byte offset 16),
// compared as unsigned 32-bit. The heap is a MIN-heap (root = smallest key), so
// HeapSortRange leaves its range in DESCENDING order and HeapPartialSort keeps the
// LARGEST (mid - first) elements of [first, last) in [first, mid), descending. The
// trailing `dead` arguments are really a comparator object that the original drops on
// the floor (never read); every level passes it down, so they are kept for ABI fidelity.

// 0x00ADDAB0: __push_heap. Sifts `value` up from `hole` while the parent is greater
// (stop at `top`); copies it into the final hole. Returns 7 * hole (the original does;
// its only in-unit caller passes it on and the callers above ignore it).
int32_t HeapPush(uint32_t* base, int32_t top, int32_t hole, const uint32_t* value, uintptr_t dead);

// 0x00ADE8E0: __adjust_heap. Sift the hole down to a leaf picking the SMALLER child,
// (moving the left child up when the right child index would be len), then HeapPush.
int32_t HeapAdjust(uint32_t* base, int32_t top, int32_t len, int32_t hole, const uint32_t* value,
                   uintptr_t dead);

// 0x00ADEA90: make_heap over [first, last). The original leaves a magic-division
// by-product in EAX on the count<2 path and the last HeapAdjust result otherwise; the
// only caller ignores it, so this is void here.
void HeapMake(uint32_t* first, uint32_t* last, uintptr_t dead);

// 0x00ADEB10: sort_heap. Pops the min-heap root to the end (count-1) times, leaving
// [first, last) in descending order. Returns the final element count the original
// computes on the way out (callers ignore it).
int32_t HeapSortRange(uint32_t* first, uint32_t* last, uintptr_t deadA, uintptr_t deadB);

// 0x00ADED70: __unguarded_partition with `greater` as the predicate: elements greater
// than *pivot collect in the left part. Returns the partition point.
uint32_t* QuickPartition(uint32_t* first, uint32_t* last, const uint32_t* pivot, uintptr_t dead);

// 0x00ADEDE0: __heap_select + sort_heap = partial_sort with `greater` (largest first).
int32_t HeapPartialSort(uint32_t* first, uint32_t* mid, uint32_t* last, uintptr_t dead);

// 0x00ADF210: introsort driver over [first, last): while the range is longer than 28
// elements and depth remains, partition at the median of first/middle/last and recurse
// into the right half; fall back to HeapPartialSort when the depth runs out. Ranges of
// 28 elements or less are left untouched (the caller's insertion sort handles them).
// The original returns whatever the last division left in EAX; no caller reads it.
void IntroSort(uint32_t* first, uint32_t* last, int32_t depth, uintptr_t dead);

// ---- u16-keyed tree find ----------------------------------------------------

// Node of the tree behind 0x00ADB480: children at +0/+4, rbtree bookkeeping at +8..+0xF
// (untouched here), signed 16-bit key at +0x10.
struct U16TreeNode {
    U16TreeNode* left;         // 0x00
    U16TreeNode* right;        // 0x04
    uint32_t parentColor[2];   // 0x08
    int16_t key;               // 0x10
};
TS3_X86_OFFSET(U16TreeNode, key, 0x10);

// The object 0x00ADB480 runs on: root pointer at +0x0C, the end sentinel ("header") at
// +0x04 (its own address is the not-found result).
struct U16Tree {
    uint32_t unknown0;     // 0x00
    void* headerLeft;      // 0x04
    uint32_t unknown8;     // 0x08
    U16TreeNode* root;     // 0x0C
};
TS3_X86_OFFSET(U16Tree, root, 0x0C);

// 0x00ADB480 (__thiscall, ret 8): tree find. Walks nodes with a signed 16-bit key
// compare, remembering the last node with key >= *key; exact matches return that node,
// everything else the header (end). Writes the result to *out and returns out.
void** TreeFindU16(U16Tree* tree, void** out, const uint16_t* key);

// ---- standalone entries -----------------------------------------------------

// 0x00ADB0E0 (cdecl): vtable stub loading the constant at 0xFAA210 (-1.0f) into ST0.
float VtableStubMinusOne0();

// 0x00ADB0F0 (stdcall, ret 0xC): the same stub with three ignored arguments.
float __stdcall VtableStubMinusOne3(uint32_t a1, uint32_t a2, uint32_t a3);

// 0x00AE0280 (cdecl): reads the object pointer at 0x011ECBC4; true when it is null or
// its dword at +0x1B4 equals 2.
bool GlobalStateIsTwo();

// 0x00AE0370
struct FlagByteObject {
    uint8_t before[0x394];
    uint8_t flag;  // 0x394
    char SetFlagByte(char value);
};
TS3_X86_OFFSET(FlagByteObject, flag, 0x394);

// ---- chunked-range intrusive-pointer assign ---------------------------------
//
// 0x00AE0880 copies the pointer range [srcCur, srcEnd) into the dst position dstCur.
// Both sides walk 64-entry (256-byte) chunks: when the src cursor reaches srcChunkEnd
// it jumps to *++srcChunkIter (+256 bytes for the new chunk end); the dst cursor jumps
// to *++dstChunkIter and gets a fresh [begin, begin+64) window. Every assignment with
// a changed pointer calls vtable slot 1 on the incoming object, stores it, then calls
// slot 2 on the replaced object. The four-dword {cur, begin, end, chunkIter} result is
// written to out (which is also returned). Args 3 and 7-9 are unused in the original.
void* ChunkedPtrAssign(void** out, void** srcCur, void* srcBegin, uint8_t* srcChunkEnd,
                       void*** srcChunkIter, void** srcEnd, void* unused7, void* unused8,
                       void* unused9, void** dstCur, void** dstBegin, void** dstEnd,
                       void** dstChunkIter);

// 0x00AE1750 / 0x00AE4770 / 0x00AE58B0: ABI-identical pass-through levels above
// ChunkedPtrAssign (the originals rebuild their 13 arguments through three 16-byte
// stack temporaries; only the callee changes).
void* ChunkedPtrAssignA(void** out, void** srcCur, void* srcBegin, uint8_t* srcChunkEnd,
                        void*** srcChunkIter, void** srcEnd, void* unused7, void* unused8,
                        void* unused9, void** dstCur, void** dstBegin, void** dstEnd,
                        void** dstChunkIter);
void* ChunkedPtrAssignB(void** out, void** srcCur, void* srcBegin, uint8_t* srcChunkEnd,
                        void*** srcChunkIter, void** srcEnd, void* unused7, void* unused8,
                        void* unused9, void** dstCur, void** dstBegin, void** dstEnd,
                        void** dstChunkIter);
void* ChunkedPtrAssignC(void** out, void** srcCur, void* srcBegin, uint8_t* srcChunkEnd,
                        void*** srcChunkIter, void** srcEnd, void* unused7, void* unused8,
                        void* unused9, void** dstCur, void** dstBegin, void** dstEnd,
                        void** dstChunkIter);

// 0x00AE6510 (__thiscall, ret 0x10): member wrapper that splats the four dst words
// (this), the four words at srcA ({srcCur, srcBegin, srcChunkEnd, srcChunkIter}) and
// the four at srcB ({srcEnd, ...}) into the 13-argument call above. The 4th argument
// is passed by the caller but never read.
struct ChunkedAssignState {
    void** dstCur;       // 0x00
    void** dstBegin;     // 0x04
    void** dstEnd;       // 0x08
    void** dstChunkIter; // 0x0C
    void* AssignChunkedRange(void** out, void** srcA, void** srcB, void* unused4);
};
TS3_X86_OFFSET(ChunkedAssignState, dstChunkIter, 0x0C);

// ---- SIMD transforms --------------------------------------------------------

// 0x00AE1850 (__thiscall, ret 0x10): transform the point (x, 0, z) with the rows at
// +0x1B0..+0x1EF (row3 = translation) as ((x*row0 + row3) + y*row1) + z*row2 in single
// precision, write component 0 to *outX and component 2 to *outZ, return outX. Only
// the matrix fields are touched; the real object is larger.
struct XZTransformObject {
    uint8_t before[0x1B0];
    float row0[4];  // 0x1B0
    float row1[4];  // 0x1C0
    float row2[4];  // 0x1D0
    float row3[4];  // 0x1E0
    void* TransformXZ(float x, float z, float* outX, float* outZ);
};
TS3_X86_OFFSET(XZTransformObject, row0, 0x1B0);
TS3_X86_OFFSET(XZTransformObject, row3, 0x1E0);

// 0x00AE3970 (__thiscall): screen-space bounds update. Converts the int sizes at
// +0x60/+0x64 to float, transforms the four corners (0,0) (0,h) (w,h) (w,0) with the
// matrix at +0x1F0..+0x22F (same op order as TransformXZ), keeps the running min/max
// of x in +0x84/+0x8C and of z in +0x88/+0x90, and stores floor(min x/z) at +0x74/+0x78
// and ceil(max x/z) at +0x7C/+0x80. Returns the ceil(maxZ) value. The floor/ceil use
// the original's x87 round-to-nearest-then-sign-of-fraction scheme (see the .cpp).
struct ScreenBoundsObject {
    uint8_t before0[0x60];
    int32_t sizeX;              // 0x60
    int32_t sizeY;              // 0x64
    uint8_t before1[0x74 - 0x68];
    int32_t floorMinX;          // 0x74
    int32_t floorMinZ;          // 0x78
    int32_t ceilMaxX;           // 0x7C
    int32_t ceilMaxZ;           // 0x80
    float minX;                 // 0x84
    float minZ;                 // 0x88
    float maxX;                 // 0x8C
    float maxZ;                 // 0x90
    uint8_t before2[0x1F0 - 0x94];
    float row0[4];              // 0x1F0
    float row1[4];              // 0x200
    float row2[4];              // 0x210
    float row3[4];              // 0x220
    int32_t UpdateBounds();
};
TS3_X86_OFFSET(ScreenBoundsObject, sizeX, 0x60);
TS3_X86_OFFSET(ScreenBoundsObject, floorMinX, 0x74);
TS3_X86_OFFSET(ScreenBoundsObject, minX, 0x84);
TS3_X86_OFFSET(ScreenBoundsObject, maxZ, 0x90);
TS3_X86_OFFSET(ScreenBoundsObject, row0, 0x1F0);
TS3_X86_OFFSET(ScreenBoundsObject, row3, 0x220);

}  // namespace ts3::misc
