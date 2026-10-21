#pragma once
#include "ts3/runtime/m02_jobs.h"
namespace ts3::runtime::m02 {
// TS3W.exe 0x0059A190: Schedule a state-1 job or queue it pending dependencies; returns unlock EAX.
uintptr_t ScheduleEligibleJob(Scheduler* self, Job* job);
inline uintptr_t __fastcall ScheduleEligibleJobEntry(Scheduler* self, void*, Job* job) {
    return ScheduleEligibleJob(self,job);
}
}
