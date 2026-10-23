#pragma once
#include <cstdint>
#include <xmmintrin.h>
#include "ts3/ui/clu_50b5f0_UI.h"
namespace ts3::math::b09 {
// TS3W.exe 0x00511BB0: Scales all rows componentwise by (x,y,z,1), promoting identity to affine; returns input.
__m128 *Kernel511BB0(ts3::ui::MatrixAccumulator *matrix, __m128 *a2);
// TS3W.exe 0x00511D60: Transforms XYZ with row3 translation and emits four lanes; returns output.
__m128 *Kernel511D60(ts3::ui::MatrixAccumulator *matrix, __m128 *a2, __m128 *a3);
// TS3W.exe 0x005127D0: Post-rotates about X, preserving shuffle/add ordering and signed kind promotion.
int Kernel5127D0(ts3::ui::MatrixAccumulator *matrix, unsigned int a2);
// TS3W.exe 0x005128C0: Post-rotates about Y, preserving shuffle/add ordering and signed kind promotion.
int Kernel5128C0(ts3::ui::MatrixAccumulator *matrix, unsigned int a2);
// TS3W.exe 0x005129B0: Post-rotates about Z, preserving shuffle/add ordering and signed kind promotion.
int Kernel5129B0(ts3::ui::MatrixAccumulator *matrix, unsigned int a2);
// TS3W.exe 0x00512AA0: Post-rotates around an axis normalized with two RSQRTPS refinements; returns axis.
__m128 *Kernel512AA0(ts3::ui::MatrixAccumulator *matrix, __m128 *a2, unsigned int a3);
// TS3W.exe 0x00512E70: Adds row.z/distance to every row.w and promotes kind to at least three.
int Kernel512E70(ts3::ui::MatrixAccumulator *matrix, float a2);
// TS3W.exe 0x00561E60: Computes the full inverse and replicated determinant, retaining original store order.
__m128 *Kernel561E60(__m128 *a1, __m128 *a2, __m128 *a3);
// TS3W.exe 0x005621D0: Computes the signed projected-row angle, with strict acos endpoint checks.
double Kernel5621D0(__m128 *a1);
// TS3W.exe 0x007A1840: Multiplies packed scalar 3x3 matrices, preserving row-dependent sum grouping.
float *Kernel7A1840(float *self, float *a2, float *a3);
// TS3W.exe 0x007C23E0: Multiplies 3x3 parts of 4x4 matrices and writes homogeneous identity borders.
float* Kernel7C23E0(float *self, float* output, float *a3);
// TS3W.exe 0x0046E2F0: Chooses the largest symmetric cofactor row after subtracting the eigenvalue.
float *Kernel46E2F0(float *a1, float *a2, float a3);
// TS3W.exe 0x0046E4B0: chooses a perpendicular vector from the largest shifted symmetric entry.
float* Kernel46E4B0(float*,float*,float);
// TS3W.exe 0x0046E650: selects an extreme eigenvalue and constructs its symmetric-matrix vector.
float* Kernel46E650(float*,float*);
// TS3W.exe 0x0092BE60: fills sixteen sine/cosine pairs using an explicit runtime divisor.
float* Kernel92BE60(float*,float);
#if defined(_M_IX86)
// TS3W.exe 0x0046E2F0: EAX input, EDX output, XMM6 shift; preserves EDX.
void Entry46E2F0();
// TS3W.exe 0x0046E4B0: EAX input, ECX output, stack shift; preserves ECX.
void Entry46E4B0();
// TS3W.exe 0x0092BE60: thiscall adapter sourcing the runtime angular divisor.
float* __fastcall Entry92BE60(float*,void*);
// TS3W.exe 0x00511BB0: ABI adapter for the reconstructed kernel.
__m128* __fastcall Entry511BB0(ts3::ui::MatrixAccumulator* self,void*,__m128* input);
// TS3W.exe 0x00511D60: ABI adapter for the reconstructed kernel.
__m128* __fastcall Entry511D60(ts3::ui::MatrixAccumulator* self,void*,__m128* out,__m128* input);
// TS3W.exe 0x005127D0: ABI adapter for the reconstructed kernel.
int __fastcall Entry5127D0(ts3::ui::MatrixAccumulator* self,void*,unsigned angle);
// TS3W.exe 0x005128C0: ABI adapter for the reconstructed kernel.
int __fastcall Entry5128C0(ts3::ui::MatrixAccumulator* self,void*,unsigned angle);
// TS3W.exe 0x005129B0: ABI adapter for the reconstructed kernel.
int __fastcall Entry5129B0(ts3::ui::MatrixAccumulator* self,void*,unsigned angle);
// TS3W.exe 0x00512AA0: ABI adapter for the reconstructed kernel.
__m128* __fastcall Entry512AA0(ts3::ui::MatrixAccumulator* self,void*,__m128* axis,unsigned angle);
// TS3W.exe 0x00512E70: ABI adapter for the reconstructed kernel.
int __fastcall Entry512E70(ts3::ui::MatrixAccumulator* self,void*,float distance);
// TS3W.exe 0x007A1840: ABI adapter for the reconstructed kernel.
float* __fastcall Entry7A1840(float* self,void*,float* out,float* right);
// TS3W.exe 0x007C23E0: ABI adapter for the reconstructed kernel.
float* __fastcall Entry7C23E0(float* self,void*,float* out,float* right);
#endif
}
