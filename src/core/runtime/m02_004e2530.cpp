#include "ts3/runtime/m02_004e2530.h"
namespace ts3::runtime::m02 {
int32_t RWRelease(RWLock* lock) {
#if defined(_M_IX86)
    const auto infinite = reinterpret_cast<const uint32_t*>(0x00FB89B4);
#else
    const uint32_t never = UINT32_MAX;
    const auto infinite = &never;
#endif
    CallGameMethod<int32_t,0x004E16F0>(&lock->mutex,infinite);
    if(lock->writer) {
        lock->writer = 0;
    } else {
        // x86 ADD wraps, including an unmatched release at zero or INT_MIN.
        --lock->counters[2];
        const int32_t remaining = static_cast<int32_t>(lock->counters[2]);
        if(remaining > 0) {
            CallGameMethod<int32_t,0x004E17B0>(&lock->mutex);
            return remaining;
        }
    }
    if(static_cast<int32_t>(lock->counters[1]) > 0)
        CallGameMethod<int32_t,0x004E1590>(&lock->writers,0);
    else if(static_cast<int32_t>(lock->counters[0]) > 0)
        CallGameMethod<int32_t,0x004E1590>(&lock->readers,1);
    CallGameMethod<int32_t,0x004E17B0>(&lock->mutex);
    return 0;
}
}
