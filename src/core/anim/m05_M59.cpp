#include "ts3/anim/m05_M59.h"
namespace ts3::anim::m59 {
namespace {
void Tables(Controller* self) {
#if defined(_M_IX86)
    self->vtable = reinterpret_cast<void*>(0x00FC2548);
    self->secondary8 = reinterpret_cast<void*>(0x00FC253C);
    self->secondaryC = reinterpret_cast<void*>(0x00FC2538);
#else
    (void)self;
    AbortNotRewritten("m59 original controller vtables");
#endif
}
void* Allocate(uint32_t size) {
#if defined(_M_IX86)
    return CallGameFunction<void*,0x004E3F90>(size,reinterpret_cast<void*>(0x00FC24E8),0,0,0,0);
#else
    (void)size; AbortNotRewritten("m59 game allocator"); return nullptr;
#endif
}
}
void* Query(void* self, uint32_t id) {
    return id == 0x074A5F9D ? static_cast<unsigned char*>(self) - 2*sizeof(void*) : nullptr;
}
Controller* Destroy(Controller* self, uint32_t flags) {
    Tables(self);
    CallGameMethod<void,0x00565B40>(self);
    if (flags & 1) CallGameFunction<void,0x004E3F70>(self);
    return self;
}
Controller* CreateBase(void* host) {
    auto* self = static_cast<Controller*>(Allocate(0x28));
    if (!self) return nullptr;
    CallGameMethod<Controller*,0x00565A80>(self,host);
    Tables(self);
    return self;
}
void* Create48(void* host) {
    void* self = Allocate(0x48);
    if (!self) return nullptr;
    return CallGameMethod<void*,0x0056B950>(self,host);
}
void* Create3C(void* host) {
    void* self = Allocate(0x3C);
    if (!self) return nullptr;
    return CallGameMethod<void*,0x0056AA70>(self,host);
}
void* Create5C(void* host) {
    void* self = Allocate(0x5C);
    if (!self) return nullptr;
    return CallGameMethod<void*,0x0056ABB0>(self,host);
}
#if defined(_M_IX86)
void* __fastcall HookQuery(void* self, void*, uint32_t id) { return Query(self, id); }
Controller* __fastcall HookDestroy(Controller* self, void*, uint32_t flags) { return Destroy(self, flags); }
Controller* __fastcall HookCreateBase(void*, void*, void* host) { return CreateBase(host); }
void* __fastcall HookCreate48(void*, void*, void* host) { return Create48(host); }
void* __fastcall HookCreate3C(void*, void*, void* host) { return Create3C(host); }
void* __fastcall HookCreate5C(void*, void*, void* host) { return Create5C(host); }
#endif
}
