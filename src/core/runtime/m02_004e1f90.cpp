#include "ts3/runtime/m02_004e1f90.h"
namespace ts3::runtime::m02 {
int32_t PoolSubmitWork(ThreadPool* pool, const void* work, uint32_t* output, int32_t flags) {
    (void)flags;
    if (pool->active == 0) return -1;
#if defined(_M_IX86)
    const auto infinite = reinterpret_cast<const uint32_t*>(0x00fb8960);
#else
    const uint32_t infinity = UINT32_MAX;
    const auto infinite = &infinity;
#endif
    CallGameMethod<int32_t, 0x004e16f0>(&pool->mutex, infinite);
    if (pool->busy >= pool->live && pool->live < pool->maximum) {
        const int32_t target = static_cast<int32_t>(static_cast<uint32_t>(pool->live) + 1u);
        CallGameMethod<void, 0x004e2080>(pool, target);
    }
    CallGameMethod<void, 0x004e1b80>(&pool->work, work);
    CallGameMethod<void, 0x004e1f30>(pool);
    if (pool->waiters == 0) {
        CallGameMethod<void, 0x004e1590>(&pool->condition, uint32_t(0));
    }
    CallGameMethod<int32_t, 0x004e17b0>(&pool->mutex);
    if (output) *output = 0;
    return -3;
}
}
