#pragma once
#include "ts3/runtime/m02_jobs.h"
namespace ts3::runtime::m02 {
// TS3W.exe 0x00599380: Unregister JobManager tuning callbacks if available, then return scheduler slot 2's full result.
uintptr_t StopService(JobService* self);
// TS3W.exe 0x00599380: ABI-compatible replace-only Service stop entry.
uintptr_t __fastcall StopServiceEntry(JobService* self,void*);
}
