// cls_bf4b20_x.cpp - portable rewrite of the EA class `CreatingNewRoof_3` unit at
// TS3W.exe 0x00BF4B20 (subsystem misc/, 8 entry points, 3260 bytes).
//
// The object samples a width x height grid of floats through vtable accessors that belong
// to other units, so every call into them goes through the bridge in ts3/layout.h: slot
// +0x10 returns the width, +0x14 the height, +0x28 samples one cell and +0x40 is this unit's
// own corner sampler.
//
// Record builders retain the original sampling order and bridge the vector growth helpers.
#include "ts3/misc/cls_bf4b20_x.h"

#include <cmath>
#include <cstring>

#include "ts3/layout.h"

namespace ts3 {
namespace misc {

// The game's vtables for this class and its base, installed by 0x00BFAD20 and by the
// destructor.
constexpr uintptr_t kVtableCreatingNewRoof3 = 0x010729F8;
constexpr uintptr_t kVtableBase = 0x0107396C;

// Constants read out of TS3W.exe, kept as bit patterns so no compiler can pick a
// neighbouring float.  0x01010960 is the nearest float to 0.001 and 0x0107278C the nearest to
// sqrt(2)/2, so the decimal spellings of those two are exact.
constexpr uint32_t kBitsTwo = 0x40000000;       // 0x00F98A58: 2.0f
constexpr uint32_t kBitsHalf = 0x3F000000;      // 0x00F9A5AC: 0.5f
constexpr uint32_t kBitsTwoPow32 = 0x4F800000;  // 0x00F98940: 4294967296.0f
constexpr uint32_t kBitsEps = 0x3A83126F;       // 0x01010960: 0.001f
constexpr uint32_t kBitsDiag = 0x3F3504F3;      // 0x0107278C: +0.70710677f
constexpr uint32_t kBitsDiagNeg = 0xBF3504F3;   // 0x01063520: -0.70710677f

namespace {

float Bits(uint32_t bits)
{
    float f;
    std::memcpy(&f, &bits, sizeof f);
    return f;
}

// Vtable slots of vt_CreatingNewRoof_3 that this unit calls through; index = byte offset/4.
constexpr int kSlotWidth = 0x10 / 4;   // 0x00A47040: width,  __thiscall ret 0
constexpr int kSlotHeight = 0x14 / 4;  // 0x00A47050: height, __thiscall ret 0
constexpr int kSlotSample = 0x28 / 4;  // 0x00A45EC0: sample, __thiscall ret 8
constexpr int kSlotCorner = 0x40 / 4;  // 0x00BF4B20: corner sampler (this unit)

int32_t Width(CreatingNewRoof_3* self)
{
    return CallGameVirtual<int32_t, kSlotWidth>(self);
}

int32_t Height(CreatingNewRoof_3* self)
{
    return CallGameVirtual<int32_t, kSlotHeight>(self);
}

float Sample(CreatingNewRoof_3* self, int32_t col, int32_t row)
{
    return CallGameVirtual<float, kSlotSample>(self, col, row);
}

float Corner(CreatingNewRoof_3* self)
{
    return CallGameVirtual<float, kSlotCorner>(self);
}

// 0x004E3F70: the game's Free(ptr).  __cdecl, and null-safe (`if (ptr) HeapFree(...)`).
void GameFree(void* ptr)
{
    CallGameFunction<void, 0x004E3F70>(ptr);
}

// 0x00A47140: the base constructor.  __thiscall with eight stack arguments (the caller at
// 0x00BFAD4D does not clean the stack up, so the callee pops them).
void BaseConstruct(CreatingNewRoof_3* self, uint32_t a0, uint32_t a1, uint32_t a2,
                   uint32_t a3, uint32_t a4, uint32_t a5, uint32_t a6, float a7)
{
    CallGameMethod<void, 0x00A47140>(self, a0, a1, a2, a3, a4, a5, a6, a7);
}

// float -> int32 the way fld/fist does it: round to nearest, ties to even; NaN and anything
// outside int32 range stores the x87 integer indefinite (0x80000000).  Same model as
// clu_adb0e0_x.cpp's FistRound.
int32_t FistRound(float v)
{
    if (std::isnan(v) || v >= 2147483648.0f || v < -2147483648.0f) {
        return INT32_MIN;
    }
    return static_cast<int32_t>(std::nearbyintf(v));
}

// The original's floor at 0x00BF62C0..0x00BF62D2: `fist` to a scratch int, `fild` it back,
// subtract, store the fraction as a float and compare its bits against 0x80000001 with
// `adc eax, -1`.  The fraction of a float against its nearest integer is exact, so
// "bits < 0x80000001" is just "the fraction is not negative" (-0.0 counts as not negative,
// matching the compare).
int32_t FloorByFist(float v)
{
    const int32_t rounded = FistRound(v);
    const float frac = v - static_cast<float>(rounded);
    uint32_t bits;
    std::memcpy(&bits, &frac, sizeof bits);
    return static_cast<int32_t>(static_cast<uint32_t>(rounded) - 1u +
                                (bits < 0x80000001u ? 1u : 0u));
}

// 0x00BF6040's row ramp converts its counter with the x87 "unsigned int" idiom (0x00BF60A4:
// fild, then add 4294967296.0 when the value reads as negative, then store to float).  For
// the non-negative counters the game produces this is plain float(v); the negative case is
// kept because a negative dimension is reachable from a corrupt object.
float UnsignedStyleFloat(int32_t v)
{
    if (v < 0) {
        // 2^32 + v is exact at extended precision, so this rounds exactly once to float,
        // like the original's fstp.
        return static_cast<float>(static_cast<long double>(v) + Bits(kBitsTwoPow32));
    }
    return static_cast<float>(v);
}

uint32_t AsWord(const void* p)
{
    return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(p));
}

}  // namespace

// TS3W.exe 0x00BF4B20 (__thiscall, plain `ret`, vtable slot +0x40).
//
// Return contract: the sampler tail-calls vtable +0x28, so the float the callers read off the
// maths stack is that call's result; EAX keeps whatever the sample left in it, which no
// caller of this slot uses.  Declaring it `float` reproduces the observable value.
float CornerSample(CreatingNewRoof_3* self)
{
    if (self->f_38 != 0) {
        if (self->f_3A != 0) {
            return Sample(self, 0, Height(self) - 1);
        }
        return Sample(self, 0, 0);
    }
    if (self->f_39 != 0) {
        return Sample(self, Width(self) - 1, 0);
    }
    return Sample(self, 0, 0);
}

// TS3W.exe 0x00BF6040 (__thiscall, plain `ret`, vtable slot +0x20).
//
// Return contract: nothing.  EAX leaves as a byproduct of the vectorised loops - 4 * width
// when the width is a multiple of four, the byte offset of the row's end otherwise, the
// height-with-flags-swapped value when a dimension is not positive - and every caller goes
// through vtable slot +0x20, which the game declares as taking no result.  So this is void
// here, as in clu_adb0e0_x.cpp's HeapMake.
//
// Memory effect: with f_38 every cell of row r gets that row's ramp value; otherwise every
// cell of column c gets that column's ramp value at grid[c + row * width].  The original
// unrolls by fours and finishes with a scalar remainder, touching exactly the same cells with
// the same value, so the plain loops below are equivalent.
void FillGrid(CreatingNewRoof_3* self)
{
    const int w = Width(self);
    const int h = Height(self);
    float* grid = static_cast<float*>(self->grid);

    if (self->f_38 != 0) {
        int value = (self->f_3A != 0) ? 0 : (h - 1);
        const int step = (self->f_3A != 0) ? 1 : -1;
        if (h <= 0) {
            return;
        }
        for (int row = 0; row < h; ++row) {
            const float filled = UnsignedStyleFloat(value);
            for (int col = 0; col < w; ++col) {
                grid[static_cast<size_t>(row) * static_cast<size_t>(w) + static_cast<size_t>(col)] =
                    filled;
            }
            value += step;
        }
        return;
    }

    const int start = (self->f_39 != 0) ? 0 : (w - 1);
    const int step = (self->f_39 != 0) ? 1 : -1;
    if (w <= 0) {
        return;
    }
    for (int col = 0; col < w; ++col) {
        // 0x00BF619F uses cvtsi2ss here (plain signed conversion), not the x87 idiom above.
        const float filled = static_cast<float>(start + step * col);
        for (int row = 0; row < h; ++row) {
            grid[static_cast<size_t>(row) * static_cast<size_t>(w) + static_cast<size_t>(col)] =
                filled;
        }
    }
}

// TS3W.exe 0x00BF6280 (__thiscall, `ret 8`, vtable slot +0x3C).
//
// Return contract: only AL is a result.  It is 12 when width >= height and 3 when width <
// height (the `sbb al,al` / the original instruction sequence / the original instruction sequence pair, an UNSIGNED compare), 0 when the
// sampled cell disagrees with the corner value by 0.001 or more, and 0 for NaN too (FCOMIP sets
// CF on unordered, so the original's `jbe` takes that path as well).  EAX's upper bytes keep a
// residue: the height returned by vtable +0x14 on the success path, and whatever the sampler
// chain left there on the zero path (the original instruction sequence at 0x00BF634B only clears AL) - the latter
// belongs to another unit's code, so it is not reproducible here and the rewrite returns a
// plain int whose low byte carries the answer.
//
// No vtable snapshot is taken: the original re-reads [this] before every slot call, and so
// does CallGameVirtual.
int PointOnGridLine(CreatingNewRoof_3* self, float ax, float ay)
{
    // (a - origin) * 2.0, each step rounded to float once, matching the subss/mulss pair.
    const float x = (ax - self->f_5C) * Bits(kBitsTwo);
    const float y = (ay - self->f_60) * Bits(kBitsTwo);

    const int32_t col = FloorByFist(x);
    const int32_t row = FloorByFist(y);
    const float sampled = Sample(self, col, row);
    // The subtraction and the fabs run on the maths stack (the sampled value was rounded to
    // float by the `fstp` at 0x00BF6307, the corner value is subtracted at extended
    // precision), so the comparison against the epsilon is an extended-precision one.
    const long double diff = std::fabs(static_cast<long double>(Corner(self)) -
                                       static_cast<long double>(sampled));
    if (!(diff < static_cast<long double>(Bits(kBitsEps)))) {
        return 0;  // also the NaN path: FCOMIP sets CF, so `jbe` takes it; AL = 0 is the result
    }
    const uint32_t w = static_cast<uint32_t>(Width(self));
    const uint32_t h = static_cast<uint32_t>(Height(self));
    // the original instruction sequence / `sbb al,al` / the original instruction sequence / the original instruction sequence is an UNSIGNED width<height
    // test that only rewrites AL; EAX's upper bytes keep the height the getter returned.
    const uint32_t low = (w < h) ? 3u : 12u;
    return static_cast<int>((h & ~0xFFu) | low);  // AL is the contract, see above
}

// TS3W.exe 0x00BF6360 (__thiscall, the original instruction sequence, vtable slot +0x44).
//
// Return contract: the original instruction sequence after the output pointer was loaded into EAX, so EAX =
// (out & ~0xFF) | 1 - a pointer-derived residue the callers ignore but which the harness
// compares, reproduced below.  Of the four stack arguments only the third (the float* below)
// is ever touched: the first, second and fourth are dead, as in the original.
int CreatingNewRoof_3::DirectionVector(uint32_t /*a0*/, uint32_t /*a1*/, void* outPointer,
                                      uint32_t /*a3*/)
{
    auto* out = static_cast<float*>(outPointer);
    const float diag = Bits(kBitsDiag);
    const float diagNeg = Bits(kBitsDiagNeg);

    if (f_38 != 0) {
        out[0] = 0.0f;
        out[1] = diag;
        out[2] = (f_3A != 0) ? diagNeg : diag;
    } else if (f_39 != 0) {
        out[0] = diagNeg;
        out[1] = diag;
        out[2] = 0.0f;
    } else {
        out[0] = diag;
        out[1] = diag;
        out[2] = 0.0f;
    }
    return static_cast<int>((static_cast<uint32_t>(reinterpret_cast<uintptr_t>(out)) & ~0xFFu) |
                            1u);
}

// TS3W.exe 0x00BFAD20 (__thiscall, the original instruction sequence): the constructor.
//
// Return contract: EAX = self on all three paths.
CreatingNewRoof_3* Create(CreatingNewRoof_3* self, void* a0, void* a1, void* a2, void* a3,
                          void* a4)
{
    // The pushes at 0x00BFAD35..0x00BFAD4A hand all five arguments straight to the base
    // constructor, then 1, 0 and the 0.5f loaded from 0x00F9A5AC.
    BaseConstruct(self, AsWord(a0), AsWord(a1), AsWord(a2), AsWord(a3), AsWord(a4), 1u, 0u,
                  Bits(kBitsHalf));

    self->vtable = reinterpret_cast<void*>(kVtableCreatingNewRoof3);

    // 0x00BFAD5E loads [a0 + 0x2AC] unconditionally, before any comparison, so a bad a0 faults
    // on every path (the volatile read keeps that load even on the dx > dy path).
    const uint32_t owner = *reinterpret_cast<const volatile uint32_t*>(
        static_cast<const uint8_t*>(a0) + 0x2AC);

    // dx = |f_1C - f_10| and dy = |f_20 - f_14|, both at extended precision.
    const long double dx = std::fabs(static_cast<long double>(self->f_1C) -
                                     static_cast<long double>(self->f_10));
    const long double dy = std::fabs(static_cast<long double>(self->f_20) -
                                     static_cast<long double>(self->f_14));
    if (dx > dy) {
        self->f_38 = 1;
        return self;
    }
    // FCOMI at 0x00BFAD70 does not pop, so 0x00BFAD83's `fsubrp st(1)` computes dx - dy and
    // the epsilon test is against |dx - dy|.  `jbe` at 0x00BFAD91 takes the store-zero branch
    // for `0.001f <= |dx - dy|`, so the byte is copied only for a difference strictly below
    // the epsilon.  NaN fails the compare and lands in the store-zero branch.  The owner
    // pointer's byte at +0x88 is not null-checked.
    if (std::fabs(dx - dy) < static_cast<long double>(Bits(kBitsEps))) {
        self->f_38 = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(owner) + 0x88);
    } else {
        self->f_38 = 0;
    }
    return self;
}

