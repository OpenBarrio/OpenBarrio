#include "ts3/runtime/m02_00599a60.h"
namespace ts3::runtime::m02 {
uintptr_t __cdecl RegisterJobService(uint32_t flag) {
#if defined(_M_IX86)
    // Keep the manager captured before lookup, even if lookup republishes the global.
    void* manager=*reinterpret_cast<void**>(uintptr_t(0x011CE3F4));
    const uintptr_t found=CallGameMethod<uintptr_t,0x0059EFE0>(manager,uint32_t(0x02A8D18B));
    if (static_cast<uint8_t>(found) != 0) return found;
    // Allocation is 0xD8, not the 0xD4 constructor field extent.
    void* storage=CallGameFunction<void*,0x004E3F90>(uint32_t(0xD8),
        reinterpret_cast<const char*>(uintptr_t(0x00FD985C)),uint32_t(0),uint32_t(0),uint32_t(0),uint32_t(0));
    void* service=nullptr;
    if (storage != nullptr) service=CallGameMethod<void*,0x00599970>(storage,flag);
    return CallGameMethod<uintptr_t,0x0059F500>(manager,service,uint32_t(1));
#else
    (void)flag;
    AbortNotRewritten("JobManager Service registration boundaries");
    return 0;
#endif
}
}
