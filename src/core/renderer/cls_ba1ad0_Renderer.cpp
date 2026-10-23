// TS3W.exe renderer class unit cls_ba1ad0_Renderer - see ts3/renderer/cls_ba1ad0_Renderer.h.
//
// The unit's dependencies (cls_b9c200_LotRenderer, cls_ba1040_Scratch, clu_6afc70_Renderer,
// clu_6cff20_x, clu_6d0a40_Lot, clu_b95d20_x) are not rewritten here: every call into them goes
// through the bridge in ts3/layout.h.

#include "ts3/renderer/cls_ba1ad0_Renderer.h"

#include <cstdint>
#include <optional>
#include <xmmintrin.h>

namespace ts3::renderer {
namespace {

// The lattice at [this+0x18] is another unit's object, so its fields are reached by byte offset
// and its methods through the bridge instead of being re-declared here.
constexpr uint32_t kLatticeField = 0x18;    // [renderer+0x18]: the lattice that gets walked
constexpr uint32_t kAcceptKeyField = 0x18;  // [lattice+0x18]: key handed to both accept tests
constexpr uint32_t kOriginXField = 0x5C;    // [lattice+0x5C]: float, world origin X
constexpr uint32_t kOriginZField = 0x60;    // [lattice+0x60]: float, world origin Z

inline void* ReadPtr(const void* self, uint32_t offset) {
    return *reinterpret_cast<void* const*>(static_cast<const uint8_t*>(self) + offset);
}
inline int32_t ReadI32(const void* self, uint32_t offset) {
    return *reinterpret_cast<const int32_t*>(static_cast<const uint8_t*>(self) + offset);
}
inline float ReadF32(const void* self, uint32_t offset) {
    return *reinterpret_cast<const float*>(static_cast<const uint8_t*>(self) + offset);
}

constexpr float kHalf = 0.5f;       // TS3W.exe 0x00F9A5AC
constexpr float kOne = 1.0f;        // TS3W.exe 0x0107A538
constexpr float kMinusOne = -1.0f;  // TS3W.exe 0x00FAA210
constexpr float kBorderOffset = 3.5f;  // TS3W.exe 0x00FEC9B8
constexpr int32_t kBorderInset = 8;  // the walk stops 8 cells short of every lattice edge
constexpr int32_t kFirstCell = 7;    // ... and starts at cell 7

// The original spills a vector to its frame, pokes +0.0 into lane 1 with movss and reloads it
// with movaps; a float array is the same thing without the frame layout.
__m128 WithZeroLane1(__m128 v) {
    alignas(16) float lanes[4];
    _mm_store_ps(lanes, v);
    lanes[1] = 0.0f;
    return _mm_load_ps(lanes);
}

// 1/sqrt of the vector's own squared lanes, refined by the two Newton steps the original
// inlines (0x00BA1C8D..0x00BA1CCB): r <- r + (1 - (r*r)*d) * (0.5*r), lane-wise, with
// d = squared[0] + squared[1] + squared[2] broadcast to every lane.
__m128 ReciprocalLength(__m128 squared) {
    const __m128 sum = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(squared, squared, 0x02),
                                             _mm_shuffle_ps(squared, squared, 0x01)),
                                  squared);
    const __m128 d = _mm_shuffle_ps(sum, sum, 0x00);
    const __m128 one = _mm_shuffle_ps(_mm_set_ss(kOne), _mm_set_ss(kOne), 0x00);
    const __m128 half = _mm_shuffle_ps(_mm_set_ss(kHalf), _mm_set_ss(kHalf), 0x00);
    __m128 r = _mm_rsqrt_ps(d);
    r = _mm_add_ps(r, _mm_mul_ps(_mm_sub_ps(one, _mm_mul_ps(_mm_mul_ps(r, r), d)), _mm_mul_ps(half, r)));
    r = _mm_add_ps(r, _mm_mul_ps(_mm_sub_ps(one, _mm_mul_ps(_mm_mul_ps(r, r), d)), _mm_mul_ps(half, r)));
    return r;
}

