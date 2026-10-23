#pragma once
#include "ts3/anim/m05_M07.h"
namespace ts3::anim::m66 {
// TS3W.exe 0x00571780: Unregister factory 075E4C89 and optionally free; returns this after free.
void* DestroyFirst(void*,uint32_t);
// TS3W.exe 0x005717B0: Unregister factory 0664FB68 and optionally free; only flag bit zero matters.
void* DestroySecond(void*,uint32_t);
// TS3W.exe 0x005717E0: Unregister factory 075E3ECB and optionally free; returns this after free.
void* DestroyThird(void*,uint32_t);
// TS3W.exe 0x00571810: Unregister factory 0681C688 and optionally free; only flag bit zero matters.
void* DestroyFourth(void*,uint32_t);
// TS3W.exe 0x00571890: Adjust secondary interface by three pointer slots and forward deletion flags.
void* DestroySecondary(void*,uint32_t);
// TS3W.exe 0x005718A0: Reset evaluator then optionally free; returns this even after free.
m07::Evaluator* DestroyEvaluator(m07::Evaluator*,uint32_t);
}
#if defined(_M_IX86)
namespace ts3::anim::m66::hooks {
void* __fastcall DestroyFirst(void*,void*,uint32_t);
void* __fastcall DestroySecond(void*,void*,uint32_t);
void* __fastcall DestroyThird(void*,void*,uint32_t);
void* __fastcall DestroyFourth(void*,void*,uint32_t);
void* __fastcall DestroySecondary(void*,void*,uint32_t);
m07::Evaluator* __fastcall DestroyEvaluator(m07::Evaluator*,void*,uint32_t);
}
#endif
