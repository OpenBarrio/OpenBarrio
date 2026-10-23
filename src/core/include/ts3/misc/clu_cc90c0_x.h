// Cluster unit clu_cc90c0_x (subsystem misc): the seventeen TS3W.exe functions that
// run the numeric heap sort (median-of-three, sift-up/adjust/make/sort, Hoare
// partition, partial sort, the introsort driver) and the four-tree composite copy
// (0x00CC9CB0/0x00CC9D30/0x00CC9DB0/0x00CCAAC0), plus two float quantizer leaves
// (0x00CCB810/0x00CCB8C0) and the 5-byte scale thunk (0x00CCEA30 -> 0x00CEF760).
// The sort compares 0x28-byte records (0x24 used): a 96-bit id first, then an (x, y)
// float key — records with an equal id compare by y, otherwise by x (NaN-fail
// semantics; the comparator is not transitive on NaN keys, as in the original).
#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::misc {

// One sorted record: 0x28 bytes total, 0x24 compared/copied; the last 4 bytes are a
// gap this unit never reads or writes.
struct QRec {
    uint32_t id0;   // +0x00 record id bits 0..31
    uint32_t id1;   // +0x04 record id bits 32..63
    uint32_t id2;   // +0x08 record id bits 64..95
    uint32_t raw3;  // +0x0C copied with the record, never compared
    float x;        // +0x10 primary key (compared when the ids differ and x differs)
    float y;        // +0x14 secondary key (compared when the ids or x are equal)
    uint32_t f1;    // +0x18 payload, copied
    uint32_t f2;    // +0x1C payload, copied
    float f3;       // +0x20 payload, copied (float: stored through the x87)
    uint8_t gap[4]; // +0x24 untouched by this unit
};
TS3_X86_OFFSET(QRec, id0, 0x00);
TS3_X86_OFFSET(QRec, x, 0x10);
TS3_X86_OFFSET(QRec, y, 0x14);
TS3_X86_OFFSET(QRec, f3, 0x20);
TS3_X86_OFFSET(QRec, gap, 0x24);
TS3_X86_SIZE(QRec, 0x28);

// One of the four binary-tree headers inside a TreeComposite (0x1C each). In a built
// tree +0x0C is the root node; when the same shape is handed to InitTree9cb0/9d30/
// 9db0 as the source, +0x0C is the head of the chain to copy. Pointers are 4-byte
// x86 addresses (this unit is x86-only game code; x64 never runs it).
struct TreeHeader {
    uint32_t owner;     // +0x00 untouched by this unit
    uint32_t leftmost;  // +0x04 leftmost node, or &self->leftmost when empty
    uint32_t rightmost; // +0x08 rightmost node, or &self->leftmost when empty
    uint32_t root;      // +0x0C root node / source chain head
    uint32_t flag;      // +0x10 zeroed (dword, then byte)
    uint32_t value14;   // +0x14 copied from the source only when a chain was built
    uint32_t value18;   // +0x18 copied from the source (threaded into node allocs)
};
TS3_X86_OFFSET(TreeHeader, leftmost, 0x04);
TS3_X86_OFFSET(TreeHeader, root, 0x0C);
TS3_X86_OFFSET(TreeHeader, value18, 0x18);
TS3_X86_SIZE(TreeHeader, 0x1C);

// The 0x80 composite the game copies: four tree headers, three floats and a byte.
struct TreeComposite {
    TreeHeader trees[4]; // +0x00, +0x1C, +0x38, +0x54
    float f70;           // +0x70 copied through the x87 (sNaN quieting)
    float f74;           // +0x74 copied through the x87
    float f78;           // +0x78 copied through the x87
    uint8_t b7c;         // +0x7C copied as a raw byte
};
TS3_X86_OFFSET(TreeComposite, trees[1], 0x1C);
TS3_X86_OFFSET(TreeComposite, trees[3], 0x54);
TS3_X86_OFFSET(TreeComposite, f70, 0x70);
TS3_X86_OFFSET(TreeComposite, b7c, 0x7C);
TS3_X86_SIZE(TreeComposite, 0x80);

// 0x00CC90C0: median-of-three over a, b, c in the unit's key order (id-equal/x-equal
// compare y, else x; NaN fails every test). Returns one of the three pointers.
QRec* MedianOfThree(QRec* a, QRec* b, QRec* c);

