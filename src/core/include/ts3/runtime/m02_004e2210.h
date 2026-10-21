#pragma once
#include "ts3/runtime/m02_layouts.h"
namespace ts3::runtime::m02 {
struct PoolConfiguration {
    int32_t minimum, maximum, initial;
    uint32_t idleTimeout;
    ThreadParameters parameters;
};
TS3_X86_SIZE(PoolConfiguration,0x28);
// TS3W.exe 0x004E2210: initialize an inactive empty pool, clamp its configuration and launch; callee failures are ignored.
bool PoolInitialize(ThreadPool* pool, const PoolConfiguration* config);
inline bool __fastcall PoolInitializeEntry(ThreadPool* pool, void*, const PoolConfiguration* config) {
    return PoolInitialize(pool,config);
}
}
