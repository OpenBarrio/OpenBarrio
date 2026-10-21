#include "ts3/io/m01_streams_00776180.h"
#include "ts3/io/m01_streams_00774dc0.h"
namespace ts3::io::m01 {
namespace {
#if defined(_M_IX86)
// EASTL fixed-vector views used only by the original resize/allocation helpers.
// Both capacities are 256 elements; the UTF16 vector therefore has 512 inline bytes.
template<class T> struct FixedBuffer {
    T *begin, *end, *capacity;
    const char* name;
    T* inlineBegin;
    T storage[256];
    explicit FixedBuffer(uintptr_t allocationName)
        : begin(storage), end(storage), capacity(storage+256),
          name(reinterpret_cast<const char*>(allocationName)), inlineBegin(storage) {}
    ~FixedBuffer() {
        if (begin && begin!=inlineBegin) CallGameFunction<void,0x004E3F70>(begin);
    }
};
#endif
}
int32_t __cdecl ReadStringUTF8(uint32_t handle, uint32_t* result) {
#if defined(_M_IX86)
    auto* stream = handle == *reinterpret_cast<uint32_t*>(kCacheHandle)
        ? *reinterpret_cast<IStream**>(kCacheStream) : LookupStream(handle);
    if (!stream) return -2;
    *result=0;
    uint32_t length=0, shift=0;
    uint8_t byte;
    do {
        // This helper tests a one-byte read. EA treats prefix failure as success.
        if (!CallGameFunction<uint8_t,0x004DD4F0>(stream,&byte)) return 0;
        length += uint32_t(byte & 0x7f) << (shift & 31);
        shift += 7;
    } while (byte & 0x80);
    if (length > 65536) return -4;
    FixedBuffer<uint8_t> bytes(0x0100C788);
    CallGameMethod<void,0x007760D0>(&bytes,length);
    FixedBuffer<uint16_t> wide(0x0100C770);
    CallGameMethod<void,0x00776120>(&wide,length);
    uint32_t actual=CallGameVirtual<uint32_t,12>(stream,bytes.begin,length);
    if (actual != length) return actual==0xffffffffu ? -3 : -1;
    int32_t characters=static_cast<int32_t>(length);
    int32_t consumed=CallGameFunction<int32_t,0x004F3CC0>(bytes.begin,length,8,wide.begin,&characters,16);
    if (static_cast<uint32_t>(consumed)!=length) return -4;
    auto* service=CallGameFunction<void*,0x007692B0>();
    *result=CallGameVirtual<uint32_t,service::NewString>(service,wide.begin,characters);
    return *result ? 0 : -5;
#else
    (void)handle; (void)result;
    NotRewritten("M01 native Scripting and encoding/fixed-vector boundaries");
#endif
}
}
