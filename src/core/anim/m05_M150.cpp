#include "ts3/anim/m05_M150.h"
#include <cstring>
namespace ts3::anim::m150impl {
Progression* Construct(Progression* self) {
#if defined(_M_IX86)
    self->vtable=reinterpret_cast<void*>(uintptr_t(0x01027848));
#else
    self->vtable=nullptr; // Original game vtable is only available in the x86 bridge.
#endif
    self->lower[0]=-1.0f;
    self->lower[1]=-1.0f;
    const uint32_t maximum=0x7f7fffff;
    std::memcpy(&self->upper[0],&maximum,4);
    std::memcpy(&self->upper[1],&maximum,4);
    self->mode=2;
    self->flags[0]=0;
    self->flags[1]=0;
    return self;
}
uintptr_t Find(Component* self,uint32_t key) {
    if(!self->host || !self->host->scene) return 0;
    return CallGameMethod<uintptr_t,0x005828b0>(self->host->scene,key);
}
uintptr_t FindNested(Component* self,uint32_t track,uint32_t key) {
    if(!self->host || !self->host->scene) return 0;
    auto* controller=CallGameMethod<void*,0x00583c10>(self->host->scene,track);
    if(!controller) return 0;
    return CallGameMethod<uintptr_t,0x00566240>(controller,key);
}
uintptr_t Query(Component* self,uint32_t track) {
    if(!self->host || !self->host->scene) return 1;
    return CallGameMethod<uintptr_t,0x00583c20>(self->host->scene,track);
}
uintptr_t Command(Component* self,uint32_t track,uint32_t value) {
    if(!self->host || !self->host->scene) return 0;
    return CallGameMethod<uintptr_t,0x00583c40>(self->host->scene,track,value);
}
double Value(Component* self,uint32_t track) {
    if(!self->host || !self->host->scene) return 0.0;
    auto* controller=CallGameMethod<void*,0x00583c10>(self->host->scene,track);
    if(!controller) return 0.0;
    return CallGameVirtual<double,7>(controller);
}
}