// The border normal of one lattice vertex: lane 1 dropped, negated, then scaled by the
// reciprocal length of lanes 0..2 (0x00BA1C4E..0x00BA1CD9).
__m128 BorderNormal(__m128 vertex) {
    const __m128 negated = _mm_sub_ps(_mm_setzero_ps(), WithZeroLane1(vertex));
    return _mm_mul_ps(ReciprocalLength(_mm_mul_ps(negated, negated)), negated);
}

// The original reads a vertex with the original instruction sequence; the offsets are all
// multiples of 16 so the load is aligned in the game, and unaligned here costs nothing.
__m128 LoadVertex(const float* vertices, int32_t byteOffset) {
    return _mm_loadu_ps(reinterpret_cast<const float*>(
        reinterpret_cast<const uint8_t*>(vertices) + byteOffset));
}

// The cell gradient determinant (0x00BF3C60..0x00BF3C82): lane-wise
// {a2*b1 - a1*b2, a0*b2 - a2*b0, a1*b0 - a0*b1, a0*b0 - a0*b0}.
__m128 CellGradient(__m128 a, __m128 b) {
    const __m128 p = _mm_mul_ps(_mm_shuffle_ps(a, a, 0x09), _mm_shuffle_ps(b, b, 0x12));
    const __m128 q = _mm_mul_ps(_mm_shuffle_ps(a, a, 0x12), _mm_shuffle_ps(b, b, 0x09));
    return _mm_sub_ps(q, p);
}

#if defined(_M_IX86)
// 0x00BA08F0 is __thiscall(this, 6 stack args) whose *caller* cleans the stack (the callers end
// with the original instruction sequence and the callee itself does `ret`), which MSVC's callee-cleaning
// __thiscall cannot express. This thunk pushes the arguments in reverse, loads ecx and pops
// the 24 bytes back. Every parameter sits at [esp+0x1C] once the pushes are done.
__declspec(naked) uint32_t __cdecl Call_00BA08F0(void* /*self*/, void* /*lattice*/, void* /*scratch*/,
                                                 float* /*coords*/, int32_t /*acceptKey*/, int32_t /*zero*/,
                                                 float /*unused*/) {
    __asm {
        mov eax, [esp + 0x1C]
        push eax                  // arg6: the unused float
        mov eax, [esp + 0x1C]
        push eax                  // arg5: 0
        mov eax, [esp + 0x1C]
        push eax                  // arg4: accept key
        mov eax, [esp + 0x1C]
        push eax                  // arg3: &coords[0]
        mov eax, [esp + 0x1C]
        push eax                  // arg2: scratch
        mov eax, [esp + 0x1C]
        push eax                  // arg1: the lattice
        mov ecx, [esp + 0x1C]     // this
        mov eax, 0x00BA08F0
        call eax
        add esp, 0x18
        retn
    }
}

// 0x00BA1760 is the same caller-cleaned shape with 3 stack args (the original instruction sequence).
__declspec(naked) uint32_t __cdecl Call_00BA1760(void* /*self*/, void* /*lattice*/, float* /*coords*/,
                                                 int32_t /*acceptKey*/) {
    __asm {
        mov eax, [esp + 0x10]
        push eax                  // arg3: accept key
        mov eax, [esp + 0x10]
        push eax                  // arg2: &coords[0]
        mov eax, [esp + 0x10]
        push eax                  // arg1: the lattice
        mov ecx, [esp + 0x10]     // this
        mov eax, 0x00BA1760
        call eax
        add esp, 0x0C
        retn
    }
}
#else
uint32_t Call_00BA08F0(void*, void*, void*, float*, int32_t, int32_t, float) {
    AbortNotRewritten("TS3W.exe 0x00BA08F0 (caller-cleaned __thiscall)");
    return 0;
}
uint32_t Call_00BA1760(void*, void*, float*, int32_t) {
    AbortNotRewritten("TS3W.exe 0x00BA1760 (caller-cleaned __thiscall)");
    return 0;
}
#endif

