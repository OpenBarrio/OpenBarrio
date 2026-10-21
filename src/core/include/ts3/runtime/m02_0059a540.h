#pragma once
#include "ts3/runtime/m02_jobs.h"
namespace ts3::runtime::m02 {
// TS3W.exe 0x0059A540: Cancel a job under an already-held mutex, detach dependencies, notify and release its scheduler reference; only the low notification byte is tested.
void CancelTransition(Scheduler* scheduler,Job* job,uint32_t notify);
inline void __fastcall CancelTransitionEntry(Scheduler* s,void*,Job* j,uint32_t notify) { CancelTransition(s,j,notify); }
}
