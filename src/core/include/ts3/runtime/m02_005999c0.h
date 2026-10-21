#pragma once
#include "ts3/runtime/m02_jobs.h"
namespace ts3::runtime::m02 {
// TS3W.exe 0x005999C0: Destroy the Service and embedded scheduler, optionally free on flag bit 0, and return the original this pointer.
JobService* DestroyJobService(JobService* self, uint32_t flags);
inline JobService* __fastcall DestroyJobServiceEntry(JobService* self, void*, uint32_t flags) {
    return DestroyJobService(self, flags);
}
}
