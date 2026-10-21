#include "ts3/runtime/m02_00599380.h"
#include "ts3/routing/clu_5a00a0_Routing.h"
namespace ts3::runtime::m02 {
uintptr_t StopService(JobService* self) {
    auto tuning=ts3::routing::GetTuningObject();
    if(tuning) {
#if defined(_M_IX86)
        const auto name=reinterpret_cast<const wchar_t*>(uintptr_t(0x00FD9774));
#else
        const auto name=L"JobManager";
#endif
        // Removal's result is deliberately ignored; shutdown always follows.
        CallGameVirtual<uintptr_t,18>(tuning,name);
    }
    return CallGameVirtual<uintptr_t,2>(&self->scheduler);
}
uintptr_t __fastcall StopServiceEntry(JobService* self,void*) { return StopService(self); }
}
