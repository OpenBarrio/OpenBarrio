#include "ts3/io/m01_streams_00775ca0.h"
#include "ts3/layout.h"

namespace ts3::io::m01 {
uint32_t __cdecl RegisterStreamBindings(void* service) {
    // Named operator-new boundary, not EaNew: six cdecl arguments, no holder header.
    // The native build retains this explicit unsupported allocation boundary.
#if defined(_M_IX86)
    const char* name = reinterpret_cast<const char*>(0x00FDE6D8);
#else
    const char* name = "ScriptFunctions_36";
#endif
    auto* binding = CallGameFunction<BindingObject*, 0x004E3F90>(
        sizeof(BindingObject), name, 0u, 0u, 0u, 0u);
    if (binding) {
#if defined(_M_IX86)
        binding->vtable = reinterpret_cast<void* const*>(kBindingVtable);
#else
        // Unreachable until the named-allocation boundary is replaced on x64.
        binding->vtable = nullptr;
#endif
    }
    // No failure rollback: the service receives ownership, even if it returns 0.
    return CallGameVirtual<uint32_t, service::RegisterBindings>(service, binding);
}
}
