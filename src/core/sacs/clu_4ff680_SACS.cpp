// Unit clu_4ff680_SACS (subsystem SACS): the twelve TS3W.exe functions that sort
// fixed-size records by a string key, rewritten together. Records are kStride =
// 0xE08 bytes; the key is the NUL-terminated string at +kKeyOff = 0x208 and an empty
// key sorts before everything. All twelve are __cdecl; the extra trailing context
// argument every call site pushes is threaded through and never read.
//
// Transcribed instruction-for-instruction from TS3W.exe (see the per-function
// comments for the addresses). EA liveness bugs are kept on purpose — the rewrites
// hang or walk off the array exactly where the originals do:
//  - 0x00982980 (Partition): a pivot with an empty key makes the right scan
//    decrement `j` forever without reading an element (pure hang). Reachable only
//    when median-of-three returns an empty-keyed element.
//  - 0x009822E0 (InsertionSortTail): the shift loop has no `first` bound and skips
//    the predecessor-empty test while the saved key is empty — an empty key in the
//    range walks off the front of the array. Callers pass non-empty ranges and rely
//    on a stop (empty predecessor, or the introsort ordering invariant).
//  - Malformed ranges (last-first not a multiple of kStride, or 0 < last-first <
//    kStride) make the insertion sorts loop forever; the entry's depth formula is
//    only defined for aligned ranges. Well-formed inputs only, like the game's.

#include "ts3/sacs/clu_4ff680_SACS.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace ts3::sacs {
namespace {

constexpr int kStride = 0xE08;  // sizeof one record
constexpr int kKeyOff = 0x208;  // key offset inside a record

static_assert(sizeof(SortRecord) == kStride, "SortRecord must be one stride");

// TS3W.exe 0x0114CAB0..0x0114CBAF: the 256-byte fold table the compare leaf at
// 0x004FF680 indexes with a SIGNED char (+ 128). Bytes >= 0x80 (table[0..127],
// signed chars -128..-1) are class codes (01/04/08/40/80), NOT case folds; the
// low half (table[128..255], signed chars 0..127) is the ASCII fold: A-Z -> a-z,
// everything else identity.
const uint8_t kFoldTable[256] = {
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,  // -128..-113
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,  // -112.. -97
    0x04, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x01, 0x08, 0x08,  //  -96.. -81
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,  //  -80.. -65
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,  //  -64.. -49
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,  //  -48.. -33
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,  //  -32.. -17
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x08, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,  //  -16..  -1
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,  //    0..  15
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,  //   16..  31
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,  //   32..  47
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,  //   48..  63
    0x40, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,  //   64..  79 (A-Z)
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,  //   80..  95
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,  //   96.. 111
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,  //  112.. 127
};

inline unsigned FoldByte(const char* p)
{
    return kFoldTable[static_cast<int>(static_cast<signed char>(*p)) + 128];
}

inline bool KeyEmptyAt(const char* rec)
{
    return rec[kKeyOff] == 0;
}

inline const char* KeyOf(const char* rec)
{
    return rec + kKeyOff;
}

}  // namespace

// TS3W.exe 0x004FF680: the compare leaf. For each pair of bytes: fold both (signed
// char index into the table at 0x0114CB30), return fold(a) - fold(b) on a mismatch,
// end both strings (return 0) when the fold is 0. cdecl, int in EAX.
int CompareStrings(const char* a, const char* b)
{
    for (;;) {
        const unsigned fa = FoldByte(a);
        const unsigned fb = FoldByte(b);
        if (fa != fb)
            return static_cast<int>(fa) - static_cast<int>(fb);
        if (fa == 0)
            return 0;
        ++a;
        ++b;
    }
}

// TS3W.exe 0x009821F0: guarded insertion sort over [first, last). Each element is
// saved to a stack buffer, then shifts left (empty saved key, or strictly below the
// predecessor) one stride at a time, bounded by `first`, and the saved element is
// placed in the hole. Returns last. cdecl.
char* InsertionSort(char* first, char* last, int)
{
    if (first == last)
        return last;  // n == 0
    char* cur = first + kStride;
    if (cur == last)
        return last;  // n == 1, nothing to do
    for (;;) {
        char saved[kStride];
        std::memcpy(saved, cur, static_cast<size_t>(kStride));
        char* hole = cur;
        if (cur != first) {
            const char* prevKey = cur - kStride + kKeyOff;
            for (;;) {
                if (!saved[kKeyOff]) {
                    // empty saved key: always shifts (empty sorts first)
                } else if (!*prevKey) {
                    break;  // empty predecessor stops it
                } else if (CompareStrings(saved + kKeyOff, prevKey) >= 0) {
                    break;  // already in order
                }
                std::memcpy(hole, hole - kStride, static_cast<size_t>(kStride));
                hole -= kStride;
                prevKey = hole - kStride + kKeyOff;
                if (hole == first)
                    break;  // the bound the unguarded variant lacks
            }
        }
        std::memcpy(hole, saved, static_cast<size_t>(kStride));
        cur += kStride;
        if (cur == last)
            return last;
    }
}

