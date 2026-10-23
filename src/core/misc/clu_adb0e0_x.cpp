// Cluster unit clu_adb0e0_x (misc): the 28-byte-element heap/introsort family, a u16
// tree find, the chunked intrusive-pointer range assign and two SIMD transforms.
// Originals: TS3W.exe 0x00ADB0E0, 0x00ADB0F0, 0x00ADB480, 0x00ADDAB0, 0x00ADE8E0,
// 0x00ADEA90, 0x00ADEB10, 0x00ADED70, 0x00ADEDE0, 0x00ADF210, 0x00AE0280, 0x00AE0370,
// 0x00AE0880, 0x00AE1750, 0x00AE1850, 0x00AE3970, 0x00AE4770, 0x00AE58B0, 0x00AE6510.
// Float math follows the originals' SSE operation order lane by lane (single rounding
// per operation, no FMA contraction at the project's /fp:precise default).
#include "ts3/misc/clu_adb0e0_x.h"

#include <cmath>
#include <cstring>

namespace ts3::misc {
namespace {

constexpr size_t kElemWords = 7;   // one heap element = 7 dwords = 28 bytes
constexpr size_t kElemKey = 4;     // the sort key lives in dword 4 (byte 16)

// Element copy, the `rep movsd 7` of the originals.
void CopyElem(uint32_t* dst, const uint32_t* src)
{
    std::memcpy(dst, src, kElemWords * sizeof(uint32_t));
}

// (a2 - a1) / 28 in bytes, exactly the originals' 0x92492493 signed division.
int32_t CountOf(const uint32_t* first, const uint32_t* last)
{
    return static_cast<int32_t>(reinterpret_cast<const char*>(last) - reinterpret_cast<const char*>(first)) / 28;
}

// float -> int32 the way the originals do it with fld/fist: round to nearest, ties to
// even; NaN and everything outside int32 range stores the x87 integer indefinite
// (0x80000000). nearbyintf honours the current rounding mode, nearest-even by default,
// which is what the game's x87 control word holds.
int32_t FistRound(float v)
{
    if (std::isnan(v) || v >= 2147483648.0f || v < -2147483648.0f)
        return INT32_MIN;
    return static_cast<int32_t>(std::nearbyintf(v));
}

// The originals' floor (adc eax, -1) / ceil (sbb eax, -1) after comparing the fraction
// bit pattern against 0x80000001: the fraction is exact, so bit(neg) < 0x80000001 is
// simply "the fraction is not negative".
int32_t FloorOf(float v)
{
    const int32_t rounded = FistRound(v);
    float frac = v - static_cast<float>(rounded);
    uint32_t bits;
    std::memcpy(&bits, &frac, sizeof bits);
    return static_cast<int32_t>(static_cast<uint32_t>(rounded) - 1u + (bits < 0x80000001u ? 1u : 0u));
}

int32_t CeilOf(float v)
{
    const int32_t rounded = FistRound(v);
    float frac = static_cast<float>(rounded) - v;
    uint32_t bits;
    std::memcpy(&bits, &frac, sizeof bits);
    return static_cast<int32_t>(static_cast<uint32_t>(rounded) + 1u - (bits < 0x80000001u ? 1u : 0u));
}

}  // namespace

// 0x00ADDAB0: the sift-up of push_heap. Parent indices come from (hole-1)>>1 (sar),
// the loop bound is a signed compare, the keys compare unsigned (jbe).
int32_t HeapPush(uint32_t* base, int32_t top, int32_t hole, const uint32_t* value, uintptr_t dead)
{
    (void)dead;
    int32_t parent = (hole - 1) >> 1;
    while (hole > top) {
        if (base[kElemWords * parent + kElemKey] <= value[kElemKey])
            break;
        CopyElem(base + kElemWords * hole, base + kElemWords * parent);
        hole = parent;
        parent = (parent - 1) >> 1;
    }
    CopyElem(base + kElemWords * hole, value);
    return kElemWords * hole;
}

// 0x00ADE8E0: the sift-down of __adjust_heap: walk the hole to the bottom picking the
// smaller child (child-- when elem[child] > elem[child-1]); when the exit leaves a lone
// left child (child == len) it moves up too; then push the value in from `top`.
int32_t HeapAdjust(uint32_t* base, int32_t top, int32_t len, int32_t hole, const uint32_t* value,
                   uintptr_t dead)
{
    int32_t child = 2 * hole + 2;
    while (child < len) {
        if (base[kElemWords * child + kElemKey] > base[kElemWords * child - (kElemWords - kElemKey)])
            --child;
        CopyElem(base + kElemWords * hole, base + kElemWords * child);
        hole = child;
        child = 2 * child + 2;
    }
    if (child == len) {
        CopyElem(base + kElemWords * hole, base + kElemWords * (child - 1));
        hole = child - 1;
    }
    return HeapPush(base, top, hole, value, dead);
}

// 0x00ADEA90: make_heap over [first, last), parents (count-2)/2 .. 0.
void HeapMake(uint32_t* first, uint32_t* last, uintptr_t dead)
{
    const int32_t count = CountOf(first, last);
    if (count < 2)
        return;
    uint32_t local[kElemWords];
    for (int32_t hole = ((count - 2) >> 1) + 1; hole-- > 0;) {
        CopyElem(local, first + kElemWords * hole);
        HeapAdjust(first, hole, count, hole, local, dead);
    }
}

// 0x00ADEB10: sort_heap. The min-heap root moves to the (shrinking) end, the displaced
// element is sifted back in; the range ends up descending.
int32_t HeapSortRange(uint32_t* first, uint32_t* last, uintptr_t, uintptr_t deadB)
{
    int32_t count = CountOf(first, last);
    if (count <= 1)
        return count;
    uint32_t local[kElemWords];
    uint32_t* end = last;
    do {
        end -= kElemWords;
        CopyElem(local, end);
        CopyElem(end, first);
        HeapAdjust(first, 0, count - 1, 0, local, deadB);
        count = CountOf(first, end);
    } while (count > 1);
    return count;
}

// 0x00ADED70: __unguarded_partition with "greater": while first's key exceeds the
// pivot's move first up; step last down once, then while the pivot exceeds last's key
// move last down; swap across and continue until the cursors cross.
uint32_t* QuickPartition(uint32_t* first, uint32_t* last, const uint32_t* pivot, uintptr_t dead)
{
    (void)dead;
    const uint32_t key = pivot[kElemKey];
    uint32_t local[kElemWords];
    for (;;) {
        while (first[kElemKey] > key)
            first += kElemWords;
        last -= kElemWords;
        while (key > last[kElemKey])
            last -= kElemWords;
        if (first >= last)
            return first;
        CopyElem(local, first);
        CopyElem(first, last);
        CopyElem(last, local);
        first += kElemWords;
    }
}

// 0x00ADEDE0: __heap_select + sort_heap: heap of [first, mid), every element of
// [mid, last) greater than the root replaces it, then HeapSortRange. Keeps the largest
// (mid - first) elements of [first, last), descending.
int32_t HeapPartialSort(uint32_t* first, uint32_t* mid, uint32_t* last, uintptr_t dead)
{
    HeapMake(first, mid, dead);
    const int32_t len = CountOf(first, mid);
    uint32_t local[kElemWords];
    for (uint32_t* p = mid; p < last; p += kElemWords) {
        if (p[kElemKey] > first[kElemKey]) {
            CopyElem(local, p);
            CopyElem(p, first);
            HeapAdjust(first, 0, len, 0, local, dead);
        }
    }
    return HeapSortRange(first, mid, 0, dead);
}

// 0x00ADF210: introsort driver (descending order). Ranges of <= 28 elements are left
// for the caller's insertion sort unless the depth is exactly 0, in which case they get
// heapsorted (a NEGATIVE depth means "do nothing" - the original's test/jne pair skips
// the fallback); the recursion runs on the right partition, the loop continues left.
void IntroSort(uint32_t* first, uint32_t* last, int32_t depth, uintptr_t dead)
{
    if (CountOf(first, last) <= 28) {
        if (depth == 0)
            HeapPartialSort(first, last, last, dead);
        return;
    }
    if (depth < 0)
        return;
    for (;;) {
        if (depth == 0) {
            HeapPartialSort(first, last, last, dead);
            return;
        }
        // Median of three over the keys of first, middle, last-1 (unsigned compares).
        const int32_t count = CountOf(first, last);
        uint32_t* mid = first + kElemWords * (count / 2);
        uint32_t* lastElem = last - kElemWords;
        const uint32_t firstKey = first[kElemKey];
        const uint32_t midKey = mid[kElemKey];
        const uint32_t lastKey = lastElem[kElemKey];
        uint32_t* pivot;
        if (firstKey <= midKey) {
            if (firstKey <= lastKey)
                pivot = midKey <= lastKey ? mid : lastElem;
            else
                pivot = first;
        } else {
            if (midKey > lastKey)
                pivot = mid;
            else
                pivot = firstKey <= lastKey ? first : lastElem;
        }
        uint32_t* split = QuickPartition(first, last, pivot, dead);
        IntroSort(split, last, depth - 1, dead);
        last = split;
        --depth;
        if (CountOf(first, last) <= 28) {
            if (depth == 0)
                HeapPartialSort(first, last, last, dead);
            return;
        }
    }
}

// ---- u16-keyed tree find ----------------------------------------------------

// 0x00ADB480: descend remembering the last node whose key is >= the search key; the
// keys are signed 16-bit (the original compares words with jl). Everything short of an
// exact match returns the header at +0x04 (the end sentinel).
void** TreeFindU16(U16Tree* tree, void** out, const uint16_t* key)
{
    const U16TreeNode* header = reinterpret_cast<const U16TreeNode*>(&tree->headerLeft);
    const U16TreeNode* node = tree->root;
    const U16TreeNode* bound = header;
    const int16_t wanted = static_cast<int16_t>(*key);
    while (node) {
        if (node->key < wanted)
            node = node->left;
        else {
            bound = node;
            node = node->right;
        }
    }
    if (bound == header || wanted < bound->key)
        bound = header;
    *out = const_cast<U16TreeNode*>(bound);
    return out;
}

// ---- standalone entries -----------------------------------------------------

// 0x00ADB0E0
float VtableStubMinusOne0()
{
    return -1.0f;
}

// 0x00ADB0F0: the same constant, stdcall with three ignored arguments.
float __stdcall VtableStubMinusOne3(uint32_t a1, uint32_t a2, uint32_t a3)
{
    (void)a1;
    (void)a2;
    (void)a3;
    return -1.0f;
}

// 0x00AE0280: the object pointer lives in the game's data at 0x011ECBC4 (x86 builds run
// inside the process and read it in place; x64 has no game to read).
bool GlobalStateIsTwo()
{
#if defined(_M_IX86)
    constexpr uintptr_t kStateObject = 0x011ECBC4;
    const uint32_t* object = *reinterpret_cast<uint32_t* const*>(kStateObject);
    return object == nullptr || object[109] == 2;  // +0x1B4
#else
    AbortNotRewritten("the misc state global 0x011ECBC4");
    return true;
#endif
}

// 0x00AE0370: store the byte, return it.
char FlagByteObject::SetFlagByte(char value)
{
    flag = static_cast<uint8_t>(value);
    return value;
}

// ---- chunked-range intrusive-pointer assign ---------------------------------

// 0x00AE0880.
void* ChunkedPtrAssign(void** out, void** srcCur, void* srcBegin, uint8_t* srcChunkEnd,
                       void*** srcChunkIter, void** srcEnd, void* unused7, void* unused8,
                       void* unused9, void** dstCur, void** dstBegin, void** dstEnd,
                       void** dstChunkIter)
{
    (void)srcBegin;
    (void)unused7;
    (void)unused8;
    (void)unused9;
    while (srcCur != srcEnd) {
        void* const incoming = *srcCur;
        void* const replaced = *dstCur;
        if (incoming != replaced) {
            if (incoming)
                CallGameVirtual<void, 1>(incoming);  // vtable +0x04
            *dstCur = incoming;
            if (replaced)
                CallGameVirtual<void, 2>(replaced);  // vtable +0x08
        }
        if (++dstCur == dstEnd) {
            dstCur = reinterpret_cast<void**>(*(dstChunkIter + 1));
            ++dstChunkIter;
            dstBegin = dstCur;
            dstEnd = dstCur + 64;  // 0x100 bytes = 64 entries
        }
        if (++srcCur == reinterpret_cast<void**>(srcChunkEnd)) {
            ++srcChunkIter;
            srcChunkEnd = reinterpret_cast<uint8_t*>(*srcChunkIter) + 256;
            srcCur = reinterpret_cast<void**>(*srcChunkIter);
        }
    }
    out[0] = dstCur;
    out[1] = dstBegin;
    out[2] = dstEnd;
    out[3] = dstChunkIter;
    return out;
}

// 0x00AE1750: pass-through level (the original shuffles its 13 arguments through three
// 16-byte stack temporaries, observable behaviour identical).
void* ChunkedPtrAssignA(void** out, void** srcCur, void* srcBegin, uint8_t* srcChunkEnd,
                        void*** srcChunkIter, void** srcEnd, void* unused7, void* unused8,
                        void* unused9, void** dstCur, void** dstBegin, void** dstEnd,
                        void** dstChunkIter)
{
    return ChunkedPtrAssign(out, srcCur, srcBegin, srcChunkEnd, srcChunkIter, srcEnd, unused7,
                            unused8, unused9, dstCur, dstBegin, dstEnd, dstChunkIter);
}

// 0x00AE4770: pass-through level above ChunkedPtrAssignA.
void* ChunkedPtrAssignB(void** out, void** srcCur, void* srcBegin, uint8_t* srcChunkEnd,
                        void*** srcChunkIter, void** srcEnd, void* unused7, void* unused8,
                        void* unused9, void** dstCur, void** dstBegin, void** dstEnd,
                        void** dstChunkIter)
{
    return ChunkedPtrAssignA(out, srcCur, srcBegin, srcChunkEnd, srcChunkIter, srcEnd, unused7,
                             unused8, unused9, dstCur, dstBegin, dstEnd, dstChunkIter);
}

// 0x00AE58B0: pass-through level above ChunkedPtrAssignB.
void* ChunkedPtrAssignC(void** out, void** srcCur, void* srcBegin, uint8_t* srcChunkEnd,
                        void*** srcChunkIter, void** srcEnd, void* unused7, void* unused8,
                        void* unused9, void** dstCur, void** dstBegin, void** dstEnd,
                        void** dstChunkIter)
{
    return ChunkedPtrAssignB(out, srcCur, srcBegin, srcChunkEnd, srcChunkIter, srcEnd, unused7,
                             unused8, unused9, dstCur, dstBegin, dstEnd, dstChunkIter);
}

// 0x00AE6510: splat the dst quad (this), the src iterator quad and the src end quad
// into the 13-argument call.
void* ChunkedAssignState::AssignChunkedRange(void** out, void** srcA, void** srcB, void* unused4)
{
    (void)unused4;
    return ChunkedPtrAssignC(out, reinterpret_cast<void**>(srcA[0]), srcA[1],
                             reinterpret_cast<uint8_t*>(srcA[2]),
                             reinterpret_cast<void***>(srcA[3]), reinterpret_cast<void**>(srcB[0]),
                             srcB[1], srcB[2], srcB[3], dstCur, dstBegin, dstEnd, dstChunkIter);
}

// ---- SIMD transforms --------------------------------------------------------

namespace {
// ((x*row0 + row3) + y*row1) + z*row2, one component, in the originals' operation
// order (y is +0.0f in both callers; 0 * row1 keeps inf/NaN propagation identical).
float TransformLane(float x, float y, float z, const float* row0, const float* row1,
                    const float* row2, const float* row3, int lane)
{
    float t = x * row0[lane];
    t += row3[lane];
    t += y * row1[lane];
    t += z * row2[lane];
    return t;
}
}  // namespace

// 0x00AE1850: lanes 0 and 2 of the transformed (x, 0, z) go to *outX / *outZ.
void* XZTransformObject::TransformXZ(float x, float z, float* outX, float* outZ)
{
    const float y = 0.0f;
    const float resultX = TransformLane(x, y, z, row0, row1, row2, row3, 0);
    const float resultZ = TransformLane(x, y, z, row0, row1, row2, row3, 2);
    *outX = resultX;
    *outZ = resultZ;
    return outX;
}

// 0x00AE3970.
int32_t ScreenBoundsObject::UpdateBounds()
{
    const float w = static_cast<float>(sizeX);
    const float h = static_cast<float>(sizeY);
    // Corner points (x, z) laid out exactly like the original's stack array.
    const float pts[8] = {0.0f, 0.0f, 0.0f, h, w, h, w, 0.0f};
    float outX[4];
    float outZ[4];
    const float y = 0.0f;
    for (int i = 0; i < 4; ++i) {
        outX[i] = TransformLane(pts[2 * i], y, pts[2 * i + 1], row0, row1, row2, row3, 0);
        outZ[i] = TransformLane(pts[2 * i], y, pts[2 * i + 1], row0, row1, row2, row3, 2);
    }
    minX = maxX = outX[0];
    minZ = maxZ = outZ[0];
    if (minX > outX[1]) minX = outX[1];
    if (outX[1] > maxX) maxX = outX[1];
    if (minZ > outZ[1]) minZ = outZ[1];
    if (outZ[1] > maxZ) maxZ = outZ[1];
    if (minX > outX[2]) minX = outX[2];
    if (outX[2] > maxX) maxX = outX[2];
    if (minZ > outZ[2]) minZ = outZ[2];
    if (outZ[2] > maxZ) maxZ = outZ[2];
    if (minX > outX[3]) minX = outX[3];
    if (outX[3] > maxX) maxX = outX[3];
    if (minZ > outZ[3]) minZ = outZ[3];
    if (outZ[3] > maxZ) maxZ = outZ[3];
    // The originals round through fld/fist/fild/fsub and take the sign of the exact
    // fraction (see FloorOf/CeilOf).
    floorMinX = FloorOf(minX);
    floorMinZ = FloorOf(minZ);
    ceilMaxX = CeilOf(maxX);
    ceilMaxZ = CeilOf(maxZ);
    return ceilMaxZ;
}

}  // namespace ts3::misc
