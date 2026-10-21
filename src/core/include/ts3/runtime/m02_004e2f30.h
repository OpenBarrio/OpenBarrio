#pragma once
#include "ts3/runtime/m02_threads.h"
namespace ts3::runtime::m02 {
// TS3W.exe 0x004E2F30: Launch a suspended C callback thread, configure it and resume, or roll back both references on failure; allocation failure remains unchecked.
void* LaunchCallback(Thread* self,void* function,void* argument,const ThreadParameters* parameters,void* wrapper);
inline void* __fastcall LaunchCallbackEntry(Thread* self,void*,void* function,void* argument,const ThreadParameters* parameters,void* wrapper) {
    return LaunchCallback(self,function,argument,parameters,wrapper);
}
}
