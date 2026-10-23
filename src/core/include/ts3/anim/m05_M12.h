#pragma once
#include <xmmintrin.h>
#include <cstdint>
namespace ts3::anim::m12 {
// TS3W.exe 0x00555820: Compute the limited constraint quaternion; sign-bit clamp and SSE refinement kept.
__m128* Constraint(__m128* out, const __m128* target, const __m128* frame, const __m128* pose);
// TS3W.exe 0x00555CE0: Blend the first constraint into pose rotation; EAX retains the helper's stack pointer or the acos import's result.
uintptr_t BlendFirst(const __m128* target,const __m128* frame,__m128* pose,float weight);
// TS3W.exe 0x00555F50: Blend the second constraint into pose rotation; EAX retains the helper's stack pointer or the acos import's result.
uintptr_t BlendSecond(const __m128* target,const __m128* frame,__m128* pose,float weight);
}
