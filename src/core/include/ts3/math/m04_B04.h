#pragma once
#include "ts3/math/m04_layouts.h"
#include <cstdint>
namespace ts3::math {
// TS3W.exe 0x00505B90: Test half-open rectangle membership.
int RectHalfOpen(const float*,float,float) noexcept;
// TS3W.exe 0x00505C80: Test half-open membership of a point pointer.
int RectPoint(const float*,const float*) noexcept;
// TS3W.exe 0x00524070: Test closed rectangle membership.
int RectClosed(const float*,float,float) noexcept;
// TS3W.exe 0x0050EEB0: Test inclusive containment of another rectangle.
int RectContains(const float*,const float*) noexcept;
// TS3W.exe 0x00505BD0: Intersect rectangles, retaining source pointer upper EAX and replacing AL.
uintptr_t RectIntersect(float*,const float*,const float*) noexcept;
// TS3W.exe 0x0051FA00: Union rectangles, preserving incoming EAX except LAHF AH on non-copy paths.
uintptr_t RectUnion(float*,const float*,const float*,uintptr_t incomingEax) noexcept;
// TS3W.exe 0x0068C050: Emit eight corners with W copied from the Z source's X lane.
Float4Value* LotCorners(const UnionBox*,Float4Value*) noexcept;
// TS3W.exe 0x0068D5D0: Transform eight corners, returning the matrix pointer.
const Float4Value* LotTransformCorners(const UnionBox*,const Float4Value*,Float4Value*) noexcept;
// TS3W.exe 0x0068D650: Project positive-W corners into bounds; EAX is dead local-array end (no hook).
void ProjectBounds(UnionBox*,const UnionBox*,const Float4Value*) noexcept;
// TS3W.exe 0x0068D7C0: Transform corners into bounds; EAX is dead local-array end (no hook).
void AffineBounds(UnionBox*,const UnionBox*,const Float4Value*) noexcept;
// TS3W.exe 0x0069D460: Compute center and radius from bounds, returning source.
const UnionBox* BoundsSphere(Float4Value*,const UnionBox*) noexcept;
// TS3W.exe 0x008C4DC0: Include a point in scalar bounds, testing only X for empty state.
const float* ScalarInclude(float*,const float*) noexcept;
// TS3W.exe 0x008C4E60: Transform scalar bounds using tagged scaled rotation and translation.
const void* ScalarTransform(float*,const void*) noexcept;
// TS3W.exe 0x008E4D10: Emit min(third,max(first,second)) by subtraction sign bits, not true bounds.
const float* TriangleWeirdBounds(float*,const float*,const float*,const float*) noexcept;
}
#if defined(_M_IX86)
namespace ts3::math {
// TS3W.exe 0x0051FA00: Exact x86 entry preserving incoming EAX before LAHF (not a C++ hook ABI).
uintptr_t __fastcall RectUnionEntry(float*,void*,const float*,const float*) noexcept;
// TS3W.exe 0x0068D650: Exact x86 entry retaining the original aligned-frame-end EAX residue.
uintptr_t __fastcall ProjectBoundsEntry(UnionBox*,void*,const UnionBox*,const Float4Value*) noexcept;
// TS3W.exe 0x0068D7C0: Exact x86 entry retaining the original aligned-frame-end EAX residue.
uintptr_t __fastcall AffineBoundsEntry(UnionBox*,void*,const UnionBox*,const Float4Value*) noexcept;
// TS3W.exe 0x00505B90: Thiscall-compatible hook entry.
int __fastcall RectHalfOpenEntry(const float*,void*,float,float) noexcept;
// TS3W.exe 0x00505C80: Thiscall-compatible hook entry.
int __fastcall RectPointEntry(const float*,void*,const float*) noexcept;
// TS3W.exe 0x00524070: Thiscall-compatible hook entry.
int __fastcall RectClosedEntry(const float*,void*,float,float) noexcept;
// TS3W.exe 0x0050EEB0: Thiscall-compatible hook entry.
int __fastcall RectContainsEntry(const float*,void*,const float*) noexcept;
// TS3W.exe 0x00505BD0: Thiscall-compatible hook entry retaining full EAX.
uintptr_t __fastcall RectIntersectEntry(float*,void*,const float*,const float*) noexcept;
// TS3W.exe 0x0068C050: Thiscall-compatible hook entry.
Float4Value* __fastcall LotCornersEntry(const UnionBox*,void*,Float4Value*) noexcept;
// TS3W.exe 0x0068D5D0: Thiscall-compatible hook entry.
const Float4Value* __fastcall LotTransformCornersEntry(const UnionBox*,void*,const Float4Value*,Float4Value*) noexcept;
// TS3W.exe 0x0069D460: Thiscall-compatible hook entry.
const UnionBox* __fastcall BoundsSphereEntry(Float4Value*,void*,const UnionBox*) noexcept;
// TS3W.exe 0x008C4DC0: Thiscall-compatible hook entry.
const float* __fastcall ScalarIncludeEntry(float*,void*,const float*) noexcept;
// TS3W.exe 0x008C4E60: Thiscall-compatible hook entry.
const void* __fastcall ScalarTransformEntry(float*,void*,const void*) noexcept;
}
#endif
