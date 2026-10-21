#pragma once
#include "ts3/runtime/m02_layouts.h"
namespace ts3::runtime::m02 {
// TS3W.exe 0x004E1C00: deactivate, wait, stop and drain the pool; drain ignores timeout and only AL of the wait result is replaced.
uint32_t PoolShutdown(ThreadPool* pool, uint32_t predicate, const uint32_t* deadline);
inline uint32_t __fastcall PoolShutdownEntry(ThreadPool* pool, void*, uint32_t predicate, const uint32_t* deadline) {
    return PoolShutdown(pool,predicate,deadline);
}
}
