#pragma once
// Stage 3 m04 batch M01: one matrix-symmetric kernel and one matrix-tagged
// (general 4x4 inverse) value kernel. Both are pure scalar-SSE ports whose
// operation and operand order match TS3W.exe exactly (NaN payloads and
// signed zeros are observable), so no fast-math style reassociation is used.
namespace ts3::math::m04m01 {

// TS3W.exe 0x0046D9B0: accumulate the upper triangle (xx, xy, xz, yy, yz, zz)
// of the weighted covariance of `count` 3-float points (12-byte stride) with
// per-point weights into `out`, about the weighted mean, and return `out`.
// EA quirks kept: the reciprocal 1/sum(w) has no zero guard (count 0 or
// all-zero weights produce inf/NaN means that never reach `out` because the
// accumulation loop then runs zero times), each covariance term is
// (component*w)*component rather than w*(component*component), the mean
// products keep the mixed operand order inv*sumX / sumY*inv / sumZ*inv, and
// `out` may alias the point or weight arrays (per-point stores interleave
// with later loads exactly as in the original unrolled body).
float* CovarianceSymmetric(float* out, int count, const float* points, const float* weights);

// TS3W.exe 0x00511DB0: general 4x4 matrix inverse by cofactors (column-major
// 16-float input, four 16-byte output rows). Returns 1 when the inverse was
// written, 0 when rejected. EA quirks kept: the input and output must be
// 16-byte aligned (movaps), the rejection test is |det| < 0x358637BD
// (9.99999997e-7) evaluated with x87 FCOMI/JBE so a NaN determinant is
// *unordered* and proceeds to compute a NaN inverse returning 1, -0.0-x
// subtractions are literal subtractions from the -0.0 constant, and the
// reciprocal is 1.0f/det with no clamp.
char MatrixInverse(const float* in, float* out);

} // namespace ts3::math::m04m01
