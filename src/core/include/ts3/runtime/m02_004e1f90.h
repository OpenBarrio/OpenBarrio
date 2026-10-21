#pragma once
#include "ts3/runtime/m02_layouts.h"
namespace ts3::runtime::m02 {
// TS3W.exe 0x004E1F90: enqueue work and grow/restart/wake workers; ignores flags, returns -1 if inactive or -3 after submission.
int32_t PoolSubmitWork(ThreadPool* pool, const void* work, uint32_t* output, int32_t flags);
inline int32_t __fastcall PoolSubmitWorkEntry(ThreadPool* pool, void*, const void* work, uint32_t* output, int32_t flags) {
    return PoolSubmitWork(pool, work, output, flags);
}
}
