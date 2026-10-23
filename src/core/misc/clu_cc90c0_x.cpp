// Cluster unit clu_cc90c0_x (subsystem misc): rewrites of the seventeen TS3W.exe
// functions declared in ts3/misc/clu_cc90c0_x.h. Original addresses are in the
// header's per-function comments; this file adds the shared machinery:
//
//  - Greater(): the unit's comparator. Ids equal or x equal -> compare y, else
//    compare x, every test in ordered (NaN-failing) form — exactly the game's
//    comiss/`ja` + ordered-`==` idiom. Never write `<=` on these floats: the game
//    uses machine jbe, which is TRUE on unordered, so Le is !Greater.
//  - StoreX87(): float stores the original does with fld/fstp, which quiet a
//    signaling NaN (set 0x400000, keep sign and payload; ±inf unchanged — probed
//    in the original binary: 7F800001 -> 7FC00001). Stores done with movss in the
//    original stay bit-exact (StoreRaw / raw memcpy).
//  - Record copies touch 0x24 bytes (the nine fields) and never the 4-byte gap.
//  - The chain builders at 0x00CC9A50/0x00CC9B00/0x00CC9BF0 are __thiscall with
//    two stack arguments (ret 8) and read [ecx+0x18] — ecx must be the TreeHeader
//    being built, which the game threads through unchanged.
//
// EA behaviours kept on purpose (all affect outputs):
//  - IntroSortPass heapsorts only when its depth budget reaches exactly 0.
//  - Quantize1e4/Quantize003 return +1 for small values of BOTH signs (the
//    negative branch's early return is not negated).
//  - SwapRecords quiets a's destination floats but hands b a's raw bits.
//  - InitTree9cb0/9d30/9db0 leave value14 at 0 when the source chain is empty.

#include "ts3/misc/clu_cc90c0_x.h"

#include <cmath>
#include <cstdint>
#include <cstring>

namespace ts3::misc {
namespace {

// ---------------------------------------------------------------------------
// Comparator and record copies
// ---------------------------------------------------------------------------

bool Greater(const QRec& a, const QRec& b)
{
    if (a.id0 == b.id0 && a.id1 == b.id1 && a.id2 == b.id2)
        return a.y > b.y;
    if (a.x == b.x)
        return a.y > b.y;
    return a.x > b.x;
}

// Pivot snapshot forms of the same test (Partition keeps the pivot's compare fields
// in registers; after a swap the memory copy may have moved).
bool GreaterPivot(const uint32_t id0, const uint32_t id1, const uint32_t id2, const float px,
                  const float py, const QRec& e)
{
    if (e.id0 == id0 && e.id1 == id1 && e.id2 == id2)
        return py > e.y;
    if (e.x == px)
        return py > e.y;
    return px > e.x;
}

bool GreaterElem(const uint32_t id0, const uint32_t id1, const uint32_t id2, const float px,
                 const float py, const QRec& e)
{
    if (e.id0 == id0 && e.id1 == id1 && e.id2 == id2)
        return e.y > py;
    if (e.x == px)
        return e.y > py;
    return e.x > px;
}

// fld/fstp store: quiet a signaling NaN the way the x87 does.
void StoreX87(float* dst, const float v)
{
    uint32_t bits;
    std::memcpy(&bits, &v, sizeof bits);
    if ((bits & 0x7F800000u) == 0x7F800000u && (bits & 0x007FFFFFu) != 0 &&
        (bits & 0x00400000u) == 0)
        bits |= 0x00400000u;
    std::memcpy(dst, &bits, sizeof bits);
}

// Copy the nine used fields; the three floats go through the x87 (originals store
// them with fld/fstp). The gap stays untouched.
void CopyQuiet(QRec* dst, const QRec& src)
{
    std::memcpy(static_cast<void*>(dst), static_cast<const void*>(&src), 0x24);
    StoreX87(&dst->x, src.x);
    StoreX87(&dst->y, src.y);
    StoreX87(&dst->f3, src.f3);
}

// Capture the nine used fields bit-exactly (originals read them with movss/mov).
void Capture(QRec* dst, const QRec& src)
{
    std::memcpy(static_cast<void*>(dst), static_cast<const void*>(&src), 0x24);
}

// ---------------------------------------------------------------------------
// The chain builders: __thiscall game functions with two stack arguments whose
// ecx must be the TreeHeader under construction.
// ---------------------------------------------------------------------------

template <class R, unsigned kAddress, class Self, class... Args>
R CallGameThreadedEcx(Self* self, Args... args)
{
#if defined(_M_IX86)
    using Fn = R(__thiscall*)(Self*, Args...);
    return reinterpret_cast<Fn>(static_cast<uintptr_t>(kAddress))(self, args...);
#else
    (void)self;
    ((void)args, ...);
    AbortNotRewritten("the clu_cc90c0_x chain builder");
    return R{};
#endif
}

// A built tree node: only the links matter to this unit (the payload was written by
// the game's AD83F0).
struct BuiltNode {
    uint32_t left;
    uint32_t right;
};

BuiltNode* NodeAt(const uint32_t addr)
{
    return reinterpret_cast<BuiltNode*>(static_cast<uintptr_t>(addr));
}

template <unsigned kBridge>
TreeHeader* InitTreeImpl(TreeHeader* self, const TreeHeader* src)
{
    self->leftmost = 0;
    self->rightmost = 0;
    self->root = 0;
    self->flag = 0;
    self->value14 = 0;
    self->value18 = src->value18;
    const uint32_t sentinel = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&self->leftmost));
    self->leftmost = sentinel;   // [edi] = edi: the empty header points at itself
    self->rightmost = sentinel;  // [esi+8] = edi
    self->root = 0;
    self->flag = 0;  // the byte store at +0x10 after the dword zero
    self->value14 = 0;
    if (src->root) {
        void* chain = reinterpret_cast<void*>(static_cast<uintptr_t>(src->root));
        BuiltNode* root =
            CallGameThreadedEcx<BuiltNode*, kBridge>(self, chain,
                                                     reinterpret_cast<void*>(static_cast<uintptr_t>(sentinel)));
        self->root = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(root));
        BuiltNode* n = root;
        while (n->left)
            n = NodeAt(n->left);
        self->leftmost = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(n));
        n = root;
        while (n->right)
            n = NodeAt(n->right);
        self->rightmost = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(n));
        self->value14 = src->value14;  // EA: only copied when a chain was built
    }
    return self;
}

