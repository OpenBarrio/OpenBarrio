#include "ts3/runtime/m02_005993b0.h"
namespace ts3::runtime::m02 {
uintptr_t ShutdownJobService(JobService* self) {
#if defined(_M_IX86)
    auto serviceAlias=reinterpret_cast<void**>(uintptr_t(0x011CE2C8));
    auto schedulerAlias=reinterpret_cast<void**>(uintptr_t(0x011CE2CC));
    *serviceAlias=nullptr;
    self->current=nullptr;
    // Slot 3 remains an original-game boundary, not a second execution of shutdown.
    const uintptr_t result=CallGameVirtual<uintptr_t,3>(&self->scheduler);
    *schedulerAlias=nullptr;
    return result;
#else
    (void)self;
    AbortNotRewritten("JobManager Service shutdown global aliases");
    return 0;
#endif
}
}
