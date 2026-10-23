#include "ts3/anim/m05_M124.h"
namespace ts3::anim::m124 {
namespace {
void* Table() {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(uintptr_t(0x01026E04));
#else
    AbortNotRewritten("M124 style vtable"); return nullptr;
#endif
}
}
uint32_t Register(void* self) {
    void* embedded=self?static_cast<unsigned char*>(self)+0x10:nullptr;
    void* world=CallGameFunction<void*,0x0044C470>();
    if(world) CallGameVirtual<void,9>(world,embedded,0x038E4E29u,0,0);
    void* route=CallGameFunction<void*,0x007692C0>();
    if(route) CallGameVirtual<void,9>(route,embedded,0x038E3C6Cu,0,0);
    return CallGameFunction<uint32_t,0x00846670>();
}
Style* Construct(Style* self,const char* name) {
    CallGameMethod<void,0x008396F0>(self,name);
    self->table=Table();
    return self;
}
uint32_t ClassId(Style*) {
    return 0x94E25800u;
}
void* Query(Style* self,uint32_t id) {
    if(id==0x34CEC21Cu) return self;
    if(id==0x94E25800u) return self;
    return nullptr;
}
#if defined(_M_IX86)
// The base destructor writes only [ecx]; EAX is live through both functions.
// A tiny ABI tail shim avoids a compiler-generated scratch-EAX vtable store.
__declspec(naked) uint32_t __fastcall Destroy(Style*,void*) {
    __asm {
        mov dword ptr [ecx], 01026E04h
        mov edx, 008396B0h
        jmp edx
    }
}
#else
void Destroy(Style* self) {
    self->table=Table();
    CallGameMethod<void,0x008396B0>(self);
}
#endif
Style* Delete(Style* self,uint32_t flags) {
    self->table=Table();
    CallGameMethod<void,0x008396B0>(self);
    if(flags & 1u) CallGameFunction<void,0x004E3F70>(self);
    return self;
}
}
