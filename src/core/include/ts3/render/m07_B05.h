#pragma once
#include "ts3/render/m07_interfaces.h"
namespace ts3::render {
// Local refinement of the scaffold's opaque DeviceHost fields. Native pointers
// grow; the wire offsets below are asserted only for the original x86 ABI.
struct Stream { void* buffer; uint32_t offset, stride; };
struct b05 {
    using Host=b05;
    uint8_t opaque00[8]; void* d3d9; void* device;
    uint8_t opaque10[8]; uint32_t draws; uint8_t opaque1c[0xb8];
    uint8_t flag; uint8_t opaqueD5[0xb]; void* privateData;
    uint8_t opaqueE4[8]; void* callback; void* userdata; void* stack;
    uint32_t stackIndex, stackCapacity;
    void* declaration; Stream streams[2]; void* indices;
    uint8_t opaque120[0x284]; uint32_t state152;
    uint8_t tail[0x7f0-0x3a8+28];
// TS3W.exe 0x0060EE90: forward GetLight, returning the exact device result.
static uint32_t GetLight(Host*,uint32_t,void*);
// TS3W.exe 0x0060EEB0: forward LightEnable, returning the exact device result.
static uint32_t LightEnable(Host*,uint32_t,uint32_t);
// TS3W.exe 0x0060EED0: set cached render state 152 through the original state helper.
static Host* SetState152(Host*,uint32_t);
// TS3W.exe 0x0060EEF0: read cached state 152 (scalar clone of the verified getter).
static uint32_t GetState152(Host*);
// TS3W.exe 0x0060EF00: refresh private viewport at privateData+4.
static uint32_t GetViewport(Host*);
// TS3W.exe 0x0060EFB0: bind declaration only on change; unchanged returns the handle.
static uintptr_t SetDeclaration(Host*,m07::IndexBuffer*);
// TS3W.exe 0x0060EFE0: unbind matching declaration; unmatched returns the argument.
static uintptr_t RemoveDeclaration(Host*,m07::IndexBuffer*);
// TS3W.exe 0x0060F020: bind aligned stream; null leaves cached offset/stride untouched.
static uintptr_t SetStream(Host*,uint32_t,m07::IndexBuffer*,uint32_t,uint32_t);
// TS3W.exe 0x0060F0A0: unbind a buffer from exactly two streams, retaining offset/stride.
static uintptr_t RemoveStreams(Host*,m07::IndexBuffer*);
// TS3W.exe 0x0060F0F0: bind indices only on change; unchanged returns the handle.
static uintptr_t SetIndices(Host*,m07::IndexBuffer*);
// TS3W.exe 0x0060F120: unbind matching indices; unmatched returns cached handle.
static uintptr_t RemoveIndices(Host*,m07::IndexBuffer*);
// TS3W.exe 0x0060F150: map primitive type, increment draw count even on failure, and draw.
static uint32_t Draw(Host*,uint32_t,uint32_t,uint32_t);
// TS3W.exe 0x0060F180: map primitive type and forward indexed draw, counting failures.
static uint32_t DrawIndexed(Host*,uint32_t,int32_t,uint32_t,uint32_t,uint32_t,uint32_t);
// TS3W.exe 0x0060F1C0: return only the flag byte (upper EAX is unspecified).
static uint8_t Flag(Host*);

    uint32_t GetLight(uint32_t i,void* p) { return GetLight(this,i,p); }
    uint32_t LightEnable(uint32_t i,uint32_t v) { return LightEnable(this,i,v); }
    Host* SetState152(uint32_t v) { return SetState152(this,v); }
    uint32_t GetState152() { return GetState152(this); }
    uint32_t GetViewport() { return GetViewport(this); }
    uintptr_t SetDeclaration(void* r) { return SetDeclaration(this,static_cast<m07::IndexBuffer*>(r)); }
    uintptr_t RemoveDeclaration(void* r) { return RemoveDeclaration(this,static_cast<m07::IndexBuffer*>(r)); }
    uintptr_t SetStream(uint32_t i,void* r,uint32_t o,uint32_t s) { return SetStream(this,i,static_cast<m07::IndexBuffer*>(r),o,s); }
    uintptr_t RemoveStreams(void* r) { return RemoveStreams(this,static_cast<m07::IndexBuffer*>(r)); }
    uintptr_t SetIndices(void* r) { return SetIndices(this,static_cast<m07::IndexBuffer*>(r)); }
    uintptr_t RemoveIndices(void* r) { return RemoveIndices(this,static_cast<m07::IndexBuffer*>(r)); }
    uint32_t Draw(uint32_t t,uint32_t s,uint32_t n) { return Draw(this,t,s,n); }
    uint32_t DrawIndexed(uint32_t t,int32_t b,uint32_t m,uint32_t v,uint32_t s,uint32_t n) { return DrawIndexed(this,t,b,m,v,s,n); }
    uint8_t Flag() { return Flag(this); }
};
// 0x120..0x3a4 is 0x284 bytes (state cache is scalar, not pointers).
TS3_X86_OFFSET(b05,device,0xc);
TS3_X86_OFFSET(b05,draws,0x18);
TS3_X86_OFFSET(b05,flag,0xd4);
TS3_X86_OFFSET(b05,privateData,0xe0);
TS3_X86_OFFSET(b05,declaration,0x100);
TS3_X86_OFFSET(b05,streams,0x104);
TS3_X86_OFFSET(b05,indices,0x11c);
}
