#include "ts3/runtime/m02_00774cb0.h"
namespace ts3::runtime::m02 {
uintptr_t __cdecl RegisterStopWatch(void* scripting) {
#if defined(_M_IX86)
    void* registration=CallGameFunction<void*,0x004E3F90>(uint32_t(4),
        reinterpret_cast<const char*>(uintptr_t(0x00FDE6D8)),uint32_t(0),uint32_t(0),uint32_t(0),uint32_t(0));
    if (registration != nullptr) {
        *static_cast<void**>(registration)=reinterpret_cast<void*>(uintptr_t(0x0100C4B8));
        *reinterpret_cast<void**>(uintptr_t(0x011D6834))=registration;
    }
    return CallGameVirtual<uintptr_t,18>(scripting,registration);
#else
    (void)scripting;
    AbortNotRewritten("StopWatch registration global and allocator boundary");
    return 0;
#endif
}
}