// ---------------------------------------------------------------------------
// Quantizer core: the x87 sequence fld t; fist r; fild r; fsubrp st(1) stores
// (float)(r - t) — FSUBRP reverses the operands — then r + 1 - CF where CF is the
// fraction's bit pattern < 0x80000001 (fraction >= -0.0). So the result is r when
// r >= t and r + 1 when r < t: the CEIL of t, whatever the fist rounding made.
// lrint honours the current rounding mode just like fist; out-of-range and NaN take
// the x87 integer-indefinite result, INT32_MIN. r - t is exact in double for every
// t this can see (|t| <= 2^31), so the float cast matches the fstp store.
// ---------------------------------------------------------------------------

int32_t QuantCore(const float t)
{
    const double d = static_cast<double>(t);
    int32_t r;
    if (d >= -2147483648.0 && d < 2147483648.0)
        r = static_cast<int32_t>(std::lrint(d));
    else
        r = INT32_MIN;
    const float frac = static_cast<float>(static_cast<double>(r) - d);
    uint32_t fracBits;
    std::memcpy(&fracBits, &frac, sizeof fracBits);
    const uint32_t rounded = static_cast<uint32_t>(r) + (fracBits < 0x80000001u ? 0u : 1u);
    return static_cast<int32_t>(rounded);
}

int32_t QuantNegate(int32_t v)
{
    return static_cast<int32_t>(0u - static_cast<uint32_t>(v));  // the original instruction sequence (wraps)
}

}  // namespace

// ---------------------------------------------------------------------------
// Sort primitives
// ---------------------------------------------------------------------------

QRec* MedianOfThree(QRec* a, QRec* b, QRec* c)
{
    if (Greater(*b, *a)) {
        if (Greater(*c, *b))
            return b;
        if (Greater(*c, *a))
            return c;
        return a;
    }
    if (Greater(*c, *a))
        return a;
    if (Greater(*c, *b))
        return c;
    return b;
}

QRec* PushHeap(QRec* base, int limit, int hole, const QRec* value)
{
    int parent = (hole - 1) >> 1;
    while (hole > limit) {
        if (!Greater(*value, base[parent]))
            break;
        CopyQuiet(&base[hole], base[parent]);
        hole = parent;
        parent = (parent - 1) >> 1;
    }
    CopyQuiet(&base[hole], *value);
    return &base[hole];
}

void SwapRecords(QRec* a, QRec* b)
{
    QRec saved;
    Capture(&saved, *a);          // a's bits, read with movss
    CopyQuiet(a, *b);             // a <- b: floats through the x87 (quiets sNaN)
    std::memcpy(b, &saved, 0x24);  // b <- a: floats bit-exact (movss)
}

QRec* AdjustHeap(QRec* base, int limit, int count, int hole, const QRec* value)
{
    int cur = hole;
    int idx = 2 * hole + 2;
    bool oneChild = idx == count;
    if (idx < count) {
        for (;;) {
            int pick = idx;
            if (Greater(base[idx - 1], base[idx]))
                pick = idx - 1;
            CopyQuiet(&base[cur], base[pick]);
            cur = pick;
            idx = 2 * pick + 2;
            oneChild = idx == count;
            if (idx >= count)
                break;
        }
    }
    if (oneChild) {
        CopyQuiet(&base[cur], base[idx - 1]);
        cur = idx - 1;
    }
    return PushHeap(base, limit, cur, value);
}

QRec* PopRoot(QRec* base, QRec* end)
{
    QRec temp;
    Capture(&temp, end[-1]);  // old root, bit-exact
    CopyQuiet(end - 1, *base);
    const int count = static_cast<int>(end - base) - 1;
    return AdjustHeap(base, 0, count, 0, &temp);
}

