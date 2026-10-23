#include "ts3/anim/m05_M65.h"
#include "ts3/anim/m05_M23.h"
#include "ts3/scratch/clu_4e3f70_Scratch.h"
namespace ts3::anim::m65 {
namespace {
void* Table(uintptr_t address) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(address);
#else
    (void)address; NotRewritten("M65 factory vtable");
#endif
}
}
Factory* Delete1660(Factory* self,uint32_t flags) {
    self->vtable=Table(0x00FC2520);
    m23::Unregister(0x067C5CAB);
    if(flags & 1u) scratch::Free4E3F70(self);
    return self;
}
Factory* Delete1690(Factory* self,uint32_t flags) {
    self->vtable=Table(0x00FC2528);
    m23::Unregister(0x07A11BA8);
    if(flags & 1u) scratch::Free4E3F70(self);
    return self;
}
Factory* Delete16C0(Factory* self,uint32_t flags) {
    self->vtable=Table(0x00FC2530);
    m23::Unregister(0x074A5F9D);
    if(flags & 1u) scratch::Free4E3F70(self);
    return self;
}
Factory* Delete16F0(Factory* self,uint32_t flags) {
    self->vtable=Table(0x00FC258C);
    m23::Unregister(0x0664ED68);
    if(flags & 1u) scratch::Free4E3F70(self);
    return self;
}
Factory* Delete1720(Factory* self,uint32_t flags) {
    self->vtable=Table(0x00FC2594);
    m23::Unregister(0x060D99B1);
    if(flags & 1u) scratch::Free4E3F70(self);
    return self;
}
Factory* Delete1750(Factory* self,uint32_t flags) {
    self->vtable=Table(0x00FC259C);
    m23::Unregister(0x060CAEEF);
    if(flags & 1u) scratch::Free4E3F70(self);
    return self;
}
}
#if defined(_M_IX86)
namespace ts3::anim::m65::hooks {
Factory* __fastcall Delete1660(Factory* self,void*,uint32_t flags) { return m65::Delete1660(self,flags); }
Factory* __fastcall Delete1690(Factory* self,void*,uint32_t flags) { return m65::Delete1690(self,flags); }
Factory* __fastcall Delete16C0(Factory* self,void*,uint32_t flags) { return m65::Delete16C0(self,flags); }
Factory* __fastcall Delete16F0(Factory* self,void*,uint32_t flags) { return m65::Delete16F0(self,flags); }
Factory* __fastcall Delete1720(Factory* self,void*,uint32_t flags) { return m65::Delete1720(self,flags); }
Factory* __fastcall Delete1750(Factory* self,void*,uint32_t flags) { return m65::Delete1750(self,flags); }
}
#endif
