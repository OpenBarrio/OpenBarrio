#pragma once
#include "ts3/runtime/m02_jobs.h"
namespace ts3::runtime::m02 {
// TS3W.exe 0x00599720: complete a job and unlink dependencies, retaining AL-only callback decisions and path-specific EAX.
uintptr_t CompleteJob(Scheduler* self, Job* job);
uintptr_t __fastcall CompleteJobEntry(Scheduler* self, void*, Job* job);
}
