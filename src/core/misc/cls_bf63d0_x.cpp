// cls_bf63d0_x.cpp - reconstruction of the TS3W.exe class unit at 0x00BF63D0..0x00BFE800.
//
//   0x00BF63D0 AddDiagonalShedWallLocators::PointOnDiagonalGridLine  vtable +0x3C, ret 8
//   0x00BF64C0 AddDiagonalShedWallLocators::DirectionVector          vtable +0x44, ret 0x10
//   0x00BFADB0 AddDiagonalShedWallLocators::Create                            ret 0x14
//   0x00BFBDE0 AddDiagonalShedWallLocators::Destroy                   vtable +0x00, ret 4
//   0x00BFE0B0 AddDiagonalShedWallLocators::EmitDiagonalRecords       vtable +0x70, ret 0xC
//   0x00BFE800 AddDiagonalShedWallLocators::EmitCornerRecords         vtable +0x74, ret 4
//
// Everything the six functions delegate to (the width/height/sampler/corner vtable slots, the
// world-to-grid transform 0x00A46D00, the base constructor 0x00A48880, Free 0x004E3F70 and the
// record vector helpers 0x00BFC610 / 0x00BFC780) belongs to other units and is still original
// code, so it is called through the bridge in ts3/layout.h.
//
// Two of the six are vtable slots of the sibling object cls_bf4b20_x shares: +0x40 is
// 0x00BF4B20 (verified there) and the sampler is 0x00A46B70, which reads +0x30/+0x3C/+0x40/
// +0x44/+0x48/+0x58 and returns 0.0f for an out-of-range cell.
#include "ts3/misc/cls_bf63d0_x.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>

#include "ts3/layout.h"