// The nine descriptor stores plus the vtable, shared by both cell triangles.
LotCellTriangle* InitCellTriangle(void* self, uintptr_t vtable, int32_t index, void* owner, int32_t reversed,
                                  int32_t stride, int32_t column, int32_t x, int32_t y, int32_t base,
                                  const float* const* values) {
    auto* cell = static_cast<LotCellTriangle*>(self);
    cell->index = index;
    cell->owner = owner;
    cell->stride = stride;
    cell->column = column;
    cell->reversed = reversed;
    cell->x = x;
    cell->y = y;
    cell->base = base;
    cell->values = values;
#if defined(_M_IX86)
    cell->vtable = reinterpret_cast<void*>(vtable);
#else
    (void)vtable;
    cell->vtable = nullptr;
#endif
    return cell;
}

// 0x006D0A80 fills the gradient from the owner's cache and reports whether it did; on a miss the
// computed vector is stored instead. 0x00BF3820 then takes the finished vector either way. The
// original tests only AL, so the cached flag is read the same way.
void FinishCellTriangle(LotCellTriangle* cell, __m128 a, __m128 b) {
    if ((CallGameMethod<uint32_t, 0x006D0A80>(cell->owner, cell->index, cell->gradient) & 0xFFu) == 0) {
        _mm_store_ps(cell->gradient, CellGradient(a, b));
    }
    CallGameFunction<void, 0x00BF3820>(cell->gradient);
}

// The cell centre coordinate of a border walk: ((counter*0.5 + origin) + 0.5 + that) * 0.5.
inline float CellCoordinate(float fromOrigin) {
    const float base = fromOrigin + kHalf + fromOrigin;
    return base * kHalf;
}

}  // namespace

void* BuildLotCellTriangleA(void* self, int32_t index, void* owner, int32_t reversed, int32_t stride,
                            int32_t column, int32_t x, int32_t y, int32_t base, const float* const* values) {
    LotCellTriangle* const cell =
        InitCellTriangle(self, 0x010724F4, index, owner, reversed, stride, column, x, y, base, values);
    const float* const heights = *values;
    const int32_t row0 = stride * y + x;
    const int32_t row1 = stride * (y + 1) + x;
    const float dx0 = heights[row1] - heights[row0];
    const float dx1 = heights[row1 + 1] - heights[row0];
    FinishCellTriangle(cell, _mm_setr_ps(kOne, dx1, kOne, kOne), _mm_setr_ps(0.0f, dx0, kOne, 0.0f));
    return cell;
}

void* BuildLotCellTriangleB(void* self, int32_t index, void* owner, int32_t reversed, int32_t stride,
                            int32_t column, int32_t x, int32_t y, int32_t base, const float* const* values) {
    LotCellTriangle* const cell =
        InitCellTriangle(self, 0x01072548, index, owner, reversed, stride, column, x, y, base, values);
    const float* const heights = *values;
    const int32_t row0 = stride * y + x + 1;    // the reference column of the lower row
    const int32_t row1 = stride * (y + 1) + x;  // ... and of the upper row, one cell earlier
    const float d0 = heights[row1] - heights[row0];
    const float d1 = heights[row1 + 1] - heights[row0];
    FinishCellTriangle(cell, _mm_setr_ps(0.0f, d1, kOne, 0.0f),
                       _mm_setr_ps(kMinusOne, d0, kOne, kMinusOne));
    return cell;
}

