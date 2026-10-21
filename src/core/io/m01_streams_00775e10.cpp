#include "ts3/io/m01_streams_00775e10.h"
namespace ts3::io::m01 {
int32_t __cdecl WriteStringUTF8(uint32_t handle, uint32_t managed) {
    if (!managed) return -6;
    IStream* stream=nullptr;
#if defined(_M_IX86)
    if (handle == *reinterpret_cast<uint32_t*>(kCacheHandle))
        stream=*reinterpret_cast<IStream**>(kCacheStream);
    else
#endif
        stream=CallGameFunction<IStream*,0x00774DC0>(handle);
    if (!stream) return -2;
    auto* service=CallGameFunction<void*,0x007692B0>();
    const int32_t length=CallGameVirtual<int32_t,23>(service,managed);
    const wchar_t* text=CallGameVirtual<const wchar_t*,22>(service,managed);
    // EA's conversion boundary constructs a three-pointer byte vector. Its
    // allocator and conversion/error policy remain owned by UTFFoundation.
    struct Bytes { uint8_t* begin; uint8_t* end; uint8_t* capacity; } bytes{};
    CallGameFunction<void*,0x004F38C0>(&bytes,text,length,0);
    const uint32_t count=static_cast<uint32_t>(bytes.end-bytes.begin);
    uint32_t remaining=count;
    uint8_t prefix[5]; unsigned used=0;
    while (remaining >= 128) {
        prefix[used++]=static_cast<uint8_t>((remaining & 0x7f) | 0x80);
        remaining >>= 7;
    }
    prefix[used++]=static_cast<uint8_t>(remaining);
    int32_t result=-3;
    if (CallGameVirtual<uint8_t,14>(stream,static_cast<const void*>(prefix),size_type(used)) &&
        CallGameVirtual<uint8_t,14>(stream,static_cast<const void*>(bytes.begin),size_type(count)))
        result=0;
    // Signed capacity span, not size: even an empty conversion can own storage.
    if (static_cast<intptr_t>(reinterpret_cast<uintptr_t>(bytes.capacity)-
            reinterpret_cast<uintptr_t>(bytes.begin)) > 1 && bytes.begin)
        CallGameFunction<void,0x004E3F70>(bytes.begin);
    return result;
}
}
