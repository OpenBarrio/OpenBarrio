// Reconstruction of TS3W.exe unit cls_bfb990_UserTool - the UserTool/AppendRoofWallLocator locator
// class (AddDiagonalShedWallLocators) and the UserTool/PlacementHelper helpers that drive it.
//
// Two objects are involved. A locator is an intrusive-refcounted block (refcount at +0x04) of 228
// bytes (record types 12 and 25) or 244 bytes (type 14) whose vtable is 0x01073240. The block that
// 0x00CC0D70/0x00CC1D60 use as `this` is the four-dword PlacementHelper frame object
// {vtable, x, tool, handler} - 0x00CC68B0 builds one from its own arguments at 0x00CC6970..0x00CC699A
// and the two drivers forward their last four arguments into it verbatim.
//
// Nothing outside this unit is rewritten yet, so every foreign callee goes through the layout
// bridge (ts3::CallGameMethod / CallGameFunction / CallGameVirtual).
//

#include <cmath>
#include <cstdint>
#include <cstring>

#include "ts3/layout.h"
#include "ts3/usertool/cls_bfb990_UserTool.h"

namespace ts3::usertool {
namespace {

constexpr uintptr_t kVtableLocator = 0x01073240u;    // installed by 0x00BFB990
constexpr uintptr_t kVtblRangeResult = 0x0107DDFCu;  // written by 0x00CC1F40
constexpr uintptr_t kVtblListResult = 0x0107DE04u;   // written by 0x00CC1F90
constexpr uintptr_t kStrLocator = 0x0107DE84u;  // "UserTool/AppendRoofWallLocator/AddDiagonalShedWallLocators"
constexpr uintptr_t kLocatorTree = 0x0114C900u;  // the tree 0x00CC68B0 walks for locator records
constexpr float kHalfDiagScale = 0.7071085572242737f;  // float at 0x0107DE44

// 0x00C035A4 reserves exactly 16 bytes, passed by address to 0x00BFE0B0.
struct WallLocatorScratch { int32_t words[4]; };
static_assert(sizeof(WallLocatorScratch) == 16, "wall locator scratch ABI");

template <class T>
T Peek(const void* base, size_t offset)
{
    T value;
    std::memcpy(&value, static_cast<const uint8_t*>(base) + offset, sizeof value);
    return value;
}

template <class T>
void Poke(void* base, size_t offset, T value)
{
    std::memcpy(static_cast<uint8_t*>(base) + offset, &value, sizeof value);
}

float PeekF(const void* base, size_t offset) { return Peek<float>(base, offset); }

// The x87 sequence at 0x00CC0E79..0x00CC0EA6 (and its three siblings): hypot(dx, dy) scaled by
// 0.70710856f, written back as a float, reloaded and converted with fistp.
struct Diagonal
{
    int32_t rounded;
    float value;
};

Diagonal ComputeDiagonal(float dx, float dy)
{
    Diagonal out{};
    out.value = static_cast<float>(std::sqrt(static_cast<double>(dx) * static_cast<double>(dx) +
                                             static_cast<double>(dy) * static_cast<double>(dy)) *
                                   static_cast<double>(kHalfDiagScale));
    // fistp with the default control word: round to nearest, ties to even, 0x80000000 for NaN and
    // for anything outside int32.
    const double r = std::nearbyint(static_cast<double>(out.value));
    if (!(r >= -2147483648.0 && r <= 2147483647.0))
        out.rounded = static_cast<int32_t>(0x80000000u);
    else
        out.rounded = static_cast<int32_t>(r);
    return out;
}

// The intrusive release both helpers end with (0x00CC0D70 at 0x00CC107D, 0x00CC1D60 at 0x00CC1EC1):
// lock xadd [obj+4], -1; when the count reaches zero the original writes 1 back - so the destructor
// sees a live object - and calls vtable slot 0 with flag 1.
int32_t ReleaseLocator(void* obj)
{
#if defined(_M_IX86)
    // Both helpers test the pointer before touching the count (0x00CC1EC1 / 0x00CC0E57): a null
    // locator is simply dropped.
    if (!obj) return 0;
    volatile long* const refs = reinterpret_cast<volatile long*>(static_cast<uint8_t*>(obj) + 0x04);
    const long old = _InterlockedExchangeAdd(refs, -1L);
    // The original leaves EAX = obj+4 on this path and 0x00CC0D70 hands it back to its caller.
    if (old - 1L != 0L) return static_cast<int32_t>(reinterpret_cast<uintptr_t>(refs));
    _InterlockedExchange(refs, 1L);
    return ts3::CallGameVirtual<int32_t, 0>(obj, 1u);
#else
    ts3::AbortNotRewritten("intrusive release of a game locator object");
    (void)obj;
    return 0;
#endif
}

// 0x00CC1D60 first switch: index bytes at 0x00CC1F00, jump table at 0x00CC1EF4, keyed type-1 over
// types 1..25 (verified against the disassembly). 0 -> 0x00CBD100 for {1,2,3,18,19,20},
// 1 -> 0x00CC0B90 for {10,11,12,23,24,25}. Second switch: index bytes at 0x00CC1F28, jump table at
// 0x00CC1F1C, keyed type-3 over types 3..25. 0 -> 0x00CBD210 for {3,6,20},
// 1 -> 0x00CC0D70 for {12,14,25}. Everything else falls through, and an out-of-range key jumps past
// both tables straight to the release.
enum Action : uint8_t { kActionFirst = 0, kActionSecond = 1, kActionNone = 2 };

Action FirstAction(int32_t type)
{
    const uint32_t i = static_cast<uint32_t>(type - 1);
    if (i > 0x18u) return kActionNone;
    switch (i + 1u)
    {
        case 1u: case 2u: case 3u: case 18u: case 19u: case 20u: return kActionFirst;
        case 10u: case 11u: case 12u: case 23u: case 24u: case 25u: return kActionSecond;
        default: return kActionNone;
    }
}

Action SecondAction(int32_t type)
{
    const uint32_t i = static_cast<uint32_t>(type - 3);
    if (i > 0x16u) return kActionNone;
    switch (i + 3u)
    {
        case 3u: case 6u: case 20u: return kActionFirst;
        case 12u: case 14u: case 25u: return kActionSecond;
        default: return kActionNone;
    }
}

#if defined(_M_IX86)
// comiss + ja: the alternative pointer is taken only when the first operand is strictly above the
// second, so equal - and unordered, i.e. NaN - values keep the first operand. The original computes
// two pairs of these; both pairs give the same value, which is why every callee gets it twice.
inline float MinKeepFirst(float a, float b) { return (a > b) ? b : a; }
// ... and the mirror image (0x00CC1DE2..0x00CC1E0E): the second pair of slots holds the maxima.
inline float MaxKeepFirst(float a, float b) { return (a > b) ? a : b; }
#endif

}  // namespace

// 0x00BFB990: C1 constructor of AddDiagonalShedWallLocators. `self` is the fresh 228-byte block,
// the five arguments are forwarded untouched to the base constructor at 0x00BFADB0.
void* __fastcall AddDiagonalShedWallLocatorsCtor(void* self, void* /*edx*/, void* a2, void* a3,
                                                 void* a4, void* a5, void* a6)
{
    ts3::CallGameMethod<void, 0x00BFADB0>(self, a2, a3, a4, a5, a6);
    Poke<void*>(self, 0x00, reinterpret_cast<void*>(kVtableLocator));
    return self;
}

// 0x00C035A0: class vtable slot. EA quirk kept: the 16-byte scratch block (the original instruction sequence + lea)
// is handed to the callee without ever being initialized.
int32_t __fastcall AddDiagonalShedWallLocatorsGetWallLocatorData(void* self, void* /*edx*/,
                                                                 int32_t mode)
{
    WallLocatorScratch scratch;  // deliberately uninitialized, as in EA
    return ts3::CallGameMethod<int32_t, 0x00BFE0B0>(self, 0, mode, &scratch);
}

// 0x00CC0D70: __thiscall(block, record, gridX), ret 8.
int32_t __fastcall AppendRoofWallLocator(void* self, void* /*edx*/, const void* record,
                                         int32_t gridX)
{
    const int32_t type = Peek<int32_t>(record, 0x2c);
    if (type != 12 && type != 14 && type != 25)
        return type - 25;  // EA: ((type-12)-2)-11, nothing else touched

#if defined(_M_IX86)
    const uint8_t flag38 = Peek<uint8_t>(record, 0x38);
    const uint32_t size = (type == 14) ? 244u : 228u;
    void* obj = ts3::CallGameFunction<void*, 0x004E3F90>(
        size, reinterpret_cast<void*>(kStrLocator), 0, 0, 0, 0);

    // The original pushes the five constructor arguments in this order (0x00CC0DC6..0x00CC0DD6);
    // ecx is the allocated block, which is only constructed when the allocation succeeded.
    void* const aTool = Peek<void*>(self, 0x08);                                          // block[8]
    void* const aLot = reinterpret_cast<void*>(static_cast<uintptr_t>(Peek<int32_t>(record, 0x0c)));
    void* const p10 = const_cast<void*>(static_cast<const void*>(static_cast<const uint8_t*>(record) + 0x10));
    void* const p1c = const_cast<void*>(static_cast<const void*>(static_cast<const uint8_t*>(record) + 0x1c));
    void* const a28 = reinterpret_cast<void*>(static_cast<uintptr_t>(Peek<int32_t>(record, 0x28)));
    if (obj)
    {
        if (type == 25)
            AddDiagonalShedWallLocatorsCtor(obj, nullptr, aTool, aLot, p10, p1c, a28);
        else if (type == 14)
            ts3::CallGameMethod<void, 0x00BFAED0>(obj, aTool, aLot, p10, p1c, a28);
        else
            ts3::CallGameMethod<void, 0x00BFADB0>(obj, aTool, aLot, p10, p1c, a28);
    }
    // Even a failed allocation goes through the intrusive-pointer assignment (0x00B96150,
    // __thiscall(&slot, value)); the original forwards its EAX on the early return, and reloads
    // obj from the slot it just wrote (0x00CC0E53).
    void* slot = nullptr;
    const int32_t assigned = ts3::CallGameMethod<int32_t, 0x00B96150>(&slot, obj);
    obj = slot;
    if (!obj)
        return assigned;  // EAX = the assignment's result, forwarded untouched

    ts3::CallGameMethod<void, 0x00A48900>(obj);

    // The four blocks between 0x00CC0E66 and 0x00CC107B differ only in which diagonal they measure,
    // which record byte selects the second float pair and which tag they pass. Type 14 behaves the
    // opposite way to types 12/25: with flag38 set, type 14 gets the short diagonal and types 12/25
    // the long one; with flag38 clear it is the other way round.
    const bool shortDiag = (type == 14) == (flag38 != 0);
    const Diagonal d = ComputeDiagonal(PeekF(obj, shortDiag ? 0x7c : 0x8c) - PeekF(obj, 0x84),
                                      PeekF(obj, shortDiag ? 0x80 : 0x90) - PeekF(obj, 0x88));
    // There is no type-25 exception: 0x00CC0EEC..0x00CC0EF4 sends types 12 and 25 to the same block
    // (only types outside {12,14,25} jump to the release, and those already returned above).
    {
        const uint8_t selector = Peek<uint8_t>(record, shortDiag ? 0x39 : 0x3a);
        // The selected float pair is the diagonal the measurement did not use, or the anchor pair
        // at +0x84/+0x88 when that flag is clear.
        float pair[2];
        if (selector)
        {
            pair[0] = PeekF(obj, shortDiag ? 0x8c : 0x7c);
            pair[1] = PeekF(obj, shortDiag ? 0x90 : 0x80);
        }
        else
        {
            pair[0] = PeekF(obj, 0x84);
            pair[1] = PeekF(obj, 0x88);
        }
        // this->vtbl[1](this, record[0xc], tag, rounded, &pair[0]) - four stack args, the slot
        // returns 0x10, which the caller's epilogue needs (no add esp follows 0x00CC107B). The
        // original also spills gridX and the rounded value next to the pair in its own frame
        // (0x00CC0EAE..0x00CC0ECB) but never passes them: gridX is dead.
        // The original loads [tool] / [[tool]+4] into EDX before the branch and keeps that vtable
        // pointer even when the nested construction overwrites it - a callee with a real vtable
        // would call through the stale entry, so the pointer is snapshotted here too.
        ts3::CallGameVirtual<int32_t, 1>(self, aLot, static_cast<uint32_t>(shortDiag ? 5u : 7u),
                                         static_cast<uint32_t>(d.rounded), pair);
    }
    (void)gridX;
    return ReleaseLocator(obj);  // EAX: obj+4 when the count survives, the destructor's result after
                                 // the delete - 0x00CC1D60 reloads its own slot either way
#endif
    return 0;
}

// 0x00CC1D60: __thiscall(block, id), ret 4.
void __fastcall ProcessWallLocator(void* self, void* /*edx*/, uint32_t id)
{
#if defined(_M_IX86)
    void* const handler = Peek<void*>(self, 0x0c);
    if (!ts3::CallGameMethod<uint8_t, 0x00B974B0>(handler, id))
        return;
    void* obj = nullptr;
    if (!ts3::CallGameMethod<uint8_t, 0x00B971F0>(handler, id, &obj))
    {
        // Unresolved id: the object - if the lookup produced one - is still released, and both
        // switches are skipped. The four "coordinate" computations the original performs in this
        // path are dead code; they write registers that nothing reads.
        ReleaseLocator(obj);
        return;
    }
    if (!obj) return;

    // The original fills four scratch slots: the two minima (first operand kept on ties, which is
    // also what comiss does for NaN) and the two maxima of the same pairs.
    const float loX = MinKeepFirst(PeekF(obj, 0x10), PeekF(obj, 0x1c));
    const float loY = MinKeepFirst(PeekF(obj, 0x14), PeekF(obj, 0x20));
    const float hiX = MaxKeepFirst(PeekF(obj, 0x1c), PeekF(obj, 0x10));
    const float hiY = MaxKeepFirst(PeekF(obj, 0x20), PeekF(obj, 0x14));
    const uint32_t key = Peek<uint32_t>(obj, 0x18);  // esi at 0x00CC1E15: the trailing int arg of
                                                      // every arm, and the key of CBD100/CBD210
    const int32_t type = Peek<int32_t>(obj, 0x2c);

    switch (FirstAction(type))
    {
        case kActionFirst:
            ts3::CallGameMethod<void, 0x00CBD100>(self, obj, loX, loY, hiX, hiY, key);
            break;
        case kActionSecond:
            ts3::CallGameMethod<void, 0x00CC0B90>(self, obj, key);
            break;
        default: break;
    }
    switch (SecondAction(Peek<int32_t>(obj, 0x2c)))
    {
        case kActionFirst:
            ts3::CallGameMethod<void, 0x00CBD210>(self, obj, loX, loY, hiX, hiY, key);
            break;
        case kActionSecond:
            AppendRoofWallLocator(self, nullptr, obj, key);
            break;
        default: break;
    }
    ReleaseLocator(obj);
#else
    ts3::AbortNotRewritten("ProcessWallLocator game lookups");
#endif
}

// 0x00CC1F40: __cdecl(out16, first, last, a4, a5, a6, a7).
void* __cdecl ProcessRoofLocatorRange(void* out, const uint32_t* first, const uint32_t* last,
                                      void* a4, void* a5, void* a6, void* a7)
{
    void* block[] = {a4, a5, a6, a7};
    for (const uint32_t* it = first; it != last; ++it)
        ProcessWallLocator(block, nullptr, *it);
    // The result block is {vtable, a5, a6, a7}: the original writes the last three arguments, not
    // the three that start the handler block (0x00CC1F63..0x00CC1F80 reads [esp+0x1c] upward).
    Poke<uintptr_t>(out, 0x00, kVtblRangeResult);
    Poke<void*>(out, 0x04, a5);
    Poke<void*>(out, 0x08, a6);
    Poke<void*>(out, 0x0c, a7);
    return out;
}

// 0x00CC1F90: __cdecl(out20, first, last, a4, a5, a6, a7, a8).
void* __cdecl ProcessRoofLocatorList(void* out, uintptr_t first, uintptr_t last, void* a4,
                                     void* a5, void* a6, void* a7, void* a8)
{
    void* block[] = {a4, a5, a6, a7};
#if defined(_M_IX86)
    for (uintptr_t node = first; node != last;
         node = ts3::CallGameFunction<uintptr_t, 0x004E0B20>(node))
        ProcessWallLocator(block, nullptr, Peek<uint32_t>(reinterpret_cast<const void*>(node), 0x10));
#else
    ts3::AbortNotRewritten("ProcessRoofLocatorList iterator helper");
    (void)first;
    (void)last;
#endif
    Poke<uintptr_t>(out, 0x00, kVtblListResult);
    Poke<void*>(out, 0x04, a5);
    Poke<void*>(out, 0x08, a6);
    Poke<void*>(out, 0x0c, a7);
    Poke<void*>(out, 0x10, a8);
    return out;
}

namespace {
struct RoofNode { RoofNode* right; RoofNode* left; RoofNode* parent; uint32_t color; int32_t id; };
struct RoofSet {
    void* allocator;
    RoofNode* left;
    RoofNode* right;
    RoofNode* root;
    uint32_t size;
    uint32_t flags;
    const char* name;
};
struct RoofTable {
    void* allocator;
    void** buckets;
    uint32_t count;
    uint32_t size;
    float load;
    float growth;
    uint32_t flags;
    const char* name;
};
struct RoofGuard { void* vtable; void* words[3]; void* object; void** first; void** last; void* rest[17]; };
#if defined(_M_IX86)
static_assert(offsetof(RoofSet, root) == 0x0c);
static_assert(offsetof(RoofNode, id) == 0x10);
static_assert(offsetof(RoofTable, buckets) == 4 && sizeof(RoofTable) == 32);
static_assert(offsetof(RoofGuard, object) == 0x10 && offsetof(RoofGuard, first) == 0x14);
static_assert(offsetof(RoofRects, last) == 4 && offsetof(RoofRects, capacity) == 8);
#endif
struct RoofRect { float x0, y0, x1, y1; };
RoofRect ReadRoofRect(void* obj)
{
    RoofRect r{PeekF(obj, 0x5c), PeekF(obj, 0x60), PeekF(obj, 0x64), PeekF(obj, 0x68)};
    if (ts3::CallGameFunction<uint8_t, 0x00C03F90>(Peek<int32_t>(obj, 0x2c))) {
        const float x = PeekF(obj, 0x5c), y = PeekF(obj, 0x60);
        const float xx = PeekF(obj, 0x64), yy = PeekF(obj, 0x68);
        r = {x > xx ? xx : x, y > yy ? yy : y, xx > x ? xx : x, yy};
        // EA bug: upper Y compares yy with itself, not with y. NaNs keep the first X.
    }
    return r;
}
bool SetHas(RoofSet* set, int32_t id)
{
    RoofNode* found = reinterpret_cast<RoofNode*>(&set->left);
    RoofNode* const end = found;
    for (RoofNode* n = set->root; n; ) {
        if (n->id < id) n = n->right;
        else { found = n; n = n->left; }
    }
    return found != end && id >= found->id;
}
void ClearRoofTable(RoofTable& t)
{
    ts3::CallGameMethod<void, 0x007D3AF0>(&t, t.buckets, t.count);
    t.size = 0;
    if (t.count > 1) ts3::CallGameFunction<void, 0x004E3F70>(t.buckets);
}
struct RoofIterator {
    void** bucket;
    void* node;
    void* end;
    explicit RoofIterator(RoofTable& t) : bucket(t.buckets), node(*bucket), end(t.buckets[t.count]) {
        while (!node) node = *++bucket;
    }
    void Next() {
        node = Peek<void*>(node, 8);
        while (!node) node = *++bucket;
    }
};
} // namespace

// 0x00CC4EA0, cdecl, no stack arguments popped. EAX is the input vector on every exit.
RoofIds* __cdecl CollectRoofOverlaps(void* tool, RoofIds* roofs, const RoofIds* candidates,
                                    void* selected, RoofRects* rectangles)
{
    void* const handler = Peek<void*>(tool, 0x2ac);
    for (uint32_t* it = roofs->first; it != roofs->last; ++it) {
        void* obj = nullptr;
        if (ts3::CallGameMethod<uint8_t, 0x00B971F0>(handler, *it, &obj)) {
            const RoofRect outer = ReadRoofRect(obj);
            float* const dest = rectangles->last;
            if (reinterpret_cast<uintptr_t>(dest) >= reinterpret_cast<uintptr_t>(rectangles->capacity))
                ts3::CallGameMethod<void, 0x00ABEAD0>(rectangles, dest, &outer);
            else {
                rectangles->last = reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(dest) + 16);
                if (dest) std::memcpy(dest, &outer, sizeof outer);
            }
            uint32_t* const candidateEnd = candidates->last;
            for (uint32_t* c = candidates->first; c != candidateEnd; ++c) {
                if (SetHas(static_cast<RoofSet*>(selected), static_cast<int32_t>(*c))) continue;
                void* other = nullptr;
                if (ts3::CallGameMethod<uint8_t, 0x00B971F0>(handler, *c, &other)) {
                    const RoofRect r = ReadRoofRect(other);
                    if ((r.x0 >= outer.x0 && outer.x1 >= r.x1 && r.y0 >= outer.y0 && outer.y1 >= r.y1) ||
                        (outer.x1 > r.x0 && r.x1 > outer.x0 && outer.y1 > r.y0 && r.y1 > outer.y0) ||
                        (outer.x0 >= r.x0 && r.x1 >= outer.x1 && outer.y0 >= r.y0 && r.y1 >= outer.y1)) {
                        void* result[2];
                        // Only the low byte of the hint is defined by EA; the callee reads a bool.
                        ts3::CallGameMethod<void, 0x00628110>(selected, result, c, 0u);
                    }
                }
                ReleaseLocator(other);
            }
        }
        ReleaseLocator(obj);
    }
    return roofs;
}

