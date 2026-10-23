#include "ts3/render/m07_B02.h"
#include <cstring>
#include <stdexcept>
// Zero-return twins start from m03_C05::DriverZero (0x006119F0), with
// stdcall stack cleanup recovered separately for each renderer slot.
// DeleteD3D9 follows GlyphMetricsMap::DestroyBase (0x004553B0), changing
// only the installed table. These are native sparse views, not wire pointers.
namespace ts3::render::m07::b02 {
namespace {
constexpr Guid pixel={{0xdc,0xdb,0x3b,0x6d,0x02,0x5b,0x15,0x44,0xb8,0x52,0xce,0x5e,0x8b,0xcc,0xb2,0x89}};
constexpr Guid vertex={{0x7e,0x55,0xc5,0xef,0x65,0x62,0x13,0x46,0x8a,0x94,0x43,0x85,0x78,0x89,0xeb,0x36}};
constexpr Guid unknown={{0,0,0,0,0,0,0,0,0xc0,0,0,0,0,0,0,0x46}};
uint32_t ComCall(void* self,size_t slot) {
    auto table=*static_cast<void* const* const*>(self);
    return reinterpret_cast<Stdcall<uint32_t,void*>>(table[slot])(self);
}
uint32_t Query(FakeShader* self,const Guid* iid,void** output,const Guid& kind) {
    if(std::memcmp(iid,&kind,16)==0 || std::memcmp(iid,&unknown,16)==0) {
        ComCall(self,1);
        *output=self;
        return 0;
    }
    *output=nullptr;
    return 0x80004002u;
}
void* const* Table(uint32_t address) {
#if defined(_M_IX86)
    return reinterpret_cast<void* const*>(uintptr_t(address));
#else
    (void)address;
    throw std::runtime_error("M07 B02 deleting destructor requires native vtable injection");
#endif
}
}
uint32_t __stdcall Zero7(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t) {
    return 0;
}
uint32_t __stdcall One(uintptr_t) {
    return 1;
}
float __stdcall FloatZero(uintptr_t) {
    return 0.0f;
}
uint32_t __stdcall Zero9(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t) {
    return 0;
}
uint32_t __stdcall NotImplemented(uintptr_t, uintptr_t, uintptr_t, uintptr_t) {
    return 0x80004001u;
}
uint32_t __stdcall Zero4(uintptr_t, uintptr_t, uintptr_t, uintptr_t) {
    return 0;
}
uint32_t __stdcall InvalidCall(uintptr_t, uintptr_t, uintptr_t) {
    return 0x8876086Au;
}
uint32_t __stdcall QueryPixel(FakeShader* self,const Guid* iid,void** output) {
    return Query(self,iid,output,pixel);
}
uint32_t __stdcall QueryVertex(FakeShader* self,const Guid* iid,void** output) {
    return Query(self,iid,output,vertex);
}
uint32_t __stdcall GetDevice(FakeShader* self,void** output) {
    *output=self->device;
    ComCall(self->device,1);
    return 0;
}
void* __fastcall DeleteD3D9(void* self,void*,uint32_t flags) {
    *static_cast<void* const**>(self)=Table(0x00FE7BC8);
    if(flags & 1) CallGameFunction<void,0x004E3F70>(self);
    return self;
}
uint32_t __stdcall AddRef(ResourceHeader* self) {
    return ComAddRef(reinterpret_cast<int32_t*>(&self->refs));
}
uint32_t __stdcall Release(ResourceHeader* self) {
    uint32_t refs=ComRelease(reinterpret_cast<int32_t*>(&self->refs));
    if(refs==0 && self) {
        auto table=static_cast<void* const*>(self->primary);
        reinterpret_cast<Thiscall<void*,void,uint32_t>>(table[17])(self,nullptr,1);
    }
    return refs;
}
void* __fastcall DeleteDevice(DevicePrefix* self,void*,uint32_t flags) {
    self->vtable=Table(0x00FE7C10);
    ComCall(self->parent,2);
    if(flags & 1) CallGameFunction<void,0x004E3F70>(self);
    return self;
}
}
