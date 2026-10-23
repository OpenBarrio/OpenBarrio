#pragma once
#include <cstdint>
#include "ts3/layout.h"
// Unit clu_b6a1e0_x (TS3W.exe, unlabeled subsystem -> misc): a k-nearest-neighbour heap
// over 2-D boxes (sort/partial-sort/keep-K by point distance), a u16 lower_bound over
// 24-byte records, a dirty-rect pixel mask painter and four small class helpers. All
// originals are in this unit; the only outside calls are the virtual methods the
// class helpers dispatch through (slots noted per function). thiscall originals are
// plain free functions here; the harness wrappers adapt the ABI with __fastcall.
namespace ts3::misc {

// A positioned object's bounding box: the four floats the distance math reads live at
// +0x10..+0x1C; the leading four floats are outside this unit's use. Field names keep
// the original disassembly's shorthand, but the geometry is the opposite of what they
// suggest: the axis distance is 0 iff [0x10..0x18/0x1C] contains the point, i.e.
// offset 0x10 is the LOWER edge of the range and 0x18 the UPPER one.
struct KnnBox {
    float head00, head04, head08, head0C;
    float hiX;  // +0x10 lower edge of x (distance reads it as the min bound)
    float hiY;  // +0x14 lower edge of y
    float loX;  // +0x18 upper edge of x
    float loY;  // +0x1C upper edge of y
};
TS3_X86_SIZE(KnnBox, 0x20);
TS3_X86_OFFSET(KnnBox, hiX, 0x10);
TS3_X86_OFFSET(KnnBox, loY, 0x1C);

// The query point 0x00B77690 measures against (its `this`).
struct KnnPoint {
    float x, y;
};
TS3_X86_SIZE(KnnPoint, 8);

// The dirty-rect cell grid 0x00B701E0 paints into.
struct PixelGrid {
    uint8_t* cells;      // +0x00 row-major base
    uint8_t pad04[0x14]; // +0x04..+0x17
    int32_t stride;      // +0x18 bytes per row
    int32_t reserved1C;  // +0x1C never touched by this unit
    int32_t minX;        // +0x20 dirty rect (inclusive)
    int32_t minY;        // +0x24
    int32_t maxX;        // +0x28 (inclusive)
    int32_t maxY;        // +0x2C
};
TS3_X86_SIZE(PixelGrid, 0x30);
TS3_X86_OFFSET(PixelGrid, stride, 0x18);
TS3_X86_OFFSET(PixelGrid, minX, 0x20);
TS3_X86_OFFSET(PixelGrid, maxY, 0x2C);

// The object whose mode selects the weight 0x00B71880 returns.
struct WeightTable {
    void** vtable;       // +0x00
    uint8_t pad04[0x1C]; // +0x04..+0x1F
    int32_t mode;        // +0x20 (3, 4 and 5 map to the three constants)
};
TS3_X86_OFFSET(WeightTable, mode, 0x20);

// The object behind the pair slot methods (0x00B71860, 0x00B71900..0x00B71980):
// dword pairs at +0x90, a forwarded child at +0x38. The originals index pairs
// unchecked (0x90 + index*8); 16 pairs keep every tested index in bounds.
struct PairTable {
    void** vtable;       // +0x00
    uint8_t pad04[0x34]; // +0x04..+0x37
    void* child;         // +0x38 (null disables ForwardPairToChild)
    uint8_t pad3C[0x54]; // +0x3C..+0x8F
    int32_t pairs[16][2]; // +0x90 index pairs, first/second per index
};
TS3_X86_OFFSET(PairTable, child, 0x38);
TS3_X86_OFFSET(PairTable, pairs, 0x90);

// The three vtable-bearing pointers 0x00B792B0 assigns (slot 0 = retain, slot 1 =
// release) plus its four flag bytes.
struct RefTriple {
    void* first;    // +0x00
    void* second;   // +0x04
    void* third;    // +0x08
    uint8_t flags[4]; // +0x0C
};
TS3_X86_SIZE(RefTriple, 16);

// One element of the release range 0x00B78CC0 walks (16-byte stride, three pointers
// released from +8 down to +0 through vtable slot 1).
struct Record16 {
    void* first;   // +0x00 released last
    void* second;  // +0x04 released second
    void* third;   // +0x08 released first
    int32_t pad;   // +0x0C
};
TS3_X86_SIZE(Record16, 16);

// 0x00B6A1E0 - lower_bound: first record whose i16 key (at offset 0 of a 24-byte
// record) is >= *key, compared signed; returns the insertion address. Returns begin
// for an empty or inverted range.
uint8_t* LowerBound24(const uint8_t* begin, const uint8_t* end, const uint16_t* key);

// 0x00B701E0 - thiscall, ret 0xC: OR `mask` into cells[y*stride + x] and grow the
// dirty rect to cover (x, y) with inclusive min/max unions. The return value is EA
// scratch (a rect field, or the address of the stack argument on the maxY store path)
// and no caller can rely on it; this returns the post-update maxY.
int32_t PaintMaskPixel(PixelGrid* self, int32_t x, int32_t y, uint8_t mask);

// 0x00B71860 - thiscall: run virtual slot 19, return virtual slot 20's result.
int32_t RefreshAndGetValue(PairTable* self);

// 0x00B71880 - thiscall, returns double: mode 3 -> 6.0f, 4 -> 1.0f, 5 -> 1.0f
// (the floats at image 0x01183B4C/0x01183B50/0x01183B54), anything else -> 0.0.
double ModeWeight(WeightTable* self);

// 0x00B71900 - thiscall, ret 0xC: pairs[index] = {first, second} at 0x90+index*8;
// returns index.
int32_t SetValuePair(PairTable* self, int32_t index, int32_t first, int32_t second);

// 0x00B71920 - thiscall, ret 4: pairs[index] = {0, 0}; returns index.
int32_t ClearValuePair(PairTable* self, int32_t index);

// 0x00B71940 - thiscall, ret 4.
int32_t GetPairFirst(PairTable* self, int32_t index);

// 0x00B71950 - thiscall, ret 4.
int32_t GetPairSecond(PairTable* self, int32_t index);

// 0x00B71980 - thiscall, ret 8: with a child, child virtual 6 (a, b), then this
// virtual 17; with no child EA returns whatever eax held (unreproducible - 0 here).
int32_t ForwardPairToChild(PairTable* self, int32_t a, int32_t b);

// 0x00B77690 - thiscall, ret 8 (`this` = query point): true iff box b's point
// distance exceeds box a's; a null box on either side yields true (EA returns it
// with dirty high bytes, so only the low byte is meaningful).
bool IsFartherThanBox(const KnnPoint* point, const KnnBox* a, const KnnBox* b);

// 0x00B78CC0 - stdcall, ret 8: release every non-null pointer of each 16-byte record
// in [begin, end) through vtable slot 1, in the order +8, +4, +0. EA returns the
// last call's eax (uninitialized for an empty range); declared void here.
void __stdcall ReleaseRecord16(const Record16* begin, const Record16* end);

// 0x00B78D10 - cdecl: insertion sort of [begin, end) ascending by box distance to
// (px, py); nulls compare as farther than any box and drift toward the end. Bounded
// by begin; empty and single-element ranges are no-ops.
void InsertSortByDistance(KnnBox** begin, KnnBox** end, float px, float py);

// 0x00B78E30 - cdecl: same ordering, but the inner walk has no lower bound - it
// starts at begin-1 and keeps shifting while elements keep sorting after it (EA
// reads and writes below begin when the closest element is first; callers pass a
// sorted region below it, which also stops the walk).
void InsertSortByDistanceFrom(KnnBox** begin, KnnBox** end, float px, float py);

// 0x00B78F40 - cdecl: sift-up insert of *slot into the heap at `hole` with parent
// floor `bottom`; null elements keep shifting until the floor. Returns base (EA's
// per-path eax is junk no caller reads).
KnnBox** HeapSiftUp(KnnBox** base, int32_t bottom, int32_t hole, KnnBox** slot, float px,
                    float py);

// 0x00B79FF0 - cdecl: sift down from `hole` over count elements (a null child picks
// the left without comparing - EA), then sift-up insert of *slot at the final hole.
KnnBox** HeapSiftDown(KnnBox** base, int32_t bottom, int32_t count, int32_t hole,
                      KnnBox** slot, float px, float py);

// 0x00B790D0 - cdecl: the slot whose element is the median of the three by distance
// (comparisons are strict; ties fall through to the earlier argument).
KnnBox** MedianOfThree(KnnBox** s1, KnnBox** s2, KnnBox** s3, float px, float py);

// 0x00B792B0 - thiscall, ret 4: assign the three pointers with retain (slot 0) on
// the incoming and release (slot 1) on the replaced (only when the pointer changes),
// then copy the four flag bytes.
RefTriple* AssignRefTriple(RefTriple* self, const RefTriple* src);

// 0x00B7AA20 - cdecl: build the max-heap over [begin, end) with sift-downs from the
// last parents down to the root; ranges smaller than 2 elements are left alone.
void HeapifyRange(KnnBox** begin, KnnBox** end, float px, float py);

// 0x00B7AA70 - cdecl: classic heapsort (extract root to the tail, re-sift) leaving
// [begin, end) ordered ascending by distance; ranges of 1 element or less are no-ops.
void HeapSortRange(KnnBox** begin, KnnBox** end, float px, float py);

// 0x00B7ABF0 - cdecl: partition of [begin, end) around *pivot by distance; nulls are
// skipped on the right and count as closer than the pivot (kept left). The pivot is
// held by value and never stored, so it may also be met in the scan. Returns the
// split slot.
KnnBox** PartitionByDistance(KnnBox** begin, KnnBox** end, KnnBox** pivot, float px, float py);

// 0x00B7AF00 - cdecl: keep the `heapEnd - base` nearest boxes of
// [base, candidatesEnd): heapify the kept range, and for each candidate closer than
// the current root swap it in (the evicted root parks in the candidate's slot), then
// heapsort the kept range.
void KeepNearestK(KnnBox** base, KnnBox** heapEnd, KnnBox** candidatesEnd, float px, float py);

// 0x00B7B490 - cdecl: introsort driver - median-of-three partition loop over ranges
// larger than 28 elements while `budget` lasts (each partition decrements the budget
// for the loop itself and for the right-half recursion), and when the budget reaches
// exactly 0 the remaining left range is finished by KeepNearestK (heapify+heapsort).
void IntroSortByDistance(KnnBox** begin, KnnBox** end, int32_t budget, float px, float py);

// 0x00B7BA00 - cdecl: full sort of [begin, end) ascending by box distance to
// (px, py): intro pass with budget 2*bitlen(count)-2, then insertion passes (the
// first 28 elements, and from element 28 onward when longer). Requires end > begin
// (EA's bit-length loop spins forever on an inverted range).
void SortByDistance(KnnBox** begin, KnnBox** end, float px, float py);

}  // namespace ts3::misc