// TS3W.exe 0x00BFBB40 (__thiscall, `ret 4`, vtable slot +0x00): the destructor.
//
// Return contract: EAX = self (0x00BFBB6F); returned here so a caller that keeps the deleting
// destructor's result sees the same value.  The original never stores the vtable that 0x00A47140 writes (0x0105ECD8) and does
// not call that constructor at all: it installs the derived vtable, frees the grid through the
// game's null-safe Free, then leaves the object pointing at the base vtable 0x0107396C and,
// when bit 0 of the flag argument is set, frees the object itself.
CreatingNewRoof_3* Destroy(CreatingNewRoof_3* self, uint32_t flags)
{
    self->vtable = reinterpret_cast<void*>(kVtableCreatingNewRoof3);
    if (self->grid != nullptr) {
        GameFree(self->grid);
    }
    self->vtable = reinterpret_cast<void*>(kVtableBase);
    if ((flags & 1u) != 0u) {
        GameFree(self);
    }
    return self;
}

// The record vector owns three native pointers; the game uses 24-byte elements.
namespace {
struct RoofRecord { float x0, z0, y0, x1, z1, y1; };
struct RoofRecords { RoofRecord* begin; RoofRecord* finish; RoofRecord* end; };
TS3_X86_SIZE(RoofRecord, 24);
TS3_X86_SIZE(RoofRecords, 12);
TS3_X86_OFFSET(RoofRecords, finish, 4);
TS3_X86_OFFSET(RoofRecords, end, 8);

void AppendCell(void* out, const RoofRecord& record)
{
    auto* vec = static_cast<RoofRecords*>(out);
    auto* pos = vec->finish;
    if (reinterpret_cast<uintptr_t>(pos) < reinterpret_cast<uintptr_t>(vec->end)) {
        // EA advances finish even when pos is null, but skips the stores.
        vec->finish = reinterpret_cast<RoofRecord*>(reinterpret_cast<uintptr_t>(pos) + sizeof record);
        if (pos) *pos = record;
    } else {
        CallGameMethod<void, 0x00BFC610>(vec, pos, &record);
    }
}
}

