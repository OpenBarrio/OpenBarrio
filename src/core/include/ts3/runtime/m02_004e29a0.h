#pragma once
#include "ts3/runtime/m02_threads.h"
namespace ts3::runtime::m02 {
// TS3W.exe 0x004E29A0: Join until an absolute tick deadline; late records suppress output and failed waits still mark exit.
uint32_t TimedJoin(Thread* self,const uint32_t* deadline,uint32_t* result);
inline uint32_t __fastcall TimedJoinEntry(Thread* self,void*,const uint32_t* deadline,uint32_t* result) {
    return TimedJoin(self,deadline,result);
}
}
