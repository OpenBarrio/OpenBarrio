#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::resource::m03::g13 {
// TS3W.exe 0x00727230: clear the owning LoadInfo completion flag and tail-dispatch the job callback.
uintptr_t __cdecl Complete(void* request);
// TS3W.exe 0x007282F0: stop the active job and drain request and pending-job queues while dropping the mutex for waits.
uintptr_t __fastcall Drain(void* self,void*);
// TS3W.exe 0x00728690: initialize AsyncLoader, preserving untouched padding and the active-job field.
void* __fastcall Construct(void* self,void*);
// TS3W.exe 0x007291E0: cancel one LoadInfo, detach clients and return the final loader-pump result.
uintptr_t __fastcall Cancel(void* self,void*,void* info);
// TS3W.exe 0x00729E10: resolve immediately or enqueue a tagged client handle; only AL replaces the unlock return.
uintptr_t __fastcall Request(void* self,void*,void* key,uint32_t type,void** handle,void* callback,uintptr_t context,uint32_t cancelMode,uint32_t flags,uint32_t priority);
}