// TS3W.exe 0x00BFD880: thiscall, ret 12. a2 is dead; only a0's low byte matters.
int CreatingNewRoof_3::EmitCellRecords(int a0, void* out, int a2)
{
    (void)a2;
    auto* self = this;
    const uint32_t stride = static_cast<uint8_t>(a0) ? 2u : 1u;
    const bool alternate = static_cast<uint8_t>(a0) != 0;
    const float ox = self->f_5C, oy = self->f_60;
    // Unsigned conversions and comparisons are deliberate (fild + 2^32, jb).
    // The stride-two path samples i+1, NOT i+stride, while its endpoint uses i+stride.
    const uint32_t columns[2] = {0, static_cast<uint32_t>(self->f_74) - 1u};
    for (unsigned side = 0; side < 2; ++side) {
        const uint32_t col = columns[side];
        const float x = static_cast<float>(double(col) * 0.5 + ox);
        if (self->f_78 == 1) continue;
        uint32_t row = 0;
        do {
            const float y0 = static_cast<float>(double(row) * 0.5 + oy);
            const float y1 = static_cast<float>(double(row + stride) * 0.5 + oy);
            const float z0 = Sample(self, static_cast<int32_t>(col), static_cast<int32_t>(row));
            const float z1 = Sample(self, static_cast<int32_t>(col), static_cast<int32_t>(row + 1));
            const bool selected = !self->f_38 && (self->f_39 ? side == 0 : side == 1);
            if (alternate ? selected : (selected || self->f_38)) {
                const bool forward = side == 0;
                const RoofRecord record = forward ? RoofRecord{x,z0,y0,x,z1,y1}
                                                  : RoofRecord{x,z1,y1,x,z0,y0};
                AppendCell(out, record);
            }
            row += stride;
        } while (row < static_cast<uint32_t>(self->f_78) - 1u);
    }
    const uint32_t rows[2] = {0, static_cast<uint32_t>(self->f_78) - 1u};
    for (unsigned side = 0; side < 2; ++side) {
        const uint32_t row = rows[side];
        const float y = static_cast<float>(double(row) * 0.5 + oy);
        if (self->f_74 == 1) continue;
        uint32_t col = 0;
        do {
            const float x0 = static_cast<float>(double(col) * 0.5 + ox);
            const float x1 = static_cast<float>(double(col + stride) * 0.5 + ox);
            const float z0 = Sample(self, static_cast<int32_t>(col), static_cast<int32_t>(row));
            const float z1 = Sample(self, static_cast<int32_t>(col + 1), static_cast<int32_t>(row));
            const bool selected = self->f_38 && (self->f_3A ? side == 0 : side == 1);
            if (alternate ? selected : (selected || !self->f_38)) {
                const RoofRecord record = side == 1 ? RoofRecord{x0,z0,y,x1,z1,y}
                                                   : RoofRecord{x1,z1,y,x0,z0,y};
                AppendCell(out, record);
            }
            col += stride;
        } while (col < static_cast<uint32_t>(self->f_74) - 1u);
    }
    // EAX is the completed side counter (2), then mov al,1: exactly 1.
    return 1;
}