// 32-bit index arithmetic wraps in the original. Each push reloads descriptor fields: the
// owner can alias caller memory, and the three calls are observable sequencing boundaries.
int32_t EmitLotCellTriangleAIndices(void* self) {
    auto* cell = static_cast<LotCellTriangle*>(self);
    const bool reversed = cell->reversed != 0; // the branch is taken only once
    const auto corner = [&](bool nextRow, bool nextColumn) {
        return static_cast<int32_t>(static_cast<uint32_t>(cell->stride) *
            (static_cast<uint32_t>(cell->y) + (nextRow ? 1u : 0u)) +
            static_cast<uint32_t>(cell->base) + static_cast<uint32_t>(cell->x) +
            (nextColumn ? 1u : 0u));
    };
    CallGameMethod<int32_t, 0x006D4BC0>(cell->owner, corner(reversed, reversed));
    CallGameMethod<int32_t, 0x006D4BC0>(cell->owner, corner(true, false));
    return CallGameMethod<int32_t, 0x006D4BC0>(cell->owner, corner(!reversed, !reversed));
}

int32_t EmitLotCellTriangleBIndices(void* self) {
    auto* cell = static_cast<LotCellTriangle*>(self);
    const bool reversed = cell->reversed != 0;
    const auto corner = [&](bool nextRow, bool nextColumn) {
        return static_cast<int32_t>(static_cast<uint32_t>(cell->stride) *
            (static_cast<uint32_t>(cell->y) + (nextRow ? 1u : 0u)) +
            static_cast<uint32_t>(cell->base) + static_cast<uint32_t>(cell->x) +
            (nextColumn ? 1u : 0u));
    };
    CallGameMethod<int32_t, 0x006D4BC0>(cell->owner, corner(!reversed, reversed));
    CallGameMethod<int32_t, 0x006D4BC0>(cell->owner, corner(true, true));
    return CallGameMethod<int32_t, 0x006D4BC0>(cell->owner, corner(reversed, !reversed));
}

