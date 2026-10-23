#pragma once
#include "ts3/render/m07_abi.h"
#include "ts3/render/m07_layouts.h"
namespace ts3::render::m07::b08 {
struct Link { Link* next; Link* prev; };
struct ResourceLink { void* opaque; Link link; };
struct FormatStride { uint8_t opaque[0x20]; uint32_t* stride; };
struct TextureBinding { uint8_t opaque[0x30]; void* texture; };
struct Rect { int32_t left,top,right,bottom; };
struct Viewport { uint32_t x,y,width,height; float minZ,maxZ; };
// Local refinement of the scaffold's opaque prefix/cache. All pointers expand natively.
struct Host {
    uint8_t opaque0[8]; void* d3d9; ComHeader* device;
    uint32_t word10,word14,drawCount; uint8_t opaque1c[0xc];
    void* window; uint32_t width,height,word34,format;
    uint8_t opaque3c[0x1c]; uint32_t capacity,word5c;
    uint32_t primitive,primitiveCount,vertexCount,stride,byteCount,word74,cursor,word7c;
    void* vertexBuffer; uint32_t word84; IndexBuffer* indexBuffer;
    uint8_t opaque8c[0x4c]; Link resources; DevicePrivate* privateData;
    uint8_t opaquee4[8]; void* callback; void* userdata;
    uint32_t* stack; uint32_t stackIndex,stackCapacity;
    uint8_t opaque100[0x60]; uint32_t desired[203],applied[203];
    uint32_t dirty[7],savedDirty[7],invalid[7],samplerDirty;
    struct Sampler { uint32_t desired[13],applied[13],dirty,savedDirty,invalid; } samplers[16];
};
TS3_X86_OFFSET(Host,resources,0xd8);
TS3_X86_OFFSET(Host,privateData,0xe0);
TS3_X86_OFFSET(Host,desired,0x160);
TS3_X86_OFFSET(Host,dirty,0x7b8);
TS3_X86_OFFSET(Host,samplers,0x810);
// TS3W.exe 0x0060F9B0: Set sampler address U through the original state cache.
uint32_t AddressU(Host*,uint32_t,uint32_t);
// TS3W.exe 0x0060F9D0: Set sampler address V through the original state cache.
uint32_t AddressV(Host*,uint32_t,uint32_t);
// TS3W.exe 0x0060F9F0: Set sampler address W through the original state cache.
uint32_t AddressW(Host*,uint32_t,uint32_t);
// TS3W.exe 0x0060FA10: Set minification and mip filters, returning the second call's EAX.
uint32_t MinMipFilter(Host*,uint32_t,uint32_t);
// TS3W.exe 0x0060FA50: Set magnification filter.
uint32_t MagFilter(Host*,uint32_t,uint32_t);
// TS3W.exe 0x0060FAA0: Invalidate cached state and dispatch original flush.
uint32_t Invalidate(Host*);
// TS3W.exe 0x0060FCE0: Restore a state frame; nonempty path returns the receiver.
uintptr_t PopState(Host*);
// TS3W.exe 0x0060FE30: Bind the optional texture at argument +30, returning zero if absent.
uintptr_t BindTexture(Host*,TextureBinding*);
// TS3W.exe 0x0060FE50: Reserve streaming vertices with wrapping unsigned size checks.
void* BeginDraw(Host*,FormatStride*,uint32_t,uint32_t,uint32_t);
// TS3W.exe 0x0060FF40: Unlock and submit streaming geometry, advancing cursor even on failure.
uint32_t EndDraw(Host*);
// TS3W.exe 0x0060FFE0: Store and set viewport; keep alias-sensitive scalar store order.
uint32_t SetViewport(Host*,const Rect*,float,float);
// TS3W.exe 0x00610040: Clip window presentation rectangles against virtual desktop (AL return).
bool ClipWindow(void*,int32_t,int32_t,Rect**,Rect**,Rect*,Rect*);
// TS3W.exe 0x00610370: Append an intrusive resource link, returning the sentinel.
Link* LinkResource(Host*,ResourceLink*);

// TS3W.exe 0x0060F9B0: ABI adapter for AddressU.
uint32_t __fastcall HookAddressU(Host* h,void*,uint32_t s,uint32_t v);

// TS3W.exe 0x0060F9D0: ABI adapter for AddressV.
uint32_t __fastcall HookAddressV(Host* h,void*,uint32_t s,uint32_t v);

// TS3W.exe 0x0060F9F0: ABI adapter for AddressW.
uint32_t __fastcall HookAddressW(Host* h,void*,uint32_t s,uint32_t v);

// TS3W.exe 0x0060FA10: ABI adapter for MinMipFilter.
uint32_t __fastcall HookMinMipFilter(Host* h,void*,uint32_t s,uint32_t v);

// TS3W.exe 0x0060FA50: ABI adapter for MagFilter.
uint32_t __fastcall HookMagFilter(Host* h,void*,uint32_t s,uint32_t v);

// TS3W.exe 0x0060FAA0: ABI adapter for Invalidate.
uint32_t __fastcall HookInvalidate(Host* h,void*);

// TS3W.exe 0x0060FCE0: ABI adapter for PopState.
uintptr_t __fastcall HookPopState(Host* h,void*);

// TS3W.exe 0x0060FE30: ABI adapter for BindTexture.
uintptr_t __fastcall HookBindTexture(Host* h,void*,TextureBinding* t);

// TS3W.exe 0x0060FE50: ABI adapter for BeginDraw.
void* __fastcall HookBeginDraw(Host* h,void*,FormatStride* f,uint32_t p,uint32_t c,uint32_t v);

// TS3W.exe 0x0060FF40: ABI adapter for EndDraw.
uint32_t __fastcall HookEndDraw(Host* h,void*);

// TS3W.exe 0x0060FFE0: ABI adapter for SetViewport.
uint32_t __fastcall HookSetViewport(Host* h,void*,const Rect* r,float a,float b);

// TS3W.exe 0x00610370: ABI adapter for LinkResource.
Link* __fastcall HookLinkResource(Host* h,void*,ResourceLink* r);
}
