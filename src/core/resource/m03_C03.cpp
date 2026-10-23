#include "ts3/resource/m03_C03.h"
#include <cstring>
#include <cmath>
#include <limits>
namespace ts3::resource {
namespace {
constexpr uint32_t Magic=0x04b35bb3;
// Opaque external SDK objects: requests are 0x48 bytes and timers 0x20 bytes.
struct Request { alignas(8) unsigned char bytes[0x48]; };
struct Timer { alignas(8) unsigned char bytes[0x20]; };
}
uintptr_t ThrottledDriver::Unregister() {
    return CallGameMethod<uintptr_t,0x00401900>(this);
}
uint64_t ThrottledDriver::Seek(ThrottleHandle* h,int64_t offset,uint32_t origin,uint32_t,uint32_t) {
    if(h->magic!=Magic) return position;
    if(origin==0) position=uint64_t(offset);
    else if(origin==1) {
        uint64_t magnitude=uint64_t(0)-uint64_t(offset);
        if(offset<0 && position<magnitude) position=0;
        else position+=uint64_t(offset);
    } else if(origin==2) {
        position=CallGameVirtual<uint64_t,8>(this,h->file);
        // EA passes h->file (not h) to slot 8, and does NOT add the
        // supplied offset in the end-relative branch.
        if(offset<0 && position<uint64_t(0)-uint64_t(offset)) position=0;
    }
    return position;
}
uint64_t ThrottledDriver::Size(ThrottleHandle* h) {
    return CallGameFunction<uint64_t,0x004026F0>(h->file);
}
uint32_t ThrottledDriver::Resize(ThrottleHandle* h,uint32_t lo,uint32_t hi) {
    return CallGameFunction<uint32_t,0x00402720>(h->file,lo,hi);
}
uint32_t ThrottledDriver::Status(ThrottleHandle* h) {
    return CallGameFunction<uint32_t,0x00402700>(h->file);
}
uintptr_t ThrottledDriver::StartDelay(void* timer,uint32_t bytes) {
    float delay;
#if defined(_M_IX86)
    // Keep x87 extended intermediates and its control-word rounding, rather
    // than replacing FISTP by a truncating C++ cast or MXCSR-based conversion.
    const float factor=perByte,base=latency, unsignedBias=4294967296.0f;
    __asm {
        fild bytes
        test bytes,80000000h
        jz nonnegative
        fadd unsignedBias
    nonnegative:
        fmul factor
        fadd base
        fstp delay
    }
#else
    delay=static_cast<float>(static_cast<long double>(bytes)*perByte+latency);
#endif
    CallGameMethod<uintptr_t,0x004F30E0>(timer,4u);
    int64_t ticks;
#if defined(_M_IX86)
    __asm { fld delay }
    __asm { fistp ticks }
#else
    const double rounded=std::nearbyint(static_cast<double>(delay));
    // x87 FISTP's indefinite integer on overflow/NaN is INT64_MIN.
    ticks=std::numeric_limits<int64_t>::min();
    if(rounded>=-9223372036854775808.0 && rounded<9223372036854775808.0)
        ticks=static_cast<int64_t>(rounded);
#endif
    return CallGameMethod<uintptr_t,0x004F34F0>(timer,uint32_t(ticks),1u);
}
ThrottledDriver* ThrottledDriver::Construct(void* name,float delay,float bandwidth) {
    CallGameMethod<uintptr_t,0x00401860>(this,name);
    latency=delay;
    perByte=1000.0f/bandwidth;
#if defined(_M_IX86)
    vtable=reinterpret_cast<void**>(uintptr_t(0x00FFEB98));
#else
    AbortNotRewritten("throttled RWFileSys native vtable");
#endif
    return this;
}
ThrottleHandle* ThrottledDriver::Open(const char* path,uint32_t mode,uint32_t) {
    for(auto* node=paths;node;node=node->next) {
        const size_t n=std::strlen(node->text);
        if(_strnicmp(node->text,path,n)==0) { path+=n; break; }
    }
    Request request;
    CallGameMethod<uintptr_t,0x00401000>(&request);
    CallGameMethod<uintptr_t,0x00401500>(&request,path,mode,0u,0u,0u);
    void* file=CallGameMethod<void*,0x004012C0>(&request);
    ThrottleHandle* handle=nullptr;
    if(file) {
        handle=CallGameFunction<ThrottleHandle*,0x00401EF0>(sizeof(ThrottleHandle));
        if(handle) { handle->file=file; handle->magic=Magic; }
        else {
            CallGameMethod<uintptr_t,0x004013A0>(&request,file,0u,0u,0u);
            uint32_t timeout=~0u;
            CallGameMethod<uintptr_t,0x00401290>(&request,&timeout);
        }
    }
    CallGameMethod<uintptr_t,0x00401040>(&request);
    return handle;
}
uintptr_t ThrottledDriver::Close(ThrottleHandle* h) {
    // On invalid magic the original leaves incoming EAX unspecified (void ABI).
    if(h->magic!=Magic) return 0;
    Request request;
    CallGameMethod<uintptr_t,0x00401000>(&request);
    CallGameMethod<uintptr_t,0x004013A0>(&request,h->file,0u,0u,0u);
    uint32_t timeout=~0u;
    CallGameMethod<uintptr_t,0x00401290>(&request,&timeout);
    h->magic=0;
    CallGameFunction<uintptr_t,0x00401F10>(h);
    return CallGameMethod<uintptr_t,0x00401040>(&request);
}
ThrottledDriver* ThrottledDriver::Destroy(uint32_t flags) {
#if defined(_M_IX86)
    vtable=reinterpret_cast<void**>(uintptr_t(0x00FFEB98));
#endif
    CallGameMethod<uintptr_t,0x004018A0>(this);
    if(flags&1u) CallGameFunction<uintptr_t,0x004E3F70>(this);
    return this;
}
namespace {
template<unsigned Address>
uint32_t Transfer(ThrottledDriver* self,ThrottleHandle* h,void* buffer,uint32_t count) {
    if(h->magic!=Magic) return 0;
    Timer timer;
    CallGameMethod<uintptr_t,0x004F35B0>(&timer,0u,0u);
    CallGameMethod<uintptr_t,0x004F34F0>(&timer,0u,0u);
    self->StartDelay(&timer,count);
    Request request;
    CallGameMethod<uintptr_t,0x00401000>(&request);
    CallGameMethod<uintptr_t,Address>(&request,h->file,buffer,uint32_t(self->position),uint32_t(self->position>>32),count,0u,0u,0u,0u);
    const uint64_t completed=CallGameMethod<uint64_t,0x004012F0>(&request);
    CallGameMethod<uintptr_t,0x007397C0>(self,&timer);
    self->position+=completed;
    CallGameMethod<uintptr_t,0x00401040>(&request);
    return uint32_t(completed);
}
}
uint32_t ThrottledDriver::Read(ThrottleHandle* h,void* buffer,uint32_t count,uint32_t,uint32_t) {
    return Transfer<0x00401400>(this,h,buffer,count);
}
uint32_t ThrottledDriver::Write(ThrottleHandle* h,void* buffer,uint32_t count,uint32_t,uint32_t) {
    return Transfer<0x00401480>(this,h,buffer,count);
}
}
