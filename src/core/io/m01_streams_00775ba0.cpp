#include "ts3/io/m01_streams_00775ba0.h"
#include "ts3/io/m01_streams_00774dc0.h"
namespace ts3::io::m01 {
uint32_t __cdecl ToArray(uint32_t handle) {
#if defined(_M_IX86)
    auto* stream = handle == *reinterpret_cast<uint32_t*>(kCacheHandle)
        ? *reinterpret_cast<IStream**>(kCacheStream) : LookupStream(handle);
    if (!stream) return 0;
    auto* scripting = CallGameFunction<void*, 0x007692B0>();
    const int32_t length = static_cast<int32_t>(CallGameVirtual<uint32_t, 7>(stream));
    if (length < 0) return 0;
    if (!CallGameVirtual<uint8_t, 10>(stream, 0, 0)) return 0;
    uint32_t array = 0;
    CallGameVirtual<void, service::NewArray>(scripting, &array, length);
    if (!array) return 0;
    void* data;
    int32_t capacity;
    CallGameVirtual<void, service::Buffer>(scripting, array, &data, &capacity);
    if (data && capacity >= length &&
        CallGameVirtual<uint32_t, 12>(stream, data, static_cast<uint32_t>(length)) == static_cast<uint32_t>(length))
        return array;
    CallGameVirtual<void, service::ReleaseManaged>(scripting, array);
    return 0;
#else
    (void)handle;
    NotRewritten("M01 native Scripting managed allocation boundary");
#endif
}
}
