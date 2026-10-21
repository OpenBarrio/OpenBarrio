#pragma once
#include "ts3/runtime/m02_jobs.h"
namespace ts3::runtime::m02 {
// Typed view of the Job allocation: the scaffold leaves +14..+23 opaque.
// In particular +18 is an owned interface pointer, not a 32-bit integer.
struct RecycleJob {
    Link links[2]; void* callback; uint32_t reserved14; void* retained;
    uint32_t reserved1c, reserved20, state;
    uintptr_t worker; uint32_t reserved2c, refs; Scheduler* scheduler;
    Link dependencies[2]; uint32_t reserved48; uint8_t flags[3], pad4f;
};
#if defined(_M_IX86)
static_assert(sizeof(RecycleJob)==0x50);
static_assert(offsetof(RecycleJob,retained)==0x18);
static_assert(offsetof(RecycleJob,state)==0x24);
#endif
// TS3W.exe 0x0059A650: Cancel dependency state, release the retained interface outside the lock, and recycle the Job; returns the recycle sentinel.
Link* Recycle(Scheduler* scheduler, RecycleJob* job);
inline Link* __fastcall RecycleEntry(Scheduler* scheduler,void*,RecycleJob* job) {
    return Recycle(scheduler,job);
}
}
