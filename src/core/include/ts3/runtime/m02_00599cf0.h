#pragma once
#include "ts3/runtime/m02_jobs.h"
namespace ts3::runtime::m02 {
struct SchedulerWorker {
    Link membership, pending;
    Semaphore semaphore;
    Thread thread;
    uintptr_t identity;
    Scheduler* scheduler;
    uint32_t config;
    Link lists[2];
    uint8_t stopped, padding[3];
};
TS3_X86_SIZE(SchedulerWorker,0x44);
TS3_X86_OFFSET(SchedulerWorker,thread,0x20);
TS3_X86_OFFSET(SchedulerWorker,identity,0x24);
TS3_X86_OFFSET(SchedulerWorker,scheduler,0x28);
// TS3W.exe 0x00599CF0: Allocate and link a worker, optionally launch/configure its thread while holding the scheduler lock.
SchedulerWorker* CreateWorker(Scheduler*,uint32_t config,bool launch,uint32_t index);
inline SchedulerWorker* __fastcall CreateWorkerEntry(Scheduler* s,void*,uint32_t c,bool l,uint32_t i) { return CreateWorker(s,c,l,i); }
}