// TS3W.exe 0x00BFDCA0: thiscall, ret 4; exactly three vector appends.
int CreatingNewRoof_3::EmitCornerRecords(void* out)
{
    auto* self = this;
    const float x0 = self->f_5C, y0 = self->f_60, x1 = self->f_64, y1 = self->f_68;
    const float a = Sample(self, static_cast<int32_t>(static_cast<uint32_t>(self->f_74)-1u),
                          static_cast<int32_t>(static_cast<uint32_t>(self->f_78)-1u));
    const float b = Sample(self, static_cast<int32_t>(static_cast<uint32_t>(self->f_74)-1u), 0);
    const float c = Sample(self, 0, static_cast<int32_t>(static_cast<uint32_t>(self->f_78)-1u));
    const float d = Sample(self, 0, 0);
    // The jbe selection includes unordered: do not replace this with std::min.
    const float ab = b > a ? a : b;
    const float cd = d > c ? c : d;
    const float lo = cd > ab ? ab : cd;
    const float top = Corner(self);
    RoofRecord records[3];
    if (self->f_38) {
        if (self->f_3A) {
            records[0] = {x1,lo,y0,x0,lo,y0};
            records[1] = {x0,lo,y0,x0,top,y1};
            records[2] = {x1,top,y1,x1,lo,y0};
        } else {
            records[0] = {x1,lo,y1,x0,lo,y1};
            records[1] = {x0,top,y0,x0,lo,y1};
            records[2] = {x1,lo,y1,x1,top,y0};
        }
    } else if (self->f_39) {
        records[0] = {x0,lo,y0,x0,lo,y1};
        records[1] = {x1,top,y0,x0,lo,y0};
        records[2] = {x1,top,y1,x0,lo,y1};
    } else {
        records[0] = {x1,lo,y1,x1,lo,y0};
        records[1] = {x1,lo,y0,x0,top,y0};
        records[2] = {x1,lo,y1,x0,top,y1};
    }
    int result = 0;
    for (const auto& record : records)
        result = CallGameMethod<int, 0x00BFC780>(out, &record);
    return static_cast<int>((static_cast<uint32_t>(result) & ~0xFFu) | 1u);
}

