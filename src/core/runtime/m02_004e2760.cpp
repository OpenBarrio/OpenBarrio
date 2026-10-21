#include "ts3/runtime/m02_004e2760.h"
#define NOMINMAX
#include <windows.h>
namespace ts3::runtime::m02 {
int32_t SemaphoreWaitTimed(t2::Semaphore* self,const uint32_t* deadline,const PlatformDispatch* platform) {
    uint32_t timeout=*deadline;
    if(timeout!=0xffffffffu && timeout!=0) {
        const uint32_t now=platform ? platform->tick(platform->context) : GetTickCount();
        // Unsigned absolute comparison, NOT wrap-aware elapsed-time arithmetic.
        timeout=*deadline>now ? *deadline-now : 0;
    }
    auto wait=[&]() -> uint32_t {
        if(platform) return platform->wait(platform->context,self->handle,timeout,false);
        return WaitForSingleObject(self->handle,timeout);
    };
    auto count=reinterpret_cast<volatile LONG*>(&self->count);
    if(self->local) {
        if(InterlockedDecrement(count)<0 && wait()!=0) {
            CallGameMethod<void,0x004E2640>(self,1);
            return -2;
        }
        const int32_t remaining=self->count;
        return remaining>0 ? remaining : 0;
    }
    const uint32_t result=wait();
    if(result==0) return InterlockedDecrement(count);
    return result==WAIT_TIMEOUT ? -2 : -1;
}
}
