#pragma once
#include <cstdint>
#include <xmmintrin.h>
namespace ts3::anim {
// TS3W.exe 0x0054F680: Destroy the evaluator, optionally free it, and return its former address.
void* DeleteConstraintEvaluator(void* self, uint32_t flags);
// TS3W.exe 0x0054F830: Emit six debug segments when enabled; preserve lazy state and the final renderer EAX.
uintptr_t EmitConstraintDebug(const void* self, const __m128* matrix);
}
#if defined(_M_IX86)
namespace ts3::anim::m08_hooks {
void* __fastcall Delete(void*, void*, uint32_t);
uintptr_t __fastcall Emit(const void*, void*, const __m128*);
}
#endif
