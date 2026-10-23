#pragma once
#include <cstdint>
namespace ts3::math {
namespace b01 {
// Explicit runtime trig state (the mapped image does not run its initializer).
struct TrigState { float scale, step; float pairs[32]; };
// TS3W.exe 0x008CA1B0: sample and clamp center +/- radius using the external RNG.
double RandomRange(float center, float radius);
// TS3W.exe 0x00406A10: exponential using x87 F2XM1/FSCALE.
double Exp(float x);
// TS3W.exe 0x00513920: ordered symmetric-range predicate.
int InRange(float x, float radius);
// TS3W.exe 0x0055DE20: approach a target with sign-bit-selected bounds, not IEEE min/max.
double Approach(float from, float to, float rate, float divisor, const float* scale);
// TS3W.exe 0x00662E20: integer power with binary32 intermediate products.
double Power(float x, int exponent);
// TS3W.exe 0x00688E40: emit a square Gaussian kernel column-first, returning the final cursor.
float* Gaussian(float* out, uint32_t count, float sigma);
// TS3W.exe 0x006B74D0: normalize against the ordered maximum of XYZ and the numerator (custom EAX/XMM0 ABI).
float MaxRatio(const float* xyzw, float numerator);
// TS3W.exe 0x006DDA90: inverse scalar response, including the discontinuous 51 sentinel.
double __stdcall Response(float x);
// TS3W.exe 0x006DDBD0: inverse response clamped by subtraction sign bits.
double __stdcall InverseResponse(float x);
// TS3W.exe 0x0070C590: associated Legendre recurrence with x87 intermediate precision.
double Legendre(int degree, int order, double x);
// TS3W.exe 0x0083A9D0: scale magnitude while explicitly preserving signed zero; EAX is scratch (LAHF).
void ScaleMagnitude(float* self, float factor);
// TS3W.exe 0x0083AA20: strict comparison additionally requiring signed-zero-aware sign ordering.
int SignedGreater(const float* self, const float* other);
// TS3W.exe 0x0083AA90: non-strict comparison additionally requiring signed-zero-aware sign ordering.
int SignedLessEqual(const float* self, const float* other);
// TS3W.exe 0x0083F440: convert radians to degrees, preserving the original sign policy, and return output.
float* __stdcall Degrees(float* out, const float* radians);
// TS3W.exe 0x008C3C70: table-based second-order sine/cosine approximation; reload table after first output.
float* SinCos(float angle, float* sine, float* cosine, const TrigState& state);
// ABI-only entry adapters: native kernels above keep testable typed inputs.
// TS3W.exe 0x0083A9D0: thiscall adapter; both recovered callers ignore EAX scratch.
void __fastcall ScaleMagnitudeEntry(float* self, void*, float factor);
// TS3W.exe 0x0083AA20: thiscall adapter for SignedGreater.
int __fastcall SignedGreaterEntry(const float* self, void*, const float* other);
// TS3W.exe 0x0083AA90: thiscall adapter for SignedLessEqual.
int __fastcall SignedLessEqualEntry(const float* self, void*, const float* other);
// TS3W.exe 0x008C3C70: cdecl entry using the external runtime trig data without copying it.
float* SinCosEntry(float angle, float* sine, float* cosine);
#if defined(_M_IX86)
// TS3W.exe 0x006B74D0: EAX input, stack numerator, XMM0 output; preserve input EAX/ECX/EDX.
void MaxRatioEntry();
#endif

}
}
