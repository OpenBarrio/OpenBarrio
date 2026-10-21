#pragma once
#include "ts3/runtime/m02_layouts.h"
namespace ts3::runtime::m02 {
// TS3W.exe 0x004E1D80: shut down the pool, clear/free both lists and destroy synchronization; returns the final condition destructor's EAX.
uint32_t PoolDestroy(ThreadPool* pool);
inline uint32_t __fastcall PoolDestroyEntry(ThreadPool* pool, void*) { return PoolDestroy(pool); }
}
