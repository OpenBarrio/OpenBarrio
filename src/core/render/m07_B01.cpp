#include "ts3/render/m07_B01.h"
#include <cstring>
namespace ts3::render::m07::b01 {
namespace {
constexpr uint32_t iid9[4]={0x81BDCBCA,0x426D64D4,0x01AD8DAE,0x5C27F447};
constexpr uint32_t iidDevice[4]={0xD0223B96,0x43FDBF7A,0x3BA4BD92,0xEBB9820D};
constexpr uint32_t iidUnknown[4]={0,0,0xC0,0x46000000};
void AddRef(ComHeader* self) {
    reinterpret_cast<ComMethod<uint32_t,ComHeader>>(self->vtable[1])(self);
}
uint32_t Query(ComHeader* self,const void* iid,void** output,const uint32_t* specific) {
    if (std::memcmp(iid,specific,16)==0 || std::memcmp(iid,iidUnknown,16)==0) {
        AddRef(self);
        *output=self;
        return 0;
    }
    *output=nullptr;
    return 0x80004002;
}
}
uint32_t __stdcall QueryD3D9(ComHeader* self,const void* iid,void** output) {
    return Query(self,iid,output,iid9);
}
uint32_t __stdcall QueryDevice(ComHeader* self,const void* iid,void** output) {
    return Query(self,iid,output,iidDevice);
}
uint32_t __stdcall Stub00611870(uintptr_t,uintptr_t,uintptr_t) {
    return 0x00000000;
}
uint32_t __stdcall Stub00611880(uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t) {
    return 0x80004001;
}
uint32_t __stdcall Stub00611890(uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t) {
    return 0x00000000;
}
uint32_t __stdcall Stub00611900(uintptr_t) {
    return 0x08000000;
}
uint32_t __stdcall Stub00611970(uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t) {
    return 0x80004001;
}
uint32_t __stdcall Stub00611980(uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t) {
    return 0x80004001;
}
uint32_t __stdcall Stub00611990(uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t) {
    return 0x80004001;
}
uint32_t __stdcall Stub006119A0(uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t) {
    return 0x80004001;
}
uint32_t __stdcall Stub006119B0(uintptr_t,uintptr_t,uintptr_t) {
    return 0x80004001;
}
uint32_t __stdcall Stub006119C0(uintptr_t,uintptr_t) {
    return 0x80004001;
}
uint32_t __stdcall GetParent(DevicePrefix* self,ComHeader** output) {
    if (!output) return 0x80004003;
    AddRef(self->parent);
    *output=self->parent;
    return 0;
}
uint32_t __stdcall GetDisplayMode(ComHeader*,uintptr_t,DisplayMode* output) {
    output->width=1024;
    output->height=768;
    output->refresh=60;
    output->format=21;
    return 0;
}
}
