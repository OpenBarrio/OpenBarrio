#pragma once
#include "ts3/runtime/m02_00599e60.h"
namespace ts3::runtime::m02 {
// TS3W.exe 0x0059A220: wait for a job while dispatching reentrant work; pooled wait nodes and incidental EAX returns are retained.
uintptr_t __fastcall WaitForJob(Scheduler*, void*, Job*);
}
