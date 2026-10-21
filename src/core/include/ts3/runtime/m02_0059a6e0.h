#pragma once
#include "ts3/runtime/m02_jobs.h"
namespace ts3::runtime::m02 {
// TS3W.exe 0x0059A6E0: Under the scheduler mutex, register the dependency edge
// "waiter waits on target" when the target job state is 1/2/4/5 (the jump table
// routes state 3 and everything outside 1..5 straight to the unlock). A cancelled
// waiter (state 7) cancels the target through 0x0059A540; a completed waiter
// (state 6) only skips the edge; a failed link allocation also cancels. Target
// state 4 is promoted to 5 after the edge work. Returns the mutex Unlock() EAX.
// EA quirk kept: the propagate trigger compares the LowerRaw insertion point with
// the waiter list head AFTER the insert, which never holds for well-formed
// circular sentinel lists, so the 0x0059A470 propagate branch stays unreachable.
uintptr_t Depend(Scheduler* scheduler,Job* target,Job* waiter);
// ABI entry for the thiscall (ret 8) scheduler dependency operation.
inline uintptr_t __fastcall DependEntry(Scheduler* scheduler,void*,Job* target,Job* waiter) {
    return Depend(scheduler,target,waiter);
}
}
