#include "ts3/math/m04_M05.h"
#include "ts3/layout.h"

#include <cstring>
#include <limits>
#if defined(_M_IX86)
#include <xmmintrin.h>
#include <emmintrin.h>
#include <cmath>
#endif

#pragma float_control(precise, on, push)

namespace ts3::math::m05 {

// TS3W.exe 0x0092D330 (1660 bytes): out = m * a, row-major 4x4, scalar SSE
// (MULSS/ADDSS per element). The original reads every source dword before the
// first output store (0x0092D91D), so overlapping out with m or a is safe.
void MatrixMultiply(float* out, const float* m, const float* a) noexcept {
    for (int row = 0; row < 4; ++row) {
        for (int column = 0; column < 4; ++column) {
            // Strictly left-associated ((t0+t1)+t2)+t3, single rounding at
            // every step exactly like the original chain.
            __m128 accumulated = _mm_mul_ss(_mm_load_ss(m + row * 4 + 0), _mm_load_ss(a + column));
            accumulated = _mm_add_ss(accumulated,
                                     _mm_mul_ss(_mm_load_ss(m + row * 4 + 1), _mm_load_ss(a + 4 + column)));
            accumulated = _mm_add_ss(accumulated,
                                     _mm_mul_ss(_mm_load_ss(m + row * 4 + 2), _mm_load_ss(a + 8 + column)));
            accumulated = _mm_add_ss(accumulated,
                                     _mm_mul_ss(_mm_load_ss(m + row * 4 + 3), _mm_load_ss(a + 12 + column)));
            _mm_store_ss(out + row * 4 + column, accumulated);
        }
    }
}

#if defined(_M_IX86)

namespace {

// Raw-bit float constructor: avoids any fp environment dependence.
inline float BitsOf(uint32_t bits) noexcept {
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

}  // namespace

// TS3W.exe 0x007CC5C0 (374 bytes): build 20-byte slope records from the 8-byte
// point pairs between range->begin and range->end; end is reset to begin first,
// the cached count at +0x10 is refreshed, and storage growth is delegated to
// the boundary container routine 0x00839220 (bridge call).
bool CurveBuild(CurveState* state, const PointRange* range) {
    state->end = state->begin;
    // ebx = sar(range->end - range->begin, 3): 32-bit arithmetic shift, so a
    // negative difference yields a negative count and skips the loop.
    int total = static_cast<int>(reinterpret_cast<uintptr_t>(range->end) -
                                 reinterpret_cast<uintptr_t>(range->begin));
    total >>= 3;

    struct Record {
        float x, y, previousX, previousY, slope;
    };
    const float* pairs = reinterpret_cast<const float*>(range->begin);
    for (int index = 0; index < total; ++index) {
        const float* pair = pairs + 2 * index;
        const float x = pair[0];
        const float y = pair[1];
        // First pair: prevX = -FLT_MAX (image 0x00FCBB3C) but prevY is its own y.
        const float previousX = index == 0 ? BitsOf(0xff7fffffu) : pair[-2];
        const float previousY = index == 0 ? y : pair[-1];
        // x87 extended subtraction: the 1e-10 comparisons see the extended
        // value while the stored slope uses the float-rounded dy (fst). The
        // compared constant is the double at 0x00F98910 widened on load.
        const long double dy = static_cast<long double>(y) - static_cast<long double>(previousY);
        const float dyRounded = static_cast<float>(dy);
        // SUBSS single-rounded, like the original's stored dx.
        const float dx = _mm_cvtss_f32(_mm_sub_ss(_mm_set_ss(x), _mm_set_ss(previousX)));
        Record record{x, y, previousX, previousY, 0.0f};
        if (!(fabsl(dy) < static_cast<long double>(1.0e-10))) {
            // Ordered dx > 1e-10 * dy (fcomip): unordered never records.
            if (!(dx > static_cast<long double>(1.0e-10) * dy))
                continue;
            // DIVSS: single-rounded like the original (an x87 FDIV would
            // double-round).
            record.slope = _mm_cvtss_f32(_mm_div_ss(_mm_set_ss(dyRounded), _mm_set_ss(dx)));
        }
        if (state->end < state->limit) {
            unsigned char* destination = static_cast<unsigned char*>(state->end);
            state->end = destination + 20;
            if (destination != nullptr)
                std::memcpy(destination, &record, sizeof record);
        } else {
            CallGameMethod<int, 0x00839220>(state, state->end, &record);
        }
    }
    state->count = static_cast<int>(
        (static_cast<const unsigned char*>(state->end) -
         static_cast<const unsigned char*>(state->begin)) / 20);
    return range->begin != range->end;
}

// TS3W.exe 0x00ABAC80 (1222 bytes incl. tail padding): overlap test of two
// four-point {float x, y} quads against their edge normals.
bool QuadOverlap(const float* quada, const float* quadb) noexcept {
    const auto subtract = [](float a, float b) {
        return _mm_cvtss_f32(_mm_sub_ss(_mm_set_ss(a), _mm_set_ss(b)));
    };
    const float dx1 = subtract(quada[5], quada[3]);
    const float dy1 = subtract(quada[4], quada[2]);
    // x87 reciprocal chain: sqrt(dx^2+dy^2) in extended precision, reciprocal
    // via 1/len (the 1.0 seeded by block 1's FLD1 stays in ST(1) and feeds the
    // FDIVR/FDIVP of blocks 2-4), stored as float.
    const long double span1 = static_cast<long double>(dx1) * dx1 +
                              static_cast<long double>(dy1) * dy1;
    const float t1 = static_cast<float>(1.0L / sqrtl(span1));
    // MULSS products (single-rounded); the original scales x by the stored
    // normal and y by the register-resident one.
    const float ex1 = _mm_cvtss_f32(_mm_mul_ss(_mm_set_ss(dx1), _mm_set_ss(t1)));
    const float ey1 = _mm_cvtss_f32(_mm_mul_ss(_mm_set_ss(t1), _mm_set_ss(dy1)));
    const float dx2 = subtract(quada[3], quada[1]);
    const float dy2 = subtract(quada[2], quada[0]);
    const long double span2 = static_cast<long double>(dx2) * dx2 +
                              static_cast<long double>(dy2) * dy2;
    const float t2 = static_cast<float>(1.0L / sqrtl(span2));
    const float ex2 = _mm_cvtss_f32(_mm_mul_ss(_mm_set_ss(dx2), _mm_set_ss(t2)));
    const float ey2 = _mm_cvtss_f32(_mm_mul_ss(_mm_set_ss(t2), _mm_set_ss(dy2)));
    const float dxB1 = subtract(quadb[5], quadb[3]);
    const float dyB1 = subtract(quadb[4], quadb[2]);
    const long double spanB1 = static_cast<long double>(dxB1) * dxB1 +
                               static_cast<long double>(dyB1) * dyB1;
    const float tB1 = static_cast<float>(1.0L / sqrtl(spanB1));
    const float exB1 = _mm_cvtss_f32(_mm_mul_ss(_mm_set_ss(dxB1), _mm_set_ss(tB1)));
    const float eyB1 = _mm_cvtss_f32(_mm_mul_ss(_mm_set_ss(tB1), _mm_set_ss(dyB1)));
    const float dxB2 = subtract(quadb[3], quadb[1]);
    const float dyB2 = subtract(quadb[2], quadb[0]);
    const long double spanB2 = static_cast<long double>(dxB2) * dxB2 +
                               static_cast<long double>(dyB2) * dyB2;
    // Block 4 ends FDIVP ST(1), ST(0) with the 1.0 left over from block 1's
    // FLD1 still in ST(1), so ST(1)/ST(0) = 1/len: a normal reciprocal, same
    // as the other three blocks (the earlier "length quirk" was a misread).
    const float tB2 = static_cast<float>(1.0L / sqrtl(spanB2));
    const float exB2 = _mm_cvtss_f32(_mm_mul_ss(_mm_set_ss(dxB2), _mm_set_ss(tB2)));
    const float eyB2 = _mm_cvtss_f32(_mm_mul_ss(_mm_set_ss(tB2), _mm_set_ss(dyB2)));

    const auto project = [](float y, float axisY, float x, float axisX) {
        // (y * ex) + (x * ey), MULSS then MULSS then ADDSS as in the original.
        return _mm_cvtss_f32(_mm_add_ss(_mm_mul_ss(_mm_set_ss(y), _mm_set_ss(axisY)),
                                        _mm_mul_ss(_mm_set_ss(x), _mm_set_ss(axisX))));
    };
    // Slot 2k = quad a, 2k+1 = quad b on normal k. Mins start at +FLT_MAX and
    // maxes at -FLT_MAX; a NaN projection always overwrites its slot.
    float minSlot[8];
    float maxSlot[8];
    for (unsigned slot = 0; slot != 8; ++slot) {
        minSlot[slot] = BitsOf(0x7f7fffffu);
        maxSlot[slot] = BitsOf(0xff7fffffu);
    }
    for (int vertex = 0; vertex != 8; ++vertex) {
        const float* quad = vertex < 4 ? quada : quadb;
        const float x = quad[2 * (vertex % 4) + 0];
        const float y = quad[2 * (vertex % 4) + 1];
        const float projections[4] = {
            project(y, ex1, x, ey1),
            project(y, ex2, x, ey2),
            project(y, exB1, x, eyB1),
            project(y, exB2, x, eyB2),
        };
        const unsigned slot = static_cast<unsigned>(vertex) >> 2;
        for (unsigned axis = 0; axis != 4; ++axis) {
            // COMISS + JA updates, with the original's operand order: the min
            // blocks compare PROJ against the slot (0x00ABAEB5 comiss xmm1,
            // [esp+eax*4+0x14]), the max blocks compare SLOT against the proj
            // (0x00ABAEEE comiss xmm2, xmm1). Either way the slot survives only
            // while it still wins; a NaN projection always takes over.
            // Slots are laid out like the original frame: quad a block first
            // ([esp+0x14 + 8k] min / [esp+0x34 + 8k] max), quad b block after.
            if (!_mm_comigt_ss(_mm_set_ss(projections[axis]), _mm_set_ss(minSlot[4 * slot + axis])))
                minSlot[4 * slot + axis] = projections[axis];
            if (!_mm_comigt_ss(_mm_set_ss(maxSlot[4 * slot + axis]), _mm_set_ss(projections[axis])))
                maxSlot[4 * slot + axis] = projections[axis];
        }
    }

    // Per axis, three quirky disjuncts; the tie test |minB - minA| < 0.001 is
    // evaluated in x87 extended precision (fcomi against the float widened
    // from image 0x001064D14), so a NaN difference never overlaps.
    const float eps = BitsOf(0x3a83126fu);
    const long double epsExtended = static_cast<long double>(eps);
    const auto add = [eps](float a, float b) {
        return _mm_cvtss_f32(_mm_add_ss(_mm_set_ss(a), _mm_set_ss(b)));
    };
    const auto above = [](float a, float b) { return _mm_comigt_ss(_mm_set_ss(a), _mm_set_ss(b)); };
    for (unsigned axis = 0; axis != 4; ++axis) {
        const float minA = minSlot[axis];
        const float maxA = maxSlot[axis];
        const float minB = minSlot[4 + axis];
        const float maxB = maxSlot[4 + axis];
        bool overlap = false;
        if (above(minA, add(minB, eps)) && above(maxB, add(minA, eps)))
            overlap = true;
        else if (above(maxA, add(minB, eps)) && above(minB, add(minA, eps)))
            overlap = true;
        else {
            const long double difference =
                fabsl(static_cast<long double>(minB) - static_cast<long double>(minA));
            if (difference < epsExtended && above(maxB, add(minA, eps)))
                overlap = true;
        }
        if (!overlap)
            return false;
    }
    return true;
}

// TS3W.exe 0x0068CCF0 (1081 bytes): translated extended-frustum / box SAT.
bool TranslatedFrustumBoxIntersect(const m04::ExtendedFrustumStorage* frustum,
                                   const scratch::Float4Pair689D10* box,
                                   const Float4Value* translation) noexcept {
    const __m128* f = reinterpret_cast<const __m128*>(frustum);
    const __m128 boxMin = _mm_load_ps(&box->first.m128_f32[0]);
    const __m128 boxMax = _mm_load_ps(&box->second.m128_f32[0]);
    const __m128 moved = _mm_load_ps(translation->lane);
    // Read-only literals (image 0x00F9A5AC 0.5, 0x00FAA210 -1, 0x00F9D2C8 1e-5,
    // 0x00FEFBD0 1e30, 0x00FF0824 -1e30; the test validates them in the map).
    const __m128 half = _mm_set1_ps(BitsOf(0x3f000000u));
    const __m128 minusOne = _mm_set1_ps(BitsOf(0xbf800000u));
    const __m128 epsilon = _mm_set1_ps(BitsOf(0x3727c5acu));
    const __m128 minInit = _mm_set1_ps(BitsOf(0x7149f2cau));
    const __m128 maxInit = _mm_set1_ps(BitsOf(0xf149f2cau));

    __m128 center = _mm_mul_ps(_mm_add_ps(boxMin, boxMax), half);
    const __m128 extent = _mm_mul_ps(_mm_sub_ps(boxMax, boxMin), half);
    center = _mm_add_ps(center, moved);
    const __m128 broadDelta = _mm_sub_ps(f[64], center);
    const __m128 broadAbs = _mm_max_ps(_mm_mul_ps(minusOne, broadDelta), broadDelta);
    if (_mm_movemask_ps(_mm_cmplt_ps(_mm_add_ps(f[65], extent), broadAbs)) & 7)
        return false;

    const __m128 centerX = _mm_shuffle_ps(center, center, 0x00);
    const __m128 centerY = _mm_shuffle_ps(center, center, 0x55);
    const __m128 centerZ = _mm_shuffle_ps(center, center, 0xaa);
    const __m128 extentX = _mm_shuffle_ps(extent, extent, 0x00);
    const __m128 extentY = _mm_shuffle_ps(extent, extent, 0x55);
    const __m128 extentZ = _mm_shuffle_ps(extent, extent, 0xaa);
    // Plane dot products: ((ny*cy) + (nx*cx)) + (nz*cz) per packed quad, and
    // box projection radii ((|ny|*ey) + (|nx|*ex)) + (|nz|*ez).
    const __m128 dot0 = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(f[20], centerY), _mm_mul_ps(f[18], centerX)),
        _mm_mul_ps(f[22], centerZ));
    const __m128 dot1 = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(f[21], centerY), _mm_mul_ps(f[19], centerX)),
        _mm_mul_ps(f[23], centerZ));
    const __m128 radius0 = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(f[26], extentY), _mm_mul_ps(f[24], extentX)),
        _mm_mul_ps(f[28], extentZ));
    const __m128 radius1 = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(f[27], extentY), _mm_mul_ps(f[25], extentX)),
        _mm_mul_ps(f[29], extentZ));
    const __m128 d0 = f[42];
    const __m128 d1 = f[43];
    const __m128 ones = _mm_castsi128_ps(_mm_set1_epi32(-1));
    const __m128 outside0 = _mm_xor_ps(_mm_cmple_ps(d0, _mm_add_ps(radius0, dot0)), ones);
    const __m128 outside1 = _mm_xor_ps(_mm_cmple_ps(d1, _mm_add_ps(radius1, dot1)), ones);
    if (_mm_movemask_ps(_mm_or_ps(outside0, outside1)) != 0)
        return false;
    const unsigned pairMask =
        static_cast<unsigned>(_mm_movemask_ps(_mm_xor_ps(_mm_cmple_ps(d1, dot1), ones))) << 4 |
        static_cast<unsigned>(_mm_movemask_ps(_mm_xor_ps(_mm_cmple_ps(d0, dot0), ones)));
    if (pairMask == 0)
        return true;

    // One mask-table entry (image 0x00FF0828) per adjacent plane pair; each
    // selected pair contributes the box-edge crosses XxE, YxE, ZxE of the
    // stored edge vector at +0x2C0 + 16*entry.
    static const uint32_t kPairMaskTable[12] = {0x11, 0x18, 0x12, 0x14, 0x28, 0x21,
                                                0x24, 0x22, 0x06, 0x0a, 0x09, 0x05};
    alignas(16) __m128 axes[36];
    __m128* writer = axes;
    const __m128 unitX = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    const __m128 unitY = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
    const __m128 unitZ = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
    for (unsigned entry = 0; entry != 12; ++entry) {
        if ((kPairMaskTable[entry] & pairMask) == 0)
            continue;
        const __m128 edge = f[44 + entry];
        const __m128 a2 = _mm_shuffle_ps(edge, edge, 0x12);  // (z,y,x,x)
        const __m128 a1 = _mm_shuffle_ps(edge, edge, 0x09);  // (y,z,x,x)
        // Per unit axis c: shuf(c,9)*(z,y,x,x) - shuf(c,0x12)*(y,z,x,x),
        // exactly the original blend (whose immediates produce the shipped
        // degenerate axes - transcribed, not repaired).
        const auto cross = [&](const __m128 c) {
            return _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(c, c, 0x09), a2),
                              _mm_mul_ps(_mm_shuffle_ps(c, c, 0x12), a1));
        };
        *writer++ = cross(unitX);
        *writer++ = cross(unitY);
        *writer++ = cross(unitZ);
    }
    if (writer == axes)
        return true;

    // Corners relative to the translated box center, scaled by 0.5 as stored.
    alignas(16) __m128 relative[8];
    for (unsigned corner = 0; corner != 8; ++corner)
        relative[corner] = _mm_mul_ps(_mm_sub_ps(f[56 + corner], center), half);

    for (const __m128* axis = axes; axis != writer; ++axis) {
        const __m128 w = *axis;
        __m128 lowest = minInit;
        __m128 highest = maxInit;
        for (unsigned corner = 0; corner != 8; ++corner) {
            const __m128 projected = _mm_mul_ps(relative[corner], w);
            const __m128 lanes = _mm_add_ps(
                _mm_add_ps(_mm_shuffle_ps(projected, projected, 0x02),
                           _mm_shuffle_ps(projected, projected, 0x01)),
                projected);
            const __m128 dot = _mm_shuffle_ps(lanes, lanes, 0x00);
            lowest = _mm_min_ps(lowest, dot);
            highest = _mm_max_ps(highest, dot);
        }
        const __m128 boxRadius = _mm_mul_ps(_mm_max_ps(_mm_mul_ps(minusOne, w), w), extent);
        const __m128 radius = _mm_shuffle_ps(
            _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(boxRadius, boxRadius, 0x02),
                                  _mm_shuffle_ps(boxRadius, boxRadius, 0x01)),
                       boxRadius),
            _mm_setzero_ps(), 0x00);
        const __m128 midSum = _mm_add_ps(highest, lowest);
        const __m128 lhs = _mm_max_ps(_mm_mul_ps(minusOne, midSum), midSum);
        const __m128 rhs = _mm_add_ps(_mm_add_ps(radius, _mm_sub_ps(highest, lowest)), epsilon);
        // COMISS + JA on lane 0: ordered lhs > rhs separates.
        if (_mm_comigt_ss(lhs, rhs))
            return false;
    }
    return true;
}

#endif  // _M_IX86

}  // namespace ts3::math::m05

#pragma float_control(pop)