// TS3W.exe 0x009822E0: unguarded tail insertion sort over [first, last). Same order
// as 0x009821F0 but the shift loop has no `first` bound and never tests the
// predecessor while the saved key is empty — EA liveness bug, kept. Returns last.
char* InsertionSortTail(char* first, char* last, int)
{
    char* cur = first;
    if (cur == last)
        return last;
    for (;;) {
        char saved[kStride];
        std::memcpy(saved, cur, static_cast<size_t>(kStride));
        char* hole = cur;
        const char* prevKey = cur - kStride + kKeyOff;
        for (;;) {
            if (!saved[kKeyOff]) {
                // EA: empty saved key shifts with no predecessor test and no bound
            } else if (!*prevKey) {
                break;  // empty predecessor stops it
            } else if (CompareStrings(saved + kKeyOff, prevKey) >= 0) {
                break;  // already in order
            }
            std::memcpy(hole, hole - kStride, static_cast<size_t>(kStride));
            hole -= kStride;
            prevKey = hole - kStride + kKeyOff;
        }
        std::memcpy(hole, saved, static_cast<size_t>(kStride));
        cur += kStride;
        if (cur == last)
            return last;
    }
}

// TS3W.exe 0x009823A0: median-of-three. Branch A (!key(a) || (key(b) && a < b))
// picks between b, c and a in that priority order; branch B (a non-empty and not
// below b) picks between a, c and b. The first test of branch B (!key(a) -> return
// a) is dead: branch B implies key(a); kept for shape. Returns one of the three
// element pointers. cdecl, 4 stack args.
char* MedianOfThree(char* a, char* b, char* c, int)
{
    const bool aEmpty = KeyEmptyAt(a);
    const bool bEmpty = KeyEmptyAt(b);
    const bool cEmpty = KeyEmptyAt(c);
    bool below = false;
    if (!aEmpty && !bEmpty)
        below = CompareStrings(KeyOf(a), KeyOf(b)) < 0;
    if (aEmpty || (!bEmpty && below)) {
        // Branch A (0x009823D9)
        if (bEmpty)
            return b;
        if (!cEmpty && CompareStrings(KeyOf(b), KeyOf(c)) < 0)
            return b;
        if (aEmpty)
            return c;
        if (cEmpty)
            return a;
        if (CompareStrings(KeyOf(a), KeyOf(c)) < 0)
            return c;
        return a;
    }
    // Branch B (0x00982442): key(a) holds here, so the first test never fires
    if (aEmpty)
        return a;  // dead, kept
    if (!cEmpty && CompareStrings(KeyOf(a), KeyOf(c)) < 0)
        return a;
    if (bEmpty)
        return c;
    if (cEmpty)
        return b;
    if (CompareStrings(KeyOf(b), KeyOf(c)) < 0)
        return c;
    return b;
}

// TS3W.exe 0x009824A0: percolate `value` up from heap index `hole`: parent =
// (hole - 1) >> 1; while hole > top, an empty parent or a parent strictly below
// `value` moves down into the hole, otherwise `value` is placed in the hole.
// Heap node indexes (not pointers); arg5 is the context, unread. cdecl.
void PushHeap(char* base, int top, int hole, char* value, int)
{
    int parent = (hole - 1) >> 1;  // SAR 1 on (hole-1)
    for (;;) {
        if (hole <= top)
            break;
        char* pe = base + static_cast<std::ptrdiff_t>(parent) * kStride;
        bool climb;
        if (KeyEmptyAt(pe))
            climb = true;  // empty parent always moves down over it
        else if (!KeyEmptyAt(value))
            climb = CompareStrings(KeyOf(pe), KeyOf(value)) < 0;
        else
            climb = false;  // empty value sorts first: stop under the parent
        if (!climb)
            break;
        std::memcpy(base + static_cast<std::ptrdiff_t>(hole) * kStride, pe,
                    static_cast<size_t>(kStride));
        hole = parent;
        parent = (hole - 1) >> 1;
    }
    std::memcpy(base + static_cast<std::ptrdiff_t>(hole) * kStride, value,
                static_cast<size_t>(kStride));
}

