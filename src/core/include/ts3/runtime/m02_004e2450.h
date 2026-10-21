#pragma once
#include "ts3/runtime/m02_threads.h"
namespace ts3::runtime::m02 {
// TS3W.exe 0x004E2450: acquire a reader (1) or writer (2) lock with an absolute deadline; only -2 times out and negative reader counts do not block writers.
int32_t RWAcquireTimed(RWLock* lock, int32_t mode, const uint32_t* deadline);
inline int32_t __fastcall RWAcquireTimedEntry(RWLock* lock, void*, int32_t mode, const uint32_t* deadline) {
    return RWAcquireTimed(lock,mode,deadline);
}
}