int32_t BuildLotBorderCells(void* self, void* lot, void* scratch, const float* const* vertices, int32_t,
                            int32_t, int32_t) {
#if !defined(_M_IX86)
    AbortNotRewritten("BuildLotBorderCells: original lattice layout and callees require x86");
#endif
    void* const lattice = ReadPtr(self, kLatticeField);
    const float originX = ReadF32(lattice, kOriginXField);
    const float originZ = ReadF32(lattice, kOriginZField);
    const int32_t columns = CallGameVirtual<int32_t, 4>(lattice);   // vtable+0x10
    const int32_t rows = CallGameVirtual<int32_t, 5>(lattice);      // vtable+0x14
    const int32_t pairProduct = 2 * columns * rows;
    CallGameMethod<void, 0x006D0AD0>(lot);
    const int32_t columnLimit = columns - kBorderInset;
    const int32_t rowLimit = rows - kBorderInset;

    // The two coordinates of a cell travel as one adjacent float pair, exactly like the
    // original's frame slots, and both callees may rewrite the pair in place.
    float coords[2] = {0.0f, 0.0f};

    // One accepted cell: both tests must report zero in AL, then the two border normals are
    // emitted and the caller gets the vertex index the quad is anchored to.
    const auto emitCell = [&](int32_t offsetA, int32_t offsetB) -> std::optional<int32_t> {
        if ((Call_00BA08F0(lattice, lattice, scratch, coords, ReadI32(lattice, kAcceptKeyField), 0, 0.0f) & 0xFFu) != 0) {
            return std::nullopt;
        }
        if ((Call_00BA1760(lattice, lattice, coords, ReadI32(lattice, kAcceptKeyField)) & 0xFFu) != 0) {
            return std::nullopt;
        }
        const int32_t index = CallGameMethod<int32_t, 0x006D0B50>(lot, 1);
        const float* const heights = *vertices;  // reloaded after accept/count calls, not on rejected cells
        alignas(16) float laneA[4];
        alignas(16) float laneB[4];
        _mm_store_ps(laneA, BorderNormal(LoadVertex(heights, offsetA)));
        _mm_store_ps(laneB, BorderNormal(LoadVertex(heights, offsetB)));
        CallGameMethod<void, 0x006D5D60>(lot, 1, laneA[0], laneA[1], laneA[2], 0.0f);
        CallGameMethod<void, 0x006D5D60>(lot, 1, laneB[0], laneB[1], laneB[2], 0.0f);
        return index;
    };

    // Right border (0x00BA1B31..0x00BA1E36): one column, rows 7..rowLimit-1.
    if (rowLimit > kFirstCell) {
        coords[0] = static_cast<float>(columnLimit) * kHalf + originX;
        int32_t offsetA = 16 * (columnLimit + 7 * columns);
        int32_t offsetB = 16 * (columnLimit + 8 * columns);
        int32_t anchor = columnLimit + 7 * columns + 1;
        int32_t cellEnd = columnLimit + 8 * columns + pairProduct;
        const int32_t oneMinusProduct = 1 - pairProduct;
        for (int32_t row = kFirstCell; row < rowLimit; ++row) {
            coords[1] = CellCoordinate(static_cast<float>(row) * kHalf + originZ);
            if (const std::optional<int32_t> index = emitCell(offsetA, offsetB)) {
                CallGameFunction<void, 0x00B9D550>(lot, pairProduct + anchor - 1, anchor - 1,
                                                   columns + anchor - 1, cellEnd, anchor, *index,
                                                   *index + 1, cellEnd + oneMinusProduct);
            }
            offsetA += 16 * columns;
            offsetB += 16 * columns;
            anchor += columns;
            cellEnd += columns;
        }
    }

    // Top border (0x00BA1E3A..0x00BA20F4): one row, columns 7..columnLimit-1.
    if (columnLimit > kFirstCell) {
        coords[1] = static_cast<float>(rowLimit) * kHalf + originZ;
        for (int32_t column = kFirstCell; column < columnLimit; ++column) {
            coords[0] = CellCoordinate(static_cast<float>(column) * kHalf + originX);
            const int32_t cell = column + rowLimit * columns;
            if (const std::optional<int32_t> index = emitCell(16 * cell, 16 * (cell + 1))) {
                CallGameFunction<void, 0x00B9D550>(lot, pairProduct + cell, pairProduct + cell + 1,
                                                   cell + 1, cell, cell + 1, cell + 2, *index + 1,
                                                   *index);
            }
        }
    }

    // Left border (0x00BA2104..0x00BA240F): the column at x = 7, rows 7..rowLimit-1.
    if (rowLimit > kFirstCell) {
        coords[0] = originX + kBorderOffset;
        int32_t offsetA = 112 * (columns + 1);
        int32_t offsetB = 128 * columns + 112;
        int32_t anchor = 7 * columns + 8;
        int32_t cell = pairProduct + 8 * columns + 7;
        const int32_t oneMinusProduct = 1 - pairProduct;
        for (int32_t row = kFirstCell; row < rowLimit; ++row) {
            coords[1] = CellCoordinate(static_cast<float>(row) * kHalf + originZ);
            if (const std::optional<int32_t> index = emitCell(offsetA, offsetB)) {
                CallGameFunction<void, 0x00B9D550>(lot, anchor + pairProduct - 1, cell,
                                                   anchor + columns - 1, anchor - 1, anchor,
                                                   cell + oneMinusProduct, *index + 1, *index);
            }
            offsetA += 16 * columns;
            offsetB += 16 * columns;
            anchor += columns;
            cell += columns;
        }
    }

    // Bottom border (0x00BA240F..0x00BA26D1): the row at y = 7, columns 7..columnLimit-1.
    if (columnLimit > kFirstCell) {
        coords[1] = originZ + kBorderOffset;
        for (int32_t column = kFirstCell; column < columnLimit; ++column) {
            coords[0] = CellCoordinate(static_cast<float>(column) * kHalf + originX);
            const int32_t cell = column + 7 * columns;
            if (const std::optional<int32_t> index = emitCell(16 * cell, 16 * (cell + 1))) {
                CallGameFunction<void, 0x00B9D550>(lot, pairProduct + cell, cell, cell + 1,
                                                   cell + 1 + pairProduct, cell + 1, *index,
                                                   *index + 1, cell + 2);
            }
        }
    }

    // eax on every exit path: the last loop's exit counter, i.e. columns - 8.
    return columnLimit;
}

}  // namespace ts3::renderer
