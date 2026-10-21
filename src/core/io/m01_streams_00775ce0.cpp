#include "ts3/io/m01_streams_00775ce0.h"
#include "ts3/io/m01_streams_00774dc0.h"
namespace ts3::io::m01 {
namespace {
struct Adapter { void* source; void* adapter; void* stream; uint32_t reserved; int32_t flags; bool ok, endian; };
struct Converted { char* begin; char* end; char* capacity; const char* name; };
TS3_X86_SIZE(Adapter, 0x18);
TS3_X86_SIZE(Converted, 0x10);
}
int32_t __cdecl WriteCharArrayUTF8(uint32_t handle, uint32_t managed, int32_t start, int32_t count) {
    if (!managed) return -6;
    if (start < 0 || count <= 0) return -7;
    IStream* stream=nullptr;
#if defined(_M_IX86)
    if (*reinterpret_cast<uint32_t*>(kCacheHandle)==handle)
        stream=*reinterpret_cast<IStream**>(kCacheStream);
    else
#endif
        stream=LookupStream(handle);
    if (!stream) return -2;
    void* data=nullptr;
    int32_t length;
    auto* scripting=CallGameFunction<void*,0x007692B0>();
    CallGameVirtual<void,service::CharBuffer>(scripting,managed,&data,&length);
    if (!data) return -6;
    if (start >= length || length-start < count) return -7;
    Adapter adapter;
    CallGameMethod<void*,0x007D91B0>(&adapter,stream,1);
    Converted converted;
#if defined(_M_IX86)
    const char* name=reinterpret_cast<const char*>(0x0100C750);
#else
    const char* name="StreamHost/UTF8";
#endif
    // EA deliberately passes the base pointer, not data + start.
    CallGameFunction<Converted*,0x004F38C0>(&converted,data,count,name);
    uint8_t ok=CallGameVirtual<uint8_t,14>(stream,converted.begin,
        static_cast<uint32_t>(converted.end-converted.begin));
    CallGameMethod<void,0x00405940>(&converted);
    CallGameMethod<void,0x007D7760>(&adapter);
    return ok ? 0 : -3;
}
}