// TS3W.exe 0x00982730: sift the hole at index `hole` down a heap of `count` records
// (children 2h+1, 2h+2; the larger child per the empty-key order moves up into the
// hole while the right child stays inside `count`; an odd last level takes the lone
// left child at count-1), then percolate `value` up from wherever the hole ended
// (0x009824A0, bounded by `top`). cdecl, 6 stack args.
void AdjustHeap(char* base, int top, int count, int hole, char* value, int context)
{
    int child = 2 * hole + 2;  // right child
    while (child < count) {
        char* right = base + static_cast<std::ptrdiff_t>(child) * kStride;
        char* left = base + static_cast<std::ptrdiff_t>(child - 1) * kStride;
        if (KeyEmptyAt(right))
            --child;  // empty right child: take the left one
        else if (!KeyEmptyAt(left) && CompareStrings(KeyOf(right), KeyOf(left)) < 0)
            --child;  // right below left: left is the larger
        std::memcpy(base + static_cast<std::ptrdiff_t>(hole) * kStride,
                    base + static_cast<std::ptrdiff_t>(child) * kStride,
                    static_cast<size_t>(kStride));
        hole = child;
        child = 2 * child + 2;
    }
    if (child == count) {
        // odd last level: the lone left child at index count-1
        std::memcpy(base + static_cast<std::ptrdiff_t>(hole) * kStride,
                    base + static_cast<std::ptrdiff_t>(count - 1) * kStride,
                    static_cast<size_t>(kStride));
        hole = count - 1;
    }
    PushHeap(base, top, hole, value, context);
}

// TS3W.exe 0x009827F0: heapify [first, last) by adjusting every internal node from
// ((count-2)>>1) down to 0. Each node's element is first saved to a stack buffer
// (its hole), AdjustHeap sifts the hole down and percolates the buffer back up.
// cdecl, void in EAX (garbage).
void MakeHeap(char* first, char* last, int context)
{
    const int count = static_cast<int>((last - first) / kStride);
    if (count < 2)
        return;
    for (int node = (count - 2) >> 1;; --node) {
        char saved[kStride];
        std::memcpy(saved, first + static_cast<std::ptrdiff_t>(node) * kStride,
                    static_cast<size_t>(kStride));
        AdjustHeap(first, node, count, node, saved, context);
        if (node == 0)
            break;
    }
}

// TS3W.exe 0x00982890: heapsort over an existing heap: move the root to the last
// heap slot, re-sift the displaced last element into the shrunken heap (count-1),
// step the end back one stride, repeat while more than one element remains.
// Returns the final element count (1; n itself when n <= 1). cdecl.
int SortHeap(char* first, char* last, int context)
{
    int n = static_cast<int>((last - first) / kStride);
    if (n <= 1)
        return n;
    char* end = last - kStride;
    for (;;) {
        char saved[kStride];
        std::memcpy(saved, end, static_cast<size_t>(kStride));
        std::memcpy(end, first, static_cast<size_t>(kStride));
        AdjustHeap(first, 0, n - 1, 0, saved, context);
        end -= kStride;
        n = static_cast<int>((end + kStride - first) / kStride);
        if (n <= 1)
            return n;
    }
}

// TS3W.exe 0x00982980: Hoare-style partition of [first, last) around the VALUE
// copied once from *pivot at entry (swaps never touch the copy). Left scan: while
// the element is empty-keyed or strictly below the pivot value, i += stride (no
// upper bound). Right scan: j -= stride; an empty pivot loops here forever (EA);
// otherwise stop at an empty-keyed element or one not above the pivot. If i < j
// the two records are swapped through a stack buffer and the scans restart;
// otherwise i is returned as the split. cdecl, 4 stack args.
char* Partition(char* first, char* last, char* pivot, int)
{
    char pivotCopy[kStride];
    std::memcpy(pivotCopy, pivot, static_cast<size_t>(kStride));
    const bool pivotEmpty = KeyEmptyAt(pivotCopy);
    char* i = first;
    char* j = last;
    for (;;) {
        for (;;) {  // left scan
            if (!KeyEmptyAt(i)) {
                if (pivotEmpty)
                    break;  // then the right scan hangs (EA)
                if (CompareStrings(KeyOf(i), KeyOf(pivotCopy)) >= 0)
                    break;  // not below the pivot: this is the stop
            }
            i += kStride;  // empty elements always advance
        }
        for (;;) {  // right scan
            j -= kStride;
            if (!pivotEmpty) {
                if (KeyEmptyAt(j) || CompareStrings(KeyOf(pivotCopy), KeyOf(j)) >= 0)
                    break;
            }
            // EA: empty pivot decrements j forever, reading nothing
        }
        if (i >= j)
            return i;
        char tmp[kStride];
        std::memcpy(tmp, i, static_cast<size_t>(kStride));
        std::memcpy(i, j, static_cast<size_t>(kStride));
        std::memcpy(j, tmp, static_cast<size_t>(kStride));
        i += kStride;
    }
}