namespace ts3 {
namespace misc {
namespace {

// Vtables this unit installs.  0x01072A70 is the class's own (Create and the destructor's first
// half), 0x0107396C is the base vtable left behind by the deleting destructor (its slot +0x00
// is 0x00624F80, followed in .rdata by the string "Lot/RugManager/AddRug").
const uint32_t kVtableClass = 0x01072A70u;
const uint32_t kVtableBase = 0x0107396Cu;

// Game addresses outside this unit, none of them rewritten yet.
const unsigned kAddrFree = 0x004E3F70;      // __cdecl Free(void*), null-safe
const unsigned kAddrBaseCtor = 0x00A48880;  // __thiscall, 6 stack args, the original instruction sequence
const unsigned kAddrToGrid = 0x00A46D00;    // __thiscall, 4 stack args, the original instruction sequence
const unsigned kAddrGrow = 0x00BFC610;      // __thiscall (vector, cursor, record), `ret 8`
const unsigned kAddrPush = 0x00BFC780;      // __thiscall (vector, record), `ret 4`

// Vtable slots of vt_AddDiagonalShedWallLocators used here; index = byte offset / 4.
const int kSlotWidth = 0x10 / 4;   // 0x00A45BA0: cellsX * 4 + 1, __thiscall plain `ret`
const int kSlotHeight = 0x14 / 4;  // 0x00A45BB0: cellsY * 4 + 1, __thiscall plain `ret`
const int kSlotSample = 0x28 / 4;  // 0x00A46B70: sample(col, row), __thiscall `ret 8`
const int kSlotCorner = 0x40 / 4;  // 0x00BF4B20: corner sample (unit cls_bf4b20_x)

// Constants the unit loads, with the address it loads them from.
const uint32_t kBitsFour = 0x40800000;       // 0x00FB2F6C: 4.0f
const uint32_t kBitsTolerance = 0x3A83126F;  // 0x01010960: 0.001f
const uint32_t kBitsGradient = 0x3F13CD3A;   // 0x01183DCC: 1/sqrt(3) = 0.5773502588f
const uint32_t kBitsNegZero = 0x80000000;    // 0x00F988F0: -0.0f
const uint32_t kBitsTwoPow32 = 0x4F800000;   // 0x00F98940: 4294967296.0f
const uint32_t kBitsQuarter = 0x3E800000;    // 0x01183DD0 / 0x01183DD4 / 0x01183DDC: 0.25f
const uint32_t kBitsNegQuarter = 0xBE800000; // 0x01183DD8: -0.25f
const uint32_t kBitsCellEps = 0x37800000;    // 0x01072780: 1.52587890625e-05f

float Bits(uint32_t bits)
{
    float f;
    std::memcpy(&f, &bits, sizeof f);
    return f;
}

uint32_t AsWord(const void* p)
{
    return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(p));
}

int32_t Width(AddDiagonalShedWallLocators* self)
{
    return CallGameVirtual<int32_t, kSlotWidth>(self);
}

int32_t Height(AddDiagonalShedWallLocators* self)
{
    return CallGameVirtual<int32_t, kSlotHeight>(self);
}

float Sample(AddDiagonalShedWallLocators* self, int32_t col, int32_t row)
{
    return CallGameVirtual<float, kSlotSample>(self, col, row);
}

float Corner(AddDiagonalShedWallLocators* self)
{
    return CallGameVirtual<float, kSlotCorner>(self);
}

// 0x00A46D00: the world-to-grid transform about the centre K1 (+0x84, +0x88).  It reads only
// [this+0x84]/[this+0x88] and writes the two results through the pointers it is given; each
// component is two single-rounded mulss plus one addss.  0x00BF63D0 hands it its own argument
// slots, so the transformed point replaces the caller's floats there.
void ToGrid(AddDiagonalShedWallLocators* self, float x, float z, float* outX, float* outZ)
{
    CallGameMethod<void, kAddrToGrid>(self, x, z, outX, outZ);
}

void GameFree(void* ptr)
{
    CallGameFunction<void, kAddrFree>(ptr);
}

// 0x00A48880: the base constructor, __thiscall with six stack arguments (the caller at
// 0x00BFADD1 does not clean up, so the callee pops them with the original instruction sequence).
void BaseConstruct(AddDiagonalShedWallLocators* self, void* a0, void* a1, void* a2,
                   void* a3, void* a4, uint32_t a5)
{
    CallGameMethod<void, kAddrBaseCtor>(self, a0, a1, a2, a3, a4, a5);
}

// float -> int32 the way fld/fist does it: round to nearest, ties to even; NaN and anything
// outside int32 range stores the x87 integer indefinite (0x80000000).
int32_t FistRound(float v)
{
    if (std::isnan(v) || v >= 2147483648.0f || v < -2147483648.0f) {
        return std::numeric_limits<int32_t>::min();
    }
    return static_cast<int32_t>(std::nearbyintf(v));
}

// 0x00BF6425..0x00BF6442: `fist` to a scratch int, `fild` it back, subtract, store the fraction
// as a float and `cmp` its bits against 0x80000001, then `adc eax,-1`.  The fraction of a float
// against its nearest integer is exact, so "bits < 0x80000001" is "the fraction is not
// negative" (-0.0 counts as not negative) and the pair is a floor by round-to-nearest-even.
// Indefinite (NaN or out of range) gives rounded = 0x80000000 with a NaN fraction whose bits
// are >= 0x80000001, so it floors to 0x7FFFFFFF, as the original does.
int32_t FloorByFist(float v)
{
    const int32_t rounded = FistRound(v);
    const float frac = v - static_cast<float>(rounded);
    uint32_t bits;
    std::memcpy(&bits, &frac, sizeof bits);
    return static_cast<int32_t>(static_cast<uint32_t>(rounded) - 1u +
                                (bits < 0x80000001u ? 1u : 0u));
}

// The x87 "unsigned int" conversion the record builders use (fild, then add 4294967296.0 when
// the value reads as negative, then store to float).  For the non-negative counters these loops
// produce it is plain float(v); the negative case is kept because a corrupt cell count reaches
// it through the width/height getters' `* 4 + 1`.
float UnsignedStyleFloat(int32_t v)
{
    if (v < 0) {
        // 2^32 + v is exact at extended precision, so this rounds exactly once to float, like
        // the original's fstp.
        return static_cast<float>(static_cast<long double>(v) + Bits(kBitsTwoPow32));
    }
    return static_cast<float>(v);
}

// The emitters' 24-byte record: two grid vertices (x, z) with the height of each end between
// them - exactly the order the six movss/stores write it in.
struct Record {
    float v[6];
};

// |diff| <= 1.52587890625e-05f, where the game broadcasts the single-rounded difference, takes
// max(diff * -1.0, diff) in SSE and compares with cmpleps.  maxps returns its second operand
// when the two compare equal, so the sign of a zero difference is lost either way; NaN stays
// NaN and fails the compare, as the original instruction sequence then does.
bool WithinCellEps(float diff)
{
    return std::fabs(diff) <= Bits(kBitsCellEps);
}

// The record push 0x00BFE0B0 inlines (identical to 0x00BFC780's fast path): header +0x04 is the
// cursor, +0x08 the end of storage.  The cursor is advanced *before* the null test, so a null
// cursor still moves the vector along by one record without writing anything - an EA quirk that
// is kept because the caller's vector is observable.
void AppendInline(void** header, const Record& rec)
{
    uint8_t* cur = static_cast<uint8_t*>(header[1]);
    const uint8_t* end = static_cast<const uint8_t*>(header[2]);
    if (!(reinterpret_cast<uintptr_t>(cur) < reinterpret_cast<uintptr_t>(end))) {
        CallGameMethod<void, kAddrGrow>(header, cur, rec.v);
        return;
    }
    header[1] = cur + 0x18;
    if (cur != nullptr) {
        std::memcpy(cur, rec.v, sizeof rec.v);
    }
}

// 0x00BFC780: the record push 0x00BFE800 calls.  EAX comes back as the element address the
// cursor had on entry (0x00BFE800's callers leave it in the register's upper bytes).
uintptr_t PushRecord(void** header, const Record& rec)
{
    return CallGameMethod<uintptr_t, kAddrPush>(header, rec.v);
}

}  // namespace

// TS3W.exe 0x00BF64C0 (__thiscall, the original instruction sequence, vtable slot +0x44).
//
// Return contract: the original instruction sequence after the output pointer was loaded into EAX, so EAX =
// (out & ~0xFF) | 1 - a pointer-derived residue the callers ignore but which the harness
// compares, reproduced below.  Of the four stack arguments only the third is ever touched; the
// first, second and fourth are dead, as in the original.
int AddDiagonalShedWallLocators::DirectionVector(uint32_t /*a0*/, uint32_t /*a1*/,
                                                 void* outPointer, uint32_t /*a3*/)
{
    auto* out = static_cast<float*>(outPointer);
    const float gradient = Bits(kBitsGradient);
    // the original instruction sequence / the original instruction sequence: the negative component is -0.0f - g, not
    // the literal -g (the two differ only for g = -0.0, which this constant is not).
    const float negGradient = Bits(kBitsNegZero) - gradient;

    if (f_38 != 0) {
        // 0x00BF64DA compares the byte at +0x3A against zero and 0x00BF64E7 branches on it: with
        // it clear the *first* component takes the negative value (0x00BF64F7), with it set the
        // third one does.
        // Read the branch flag before any output store: out can alias +0x38..+0x3A.
        const bool ascending = f_3A != 0;
        out[1] = gradient;
        if (ascending) {
            out[2] = negGradient;
            out[0] = gradient;
        } else {
            out[0] = negGradient;
            out[2] = gradient;
        }
    } else if (f_39 != 0) {
        out[0] = negGradient;
        out[1] = gradient;
        out[2] = negGradient;
    } else {
        out[0] = gradient;
        out[1] = gradient;
        out[2] = gradient;
    }
    return static_cast<int>((AsWord(out) & ~0xFFu) | 1u);
}

// TS3W.exe 0x00BF63D0 (__thiscall, `ret 8`, vtable slot +0x3C).
//
// Return contract: only AL is a result.  It is 12 when the unsigned width >= the unsigned
// height and 3 when it is smaller (the original instruction sequence / `sbb al,al` / the original instruction sequence / the original instruction sequence,
// an unsigned test that only rewrites AL), 0 when the sampled cell disagrees with the corner
// value by 0.001 or more, and 0 for NaN too (FCOMIP sets CF on unordered, so the `jbe` at
// 0x00BF648D takes that path as well).  EAX's upper bytes keep the height the last getter
// returned on success. On failure only AL is defined: the corner callee's incidental EAX
// survives above it (0x00BF64B4 is xor al,al, NOT xor eax,eax). We return a plain zero on
// that path; compare hooks compare exactly AL, not the dependency's volatile residue.
int AddDiagonalShedWallLocators::PointOnDiagonalGridLine(float a2, float a3)
{
    // 0x00A46D00 transforms the point about the centre and writes it back through the pointers
    // it is given - in the original those are the argument slots, so the transformed values
    // replace a2/a3 for everything below.
    float x = a2;
    float z = a3;
    ToGrid(this, x, z, &x, &z);

    // 4.0f * each coordinate, one rounding each (movss/mulss), then floored by fist/fild/adc.
    const int32_t col = FloorByFist(x * Bits(kBitsFour));
    const int32_t row = FloorByFist(z * Bits(kBitsFour));

    // The sampler runs before the corner sampler (0x00BF646E, then 0x00BF647B); its result is
    // rounded to float by the `fstp` at 0x00BF6470 while the corner value is subtracted at
    // extended precision, so the compare against the tolerance is an extended-precision one.
    const float sampled = Sample(this, col, row);
    const long double diff =
        std::fabs(static_cast<long double>(Corner(this)) - static_cast<long double>(sampled));
    if (!(diff < static_cast<long double>(Bits(kBitsTolerance)))) {
        return 0;  // AL = 0 (the original instruction sequence at 0x00BF64B4); upper EAX is not a result
    }

    const uint32_t w = static_cast<uint32_t>(Width(this));
    const int32_t h = Height(this);
    const uint32_t low = (w < static_cast<uint32_t>(h)) ? 3u : 12u;
    return static_cast<int>((static_cast<uint32_t>(h) & ~0xFFu) | low);
}

// TS3W.exe 0x00BFADB0 (__thiscall, the original instruction sequence): the constructor.
//
// Return contract: EAX = self on all three paths.
AddDiagonalShedWallLocators* AddDiagonalShedWallLocators::Create(void* a0, void* a1, void* a2,
                                                                 void* a3, void* a4)
{
    // 0x00BFADBE..0x00BFADCE hand the five arguments straight to the base constructor, plus 1.
    BaseConstruct(this, a0, a1, a2, a3, a4, 1u);

    vtable = reinterpret_cast<void*>(kVtableClass);

    // 0x00BFADBA keeps a0 in EDI and 0x00BFADEA loads [a0 + 0x2AC] unconditionally, before any
    // comparison, so a bad a0 faults on every path - the volatile read keeps that load on the
    // cellsX > cellsY path too.  The pointer it yields is only dereferenced for equal counts
    // and is never null-checked (the second EA bug).
    const auto* owner =
        *reinterpret_cast<const uint8_t* const volatile*>(static_cast<const uint8_t*>(a0) + 0x2AC);

    // the original instruction sequence with the two cell counts is read by `jbe`/`jne` as an UNSIGNED pair.
    const uint32_t x = static_cast<uint32_t>(cellsX);
    const uint32_t y = static_cast<uint32_t>(cellsY);
    if (x > y) {
        f_38 = 1;
        return this;
    }
    if (x != y) {
        f_38 = 0;
        return this;
    }
    f_38 = owner[0x88];
    return this;
}

// TS3W.exe 0x00BFBDE0 (__thiscall, `ret 4`, vtable slot +0x00): the deleting destructor.
//
// Return contract: EAX = self (0x00BFBE0F), kept because a caller that uses the result of the
// vtable's slot +0x00 would otherwise read a different register.  The original installs the
// class vtable first, frees the grid through the game's null-safe Free, and then stores the
// base vtable 0x0107396C *unconditionally* - the the original instruction sequence at 0x00BFBDF9 happens
// before that store but only guards the second Free, so an object that is not being freed is
// still left on the base vtable.
AddDiagonalShedWallLocators* AddDiagonalShedWallLocators::Destroy(uint32_t flags)
{
    vtable = reinterpret_cast<void*>(kVtableClass);
    if (grid != nullptr) {
        GameFree(grid);
    }
    vtable = reinterpret_cast<void*>(kVtableBase);
    if ((flags & 1u) != 0u) {
        GameFree(this);
    }
    return this;
}

// TS3W.exe 0x00BFE0B0 (__thiscall, the original instruction sequence, vtable slot +0x70): the diagonal edge records.
//
// Two strips are walked with the same body: outer i over the two columns {0, width-1} stepping
// down the rows, then outer k over the two rows {0, height-1} stepping across the columns.  A
// strip with a single cell in its walk direction emits nothing (`je` after the original instruction sequence, which
// is why the bound is re-read from the getter every iteration).  Each step contributes the pair
// (cell j, cell j + step) and appends at most one 24-byte record, with the two endpoints swapped
// on the second pass of each strip.
//
// Return contract: AL = 1; the upper bytes of EAX hold the last width getter's result minus one
// (the the original instruction sequence of the loop bound at 0x00BFE7CC, or the zero it leaves on the `continue`
// path), which no caller is known to read but which the harness compares.
int AddDiagonalShedWallLocators::EmitDiagonalRecords(uint32_t a0, void* outPointer,
                                                     uint32_t /*a2*/)
{
    auto** out = static_cast<void**>(outPointer);
    // the original instruction sequence - only the low byte of the first argument is a flag, so a0 =
    // 0x100 means "off" and gives the stride of 2.
    const uint8_t flag = static_cast<uint8_t>(a0);
    const int32_t step = (flag != 0u) ? 4 : 2;  // the original instruction sequence over setne's 0/1

    // The four corner cells, in the original's call order (each getter's result is spelled out
    // so the sequence is fixed): the height getter's the original instruction sequence is pushed first and the width
    // getter's second, and the sampler pops both, taking the width value as its column.
    const int32_t heightCells = Height(this);  // 0x00BFE0C6
    const int32_t widthCells = Width(this);    // 0x00BFE0D3
    const float farCorner = Sample(this, widthCells - 1, heightCells - 1);
    const int32_t widthAgain = Width(this);    // 0x00BFE0EE
    const float farColumn = Sample(this, widthAgain - 1, 0);
    const int32_t heightAgain = Height(this);  // 0x00BFE106
    const float lastRow = Sample(this, 0, heightAgain - 1);
    const float origin = Sample(this, 0, 0);

    // Lowest of the four: the first pair is compared on the maths stack (`ja` at 0x00BFE137, so
    // an unordered pair falls to the second operand), the rest with comiss, where `jbe` keeps
    // the *second* value on a tie.
    const float lowRows = (farColumn > farCorner) ? farCorner : farColumn;
    const float lowCols = (origin > lastRow) ? lastRow : origin;
    const float lowest = (lowCols > lowRows) ? lowRows : lowCols;

    const float cornerValue = Corner(this);  // stored as a float at 0x00BFE170
    const float centreX = k1x;
    const float centreZ = k1z;

    // ---- strip 1: the two edge columns, walking the rows ----
    const int32_t columnEnd = Width(this) - 1;  // 0x00BFE1AB, also the array's second entry
    const int32_t columns[2] = {0, columnEnd};
    for (int32_t i = 0; i < 2; ++i) {
        const int32_t column = columns[i];
        if (Height(this) - 1 == 0) {  // 0x00BFE1CF: a single row contributes nothing
            continue;
        }
        const float columnCoord = UnsignedStyleFloat(column);
        const float columnQuarter = columnCoord * Bits(kBitsQuarter);  // xmm1 and xmm2: same value
        for (int32_t row = 0;;) {
            const int32_t nextRow = row + step;
            const float rowCoord = UnsignedStyleFloat(row);
            const float nextCoord = UnsignedStyleFloat(nextRow);
            // Each coordinate is mulss, addss, addss: three roundings, in that order.
            const float x1 = rowCoord * Bits(kBitsNegQuarter) + columnQuarter;
            const float z1 = rowCoord * Bits(kBitsQuarter) + columnQuarter;
            const float x2 = nextCoord * Bits(kBitsNegQuarter) + columnQuarter;
            const float z2 = nextCoord * Bits(kBitsQuarter) + columnQuarter;
            const float h1 = Sample(this, column, row);
            const float h2 = Sample(this, column, nextRow);

            // With the flag set both ends must be flat against each other and against the
            // lowest corner; either way a pair whose ends both sit on the corner value is
            // dropped (0x00BFE382..0x00BFE3A4).
            bool keep = true;
            if (flag != 0u) {
                keep = WithinCellEps(h1 - h2) && WithinCellEps(h1 - lowest);
            }
            if (keep && WithinCellEps(h1 - cornerValue)) {
                keep = !WithinCellEps(h2 - cornerValue);
            }
            if (keep) {
                const Record first{{x1 + centreX, h1, z1 + centreZ, x2 + centreX, h2, z2 + centreZ}};
                const Record second{{x2 + centreX, h2, z2 + centreZ, x1 + centreX, h1,
                                     z1 + centreZ}};
                AppendInline(out, (i == 0) ? first : second);
            }

            const int32_t rows = Height(this) - 1;  // 0x00BFE4A0: the bound is re-read
            row = nextRow;
            if (!(static_cast<uint32_t>(row) < static_cast<uint32_t>(rows))) {
                break;
            }
        }
    }

    // ---- strip 2: the two edge rows, walking the columns ----
    const int32_t rowEnd = Height(this) - 1;  // 0x00BFE4D5, also the array's second entry
    const int32_t rows[2] = {0, rowEnd};
    int32_t lastWidth = 0;
    for (int32_t k = 0; k < 2; ++k) {
        const int32_t row = rows[k];
        lastWidth = Width(this);  // 0x00BFE4EC, and the bound check below re-reads it
        if (lastWidth - 1 == 0) {
            continue;
        }
        const float rowCoord = UnsignedStyleFloat(row);
        const float rowNegQuarter = rowCoord * Bits(kBitsNegQuarter);
        const float rowQuarter = rowCoord * Bits(kBitsQuarter);
        for (int32_t col = 0;;) {
            const int32_t nextCol = col + step;
            const float colCoord = UnsignedStyleFloat(col);
            const float nextCoord = UnsignedStyleFloat(nextCol);
            const float x1 = colCoord * Bits(kBitsQuarter) + rowNegQuarter;
            const float z1 = colCoord * Bits(kBitsQuarter) + rowQuarter;
            const float x2 = nextCoord * Bits(kBitsQuarter) + rowNegQuarter;
            const float z2 = nextCoord * Bits(kBitsQuarter) + rowQuarter;
            const float h1 = Sample(this, col, row);
            const float h2 = Sample(this, nextCol, row);

            bool keep = true;
            if (flag != 0u) {
                keep = WithinCellEps(h1 - h2) && WithinCellEps(h1 - lowest);
            }
            if (keep && WithinCellEps(h1 - cornerValue)) {
                keep = !WithinCellEps(h2 - cornerValue);
            }
            if (keep) {
                const Record first{{x1 + centreX, h1, z1 + centreZ, x2 + centreX, h2,
                                    z2 + centreZ}};
                const Record second{{x2 + centreX, h2, z2 + centreZ, x1 + centreX, h1,
                                     z1 + centreZ}};
                // 0x00BFE6CD tests the original instruction sequence (the second row emits near-first),
                // the *opposite* polarity from strip 1's the original instruction sequence at
                // 0x00BFE3AA (the first column emits near-first): this strip starts out with the
                // far endpoint.  The asymmetry is in the original's code, so it is kept (EA
                // quirk).
                AppendInline(out, (k == 1) ? first : second);
            }

            lastWidth = Width(this);  // 0x00BFE7CA: the last getter EAX the caller sees
            col = nextCol;
            if (!(static_cast<uint32_t>(col) < static_cast<uint32_t>(lastWidth - 1))) {
                break;
            }
        }
    }

    return static_cast<int>((static_cast<uint32_t>(lastWidth - 1) & ~0xFFu) | 1u);
}

// TS3W.exe 0x00BFE800 (__thiscall, `ret 4`, vtable slot +0x74): the three corner records.
//
// The same four corner samples and the same lowest-of-four selection as 0x00BFE0B0, but the
// records are the locator's own quad edges, built from the corner pairs at +0x7C..+0x98: two
// full edges from the centre K1 (or K0/K3) at the lowest height P, then one edge from each
// corner to its neighbour at the corner-sample height Q.  Exactly three records go through
// 0x00BFC780 on every flag path.
//
// Return contract: AL = 1; EAX's upper bytes hold the element address the last push-back
// returned (0x00BFC780 leaves the cursor there), so the residue is reproduced from the call.
int AddDiagonalShedWallLocators::EmitCornerRecords(void* outPointer)
{
    auto** out = static_cast<void**>(outPointer);

    const int32_t heightCells = Height(this);  // 0x00BFE80F
    const int32_t widthCells = Width(this);    // 0x00BFE81C
    const float farCorner = Sample(this, widthCells - 1, heightCells - 1);
    const int32_t widthAgain = Width(this);    // 0x00BFE836
    const float farColumn = Sample(this, widthAgain - 1, 0);
    const int32_t heightAgain = Height(this);  // 0x00BFE84E
    const float lastRow = Sample(this, 0, heightAgain - 1);
    const float origin = Sample(this, 0, 0);

    const float lowRows = (farColumn > farCorner) ? farCorner : farColumn;  // x87 `ja` 0x00BFE885
    const float lowCols = (origin > lastRow) ? lastRow : origin;            // comiss `ja` 0x00BFE8A0
    const float p = (lowCols > lowRows) ? lowRows : lowCols;                // `jbe` keeps lowCols
    const float q = Corner(this);

    // The four flag paths, in the order the record slots are written.  P = the lowest corner
    // sample height, Q = the corner sample; the first triple of each record is one endpoint's
    // (x, height, z) and the second triple the other end.  The bounds are left to the
    // initializer list so that the loop below is the only place where the count is written.
    const Record setAscending[] = {{k1x, p, k1z, k2x, p, k2z}, {k1x, p, k1z, k0x, q, k0z},
                                   {k2x, p, k2z, k3x, q, k3z}};
    const Record setDescending[] = {{k3x, p, k3z, k0x, p, k0z}, {k0x, p, k0z, k1x, q, k1z},
                                    {k3x, p, k3z, k2x, q, k2z}};
    const Record setColumns[] = {{k1x, p, k1z, k0x, p, k0z}, {k1x, p, k1z, k2x, q, k2z},
                                 {k0x, p, k0z, k3x, q, k3z}};
    const Record setRows[] = {{k2x, p, k2z, k3x, p, k3z}, {k3x, p, k3z, k0x, q, k0z},
                              {k2x, p, k2z, k1x, q, k1z}};

    const Record* set;
    if (f_38 != 0) {
        set = (f_3A != 0) ? setAscending : setDescending;
    } else {
        set = (f_39 != 0) ? setColumns : setRows;
    }

    uintptr_t residue = 0;
    for (int i = 0; i < 3; ++i) {
        residue = PushRecord(out, set[i]);
    }
    return static_cast<int>((residue & ~0xFFu) | 1u);
}

}  // namespace misc
}  // namespace ts3