// 0x00CC9290: sift-up: percolate `value` from heap index `hole` towards `limit`
// (parent = (hole-1) >> 1, arithmetic) while it is strictly greater than its parent,
// then place `value` in the hole it stopped at. Returns &base[hole].
QRec* PushHeap(QRec* base, int limit, int hole, const QRec* value);

// 0x00CC9390: swap two records. EA asymmetry: b receives a's floats bit-exactly
// (movss), a receives b's floats through the x87 (fld/fstp quiets a signaling NaN —
// sets bit 0x400000, keeping sign and payload). The gap bytes stay untouched.
void SwapRecords(QRec* a, QRec* b);

// 0x00CC9600: sift the hole at `hole` down through the `count`-record heap choosing
// the larger child, then percolate `value` up from where it stopped back to `limit`.
// Returns the PushHeap result.
QRec* AdjustHeap(QRec* base, int limit, int count, int hole, const QRec* value);

// 0x00CC9730
// (captured bit-exactly) through the count-1 heap. Returns the AdjustHeap result.
QRec* PopRoot(QRec* base, QRec* end);

// 0x00CC98C0: heapify [first, last) bottom-up (AdjustHeap for each non-leaf).
void MakeHeap(QRec* first, QRec* last);

// 0x00CC9970: heapsort: pop the root into the shrinking end until one record is left.
void SortHeap(QRec* first, QRec* last);

// 0x00CC9CB0: build the tree at `self` from the source chain at src->root: zeroes
// the header, points leftmost/rightmost at the header itself, then runs the
// 0x00CC9A50 chain builder (ecx = self — the builder reads self->value18) and walks
// the result for leftmost/rightmost. Empty chain: value14 stays 0 (EA: src->value14
// is only copied when a chain was built). thiscall ret 4, returns self.
TreeHeader* InitTree9cb0(TreeHeader* self, const TreeHeader* src);

// 0x00CC9D30: same body as InitTree9cb0 with the 0x00CC9B00 chain builder.
TreeHeader* InitTree9d30(TreeHeader* self, const TreeHeader* src);

// 0x00CC9DB0: same body as InitTree9cb0 with the 0x00CC9BF0 chain builder.
TreeHeader* InitTree9db0(TreeHeader* self, const TreeHeader* src);

// 0x00CC9E30: Hoare partition of [first, last) around a VALUE snapshot of *pivot
// (taken once; swaps never change the compared values). Forward scan stops at the
// first element not strictly greater than the snapshot, the back scan at the first
// not strictly greater than it from the right, then the two are swapped. Returns the
// split element pointer (== last when the range is one element).
QRec* Partition(QRec* first, QRec* last, const QRec* pivot);

// 0x00CC9F00: partial sort: heapify [first, middle), then for every element of
// [middle, end) strictly below the heap root swap the root down into it and re-sift
// the saved element, then heapsort [first, middle).
void PartialSort(QRec* first, QRec* middle, QRec* end);

// 0x00CCA200: introsort driver: while the range holds more than 28 records and depth
// remains, median-of-three + Partition and recurse into the right half (the left is
// looped over); the heapsort fallback runs only when the depth budget reaches
// exactly 0 (EA: a caller passing depth < 0, or a range that drops to <= 28 with
// depth still positive, leaves it unsorted).
void IntroSortPass(QRec* first, QRec* last, int depth);

// 0x00CCAAC0: copy the whole composite: trees 0/1/2/3 through InitTree9cb0/9db0/
// 9d30/9db0 (in that order), then f70/f74/f78 through the x87 and the byte at +0x7C.
// thiscall ret 4, returns self.
TreeComposite* InitTreeSet(TreeComposite* self, const TreeComposite* src);

// 0x00CCB810: quantizer, threshold 0.0001f at 0x00F988E8, doubled step: values with
// |v| <= threshold return +1 for BOTH signs (EA: the negative branch does not negate
// the early return); otherwise CEIL((|v|-thr)*2) — the x87 sequence stores r - t
// (fsubrp reverses) and adds 1 when that fraction is negative — and negate for v < 0
// (unsigned, so INT_MIN maps to itself). NaNs take the positive compute path.
int Quantize1e4(float v);

// 0x00CCB8C0: same quantizer with threshold 0.003f at 0x010459E4 and no doubling.
int Quantize003(float v);

// 0x00CCEA30: 5-byte jump to 0x00CEF760 — reads the scale at obj+0x58 (0.f for a
// null obj; the 0x011ED780-flagged vtable path lives entirely in the game code).
float ObjectScale(void* obj);

}  // namespace ts3::misc
