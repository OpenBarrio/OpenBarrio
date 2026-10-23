#pragma once
#include "ts3/layout.h"
#include "ts3/math/float_values.h"
#include "ts3/math/m04_layouts.h"
#include <cstdint>

namespace ts3::math::m05 {

// TS3W.exe 0x0092D330: row-major 4x4 multiply out = m * a (m = arg1, a = arg2).
// Scalar SSE per element, additions strictly left-associated ((t0+t1)+t2)+t3;
// every input load completes before the first output store, so overlapping
// out with m or a reproduces the original read-all-then-write behavior.
// Plain __cdecl, returns void (EAX leaves the out pointer as scratch residue).
void MatrixMultiply(float* out, const float* m, const float* a) noexcept;

// Owner container mutated by CurveBuild: begin/end/limit/allocator pointers and
// the cached record count at +0x10, filled with 20-byte records
// {x, y, prevX, prevY, slope}. Not a claim about the complete original class.
struct CurveState {
    void* begin;
    void* end;
    void* limit;
    void* allocatorField;
    int32_t count;
};
TS3_X86_SIZE(CurveState, 20);
TS3_X86_OFFSET(CurveState, end, 4);
TS3_X86_OFFSET(CurveState, limit, 8);
TS3_X86_OFFSET(CurveState, allocatorField, 0xC);
TS3_X86_OFFSET(CurveState, count, 0x10);

// {begin, end} pair the original dereferences for its 8-byte {float x, y}
// input points (range->begin[8*i], half-open).
struct PointRange {
    const unsigned char* begin;
    const unsigned char* end;
};
TS3_X86_SIZE(PointRange, 8);

// TS3W.exe 0x007CC5C0: append slope records for the 8-byte {float x, y} pairs
// between range->begin and range->end onto *state (end reset to begin first).
// dy comes from an x87 subtraction kept in extended precision for the double
// 1e-10 tests; the first pair's prevX is -FLT_MAX but its prevY is its own y,
// so the first record always has dy == 0 and slope == 0. Full storage growth is
// delegated to the boundary container routine at 0x00839220 (bridge call); in
// x64 builds that growth path aborts instead of calling game code.
// __thiscall, ret 4; returns range->begin != range->end in AL (EAX = 0/1).
bool CurveBuild(CurveState* state, const PointRange* range);

// TS3W.exe 0x00ABAC80: overlap test of two four-point {float x, y} quads
// (32 bytes each) against the unit edge normals of quad a's v1->v2 / v0->v1
// edges and quad b's v1->v2 / v0->v1 edges. EA quirk kept: the per-axis
// disjunction also accepts the tie |minB - minA| < 0.001 evaluated in x87
// extended precision; NaN projections always overwrite their min/max slots.
// __cdecl, result in AL only (upper EAX keeps pointer residue on the true path).
bool QuadOverlap(const float* quada, const float* quadb) noexcept;

// TS3W.exe 0x0068CCF0: translated extended-frustum / box separation sweep over
// the packed plane coefficients at frustum +0x120..+0x2BF, per-pair edge
// vectors at +0x2C0..+0x37F, corners at +0x380..+0x3FF and cached broad bounds
// at +0x400/+0x410 (see m04::ExtendedFrustumStorage). Returns false as soon as
// any stage separates; corner offsets are scaled by 0.5 exactly as in the
// original, which the |min+max| > (max-min) + box radius + 1e-5 test cancels.
// __thiscall, ret 8; returns only AL (upper EAX keeps scratch residue).
bool TranslatedFrustumBoxIntersect(const m04::ExtendedFrustumStorage* frustum,
                                   const scratch::Float4Pair689D10* box,
                                   const Float4Value* translation) noexcept;

}  // namespace ts3::math::m05
