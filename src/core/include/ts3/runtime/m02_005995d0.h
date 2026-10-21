#pragma once
#include "ts3/runtime/m02_jobs.h"
namespace ts3::runtime::m02 {
// TS3W.exe 0x005995D0: drain queue 10 except the reserved job, then poll slot 9; test only AL and preserve the final full return register.
uintptr_t DrainScheduler(Scheduler* scheduler);
inline uintptr_t __fastcall DrainSchedulerEntry(Scheduler* scheduler, void*) { return DrainScheduler(scheduler); }
}
