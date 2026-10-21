#include "ts3/io/m01_streams_00774dc0.h"

namespace ts3::io::m01 {
IStream* __cdecl LookupStream(uint32_t handle) {
    if ((handle & 0xff000000u) != 0x60000000u) return nullptr;
#if defined(_M_IX86)
    void* provider = CallGameFunction<void*, 0x007692B0>();
    auto* stream = CallGameVirtual<IStream*, service::Lookup>(provider, handle);
    // This is the miss helper, not the caller's cache-hit fast path. Even a
    // matching cached handle must reach Lookup; a failed lookup changes nothing.
    if (stream) {
        *reinterpret_cast<IStream**>(kCacheStream) = stream;
        *reinterpret_cast<uint32_t*>(kCacheHandle) = handle;
    }
    return stream;
#else
    NotRewritten("M01 native Scripting handle lookup boundary");
#endif
}
}
