#pragma once
#include "ts3/runtime/m02_jobs.h"
namespace ts3::runtime::m02 {
// TS3W.exe 0x005992D0: Start the scheduler and publish/register the Service; only AL determines startup success.
uint8_t StartJobService(JobService* self);
inline uint8_t __fastcall StartJobServiceEntry(JobService* self,void*) { return StartJobService(self); }
}
