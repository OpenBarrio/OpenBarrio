#include "ts3/runtime/m02_005992d0.h"
namespace ts3::runtime::m02 {
uint8_t StartJobService(JobService* self) {
    auto* scheduler=&self->scheduler;
    const uint8_t started=CallGameVirtual<uint8_t,1>(scheduler,uint32_t(1));
    if(!started) return started;
    const uint32_t flags=self->flag ? 0x80000005u : 0x80000001u;
    self->current=CallGameVirtual<Job*,5>(scheduler,flags,uint32_t(0),uint32_t(-1));
#if defined(_M_IX86)
    *reinterpret_cast<Scheduler**>(uintptr_t(0x011CE2CC))=scheduler;
    const auto* group=reinterpret_cast<const wchar_t*>(uintptr_t(0x00FD978C));
    const auto* name=reinterpret_cast<const wchar_t*>(uintptr_t(0x00FD9774));
#else
    const wchar_t* group=nullptr; const wchar_t* name=nullptr;
    AbortNotRewritten("JobManager Service globals");
#endif
    // Deliberately query three times: the singleton may differ between calls.
    if(auto* manager=CallGameFunction<void*,0x005A00A0>())
        CallGameVirtual<uint32_t,6>(manager,group,uint32_t(10000));
    if(auto* manager=CallGameFunction<void*,0x005A00A0>())
        CallGameVirtual<uint32_t,14>(manager,&self->config,name,uint32_t(1),uint32_t(100),uint32_t(1));
    if(auto* manager=CallGameFunction<void*,0x005A00A0>())
        CallGameVirtual<uint32_t,6>(manager,static_cast<const wchar_t*>(nullptr),uint32_t(0x7FFFFFFF));
#if defined(_M_IX86)
    *reinterpret_cast<void**>(uintptr_t(0x011CE2C8))=&self->secondary;
#endif
    return 1;
}
}
