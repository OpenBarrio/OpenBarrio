#include "ts3/io/m01_streams_00775830.h"
#include "ts3/io/m01_stream_leaves.h"
#include "ts3/casutils/clu_5a4cb0_CASUtils.h"
#include <cstring>

namespace ts3::io::m01 {
uint32_t __cdecl CreateFromBuffer(uint32_t managed) {
    if (!managed) return 0;
    void* data;
    int32_t length;
    auto* scripting = CallGameFunction<void*, 0x007692B0>();
    CallGameVirtual<void, service::Buffer>(scripting, managed, &data, &length);
#if defined(_M_IX86)
    const char* name = reinterpret_cast<const char*>(0x0100C724);
#else
    const char* name = "StreamHost/MemoryStream";
#endif
    auto* storage = CallGameFunction<MemoryStream*, 0x004E3F90>(
        sizeof(MemoryStreamLayout), name, 0u, 0u, 0u, 0u);
    if (!storage) return 0;
    auto* stream = ConstructShared(storage, nullptr, nullptr, 0u, nullptr);
    if (!stream) return 0;
    CallGameVirtual<int32_t, 1>(stream);
    CallGameMethod<void, 0x004DEAC0>(stream, 1, 1.0f);
    // EA tests AL, not the complete EAX returned by SetSize.
    if (!CallGameVirtual<uint8_t, 8>(stream, static_cast<uint32_t>(length))) {
        CallGameVirtual<int32_t, 2>(stream);
        return 0;
    }
    std::memcpy(stream->Data(), data, static_cast<size_t>(length));
    CallGameMethod<void, 0x004DEAC0>(stream, 1, 0.0f);
    auto handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(
        casutils::CreateAndNotify774E00(stream)));
    // RegisterHandle adds its own reference only on success. Drop our reference
    // even on registration failure, after making the buffer non-resizable.
    CallGameVirtual<int32_t, 2>(stream);
    return handle;
}
}
