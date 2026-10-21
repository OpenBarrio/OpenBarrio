#pragma once
#include "ts3/runtime/m02_jobs.h"
namespace ts3::runtime::m02 {
// Cleanup view: unlike Job's opaque payload, +18 contains a native pointer.
struct SchedulerCleanupJob {
    Link links[2]; void* callback; uint32_t reserved14; void* resource;
    uint32_t reserved1c, reserved20, state; uintptr_t worker; uint32_t detached;
    uint8_t tail[32];
};
struct SchedulerCleanupWorker { Link link; Thread thread; };
TS3_X86_OFFSET(SchedulerCleanupJob,resource,0x18);
TS3_X86_OFFSET(SchedulerCleanupJob,detached,0x2c);
TS3_X86_SIZE(SchedulerCleanupJob,0x50);
TS3_X86_OFFSET(SchedulerCleanupWorker,thread,8);
// TS3W.exe 0x00599BF0: drain the scheduler, release reserved work, and free queued jobs, dependencies and workers in tail order.
void FreeSchedulerResources(Scheduler* scheduler);
inline void __fastcall FreeSchedulerResourcesEntry(Scheduler* scheduler, void*) { FreeSchedulerResources(scheduler); }
}