#if defined(_M_IX86)
int __fastcall RoofEmitCellRecordsHook(CreatingNewRoof_3* self, void*, int a0, void* out, int a2)
{
    return self->EmitCellRecords(a0, out, a2);
}
int __fastcall RoofEmitCornerRecordsHook(CreatingNewRoof_3* self, void*, void* out)
{
    return self->EmitCornerRecords(out);
}
#endif

}  // namespace misc
}  // namespace ts3

TS3_X86_SIZE(ts3::misc::CreatingNewRoof_3, 124);
TS3_X86_OFFSET(ts3::misc::CreatingNewRoof_3, vtable, 0x00);
TS3_X86_OFFSET(ts3::misc::CreatingNewRoof_3, _header_04, 0x04);
TS3_X86_OFFSET(ts3::misc::CreatingNewRoof_3, f_10, 0x10);
TS3_X86_OFFSET(ts3::misc::CreatingNewRoof_3, f_14, 0x14);
TS3_X86_OFFSET(ts3::misc::CreatingNewRoof_3, f_1C, 0x1C);
TS3_X86_OFFSET(ts3::misc::CreatingNewRoof_3, f_20, 0x20);
TS3_X86_OFFSET(ts3::misc::CreatingNewRoof_3, f_30, 0x30);
TS3_X86_OFFSET(ts3::misc::CreatingNewRoof_3, f_38, 0x38);
TS3_X86_OFFSET(ts3::misc::CreatingNewRoof_3, f_39, 0x39);
TS3_X86_OFFSET(ts3::misc::CreatingNewRoof_3, f_3A, 0x3A);
TS3_X86_OFFSET(ts3::misc::CreatingNewRoof_3, f_3C, 0x3C);
TS3_X86_OFFSET(ts3::misc::CreatingNewRoof_3, f_40, 0x40);
TS3_X86_OFFSET(ts3::misc::CreatingNewRoof_3, f_44, 0x44);
TS3_X86_OFFSET(ts3::misc::CreatingNewRoof_3, grid, 0x48);
TS3_X86_OFFSET(ts3::misc::CreatingNewRoof_3, f_58, 0x58);
TS3_X86_OFFSET(ts3::misc::CreatingNewRoof_3, f_5C, 0x5C);
TS3_X86_OFFSET(ts3::misc::CreatingNewRoof_3, f_60, 0x60);
TS3_X86_OFFSET(ts3::misc::CreatingNewRoof_3, f_74, 0x74);
TS3_X86_OFFSET(ts3::misc::CreatingNewRoof_3, f_78, 0x78);
TS3_X86_OFFSET(ts3::misc::CreatingNewRoof_3, f_64, 0x64);
TS3_X86_OFFSET(ts3::misc::CreatingNewRoof_3, f_68, 0x68);
