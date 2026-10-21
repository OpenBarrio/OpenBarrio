#include "ts3/runtime/m02_004e2210.h"
#include <windows.h>
namespace ts3::runtime::m02 {
bool PoolInitialize(ThreadPool* pool, const PoolConfiguration* config) {
    if(pool->active || !config || pool->live) return false;
    pool->active=1;
    pool->minimum=config->minimum;
    pool->maximum=config->maximum;
    InterlockedExchange(reinterpret_cast<LONG*>(&pool->live),config->initial);
    pool->idleTimeout=config->idleTimeout;
    pool->parameters=config->parameters;
    // Maximum uses an UNSIGNED comparison; both live clamps use signed comparisons.
    if(uint32_t(pool->maximum)>16u) pool->maximum=16;
    if(pool->live<pool->minimum) InterlockedExchange(reinterpret_cast<LONG*>(&pool->live),pool->minimum);
    if(pool->live>pool->maximum) InterlockedExchange(reinterpret_cast<LONG*>(&pool->live),pool->maximum);
    pool->parameters.reserved=0;
    struct SyncParameters { uint8_t enabled; char name[16]; } conditionParams, mutexParams;
    CallGameMethod<void*,0x004E1340>(&conditionParams,1,static_cast<const char*>(nullptr));
    CallGameMethod<int,0x004E1380>(&pool->condition,&conditionParams);
    CallGameMethod<void*,0x004E1340>(&mutexParams,1,static_cast<const char*>(nullptr));
    CallGameMethod<int,0x004E1690>(&pool->mutex,&mutexParams);
#if defined(_M_IX86)
    const auto infinite=reinterpret_cast<const uint32_t*>(0x00FB8960);
#else
    const uint32_t never=UINT32_MAX; const auto infinite=&never;
#endif
    CallGameMethod<int,0x004E16F0>(&pool->mutex,infinite);
    int32_t initial=InterlockedExchange(reinterpret_cast<LONG*>(&pool->live),0);
    CallGameMethod<uintptr_t,0x004E2080>(pool,initial);
    CallGameMethod<int,0x004E17B0>(&pool->mutex);
    return true;
}
}
