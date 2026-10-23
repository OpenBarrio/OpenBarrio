// TS3W.exe renderer class unit cls_ba1ad0_Renderer (0x00BA1AD0/0x00BF1570/0x00BF1A40/0x00BF3B80/0x00BF3F20):
// the height-field lattice cell helpers - two cell-triangle descriptors (one per triangle of a
// lattice quad), their index emitters, and the border-cell builder of the scratch renderer.
#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::renderer {

// TS3W.exe 0x00BF3B80/0x00BF3F20 build the cell-triangle descriptor. The original writes the
// gradient with movaps, so +0x30 must be 16-byte aligned (the object itself is 16-byte aligned).
struct alignas(16) LotCellTriangle {
    void* vtable;                // +0x00: 0x010724F4 (triangle A) or 0x01072548 (triangle B)
    int32_t index;               // +0x04: lattice index, the cache key handed to 0x006D0A80
    void* owner;                 // +0x08: index-array owner (0x006D0A80's and 0x006D4BC0's `this`)
    int32_t reversed;            // +0x0C: non-zero reverses the emitted index order
    int32_t stride;              // +0x10: lattice row stride, in vertices
    int32_t column;              // +0x14: never read by this unit
    int32_t x;                   // +0x18: cell corner column
    int32_t y;                   // +0x1C: cell corner row
    int32_t base;                // +0x20: index bias added to every emitted index
    const float* const* values;  // +0x24: pointer to the height-array pointer
    uint8_t gap28[8];            // +0x28..+0x2F: never touched by this unit
    float gradient[4];           // +0x30: (dx, 1.0f, -dy, 0.0f), or the cached vector
};
TS3_X86_SIZE(LotCellTriangle, 0x40);
TS3_X86_OFFSET(LotCellTriangle, vtable, 0x00);
TS3_X86_OFFSET(LotCellTriangle, index, 0x04);
TS3_X86_OFFSET(LotCellTriangle, owner, 0x08);
TS3_X86_OFFSET(LotCellTriangle, reversed, 0x0C);
TS3_X86_OFFSET(LotCellTriangle, stride, 0x10);
TS3_X86_OFFSET(LotCellTriangle, column, 0x14);
TS3_X86_OFFSET(LotCellTriangle, x, 0x18);
TS3_X86_OFFSET(LotCellTriangle, y, 0x1C);
TS3_X86_OFFSET(LotCellTriangle, base, 0x20);
TS3_X86_OFFSET(LotCellTriangle, values, 0x24);
TS3_X86_OFFSET(LotCellTriangle, gradient, 0x30);

// TS3W.exe 0x00BF3B80: __thiscall(this, 9 stack args), ret 0x24, returns `this`.
// Fills the descriptor (vtable 0x010724F4 = triangle A) and computes its gradient: with
// i0 = stride*y + x and i1 = stride*(y+1) + x over (*values), the pair
// dx0 = values[i1] - values[i0], dx1 = values[i1+1] - values[i0] gives
// (dx0 - dx1, 1.0f, -dx0, 0.0f). 0x006D0A80 first tries the owner's cached vector for `index`
// and only writes the computed one when the cache misses; 0x00BF3820 then takes the finished
// vector either way. EA quirks kept: the `base` argument is stored but never used here, and the
// SSE determinant is evaluated lane-wise exactly as the original's shufps/mulps/subps sequence.
void* BuildLotCellTriangleA(void* self, int32_t index, void* owner, int32_t reversed, int32_t stride,
                            int32_t column, int32_t x, int32_t y, int32_t base,
                            const float* const* values);

// TS3W.exe 0x00BF3F20: __thiscall(this, 9 stack args), ret 0x24, returns `this`. Triangle B
// (vtable 0x01072548): the same shape with the neighbouring column as the reference,
// d0 = values[i1] - values[i0+1], d1 = values[i1+1] - values[i0+1] giving (d0 - d1, 1.0f, -d1, 0.0f).
void* BuildLotCellTriangleB(void* self, int32_t index, void* owner, int32_t reversed, int32_t stride,
                            int32_t column, int32_t x, int32_t y, int32_t base,
                            const float* const* values);

// TS3W.exe 0x00BF1570: __thiscall(this), ret 0, vtable slot 0 of 0x010724F4 (and of the two
// derived tables 0x010725A0/0x010725E8). Appends triangle A's three indices to the owner's index
// array through 0x006D4BC0: (i0+base, i1+base, i1+base+1) when [this+0x0C] == 0, the same three in
// reverse order otherwise. Returns the last 0x006D4BC0 result in eax, as the original leaves it.
int32_t EmitLotCellTriangleAIndices(void* self);

// TS3W.exe 0x00BF1A40: __thiscall(this), ret 0, vtable slot 0 of 0x01072548 (and 0x01072558 /
// 0x01072624). Triangle B's indices: (i1+base, i1+base+1, i0+base+1), reversed when [this+0x0C]
// is non-zero. Returns the last 0x006D4BC0 result in eax.
int32_t EmitLotCellTriangleBIndices(void* self);

// TS3W.exe 0x00BA1AD0: __thiscall(this, 6 stack args), ret 0x18. Walks the four borders of the
// lattice held by [this+0x18] (its column count via vtable slot 0x10, row count via slot 0x14,
// origin floats at +0x5C/+0x60, accept key at +0x18), inset by 8 on every side, and for each
// accepted cell emits the two border normals through 0x006D5D60 and the quad through 0x00B9D550.
// The cell pair is (vertices[offA], vertices[offB]) with lane 1 zeroed, each normalised in XZ with
// the original's two SSE rsqrt Newton steps and negated. Args 4..6 are never read. Returns
// columns - 8, the value eax keeps on every path (the last loop's exit counter).
// EA quirks kept: 0x00BA08F0/0x00BA1760 are tested with the original instruction sequence (only the low byte counts)
// and the two coordinate floats live in one frame pair that the callees may rewrite in place.
int32_t BuildLotBorderCells(void* self, void* lot, void* scratch, const float* const* vertices,
                            int32_t arg4, int32_t arg5, int32_t arg6);

}  // namespace ts3::renderer
