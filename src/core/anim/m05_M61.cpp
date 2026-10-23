#include "ts3/anim/m05_M61.h"
namespace ts3::anim::m61 {
namespace {
void* DeleteScratch(void* self, uint32_t flags, uintptr_t table, uint32_t key) {
#if defined(_M_IX86)
    *static_cast<void**>(self)=reinterpret_cast<void*>(table);
#else
    (void)table;
    AbortNotRewritten("M61 original Scratch vtable");
#endif
    CallGameFunction<void,0x0054C510>(key,self);
    if(flags & 1) CallGameFunction<void,0x004E3F70>(self);
    return self;
}
}
void* __fastcall Delete11E0(void* self, void*, uint32_t flags) {
    return DeleteScratch(self,flags,0x00FC2350,0x53DEEC77);
}
void* __fastcall Delete1210(void* self, void*, uint32_t flags) {
    return DeleteScratch(self,flags,0x00FC2360,0x073BB514);
}
void* __fastcall Delete1240(void* self, void*, uint32_t flags) {
    return DeleteScratch(self,flags,0x00FC2370,0x9419C8AB);
}
void* __fastcall Delete1270(void* self, void*, uint32_t flags) {
    return DeleteScratch(self,flags,0x00FC2380,0x33E02B1D);
}
void* __fastcall Delete12A0(void* self, void*, uint32_t flags) {
    return DeleteScratch(self,flags,0x00FC2390,0xD3DFC020);
}
void* __fastcall Delete12D0(void* self, void*, uint32_t flags) {
    return DeleteScratch(self,flags,0x00FC23A0,0xD4E4CCF8);
}
}
