#include "ts3/runtime/m02_004e2450.h"
namespace ts3::runtime::m02 {
int32_t RWAcquireTimed(RWLock* lock, int32_t mode, const uint32_t* deadline) {
#if defined(_M_IX86)
    const auto infinite=reinterpret_cast<const uint32_t*>(0x00FB89B4);
#else
    const uint32_t never=0xffffffffu;
    const auto infinite=&never;
#endif
    // The mutex acquisition return is ignored, and uses infinity rather than the caller's deadline.
    CallGameMethod<int32_t,0x004E16F0>(&lock->mutex,infinite);
    int32_t result=0;
    if(mode==1) {
        while(lock->writer) {
            ++lock->counters[0];
            auto waited=CallGameMethod<int32_t,0x004E14C0>(&lock->readers,&lock->mutex,deadline);
            --lock->counters[0];
            if(waited==-2) {
                CallGameMethod<int32_t,0x004E17B0>(&lock->mutex);
                return -2;
            }
        }
        ++lock->counters[2];
        result=static_cast<int32_t>(lock->counters[2]);
    } else if(mode==2) {
        while(static_cast<int32_t>(lock->counters[2])>0 || lock->writer) {
            ++lock->counters[1];
            auto waited=CallGameMethod<int32_t,0x004E14C0>(&lock->writers,&lock->mutex,deadline);
            --lock->counters[1];
            if(waited==-2) {
                CallGameMethod<int32_t,0x004E17B0>(&lock->mutex);
                return -2;
            }
        }
        lock->writer=CallGameFunction<uintptr_t,0x004E1270>();
        result=1;
    }
    CallGameMethod<int32_t,0x004E17B0>(&lock->mutex);
    return result;
}
}
