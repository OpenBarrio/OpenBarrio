#pragma once
#include "ts3/runtime/m02_jobs.h"
namespace ts3::runtime::m02 {
// TS3W.exe 0x005993B0: Clear Service/current-job aliases, shut down the embedded scheduler, then clear its alias; preserve scheduler EAX.
uintptr_t ShutdownJobService(JobService* self);
inline uintptr_t __fastcall ShutdownJobServiceEntry(JobService* self,void*) {
    return ShutdownJobService(self);
}
}
