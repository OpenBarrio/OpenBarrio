#include "ts3/runtime/m02_004e1d80.h"
#include "ts3/runtime/m02_004e1c00.h"
namespace ts3::runtime::m02 {
uint32_t PoolDestroy(ThreadPool* pool) {
#if defined(_M_IX86)
    const auto infinite = reinterpret_cast<const uint32_t*>(0x00fb8960);
#else
    const uint32_t infinity = UINT32_MAX;
    const auto infinite = &infinity;
#endif
    PoolShutdown(pool, 2, infinite);
    CallGameMethod<void, 0x004e18e0>(&pool->work);
    CallGameFunction<void, 0x004e3f70>(pool->work.sentinel.next);
    CallGameFunction<void, 0x004e3f70>(pool->work.sentinel.prev);
    CallGameMethod<void, 0x004e1920>(&pool->threads);
    CallGameFunction<void, 0x004e3f70>(pool->threads.sentinel.next);
    CallGameFunction<void, 0x004e3f70>(pool->threads.sentinel.prev);
    CallGameMethod<void, 0x004e1670>(&pool->mutex);
    return CallGameMethod<uint32_t, 0x004e14a0>(&pool->condition);
}
}
