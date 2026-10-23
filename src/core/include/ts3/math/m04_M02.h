#pragma once
#include <stdint.h>
#include <xmmintrin.h>

namespace ts3::math::m04m02 {

// TS3W.exe 0x00577260: decompose a 64-byte aligned matrix into a 48-byte SRT; returns the input pointer.
// ECX=out, one stack argument=in, RET 4 (fastcall's dummy EDX models thiscall).
// Translation is copied from in+0x30, quaternion written at out+0x10, scale
// (sx,sy,sz,sz) at out+0x20; out+0x30 is untouched. Trace > 0 selects the
// trace branch; unordered comparisons fall through the largest-diagonal chain.
// Preserve branch-specific shuffle order, rsqrtps with two Newton refinements,
// and late input reloads after quaternion/translation writes (overlap matters).
void* __fastcall DecomposeSrt(void* out, void* edxUnused, const void* in);

// TS3W.exe 0x00678050: test a triangle against an axis-aligned box with strict face/normal rejection.
// Historical name retained: segA/segB are the box's lower/upper corners, not
// endpoints of a line segment. Five aligned 16-byte inputs; no writes. Returns
// an AL boolean (upper EAX is unspecified). Uses XYZ only, including the AABB
// reduction. EA quirks kept: signed half-differences (no abs), >= rejection for
// AABB/normal but > for edge axes, unordered projections never separate,
// max(-1*x,x) absolute values, and ((z+y)+x) horizontal sums.
unsigned char __cdecl SweptPointTriangleOverlap(const __m128* v0, const __m128* v1,
                                                const __m128* v2, const __m128* segA,
                                                const __m128* segB);

} // namespace ts3::math::m04m02
