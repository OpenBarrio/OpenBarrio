#include "ts3/io/m01_streams_00776420.h"
#include "ts3/io/m01_streams_00774dc0.h"
namespace ts3::io::m01 {
namespace {
struct Bytes {
    char* begin; char* end; char* capacity; const char* name; char* fixed;
    char storage[256];
};
TS3_X86_SIZE(Bytes,0x114);
}
int32_t __cdecl ReadCharArrayUTF8(uint32_t handle,uint32_t managed,int32_t start,int32_t count) {
    IStream* stream=nullptr;
#if defined(_M_IX86)
    if (*reinterpret_cast<uint32_t*>(kCacheHandle)==handle)
        stream=*reinterpret_cast<IStream**>(kCacheStream);
    else
#endif
        stream=LookupStream(handle);
    if (!stream) return -2;
    if (!managed) return -6;
    void* data=nullptr;
    int32_t length=0;
    auto* scripting=CallGameFunction<void*,0x007692B0>();
    CallGameVirtual<void,service::Buffer>(scripting,managed,&data,&length);
    if (start<0 || count<0 || start>length || length-start<count) return -7;
    if (!count) return 0;
    Bytes bytes;
    CallGameMethod<void*,0x0072F220>(&bytes);
#if defined(_M_IX86)
    bytes.name=reinterpret_cast<const char*>(0x0100C7A0);
#else
    bytes.name="StreamHost/ReadCharArrayUTF8";
#endif
    auto cleanup=[&]() {
        if (bytes.begin && bytes.begin!=bytes.fixed)
            CallGameFunction<void,0x004E3F70>(bytes.begin);
    };
    int32_t total=0, remaining=count;
    for (;;) {
        CallGameMethod<void,0x007760D0>(&bytes,static_cast<uint32_t>(total+remaining));
        char* chunk=bytes.begin+total;
        int32_t got=static_cast<int32_t>(CallGameVirtual<size_type,12>(stream,chunk,static_cast<size_type>(remaining)));
        if (got==-1) { cleanup(); return -3; }
        total+=got;
        if (got<remaining) break;
        int32_t extra=0;
        for (int32_t i=0;i<remaining;++i)
            extra+=CallGameFunction<int32_t,0x004F3CA0>(chunk+i);
        remaining=extra;
        if (!remaining) break;
    }
    int32_t output=count;
    // The original passes data, not data + start, and uses count as UTF16 capacity.
    uint32_t consumed=CallGameFunction<uint32_t,0x004F3CC0>(bytes.begin,total,8,data,&output,16);
    cleanup();
    return consumed<static_cast<uint32_t>(total) ? -4 : output;
}
}
