#pragma once
#include "ts3/runtime/m02_T2.h"
namespace ts3::runtime::m02 {
// TS3W.exe 0x004E2760: wait against an absolute deadline and reconcile semaphore counts; local wait errors all return -2, shared errors return -1.
int32_t SemaphoreWaitTimed(t2::Semaphore*,const uint32_t* deadline,const PlatformDispatch* platform=nullptr);
inline int32_t __fastcall SemaphoreWaitTimedEntry(t2::Semaphore* self,void*,const uint32_t* deadline) {
    return SemaphoreWaitTimed(self,deadline);
}
}
