#pragma once
#include "ts3/runtime/m02_jobs.h"
namespace ts3::runtime::m02 {
// TS3W.exe 0x00599A60: Allocate and register JobManager Service unless lookup AL is nonzero; register null on allocation failure and preserve full EAX.
uintptr_t __cdecl RegisterJobService(uint32_t flag);
}