void MakeHeap(QRec* first, QRec* last)
{
    const int n = static_cast<int>(last - first);
    if (n >= 2) {
        int k = (n - 2) >> 1;
        for (;;) {
            QRec temp;
            Capture(&temp, first[k]);
            AdjustHeap(first, k, n, k, &temp);
            if (k == 0)
                break;
            --k;
        }
    }
}

void SortHeap(QRec* first, QRec* last)
{
    int n = static_cast<int>(last - first);
    while (n > 1) {
        PopRoot(first, last);
        --last;
        --n;
    }
}

// ---------------------------------------------------------------------------
// Tree construction (x86 game bridges; empty sources never call into the game)
// ---------------------------------------------------------------------------

TreeHeader* InitTree9cb0(TreeHeader* self, const TreeHeader* src)
{
    return InitTreeImpl<0x00CC9A50>(self, src);
}

TreeHeader* InitTree9d30(TreeHeader* self, const TreeHeader* src)
{
    return InitTreeImpl<0x00CC9B00>(self, src);
}

TreeHeader* InitTree9db0(TreeHeader* self, const TreeHeader* src)
{
    return InitTreeImpl<0x00CC9BF0>(self, src);
}

TreeComposite* InitTreeSet(TreeComposite* self, const TreeComposite* src)
{
    InitTree9cb0(&self->trees[0], &src->trees[0]);
    InitTree9db0(&self->trees[1], &src->trees[1]);
    InitTree9d30(&self->trees[2], &src->trees[2]);
    InitTree9db0(&self->trees[3], &src->trees[3]);
    StoreX87(&self->f70, src->f70);
    StoreX87(&self->f74, src->f74);
    StoreX87(&self->f78, src->f78);
    self->b7c = src->b7c;
    return self;
}

// ---------------------------------------------------------------------------
// Partition, partial sort, introsort
// ---------------------------------------------------------------------------

QRec* Partition(QRec* first, QRec* last, const QRec* pivot)
{
    const uint32_t id0 = pivot->id0;
    const uint32_t id1 = pivot->id1;
    const uint32_t id2 = pivot->id2;
    const float px = pivot->x;
    const float py = pivot->y;
    QRec* lo = first;
    QRec* hi = last;
    for (;;) {
        while (GreaterPivot(id0, id1, id2, px, py, *lo))
            ++lo;
        do
            --hi;
        while (GreaterElem(id0, id1, id2, px, py, *hi));
        if (lo >= hi)
            return lo;
        SwapRecords(lo, hi);
        ++lo;
    }
}

void PartialSort(QRec* first, QRec* middle, QRec* end)
{
    MakeHeap(first, middle);
    if (middle < end) {
        const int rounds =
            static_cast<int>((static_cast<char*>(static_cast<void*>(end)) -
                              static_cast<char*>(static_cast<void*>(middle)) - 1) /
                             0x28) +
            1;
        const int heapCount = static_cast<int>(middle - first);
        for (int i = 0; i < rounds; ++i) {
            QRec* target = middle + i;
            if (Greater(*first, *target)) {
                QRec saved;
                Capture(&saved, *target);
                CopyQuiet(target, *first);
                AdjustHeap(first, 0, heapCount, 0, &saved);
            }
        }
    }
    SortHeap(first, middle);
}

void IntroSortPass(QRec* first, QRec* last, int depth)
{
    int n = static_cast<int>(last - first);
    while (n > 28 && depth > 0) {
        --depth;
        QRec* median = MedianOfThree(first, first + n / 2, last - 1);
        QRec* split = Partition(first, last, median);
        IntroSortPass(split, last, depth);
        last = split;
        n = static_cast<int>(last - first);
    }
    if (depth == 0)
        PartialSort(first, last, last);
}

// ---------------------------------------------------------------------------
// Quantizers and the scale thunk
// ---------------------------------------------------------------------------

int Quantize1e4(float v)
{
    const float kThreshold = 0.0001f;  // 0x00F988E8
    const float kScale = 2.0f;         // 0x00F98A58
    if (v < 0.0f) {                    // comiss +0.0, x; jbe takes the positive path
        const float magnitude = -0.0f - v;
        if (magnitude <= kThreshold)
            return 1;  // EA: not negated on the way out
        return QuantNegate(QuantCore((magnitude - kThreshold) * kScale));
    }
    if (kThreshold >= v)  // jae: ordered >= only — NaNs fall through to the compute
        return 1;
    return QuantCore((v - kThreshold) * kScale);
}

int Quantize003(float v)
{
    const float kThreshold = 0.003f;  // 0x010459E4
    if (v < 0.0f) {
        const float magnitude = -0.0f - v;
        if (magnitude <= kThreshold)
            return 1;
        return QuantNegate(QuantCore(magnitude - kThreshold));
    }
    if (kThreshold >= v)
        return 1;
    return QuantCore(v - kThreshold);
}

float ObjectScale(void* obj)
{
    return CallGameFunction<float, 0x00CEF760>(obj);
}

}  // namespace ts3::misc