TS3_X86_SIZE(ts3::misc::AddDiagonalShedWallLocators, 228);
TS3_X86_OFFSET(ts3::misc::AddDiagonalShedWallLocators, vtable, 0x00);
TS3_X86_OFFSET(ts3::misc::AddDiagonalShedWallLocators, _header_04, 0x04);
TS3_X86_OFFSET(ts3::misc::AddDiagonalShedWallLocators, f_30, 0x30);
TS3_X86_OFFSET(ts3::misc::AddDiagonalShedWallLocators, f_38, 0x38);
TS3_X86_OFFSET(ts3::misc::AddDiagonalShedWallLocators, f_39, 0x39);
TS3_X86_OFFSET(ts3::misc::AddDiagonalShedWallLocators, f_3A, 0x3A);
TS3_X86_OFFSET(ts3::misc::AddDiagonalShedWallLocators, f_3C, 0x3C);
TS3_X86_OFFSET(ts3::misc::AddDiagonalShedWallLocators, f_40, 0x40);
TS3_X86_OFFSET(ts3::misc::AddDiagonalShedWallLocators, f_44, 0x44);
TS3_X86_OFFSET(ts3::misc::AddDiagonalShedWallLocators, grid, 0x48);
TS3_X86_OFFSET(ts3::misc::AddDiagonalShedWallLocators, f_58, 0x58);
TS3_X86_OFFSET(ts3::misc::AddDiagonalShedWallLocators, k0x, 0x7C);
TS3_X86_OFFSET(ts3::misc::AddDiagonalShedWallLocators, k0z, 0x80);
TS3_X86_OFFSET(ts3::misc::AddDiagonalShedWallLocators, k1x, 0x84);
TS3_X86_OFFSET(ts3::misc::AddDiagonalShedWallLocators, k1z, 0x88);
TS3_X86_OFFSET(ts3::misc::AddDiagonalShedWallLocators, k2x, 0x8C);
TS3_X86_OFFSET(ts3::misc::AddDiagonalShedWallLocators, k2z, 0x90);
TS3_X86_OFFSET(ts3::misc::AddDiagonalShedWallLocators, k3x, 0x94);
TS3_X86_OFFSET(ts3::misc::AddDiagonalShedWallLocators, k3z, 0x98);
TS3_X86_OFFSET(ts3::misc::AddDiagonalShedWallLocators, cellsX, 0xDC);
TS3_X86_OFFSET(ts3::misc::AddDiagonalShedWallLocators, cellsY, 0xE0);
