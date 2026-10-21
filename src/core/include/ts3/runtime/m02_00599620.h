#pragma once
#include "ts3/runtime/m02_jobs.h"
namespace ts3::runtime::m02 {
// TS3W.exe 0x00599620: Allocate/reuse a Job and return its public second-link handle; recycled fields remain untouched.
uint32_t AllocateJob(Scheduler* self, void** output);
inline uint32_t __fastcall AllocateJobEntry(Scheduler* self, void*, void** output) { return AllocateJob(self,output); }
}
