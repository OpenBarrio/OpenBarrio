// Unit clu_b6a1e0_x — TS3W.exe k-nearest heap over 2-D boxes, a u16 lower_bound over
// 24-byte records, a dirty-rect painter and the small class helpers. Every function
// carries its original address; EA behaviours that affect outputs (unbounded scans,
// null quirks, scratch returns) are reproduced on purpose — see the project notes and the
// unit brief.
#include "ts3/misc/clu_b6a1e0_x.h"

#include <cstring>

namespace ts3::misc {
namespace {

// The distance the originals build with comiss on each axis: `hi > p` must branch
// exactly like the game's `jbe` (taken on hi <= p *or unordered*), which the `!`
// formulation reproduces for NaN inputs as well.
inline float AxisDistance(float p, float hi, float lo)
{
    if (!(hi > p)) {
        return (p > lo) ? (p - lo) : 0.0f;
    }
    return hi - p;
}

// Box distance to the query point: AxisDist(x) + AxisDist(y) (the add order differs
// from the asm's y+x, but FP addition is commutative on the values involved).
inline float BoxDistance(const KnnBox* box, float px, float py)
{
    return AxisDistance(px, box->hiX, box->loX) + AxisDistance(py, box->hiY, box->loY);
}

}  // namespace

// 0x00B6A1E0 — cdecl, plain ret. Signed i16 lower_bound over 24-byte records keyed
// at +0; count = (end-begin)/24 once (signed, negative -> immediate begin), the
// search then slides `cur` only. Key loaded once before the loop in the original.
uint8_t* LowerBound24(const uint8_t* begin, const uint8_t* end, const uint16_t* key)
{
    const uint8_t* cur = begin;
    int32_t count = (int32_t)((end - begin) / 24);
    if (count <= 0) return const_cast<uint8_t*>(cur);
    int16_t want;
    std::memcpy(&want, key, sizeof(want));
    while (count > 0) {
        const int32_t mid = count >> 1;
        const uint8_t* rec = cur + (std::ptrdiff_t)mid * 24;
        int16_t recKey;
        std::memcpy(&recKey, rec, sizeof(recKey));
        if (recKey >= want) {
            count = mid;
        } else {
            cur = rec + 24;
            count -= mid + 1;
        }
    }
    return const_cast<uint8_t*>(cur);
}

// 0x00B701E0 — thiscall, ret 0xC. OR the mask into the cell, then union (x, y) into
// the inclusive dirty rect (min: `field >= v` stores v; max: `v >= field` stores v).
// EA's return is scratch (a rect field, or &stack-y on the maxY store path) — this
// returns the post-update maxY, which matches the original's non-store path exactly.
int32_t PaintMaskPixel(PixelGrid* self, int32_t x, int32_t y, uint8_t mask)
{
    self->cells[(std::intptr_t)self->stride * y + x] |= mask;
    if (self->minX >= x) self->minX = x;
    if (self->minY >= y) self->minY = y;
    if (x >= self->maxX) self->maxX = x;
    if (y >= self->maxY) self->maxY = y;
    return self->maxY;
}

// 0x00B71860 — thiscall, plain ret. Slot 19 first (result dropped), tail-call slot 20.
int32_t RefreshAndGetValue(PairTable* self)
{
    CallGameVirtual<void, 19>(self);
    return CallGameVirtual<int32_t, 20>(self);
}

// 0x00B71880 — thiscall, returns double (fld of a float). The three mode floats live
// in the image at 0x01183B4C/0x01183B50/0x01183B54 (6.0f, 1.0f, 1.0f); the promotion
// path matters only for bits, and float->double of these values is exact.
double ModeWeight(WeightTable* self)
{
    switch (self->mode) {
        case 3: return static_cast<double>(6.0f);  // image 0x01183B4C
        case 4: return static_cast<double>(1.0f);  // image 0x01183B50
        case 5: return static_cast<double>(1.0f);  // image 0x01183B54
        default: return 0.0;
    }
}

// 0x00B71900 — thiscall, ret 0xC. Unchecked 0x90+index*8 pair store, returns index.
int32_t SetValuePair(PairTable* self, int32_t index, int32_t first, int32_t second)
{
    self->pairs[index][0] = first;
    self->pairs[index][1] = second;
    return index;
}

// 0x00B71920 — thiscall, ret 4.
int32_t ClearValuePair(PairTable* self, int32_t index)
{
    self->pairs[index][0] = 0;
    self->pairs[index][1] = 0;
    return index;
}

// 0x00B71940 — thiscall, ret 4.
int32_t GetPairFirst(PairTable* self, int32_t index)
{
    return self->pairs[index][0];
}

// 0x00B71950 — thiscall, ret 4.
int32_t GetPairSecond(PairTable* self, int32_t index)
{
    return self->pairs[index][1];
}

// 0x00B71980 — thiscall, ret 8. Child virtual 6 (a, b) then this virtual 17; with no
// child the original returns whatever eax held (never written on that path) — 0 here.
int32_t ForwardPairToChild(PairTable* self, int32_t a, int32_t b)
{
    if (void* child = self->child) {
        CallGameVirtual<void, 6>(child, a, b);
        return CallGameVirtual<int32_t, 17>(self);
    }
    return 0;
}

// 0x00B77690 — thiscall, ret 8; `this` is the query point. Null on either side ->
// true (EA returns 1 with eax's high bytes left dirty — only al is meaningful).
bool IsFartherThanBox(const KnnPoint* point, const KnnBox* a, const KnnBox* b)
{
    if (!a || !b) return true;
    const float da = BoxDistance(a, point->x, point->y);
    const float db = BoxDistance(b, point->x, point->y);
    return db > da;
}

// 0x00B78CC0 — stdcall, ret 8. Range walk with unsigned bound; per record the +8
// pointer releases first, then +4, then +0, each through vtable slot 1 when non-null.
// EA's return is the last call's eax (uninit for an empty range) — declared void.
void __stdcall ReleaseRecord16(const Record16* begin, const Record16* end)
{
    for (const Record16* rec = begin; rec < end; ++rec) {
        if (rec->third) CallGameVirtual<void, 1>(rec->third);
        if (rec->second) CallGameVirtual<void, 1>(rec->second);
        if (rec->first) CallGameVirtual<void, 1>(rec->first);
    }
}

// 0x00B78D10 — cdecl. Bounded insertion sort (inner loop stops at begin); a null on
// either side of a compare counts as farther, so nulls drift toward the end.
void InsertSortByDistance(KnnBox** begin, KnnBox** end, float px, float py)
{
    if (begin == end) return;
    for (KnnBox** i = begin + 1; i != end; ++i) {
        KnnBox* cur = *i;
        KnnBox** j = i;
        while (j != begin) {
            KnnBox* prev = *(j - 1);
            // `distPrev <= distCur` as the asm's comiss+jbe: also taken on unordered,
            // i.e. !(distPrev > distCur) — a plain <= would diverge on NaN.
            if (cur && prev && !(BoxDistance(prev, px, py) > BoxDistance(cur, px, py))) break;
            *j = prev;
            --j;
        }
        *j = cur;
    }
}

// 0x00B78E30 — cdecl. Same ordering, but the inner walk starts at begin-1 with NO
// lower bound: while the element keeps sorting after the hole it shifts, so the
// closest-first element walks below begin (EA OOB — kept). Callers run it behind a
// sorted prefix that stops the walk.
void InsertSortByDistanceFrom(KnnBox** begin, KnnBox** end, float px, float py)
{
    for (KnnBox** i = begin; i != end; ++i) {
        KnnBox* cur = *i;
        KnnBox** hole = i;
        KnnBox** j = i - 1;
        for (;;) {
            KnnBox* e = *j;
            // Same comiss+jbe semantics as the bounded sort: break on dist(e) <= dist(cur),
            // unordered included: !(dist(e) > dist(cur)).
            if (cur && e && !(BoxDistance(e, px, py) > BoxDistance(cur, px, py))) break;
            *hole-- = e;
            --j;
        }
        *hole = cur;
    }
}

// 0x00B78F40 — cdecl. Sift-up: while the parent sorts after *slot (or either side is
// null — EA shifts anyway), move the parent down into the hole; stop at `bottom`.
// EA's returns are path junk (parent index / the element as an int) — returns base.
KnnBox** HeapSiftUp(KnnBox** base, int32_t bottom, int32_t hole, KnnBox** slot, float px,
                    float py)
{
    int32_t parent = (hole - 1) >> 1;
    if (hole <= bottom) {
        base[hole] = *slot;
        return base;
    }
    for (;;) {
        KnnBox* pe = base[parent];
        KnnBox* ee = *slot;
        // Stop when the element sorts at-or-before the parent, unordered included
        // (comiss+jbe): !(dist(ee) > dist(parent)).
        if (pe && ee && !(BoxDistance(ee, px, py) > BoxDistance(pe, px, py))) break;
        base[hole] = pe;
        hole = parent;
        parent = (hole - 1) >> 1;
        if (hole <= bottom) {
            base[hole] = *slot;
            return base;
        }
    }
    base[hole] = *slot;
    return base;
}

// 0x00B79FF0 — cdecl. Sift down: at each level take the larger child, except when a
// child pointer is null the left is taken WITHOUT comparing (EA quirk). If the walk
// stops exactly at `count`, the last left child is moved up first; then *slot is
// sift-up-inserted at the final hole.
KnnBox** HeapSiftDown(KnnBox** base, int32_t bottom, int32_t count, int32_t hole,
                      KnnBox** slot, float px, float py)
{
    int32_t child = 2 * hole + 2;
    while (child < count) {
        KnnBox* right = base[child];
        KnnBox* left = base[child - 1];
        if (!right || !left || BoxDistance(left, px, py) > BoxDistance(right, px, py)) {
            --child;  // null child: left taken with no distance compare
        }
        base[hole] = base[child];
        hole = child;
        child = 2 * child + 2;
    }
    if (child == count) {
        base[hole] = base[count - 1];
        hole = count - 1;
    }
    return HeapSiftUp(base, bottom, hole, slot, px, py);
}

// 0x00B790D0 — cdecl (plain ret, point passed as the two float args). Median of the
// three by distance via IsFartherThanBox on a stack point {px, py}: each branch
// returns the SLOT pointer (s1/s2/s3), never the element.
KnnBox** MedianOfThree(KnnBox** s1, KnnBox** s2, KnnBox** s3, float px, float py)
{
    const KnnPoint point{px, py};
    const KnnBox* b1 = *s1;
    const KnnBox* b2 = *s2;
    if (IsFartherThanBox(&point, b1, b2)) {  // dist(b2) > dist(b1): b1 closer
        if (IsFartherThanBox(&point, b2, *s3)) return s2;
        if (IsFartherThanBox(&point, b1, *s3)) return s3;
        return s1;
    }
    if (IsFartherThanBox(&point, b1, *s3)) return s1;
    if (IsFartherThanBox(&point, b2, *s3)) return s3;
    return s2;
}

// 0x00B792B0 — thiscall, ret 4. Per pointer: when src differs, retain (slot 0) the
// incoming, store it, release (slot 1) the replaced; then copy the four flag bytes.
RefTriple* AssignRefTriple(RefTriple* self, const RefTriple* src)
{
    for (int k = 0; k < 3; ++k) {
        void* incoming = (&src->first)[k];
        void* old = (&self->first)[k];
        if (incoming != old) {
            if (incoming) CallGameVirtual<void, 0>(incoming);
            (&self->first)[k] = incoming;
            if (old) CallGameVirtual<void, 1>(old);
        }
    }
    for (int k = 0; k < 4; ++k) self->flags[k] = src->flags[k];
    return self;
}

// 0x00B7AA20 — cdecl. Heapify: sift-down each parent slot from ((count-2)>>1) down to
// 0 (rootBound and hole are both the current index). Ranges of < 2 are untouched.
void HeapifyRange(KnnBox** begin, KnnBox** end, float px, float py)
{
    const int32_t count = (int32_t)((char*)end - (char*)begin) >> 2;
    if (count < 2) return;
    int32_t hole = ((count - 2) >> 1) + 1;
    do {
        --hole;
        KnnBox* elem = begin[hole];
        HeapSiftDown(begin, hole, count, hole, &elem, px, py);
    } while (hole != 0);
}

// 0x00B7AA70 — cdecl. Heapsort: park the root at the tail, re-sift with count-1,
// repeat while more than 1 element remains.
void HeapSortRange(KnnBox** begin, KnnBox** end, float px, float py)
{
    int32_t count = (int32_t)((char*)end - (char*)begin) >> 2;
    if (count <= 1) return;
    KnnBox** last = end - 1;
    do {
        KnnBox* root = *begin;
        KnnBox* elem = *last;
        *last = root;
        HeapSiftDown(begin, 0, count - 1, 0, &elem, px, py);
        --last;
        count = (int32_t)(((char*)last + 4) - (char*)begin) >> 2;
    } while (count > 1);
}

// 0x00B7ABF0 — cdecl, plain ret. Left scan advances while the element is closer than
// the pivot (stops at the first >= pivot; a null element or pivot advances — a null
// pivot eventually walks off, same as EA); right scan (decrementing, nulls skipped)
// stops at the first element <= pivot; swap and advance. The pivot is held by value
// and may be met during the scan. Returns the split slot (unsigned bound test).
KnnBox** PartitionByDistance(KnnBox** begin, KnnBox** end, KnnBox** pivot, float px,
                             float py)
{
    KnnBox* pv = *pivot;
    KnnBox** left = begin;
    KnnBox* le;
    for (;;) {
        for (;;) {
            le = *left;
            // Stop at-or-beyond the pivot, unordered included: !(dist(pv) > dist(le)).
            if (le && pv && !(BoxDistance(pv, px, py) > BoxDistance(le, px, py))) break;
            ++left;
        }
        KnnBox* re;
        for (;;) {
            re = *--end;
            if (re && !(BoxDistance(re, px, py) > BoxDistance(pv, px, py))) break;
        }
        if (left >= end) break;
        *left = re;
        *end = le;
        ++left;
    }
    return left;
}

// 0x00B7AF00 — cdecl. Keep the nearest (heapEnd-base) of [base, candidatesEnd):
// heapify the kept range, then for each candidate: null-or-closer-than-root swaps it
// in (the evicted root parks in the candidate's slot, heap size unchanged), and the
// kept range is heapsorted at the end. (The pre-compaction claim that EA passes the
// candidate-end pointer as the heapify px is FALSE — the push is a4, the real px.)
void KeepNearestK(KnnBox** base, KnnBox** heapEnd, KnnBox** candidatesEnd, float px,
                  float py)
{
    HeapifyRange(base, heapEnd, px, py);
    const int32_t heapCount = (int32_t)((char*)heapEnd - (char*)base) >> 2;
    for (KnnBox** cand = heapEnd; cand < candidatesEnd; ++cand) {
        KnnBox* c = *cand;
        KnnBox* r = *base;
        if (!c || !r || BoxDistance(r, px, py) > BoxDistance(c, px, py)) {
            KnnBox* elem = c;
            *cand = r;
            HeapSiftDown(base, 0, heapCount, 0, &elem, px, py);
        }
    }
    HeapSortRange(base, heapEnd, px, py);
}

// 0x00B7B490 — cdecl. Partition loop while the range is larger than 28 elements and
// the budget lasts: median-of-three pivot, partition, recurse into the RIGHT half
// with budget-1 (the loop's own budget slot is decremented too — IDA's `--a3` is
// real, the earlier "budget never decrements" claim was wrong), then continue on the
// left half. When the budget lands on exactly 0 the remaining left range is finished
// by KeepNearestK (heapify+heapsort with a2==a3, i.e. no candidates).
void IntroSortByDistance(KnnBox** begin, KnnBox** end, int32_t budget, float px, float py)
{
    KnnBox** left = begin;
    KnnBox** right = end;
    int32_t size = (int32_t)(((char*)right - (char*)left) & ~(std::ptrdiff_t)3);
    if (size > 0x70) {
        while (budget > 0) {
            const int32_t count = (int32_t)(((char*)right - (char*)left) >> 2);
            const int32_t mid = count / 2;
            KnnBox** median = MedianOfThree(left, left + mid, right - 1, px, py);
            KnnBox** boundary = PartitionByDistance(left, right, median, px, py);
            --budget;
            IntroSortByDistance(boundary, right, budget, px, py);
            right = boundary;
            size = (int32_t)(((char*)right - (char*)left) & ~(std::ptrdiff_t)3);
            if (size <= 0x70) break;
        }
    }
    if (budget == 0) KeepNearestK(left, right, right, px, py);
}

// 0x00B7BA00 — cdecl. Full sort: bit-length budget intro pass, then insertion sorts —
// the whole range when <= 28 elements, else [begin, begin+28) and [begin+28, end).
// Requires end > begin: EA's bit-length loop (arithmetic shift) never terminates on
// an inverted range, which is not reproduced (callers pass end > begin).
void SortByDistance(KnnBox** begin, KnnBox** end, float px, float py)
{
    if (begin == end) return;
    const int32_t count = (int32_t)(end - begin);
    int32_t bits = 0;
    for (int32_t v = count; v != 0; v >>= 1) ++bits;
    IntroSortByDistance(begin, end, 2 * bits - 2, px, py);
    if (count <= 28) {
        InsertSortByDistance(begin, end, px, py);
    } else {
        InsertSortByDistance(begin, begin + 28, px, py);
        InsertSortByDistanceFrom(begin + 28, end, px, py);
    }
}

}  // namespace ts3::misc
