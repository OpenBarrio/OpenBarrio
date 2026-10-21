#pragma once
#include "ts3/runtime/m02_jobs.h"
namespace ts3::runtime::m02 {
struct DispatchWorker {
    uintptr_t reserved[2]; Link waiting; Semaphore semaphore;
    uint32_t reserved20[3]; int32_t priority; Link ready; Job* selected;
    uint32_t reserved3c; uint8_t stopped, padding[3];
};
TS3_X86_OFFSET(DispatchWorker,priority,0x2c);
TS3_X86_OFFSET(DispatchWorker,ready,0x30);
TS3_X86_OFFSET(DispatchWorker,selected,0x38);
TS3_X86_OFFSET(DispatchWorker,stopped,0x40);
// TS3W.exe 0x00599E60: dispatch eligible worker/global jobs, optionally waiting or checking a deadline; returns AL only, with signed priority and tail-based emptiness tests retained.
uint8_t DispatchEligible(Scheduler*, DispatchWorker*, DeadlineTimer*, uint32_t noWait, uint32_t single, const PlatformDispatch* clock=nullptr);
uint8_t __fastcall DispatchEligibleEntry(Scheduler*, void*, DispatchWorker*, DeadlineTimer*, uint32_t noWait, uint32_t single);
}
