#include "ts3/anim/m05_M151.h"
#include <cstring>
#include <initializer_list>
namespace ts3::anim::m151 {
uintptr_t Lookup(Component* self, uint32_t key) {
    if (!self->state || !self->state->manager) return 0;
    return CallGameMethod<uintptr_t,0x00583BF0>(self->state->manager,key);
}
uint32_t Query(Component* self) {
    uint32_t result = self->flags >> 15;
    result = (result & 0xffffff00u) | (result & 1);
    if (!(result & 1) && self->state && self->state->manager)
        return CallGameMethod<uint32_t,0x00582830>(self->state->manager);
    return result;
}
uintptr_t Refresh(Component* self) {
#if defined(_M_IX86)
    void* global = *reinterpret_cast<void**>(0x011DC480);
#else
    void* global = nullptr;
#endif
    CallGameMethod<void,0x009357E0>(self->identity,global);
    struct Request { uint32_t type; void* object; uint32_t low,high; };
    Request request{0x03dfe432,nullptr,self->identity->low,self->identity->high};
    uintptr_t result = CallGameFunction<uintptr_t,0x00692F70>(&request);
    if (request.object) {
        alignas(16) unsigned char output[64];
        return CallGameVirtual<uintptr_t,0>(request.object,output);
    }
    return result;
}
Pose* InitPose(Pose* self) {
    std::memset(self,0,sizeof(*self));
    for (unsigned i : {7u,11u,15u,27u,35u,40u,41u}) self->values[i] = 1.0f;
    return self;
}
State* InitState(State* self, void* host) {
    InitPose(&self->pose);
    self->reserved = 0;
    self->weight = 1.0f;
    self->rate = 1.0f;
    void* memory = CallGameFunction<void*,0x004E3F90>(24,"Animation/TrackManager",0,0,0,0);
    self->manager = memory ? CallGameMethod<void*,0x00583E30>(memory,host) : nullptr;
    return self;
}
Renderer* InitRenderer(Renderer* self) {
#if defined(_M_IX86)
    self->vtable = reinterpret_cast<void*>(0x01043450);
#else
    AbortNotRewritten("M151 renderer vtable");
#endif
    self->service = CallGameFunction<void*,0x006FDE10>();
    return self;
}
}
