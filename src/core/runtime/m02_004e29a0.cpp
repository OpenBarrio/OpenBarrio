#include "ts3/runtime/m02_004e29a0.h"
namespace ts3::runtime::m02 {
namespace {
template<class R,unsigned A,class... Args> R Import(Args... args) {
#if defined(_M_IX86)
    using Fn=R(__stdcall*)(Args...);
    return (*reinterpret_cast<Fn*>(uintptr_t(A)))(args...);
#else
    ((void)args,...); AbortNotRewritten("m02 timed join import"); return R{};
#endif
}
uint32_t Tick() { return CallGameFunction<uint32_t,0x004E1330>(); }
}
uint32_t TimedJoin(Thread* self,const uint32_t* deadline,uint32_t* result) {
    if(!self->record) {
        while(!self->record || !self->record->handle) {
            if(Tick() >= *deadline) break;
            const uint32_t one=1;
            CallGameFunction<uint32_t,0x004E1320>(&one);
        }
        if(!self->record) return 0;
        // EA discards the caller's output pointer when the record arrived late.
        result=nullptr;
    }
    if(self->record->handle) {
        uint32_t timeout=*deadline;
        if(timeout!=0 && timeout!=UINT32_MAX) {
            const uint32_t now=Tick();
            timeout=*deadline>now ? *deadline-now : 0;
        }
        const uint32_t wait=Import<uint32_t,0x00F951A4>(self->record->handle,timeout);
        if(wait==0x102) return 1;
        Import<int,0x00F95398>(self->record->handle);
        self->record->handle=nullptr;
        self->record->state=2;
    }
    if(result) *result=self->record->result;
    return 2;
}
}
