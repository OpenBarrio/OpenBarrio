#include "ts3/anim/m05_B17.h"
#include "ts3/layout.h"
namespace ts3::anim::b17 {
float __fastcall ScaleValue(Wrapper* self, void*) {
    return self->value;
}
uintptr_t __fastcall SetScaled(Wrapper* self, void*, float value) {
    float argument = 0.0f;
    if (self->scale > 0.0f) argument = (1.0f / self->scale) * value;
    return CallGameVirtual<uintptr_t,4>(self->child, argument);
}
double __fastcall Scaled5(Wrapper* self, void*) {
    return CallGameVirtual<double,5>(self->child) * static_cast<double>(self->scale);
}
double __fastcall Scaled6(Wrapper* self, void*) {
    return CallGameVirtual<double,6>(self->child) * static_cast<double>(self->scale);
}
double __fastcall Scaled7(Wrapper* self, void*) {
    return CallGameVirtual<double,7>(self->child) * static_cast<double>(self->scale);
}
uintptr_t __fastcall Forward3(Wrapper* self, void*, void* argument) {
    return CallGameVirtual<uintptr_t,3>(self->child, argument);
}
uint32_t __fastcall TypeA(Wrapper* self, void*) {
    return 0x075e3ecbu;
}
void* __fastcall QueryA(Wrapper* self, void*, uint32_t id) {
    if (id == 0x075e3ecbu || id == 0x7489d72bu) return reinterpret_cast<unsigned char*>(self) - 2*sizeof(void*);
    return nullptr;
}
uintptr_t __fastcall UpdateScaled(Wrapper* self, void*, float value) {
    self->base.refs = static_cast<int32_t>(static_cast<uint32_t>(self->base.refs) + 1u);
    CallGameVirtual<uintptr_t,2>(self->child, self->scale * value);
    uint32_t remaining = static_cast<uint32_t>(self->base.refs) - 1u;
    self->base.refs = static_cast<int32_t>(remaining);
    if (remaining == 0) {
        self->base.refs = 1;
        return CallGameVirtual<uintptr_t,0>(self, uint32_t(1));
    }
    return remaining;
}
uintptr_t __fastcall SetDirect(Wrapper* self, void*, float value) {
    return CallGameVirtual<uintptr_t,4>(self->child, value);
}
double __fastcall Direct6(Wrapper* self, void*) {
    return CallGameVirtual<double,6>(self->child);
}
void __fastcall NotifyChild(Wrapper* self, void*) {
    if (self->child) CallGameVirtual<void,1>(self->child);
}
uint32_t __fastcall TypeB(Wrapper* self, void*) {
    return 0x075e4c89u;
}
void* __fastcall QueryB(Wrapper* self, void*, uint32_t id) {
    if (id == 0x075e4c89u || id == 0x7489d72bu) return reinterpret_cast<unsigned char*>(self) - 2*sizeof(void*);
    return nullptr;
}
uintptr_t __fastcall UpdateDirect(Wrapper* self, void*, float value) {
    self->base.refs = static_cast<int32_t>(static_cast<uint32_t>(self->base.refs) + 1u);
    CallGameVirtual<uintptr_t,2>(self->child, value);
    uint32_t remaining = static_cast<uint32_t>(self->base.refs) - 1u;
    self->base.refs = static_cast<int32_t>(remaining);
    if (remaining == 0) {
        self->base.refs = 1;
        return CallGameVirtual<uintptr_t,0>(self, uint32_t(1));
    }
    return remaining;
}
}
