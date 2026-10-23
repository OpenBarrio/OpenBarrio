#include "ts3/anim/m05_M60.h"
namespace ts3::anim {
void* M60::Create30(void* host) {
    void* p = CallGameFunction<void*,0x004E3F90>(0x30u,static_cast<uintptr_t>(0x00FC24E8),0u,0u,0u,0u);
    if (!p) return nullptr;
    return CallGameMethod<void*,0x0056FBA0>(p,host);
}
void* M60::Create48(void* host) {
    void* p = CallGameFunction<void*,0x004E3F90>(0x48u,static_cast<uintptr_t>(0x00FC24E8),0u,0u,0u,0u);
    if (!p) return nullptr;
    return CallGameMethod<void*,0x0056C650>(p,host);
}
void* M60::Create38A(void* host) {
    void* p = CallGameFunction<void*,0x004E3F90>(0x38u,static_cast<uintptr_t>(0x00FC24E8),0u,0u,0u,0u);
    if (!p) return nullptr;
    return CallGameMethod<void*,0x0056F790>(p,host);
}
void* M60::Create38B(void* host) {
    void* p = CallGameFunction<void*,0x004E3F90>(0x38u,static_cast<uintptr_t>(0x00FC24E8),0u,0u,0u,0u);
    if (!p) return nullptr;
    return CallGameMethod<void*,0x0056BDA0>(p,host);
}
void* M60::DeleteA(uint32_t flags) {
#if defined(_M_IX86)
    vtable = reinterpret_cast<void*>(0x00FC2330);
#else
    AbortNotRewritten("M60 registry vtable");
#endif
    CallGameFunction<void,0x0054C510>(0x13EA6F90u,this);
    if (flags & 1) CallGameFunction<void,0x004E3F70>(this);
    return this;
}
void* M60::DeleteB(uint32_t flags) {
#if defined(_M_IX86)
    vtable = reinterpret_cast<void*>(0x00FC2340);
#else
    AbortNotRewritten("M60 registry vtable");
#endif
    CallGameFunction<void,0x0054C510>(0xF40282A6u,this);
    if (flags & 1) CallGameFunction<void,0x004E3F70>(this);
    return this;
}
}