// TS3W.exe 0x00982A70: partial sort. Heapify [first, middle); then for each
// element v of (middle, end) — empty-keyed, or strictly below the heap root —
// swap it with the root and re-sift the root into the count-element heap; finally
// heapsort [first, middle) (0x00982890). Returns that count. cdecl.
int PartialSort(char* first, char* middle, char* end, int context)
{
    MakeHeap(first, middle, context);
    const int heapCount = static_cast<int>((middle - first) / kStride);
    for (char* v4 = middle; v4 < end; v4 += kStride) {
        bool swapIt;
        if (KeyEmptyAt(v4))
            swapIt = true;  // empty sorts first: always down into the heap
        else if (KeyEmptyAt(first))
            swapIt = false;  // empty root: nothing is below it
        else
            swapIt = CompareStrings(KeyOf(v4), KeyOf(first)) < 0;
        if (!swapIt)
            continue;  // at or above the root: leave it in place
        char saved[kStride];
        std::memcpy(saved, v4, static_cast<size_t>(kStride));
        std::memcpy(v4, first, static_cast<size_t>(kStride));
        AdjustHeap(first, 0, heapCount, 0, saved, context);
    }
    return SortHeap(first, middle, context);
}

// TS3W.exe 0x00982CB0: the introsort driver. Ranges of <= 28 records, or depth 0,
// fall through to PartialSort over [first, split) (split == last at entry, so the
// whole range is heapsorted); otherwise median-of-three + partition, recurse into
// the right half [split, last) at depth-1 and loop over the left half. depth comes
// from 0x00982DF0 as 2*bitlen-2. Returns the last computed element count. cdecl.
int IntroSortPass(char* first, char* last, int depth, int context)
{
    char* v5 = last;
    int result = static_cast<int>((v5 - first) / kStride);
    if (result <= 28) {
        if (depth == 0)
            return PartialSort(first, v5, v5, context);
        return result;
    }
    for (;;) {
        if (depth <= 0)
            break;
        char* mid = first + static_cast<std::ptrdiff_t>(result / 2) * kStride;
        char* pivot = MedianOfThree(first, mid, v5 - kStride, context);
        --depth;
        char* split = Partition(first, v5, pivot, context);
        IntroSortPass(split, v5, depth, context);
        v5 = split;
        result = static_cast<int>((v5 - first) / kStride);
        if (result <= 28) {
            if (depth == 0)
                return PartialSort(first, v5, v5, context);
            return result;
        }
    }
    if (depth == 0)
        return PartialSort(first, v5, v5, context);
    return result;
}

// TS3W.exe 0x00982DF0: the sort entry. Empty range returns; otherwise compute the
// bit length of the element count and hand the range to 0x00982CB0 with depth
// 2*bitlen-2 (negative for an empty count — malformed input only); then finish:
// n <= 28 a single guarded insertion sort, n > 28 the guarded sort over the first
// 28 records (0x0188E0 = 100576 = 28 strides) plus the unguarded one over the rest
// (the introsort left the array almost sorted, so the unguarded shift always finds
// its stop). cdecl, void in EAX (garbage).
void SortRecords(char* first, char* last, int context)
{
    if (first == last)
        return;
    const int count = static_cast<int>((last - first) / kStride);
    int bitlen = 0;
    for (int v = count; v != 0; v >>= 1)
        ++bitlen;
    IntroSortPass(first, last, 2 * bitlen - 2, context);
    if (count <= 28) {
        InsertionSort(first, last, context);
    } else {
        InsertionSort(first, first + 0x188E0, context);
        InsertionSortTail(first + 0x188E0, last, context);
    }
}

}  // namespace ts3::sacs
