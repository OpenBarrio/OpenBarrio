#pragma once
#include <cstdint>
#include <xmmintrin.h>
namespace ts3::math::b08 {
// TS3W.exe 0x00928C30: Project four XYZ differences by dropping a selected axis; retain invalid-axis writes and EAX residue.
int Project(int axis, const float* src, const float* origin, float* out);
// TS3W.exe 0x009696E0: Wrap only Y to within pi of the reference; unordered comparisons stop.
__m128* __stdcall Wrap(__m128* a1, __m128* a2);
// TS3W.exe 0x0097ED40: Transform direction with forced positive-zero W, retaining fourth-row multiplication.
__m128* Transform(__m128* a1, __m128* a2, __m128* a3);
// TS3W.exe 0x0097EDB0: Construct angular vector, including duplicated X/W.
__m128* Angles(__m128* out, float a, float b);
// TS3W.exe 0x009E55D0: Cross two differences, duplicating Z into W.
__m128* Cross(__m128* result, __m128* a2, __m128* a3, __m128* a4);
// TS3W.exe 0x009E5660: Subtract XYZ and duplicate Z into W.
__m128* Difference(__m128* result, __m128* a2, __m128* a3);
// TS3W.exe 0x00A858D0: Rotate and quantize two coordinates with truncation and low-word stores.
uint16_t* Quantize(const float* origin, float c, float s, float x, float y, float sx, float sy, uint16_t* u, uint16_t* v);
// TS3W.exe 0x00AFD7C0: Construct cosine/zero/negative-sine vector with duplicated Z.
__m128* Heading(float angle, __m128* out);
// TS3W.exe 0x00AFD9E0: Snap XY with x87 rounding (not truncation), then reload and return raw Z.
uint32_t Snap(const float* src, float step, float* out);
// TS3W.exe 0x00C11900: Construct spherical direction using unspilled x87 cosine products.
__m128* Spherical(__m128* out, float a, float b);
// TS3W.exe 0x00C13780: Build scaled frame with two RSQRT Newton refinements and explicit runtime offset.
__m128* Frame(__m128* a1, __m128* a2, __m128* a3, __m128* a4, float offset);
// TS3W.exe 0x00C171F0: Manhattan XYZ distance with float differences and x87 double-precision sum.
double __stdcall Distance(__m128* a, __m128* b);
// TS3W.exe 0x00C474A0: Normalize signed-byte planar perpendicular with two RSQRT refinements.
__m128* Normal(const signed char* a1, __m128* a2);
// TS3W.exe 0x00C13780: ABI entry loading the external runtime offset.
__m128* __stdcall FrameEntry(__m128*,__m128*,__m128*,__m128*);
#if defined(_M_IX86)
// TS3W.exe 0x009E55D0: EAX-output/ECX-input ABI entry.
void CrossEntry();
// TS3W.exe 0x009E5660: EAX-output/ECX-input ABI entry.
void DifferenceEntry();
// TS3W.exe 0x00C11900: EAX-output ABI entry.
void SphericalEntry();
#endif
}