// 0x00CC68B0: all nine arguments are cdecl; return contract is AL only.
uint8_t __cdecl PlaceRoofs(void* tool, RoofIds* roofs, const RoofIds* candidates, void* remove,
                           void* walls, void* placement, uint32_t mode, uint32_t flags,
                           uint32_t extra)
{
    void* const lot = Peek<void*>(tool, 0x298);
    if (roofs->first == roofs->last) return 0;
    RoofSet selected{};
    selected.left = selected.right = reinterpret_cast<RoofNode*>(&selected.left);
    selected.name = reinterpret_cast<const char*>(0x0107E050u);
    RoofRects rectangles{nullptr, nullptr, nullptr, selected.name};
    CollectRoofOverlaps(tool, roofs, candidates, &selected, &rectangles);
    RoofTable wallTable{nullptr, reinterpret_cast<void**>(kLocatorTree), 1, 0, 1.f, 2.f, 0,
                        reinterpret_cast<const char*>(0x0107E038u)};
    void* result[5];
    ProcessRoofLocatorRange(result, roofs->first, roofs->last, reinterpret_cast<void*>(kVtblRangeResult),
                            &wallTable, tool, Peek<void*>(tool, 0x2ac));
    RoofIterator check(wallTable);
    RoofGuard guard; // constructed by the original; do not preinitialize its private state
    ts3::CallGameMethod<void, 0x00B1A680>(&guard, tool);
    uint8_t processed = 0;
    bool valid = true;
    while (check.node != check.end) {
        if (!valid) break;
        valid = ts3::CallGameMethod<uint8_t, 0x00D47D00>(&guard, check.node, flags) != 0;
        check.Next();
    }
    if (valid) {
        for (RoofIterator it(wallTable); it.node != it.end; it.Next())
            if (walls) ts3::CallGameMethod<void, 0x00D2E380>(walls, tool, it.node, 1);
        uint32_t* const roofEnd = roofs->last;
        for (uint32_t* it = roofs->first; it != roofEnd; ++it) {
            processed = 1;
            if (mode) ts3::CallGameFunction<void, 0x00CC0B10>(tool, *it, mode);
            if (remove) ts3::CallGameMethod<void, 0x00D29D00>(remove, tool, *it, extra);
        }
        RoofTable roofTable{nullptr, reinterpret_cast<void**>(kLocatorTree), 1, 0, 1.f, 2.f, 0,
                            reinterpret_cast<const char*>(0x0107E020u)};
        ProcessRoofLocatorList(result, reinterpret_cast<uintptr_t>(selected.right),
                              reinterpret_cast<uintptr_t>(&selected.left),
                              reinterpret_cast<void*>(kVtblListResult), &roofTable, tool,
                              Peek<void*>(tool, 0x2ac), &rectangles);
        void* const manager = Peek<void*>(reinterpret_cast<void*>(0x011ED034u), 0);
        const uint8_t* const material = static_cast<uint8_t*>(
            ts3::CallGameMethod<void*, 0x00C8BC10>(manager, 2)) + 8;
        for (RoofIterator it(roofTable); it.node != it.end; it.Next()) {
            if (ts3::CallGameMethod<uint8_t, 0x00B49430>(lot, it.node)) {
                const void* const existing = ts3::CallGameMethod<void*, 0x00B49500>(lot, it.node, 0);
                if (std::memcmp(existing, material, 16) != 0) continue;
                ts3::CallGameMethod<void, 0x00D2E380>(walls, tool, it.node, 1);
            }
            ts3::CallGameMethod<void, 0x00D2DC30>(placement, tool, it.node, material, 0, 0);
        }
        ClearRoofTable(roofTable);
    }
    guard.vtable = reinterpret_cast<void*>(0x0106AE04u);
    for (void** it = guard.first; it < guard.last; ++it)
        if (*it) ts3::CallGameVirtual<void, 1>(*it);
    if (guard.first) ts3::CallGameFunction<void, 0x004E3F70>(guard.first);
    if (guard.object) ts3::CallGameVirtual<void, 1>(guard.object);
    guard.vtable = reinterpret_cast<void*>(0x0107396Cu);
    ClearRoofTable(wallTable);
    if (rectangles.first) ts3::CallGameFunction<void, 0x004E3F70>(rectangles.first);
    for (RoofNode* n = selected.root; n; ) {
        ts3::CallGameMethod<void, 0x006A0850>(&selected.left, n->right);
        RoofNode* next = n->left;
        ts3::CallGameFunction<void, 0x004E3F70>(n);
        n = next;
    }
    return processed;
}

}  // namespace ts3::usertool
