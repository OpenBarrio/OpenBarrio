// m04_M02.cpp - Stage 3 m04 batch M02.
// 0x00577260 (pose): 3x4 basis -> translation/quaternion/scale decomposition
//   with the rsqrtps + two Newton-step normalization, the trace/largest-
//   diagonal branch selection and each branch's exact edge sums and
//   quaternion lane order.
// 0x00678050 (intersection): box vs triangle separating-axis test
//   (world-axis AABB overlap, triangle normal, nine edge/world crosses) with
//   the NaN-never-separates comiss tests and the exact dot operand orders.
// Both are exact SSE ports; every statement below performs one original
// instruction (or one unrolled horizontal-add pair) in the original operand
// order, so NaN payloads and signed zeros match bit for bit.
#include "ts3/math/m04_M02.h"
#include <cstring>
#include <xmmintrin.h>

namespace ts3::math::m04m02 {
namespace {

// Read-only image constants: 00F98964, 0107A538, 00F9A5AC, 00FAA210.
__m128 ConstBits(uint32_t bits) {
    float f;
    std::memcpy(&f, &bits, 4);
    return _mm_set1_ps(f);
}
const __m128 kZero = ConstBits(0x00000000u);   // [0x00F98964]
const __m128 kOne = ConstBits(0x3f800000u);    // [0x0107A538]
const __m128 kHalf = ConstBits(0x3f000000u);   // [0x00F9A5AC]
const __m128 kNegOne = ConstBits(0xbf800000u); // [0x00FAA210]

// One shuffle instruction (immediate must be a template parameter).
template<int imm>
__m128 Shuf(__m128 a, __m128 b) { return _mm_shuffle_ps(a, b, imm); }
// The original horizontal add: ((v[2] + v[1]) + v[0]); only lane 0 is ever
// consumed (via the broadcast below or a comiss), matching movss/shufps use.
__m128 HSum(__m128 v) { return _mm_add_ps(_mm_add_ps(Shuf<2>(v, v), Shuf<1>(v, v)), v); }
__m128 Bc(__m128 v) { return Shuf<0>(v, v); }
// The original |x|: max(-1*x, x); maxps returns the second operand on NaN.
__m128 AbsPs(__m128 v) { return _mm_max_ps(_mm_mul_ps(v, kNegOne), v); }

} // namespace

// TS3W.exe 0x00577260. Direct transcription; xmm names in comments.
void* __fastcall DecomposeSrt(void* outV, void* /*edxUnused*/, const void* inV) {
    float* out = static_cast<float*>(outV);
    const float* in = static_cast<const float*>(inV);
    const __m128 r0 = _mm_load_ps(in + 0);              // movaps xmm0,[eax]
    const __m128 r1 = _mm_load_ps(in + 4);              // movaps xmm1,[eax+10]
    const __m128 r2 = _mm_load_ps(in + 8);              // movaps xmm2,[eax+20]
    const __m128 bc00 = Shuf<0x00>(r0, r0);             // m00 in every lane
    const __m128 bc11 = Shuf<0x55>(r1, r1);             // m11
    const __m128 bc22 = Shuf<0xaa>(r2, r2);             // m22
    const __m128 sum0011 = _mm_add_ps(bc00, bc11);      // xmm4 (also saved [esp+30])
    const __m128 trace = _mm_add_ps(sum0011, bc22);     // t broadcast
    const float m00f = _mm_cvtss_f32(bc00);
    const float m11f = _mm_cvtss_f32(bc11);
    const float m22f = _mm_cvtss_f32(bc22);
    const float tr = _mm_cvtss_f32(trace);
    const float kZeroF = _mm_cvtss_f32(kZero);
    __m128 q;

    if (tr > kZeroF) {
        // Trace branch (0x5772D1): s = sqrt(1+t); k = 0.5/s.
        const __m128 s = _mm_sqrt_ps(_mm_add_ps(kOne, trace));
        const __m128 k = _mm_div_ps(kHalf, s);
        const __m128 x4 = _mm_mul_ps(
            _mm_sub_ps(Shuf<0x55>(r0, r0), Shuf<0x00>(r1, r1)), k); // (m01-m10)*k
        const __m128 y4 = _mm_mul_ps(
            _mm_sub_ps(Shuf<0x00>(r2, r2), Shuf<0xaa>(r0, r0)), k); // (m20-m02)*k
        const __m128 z4 = _mm_mul_ps(
            _mm_sub_ps(Shuf<0xaa>(r1, r1), Shuf<0x55>(r2, r2)), k); // (m12-m21)*k
        const __m128 w = _mm_mul_ps(s, kHalf);                      // s*0.5
        q = Shuf<0x88>(Shuf<0x10>(z4, y4), Shuf<0x32>(x4, w));      // (z4, y4, x4, w)
    } else if ((m00f > m11f) && (m00f > m22f)) {
        // m00 branch (0x57737B): D = m00-(m22+m11); s = sqrt(1+D); k = 0.5/s.
        const __m128 d = _mm_sub_ps(bc00, _mm_add_ps(bc22, bc11));
        const __m128 s = _mm_sqrt_ps(_mm_add_ps(kOne, d));
        const __m128 k = _mm_div_ps(kHalf, s);
        const __m128 x4 = _mm_mul_ps(
            _mm_add_ps(Shuf<0x00>(r1, r1), Shuf<0x55>(r0, r0)), k); // (m10+m01)*k
        const __m128 y4 = _mm_mul_ps(
            _mm_add_ps(Shuf<0x00>(r2, r2), Shuf<0xaa>(r0, r0)), k); // (m20+m02)*k
        const __m128 z4 = _mm_mul_ps(
            _mm_sub_ps(Shuf<0xaa>(r1, r1), Shuf<0x55>(r2, r2)), k); // (m12-m21)*k
        const __m128 w = _mm_mul_ps(s, kHalf);                      // s*0.5
        q = Shuf<0x88>(Shuf<0x10>(w, x4), Shuf<0x32>(y4, z4));      // (w, x4, y4, z4)
    } else if (m11f > m22f) {
        // m11 branch (0x577409): D = m11-(m22+m00); s = sqrt(1+D); k = 0.5/s.
        const __m128 d = _mm_sub_ps(bc11, _mm_add_ps(bc22, bc00));
        const __m128 s = _mm_sqrt_ps(_mm_add_ps(kOne, d));
        const __m128 k = _mm_div_ps(kHalf, s);
        const __m128 x4 = _mm_mul_ps(
            _mm_add_ps(Shuf<0x00>(r1, r1), Shuf<0x55>(r0, r0)), k); // (m10+m01)*k
        const __m128 y4 = _mm_mul_ps(
            _mm_sub_ps(Shuf<0x00>(r2, r2), Shuf<0xaa>(r0, r0)), k); // (m20-m02)*k
        const __m128 z4 = _mm_mul_ps(
            _mm_add_ps(Shuf<0x55>(r2, r2), Shuf<0xaa>(r1, r1)), k); // (m21+m12)*k
        const __m128 w = _mm_mul_ps(s, kHalf);                      // s*0.5 (0x577461: xmm0=[esp+0x20]=s)
        q = Shuf<0x88>(Shuf<0x10>(x4, w), Shuf<0x32>(z4, y4));      // (x4, w, z4, y4)
    } else {
        // m22 branch (0x577478): D = m22-(m00+m11); s = sqrt(1+D); k = 0.5/s.
        const __m128 d = _mm_sub_ps(bc22, sum0011);
        const __m128 s = _mm_sqrt_ps(_mm_add_ps(kOne, d));
        const __m128 k = _mm_div_ps(kHalf, s);
        const __m128 x4 = _mm_mul_ps(
            _mm_sub_ps(Shuf<0x55>(r0, r0), Shuf<0x00>(r1, r1)), k); // (m01-m10)*k
        const __m128 y4 = _mm_mul_ps(
            _mm_add_ps(Shuf<0x00>(r2, r2), Shuf<0xaa>(r0, r0)), k); // (m20+m02)*k
        const __m128 z4 = _mm_mul_ps(
            _mm_add_ps(Shuf<0x55>(r2, r2), Shuf<0xaa>(r1, r1)), k); // (m21+m12)*k
        const __m128 w = _mm_mul_ps(s, kHalf);                      // s*0.5
        q = Shuf<0x88>(Shuf<0x10>(y4, z4), Shuf<0x32>(w, x4));      // (y4, z4, w, x4)
    }

    _mm_store_ps(out + 4, q);                              // movaps [ecx+10]: raw q
    const __m128 qs = _mm_load_ps(out + 4);                // movaps [ecx+10] reload
    const __m128 sq = _mm_mul_ps(qs, qs);                  // xmm0 = q*q
    const __m128 sp = _mm_add_ps(Shuf<0x0e>(sq, sq), sq);  // (a0+a2, a1+a3, ...)
    const __m128 sp1 = _mm_add_ps(Shuf<1>(sp, sp), sp);    // lane0 = (a1+a3)+(a0+a2)
    const __m128 norm = Bc(sp1);                           // S = sum of squares broadcast
    // Two Newton-Raphson steps from rsqrtps, seeded with 1.0 (xmm0/xmm3 = 1).
    __m128 x = _mm_rsqrt_ps(norm);
    __m128 t = _mm_sub_ps(kOne, _mm_mul_ps(_mm_mul_ps(x, x), norm));
    x = _mm_add_ps(_mm_mul_ps(t, _mm_mul_ps(x, kHalf)), x);
    t = _mm_sub_ps(kOne, _mm_mul_ps(_mm_mul_ps(x, x), norm));
    x = _mm_add_ps(_mm_mul_ps(t, _mm_mul_ps(x, kHalf)), x);
    _mm_store_ps(out + 4, _mm_mul_ps(x, qs));              // normalized q

    // Reads happen after the stores, so aliasing behaves like the original.
    _mm_store_ps(out + 0, _mm_load_ps(in + 12));           // translation [eax+30] -> [ecx]
    const __m128 r2bits = _mm_load_ps(in + 8);             // re-read after the stores
    const __m128 r1bits = _mm_load_ps(in + 4);
    const __m128 r0bits = _mm_load_ps(in + 0);
    const __m128 row2 = _mm_mul_ps(r2bits, r2bits);
    const __m128 row1 = _mm_mul_ps(r1bits, r1bits);
    const __m128 row0 = _mm_mul_ps(r0bits, r0bits);
    const __m128 sz = HSum(row2);                          // ((z^2+y^2)+x^2)
    const __m128 sy = HSum(row1);
    const __m128 sx = HSum(row0);
    const __m128 scale = Shuf<0x28>(Shuf<0x10>(_mm_sqrt_ps(Bc(sx)), _mm_sqrt_ps(Bc(sy))),
                                    _mm_sqrt_ps(Bc(sz)));    // (sx, sy, sz, sz)
    _mm_store_ps(out + 8, scale);                          // movaps [ecx+20]
    return const_cast<void*>(inV);
}

// TS3W.exe 0x00678050. Direct transcription of the 11 separating-axis tests.
unsigned char __cdecl SweptPointTriangleOverlap(const __m128* v0p, const __m128* v1p,
                                                const __m128* v2p, const __m128* segAp,
                                                const __m128* segBp) {
    const __m128 v2 = _mm_load_ps(reinterpret_cast<const float*>(v2p)); // movaps [edx]
    const __m128 v1 = _mm_load_ps(reinterpret_cast<const float*>(v1p)); // movaps [ecx]
    const __m128 v0 = _mm_load_ps(reinterpret_cast<const float*>(v0p)); // movaps [eax]
    const __m128 segB = _mm_load_ps(reinterpret_cast<const float*>(segBp)); // [ebp+18]
    const __m128 segA = _mm_load_ps(reinterpret_cast<const float*>(segAp)); // [ebp+14]
    const __m128 e0 = _mm_sub_ps(v1, v0);
    const __m128 e1 = _mm_sub_ps(v2, v1);
    const __m128 e2 = _mm_sub_ps(v2, v0);
    // n = sh12(e2)*sh9(e0) - sh9(e2)*sh12(e0)
    const __m128 n = _mm_sub_ps(_mm_mul_ps(Shuf<0x12>(e2, e2), Shuf<0x09>(e0, e0)),
                                _mm_mul_ps(Shuf<0x09>(e2, e2), Shuf<0x12>(e0, e0)));

    // Test 0: world axes via the triangle AABB (0x6780BB..0x678179).
    const __m128 triMin = _mm_min_ps(_mm_min_ps(v0, v1), v2);
    const __m128 triMax = _mm_max_ps(_mm_max_ps(v0, v1), v2);
    const __m128 halfDelta = _mm_mul_ps(_mm_sub_ps(segB, segA), kHalf);
    const __m128 center = _mm_mul_ps(_mm_add_ps(segB, segA), kHalf);
    const __m128 triHalf = _mm_mul_ps(_mm_sub_ps(triMax, triMin), kHalf);
    const __m128 c = _mm_sub_ps(center, _mm_mul_ps(_mm_add_ps(triMax, triMin), kHalf));
    const __m128 mask = _mm_cmple_ps(triHalf, _mm_sub_ps(AbsPs(c), halfDelta));
    const __m128 orHL = _mm_or_ps(_mm_unpackhi_ps(mask, mask), _mm_unpacklo_ps(mask, mask));
    const __m128 collapsed = _mm_or_ps(Shuf<0x55>(mask, mask), _mm_unpacklo_ps(orHL, orHL));
    if (_mm_movemask_ps(_mm_cmpeq_ps(collapsed, kZero)) != 0xF) return 0;

    // Test 1: triangle normal (0x678181..0x67821D): comiss/jae, NaN continues.
    const __m128 projN = HSum(_mm_mul_ps(AbsPs(n), halfDelta));
    const __m128 cn = Bc(HSum(_mm_mul_ps(c, n)));
    if (_mm_comige_ss(AbsPs(cn), Bc(projN))) return 0;

    // Edge-axis tests. c2/c0 measure the segment center from the tested edge's
    // midpoint; he2/he0 are that edge's half vector. The mulps operand order
    // alternates between the e0 block (axis first) and the e2/e1 blocks
    // (he/c first); both are observable when both operands are NaN.
    const __m128 e0x = Shuf<0x00>(e0, e0);
    const __m128 e0y = Shuf<0x55>(e0, e0);
    const __m128 e0z = Shuf<0xaa>(e0, e0);
    const __m128 e1x = Shuf<0x00>(e1, e1);
    const __m128 e1y = Shuf<0x55>(e1, e1);
    const __m128 e1z = Shuf<0xaa>(e1, e1);
    const __m128 e2x = Shuf<0x00>(e2, e2);
    const __m128 e2y = Shuf<0x55>(e2, e2);
    const __m128 e2z = Shuf<0xaa>(e2, e2);
    const __m128 neg0y = _mm_sub_ps(kZero, e0y);
    const __m128 neg0z = _mm_sub_ps(kZero, e0z);
    const __m128 neg0x = _mm_sub_ps(kZero, e0x);
    const __m128 neg2y = _mm_sub_ps(kZero, e2y);
    const __m128 neg2z = _mm_sub_ps(kZero, e2z);
    const __m128 neg2x = _mm_sub_ps(kZero, e2x);
    const __m128 neg1y = _mm_sub_ps(kZero, e1y);
    const __m128 neg1z = _mm_sub_ps(kZero, e1z);
    const __m128 neg1x = _mm_sub_ps(kZero, e1x);
    const __m128 he2 = _mm_mul_ps(kHalf, e2);                                   // 0.5*e2
    const __m128 c2 = _mm_sub_ps(center, _mm_mul_ps(_mm_add_ps(v2, v0), kHalf)); // v2+v0 mid
    const __m128 he0 = _mm_mul_ps(kHalf, e0);                                   // 0.5*e0
    const __m128 c0 = _mm_sub_ps(center, _mm_mul_ps(_mm_add_ps(v1, v0), kHalf)); // v1+v0 mid

    // One axis test: separated iff |d2| > |d1| + hsum(|axis|*hd) (comiss/ja).
    auto AxisCheck = [&](const __m128 axis, const __m128 d1v, const __m128 d2v) {
        const __m128 d1 = Bc(HSum(d1v));
        const __m128 proj = Bc(HSum(_mm_mul_ps(AbsPs(axis), halfDelta)));
        const __m128 d2 = Bc(HSum(d2v));
        return _mm_comigt_ss(AbsPs(d2), _mm_add_ps(AbsPs(d1), proj));
    };

    // Block 3-5: crosses of e0, tested against the v0-v2 interval (axis first).
    const __m128 ax3 = Shuf<0x28>(Shuf<0x10>(kZero, e0z), neg0y);            // (0, e0z, -e0y)
    if (AxisCheck(ax3, _mm_mul_ps(ax3, he2), _mm_mul_ps(ax3, c2))) return 0;
    const __m128 ax4 = Shuf<0x28>(Shuf<0x10>(neg0z, kZero), e0x);            // (-e0z, 0, e0x)
    if (AxisCheck(ax4, _mm_mul_ps(ax4, he2), _mm_mul_ps(ax4, c2))) return 0;
    const __m128 ax5 = Shuf<0x28>(Shuf<0x10>(e0y, neg0x), kZero);            // (e0y, -e0x, 0)
    if (AxisCheck(ax5, _mm_mul_ps(ax5, he2), _mm_mul_ps(ax5, c2))) return 0;

    // Block 6-8: crosses of e2, tested against the v0-v1 interval (he/c first).
    const __m128 ax6 = Shuf<0x28>(Shuf<0x10>(kZero, e2z), neg2y);            // (0, e2z, -e2y)
    if (AxisCheck(ax6, _mm_mul_ps(he0, ax6), _mm_mul_ps(c0, ax6))) return 0;
    const __m128 ax7 = Shuf<0x28>(Shuf<0x10>(neg2z, kZero), e2x);            // (-e2z, 0, e2x)
    if (AxisCheck(ax7, _mm_mul_ps(he0, ax7), _mm_mul_ps(c0, ax7))) return 0;
    const __m128 ax8 = Shuf<0x28>(Shuf<0x10>(e2y, neg2x), kZero);            // (e2y, -e2x, 0)
    if (AxisCheck(ax8, _mm_mul_ps(he0, ax8), _mm_mul_ps(c0, ax8))) return 0;

    // Block 9-11: crosses of e1, also against the v0-v1 interval (he/c first).
    const __m128 ax9 = Shuf<0x28>(Shuf<0x10>(kZero, e1z), neg1y);            // (0, e1z, -e1y)
    if (AxisCheck(ax9, _mm_mul_ps(he0, ax9), _mm_mul_ps(c0, ax9))) return 0;
    const __m128 ax10 = Shuf<0x28>(Shuf<0x10>(neg1z, kZero), e1x);           // (-e1z, 0, e1x)
    if (AxisCheck(ax10, _mm_mul_ps(he0, ax10), _mm_mul_ps(c0, ax10))) return 0;
    const __m128 ax11 = Shuf<0x28>(Shuf<0x10>(e1y, neg1x), kZero);           // (e1y, -e1x, 0)
    if (AxisCheck(ax11, _mm_mul_ps(he0, ax11), _mm_mul_ps(c0, ax11))) return 0;

    return 1;
}

} // namespace ts3::math::m04m02
