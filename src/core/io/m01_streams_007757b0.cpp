#include "ts3/io/m01_streams_007757b0.h"
#include "ts3/io/m01_stream_leaves.h"

namespace ts3::io::m01 {
uint32_t __cdecl Create()
{
    // Ordinary engine new/delete, NOT the allocator-header EaNew family.
#if defined(_M_IX86)
    const char* name = reinterpret_cast<const char*>(0x0100C724);
#else
    const char* name = "StreamHost";
#endif
    auto* storage = CallGameFunction<MemoryStream*, 0x004E3F90>(
        sizeof(MemoryStreamLayout), name, 0u, 0u, 0u, 0u);
    if (!storage)
        return 0;
    auto* stream = ConstructShared(storage, nullptr, nullptr, 0, nullptr);
    if (!stream)
        return 0;
    CallGameVirtual<int32_t, 1>(stream);
    CallGameMethod<bool, 0x004DEAC0>(stream, 1, 1.0f);
    auto* scripting = CallGameFunction<void*, 0x007692B0>();
    uint32_t handle = CallGameVirtual<uint32_t, 50>(scripting, stream, kHandleTag);
    if (handle)
        CallGameVirtual<int32_t, 1>(stream);
    CallGameVirtual<int32_t, 2>(stream);
    return handle;
}
}
