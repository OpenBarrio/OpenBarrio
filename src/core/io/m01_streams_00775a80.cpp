#include "ts3/io/m01_streams_00775a80.h"
namespace ts3::io::m01 {
namespace {
IStream* ResolveCopyHandle(uint32_t handle) {
#if defined(_M_IX86)
    if (handle == *reinterpret_cast<uint32_t*>(kCacheHandle))
        return *reinterpret_cast<IStream**>(kCacheStream);
#endif
    // Original lookup owns tag validation and updates the borrowed-pointer cache
    // only on success. In particular a stale cached handle bypasses validation.
    return CallGameFunction<IStream*, 0x00774DC0>(handle);
}
}
int32_t __cdecl CopyStream(uint32_t destination, uint32_t source) {
    uint8_t buffer[4096];
    auto* output = ResolveCopyHandle(destination);
    if (!output) return -2;
    auto* input = ResolveCopyHandle(source);
    if (!input) return -2;
    size_type remaining = CallGameVirtual<size_type, 7>(input);
    if (remaining == kSizeTypeError) return -3;
    if (!CallGameVirtual<uint8_t, 10>(input, off_type(0), kPositionTypeBegin)) return -3;
    while (remaining > 0) {
        const size_type chunk = remaining > sizeof(buffer) ? sizeof(buffer) : remaining;
        if (CallGameVirtual<size_type, 12>(input, buffer, chunk) != chunk) return -3;
        if (!CallGameVirtual<uint8_t, 14>(output, buffer, chunk)) return -3;
        remaining -= chunk;
    }
    return 0;
}
}
