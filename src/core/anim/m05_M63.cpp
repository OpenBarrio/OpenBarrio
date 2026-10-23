#include "ts3/anim/m05_M63.h"
#include "ts3/layout.h"
namespace ts3::anim::m63 {
namespace {
void* Delete(void* self, uint32_t flags, uintptr_t table, uint32_t id) {
#if defined(_M_IX86)
    *static_cast<void**>(self) = reinterpret_cast<void*>(table);
#else
    (void)table;
    AbortNotRewritten("m63 game vtable");
#endif
    CallGameFunction<void,0x0054c510>(id,self);
    if (flags & 1u) {
        CallGameFunction<void,0x004e3f70>(self);
    }
    return self;
}
}
void* Delete0(void* self, uint32_t flags) {
    return Delete(self,flags,0x00fc2470,0x87eb0331);
}
void* Delete1(void* self, uint32_t flags) {
    return Delete(self,flags,0x00fc2480,0x060d998e);
}
void* Delete2(void* self, uint32_t flags) {
    return Delete(self,flags,0x00fc2490,0x065f6bc2);
}
void* Delete3(void* self, uint32_t flags) {
    return Delete(self,flags,0x00fc24a0,0x06662b00);
}
void* Delete4(void* self, uint32_t flags) {
    return Delete(self,flags,0x00fc24b0,0x0b1dc7dc);
}
void* Delete5(void* self, uint32_t flags) {
    return Delete(self,flags,0x00fc24c0,0x0bff764e);
}
#if defined(_M_IX86)
void* __fastcall HookDelete0(void* self, void*, uint32_t flags) { return Delete0(self,flags); }
void* __fastcall HookDelete1(void* self, void*, uint32_t flags) { return Delete1(self,flags); }
void* __fastcall HookDelete2(void* self, void*, uint32_t flags) { return Delete2(self,flags); }
void* __fastcall HookDelete3(void* self, void*, uint32_t flags) { return Delete3(self,flags); }
void* __fastcall HookDelete4(void* self, void*, uint32_t flags) { return Delete4(self,flags); }
void* __fastcall HookDelete5(void* self, void*, uint32_t flags) { return Delete5(self,flags); }
#endif
}
