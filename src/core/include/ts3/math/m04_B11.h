#pragma once
#include <xmmintrin.h>
namespace ts3::math::b11 {
// TS3W.exe 0x0054C8D0: composes SRT values, preserving ordered output/source overlap and returning the second source.
__m128 *ComposePose(__m128 *a1, __m128 *a2, __m128 *a3);
// TS3W.exe 0x0054EEF0: inverts quaternion and scale using two reciprocal refinements; translation is not divided by scale.
__m128 *InvertPose(__m128 *a1, __m128 *a2);
// TS3W.exe 0x005548C0: accumulates weighted pose, using the negative branch for unordered quaternion dot products.
float *AccumulatePose(__m128 *a1, float *a2, __m128 *a3, float a4);
// TS3W.exe 0x00574230: multiplies four-row matrices after loading all inputs.
__m128 *MultiplyMatrix(__m128 *a1, __m128 *a2, __m128 *a3);
// TS3W.exe 0x00577160: converts translation/quaternion/scale to four rows with XYZ masking.
__m128 *SrtMatrix(__m128 *self, __m128 *a2);
// TS3W.exe 0x009576B0: converts quaternion at +10 and translation at +20 to four rows.
__m128 *RigidMatrix(__m128 *self, __m128 *a2);
// TS3W.exe 0x009614A0: writes rigid matrix at owner +1B0, returning the source.
__m128 *StoreRigidMatrix(__m128 *self, __m128 *a2);
// TS3W.exe 0x00555080: transforms a point with ordered translation reload, returning pose.
__m128* TransformPoint(__m128* vector, __m128* pose, __m128* out);
// TS3W.exe 0x00555110: rotates four lanes, returning pose rather than output.
__m128* RotateVector(__m128* vector, __m128* pose, __m128* out);
// TS3W.exe 0x0067ACD0: computes rotated axis extent maximum, with X duplicated in the first axis W.
__m128* RotatedExtent(__m128* extent, __m128* quaternion, __m128* out);
// TS3W.exe 0x0070CBA0: maps square coordinates to a sphere, duplicating X in W.
__m128* SphereProjection(float u,float v,int flip,__m128* out);
// TS3W.exe 0x0070CCA0: maps cube face coordinates to a normalized direction (faces 0..5).
__m128* CubeProjection(int face,float u,float v,__m128* out);
// TS3W.exe 0x00577160: thiscall entry preserving EDX for Animation wrapper 577240.
__m128* __fastcall SrtMatrixEntry(__m128* self,void*,__m128* out);
// TS3W.exe 0x009576B0: thiscall entry for rigid matrix conversion.
__m128* __fastcall RigidMatrixEntry(__m128* self,void*,__m128* out);
// TS3W.exe 0x009614A0: thiscall entry for owner-relative rigid matrix store.
__m128* __fastcall StoreRigidMatrixEntry(__m128* self,void*,__m128* source);
}
