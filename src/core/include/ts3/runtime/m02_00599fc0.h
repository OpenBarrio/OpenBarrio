#pragma once
#include "ts3/runtime/m02_jobs.h"
namespace ts3::runtime::m02 {
// TS3W.exe 0x00599FC0: route a job to completion or a ready queue, retaining masked state bits and path-specific EAX.
uintptr_t RouteJob(Scheduler* self, Job* job);
uintptr_t __fastcall RouteJobEntry(Scheduler* self, void*, Job* job);
}
