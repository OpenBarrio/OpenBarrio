#pragma once
#include "ts3/layout.h"
namespace ts3::render::b31_detail {
struct Vector { void* begin; void* end; void* capacity; const char* tag; };
struct RoofSolver {
    void** vtable; void* owner; void* service; void* settings;
    uint8_t initialized; uint8_t pad11[3]; uint32_t state; float progress;
    uint8_t opaque1c[0x30]; void* texture;
    Vector samples, map, blurred, sampleFilter, blurFilter;
    float minimum, maximum, inverse, scale;
    uint32_t cursor, prepared, extra, width, height, dimension, bytes, count;
};
TS3_X86_OFFSET(RoofSolver, texture, 0x4c);
TS3_X86_OFFSET(RoofSolver, samples, 0x50);
TS3_X86_OFFSET(RoofSolver, minimum, 0xa0);
TS3_X86_OFFSET(RoofSolver, count, 0xcc);
// TS3W.exe 0x006888F0: Initialize owner and service only once (byte-sized flag).
void Bind(RoofSolver*, void*, void*);
// TS3W.exe 0x00688920: Tick unfinished solvers when the service's low return byte is true.
void Tick(RoofSolver*, uint32_t, float);
// TS3W.exe 0x00688960: Return minus one before work starts, otherwise the stored progress.
float Progress(RoofSolver*);
// TS3W.exe 0x006895E0: Construct the roof solver and five tagged vectors.
RoofSolver* Construct(RoofSolver*);
// TS3W.exe 0x00689D50: Destroy, optionally free on flag bit zero, and return the old receiver.
RoofSolver* Delete(RoofSolver*, uint32_t);
// TS3W.exe 0x0068A330: Resize working buffers and reset their sentinel values.
void* Reset(RoofSolver*);
// TS3W.exe 0x0068A400: Advance both roof passes, upload the result, then clear and compact working vectors.
uint32_t Solve(RoofSolver*, uint32_t, float);
// TS3W.exe 0x0068A620: Reset on the matching notification and always return false in AL.
bool Notify(RoofSolver*, uint32_t, uint32_t);
struct RoofSettings { uint8_t opaque[8]; uint8_t halfResolution; uint8_t opaque09[0x33]; uint32_t texels,dimension; float scale; };
TS3_X86_OFFSET(RoofSettings,texels,0x3c);
// TS3W.exe 0x0068A7F0: Allocate and initialize the roof texture; already-initialized receivers retain entry EAX.
uintptr_t InitRoof(RoofSolver*,void*,void*,uint32_t,uint32_t,uintptr_t entryEax);
struct Texture { void** vtable; uint32_t refs; void** secondary; uint8_t opaque[8]; uint32_t width,height; };
TS3_X86_OFFSET(Texture,height,0x18);
struct Probe { uint8_t opaque[0x2c50]; Texture* textures[6]; uint8_t tail[0x2f]; uint8_t dynamic; };
TS3_X86_OFFSET(Probe,dynamic,0x2c97);
// TS3W.exe 0x006B3690: Create probe cube maps and account every retained texture; cached entry retains EAX.
uintptr_t InitProbe(Probe*,uintptr_t entryEax);
struct alignas(16) LotLightmap {
 uint8_t opaque[0x14];uint32_t width,height,x,y;uint8_t opaque24[8];void* normalSource;
 uint8_t opaque30[0xc0];void* positionParameter;void* colorParameter;uint8_t opaqueF8[8];void* normalParameter;void* heightParameter;uint8_t opaque108[8];
 alignas(16) float position[4];alignas(16) float color[4];
};
TS3_X86_OFFSET(LotLightmap,position,0x110);
struct alignas(16) LotLight {
 void** vtable;uint8_t opaque04[12];float channels[4];uint8_t opaque20[0xa0];uint64_t key;uint8_t opaqueC8[8];uint32_t excluded;uint8_t opaqueD4[0x1c];alignas(16) float color[4];
};
TS3_X86_OFFSET(LotLight,color,0xf0);
// TS3W.exe 0x00C292B0: Bind terrain maps and render intersecting lights, testing the full 64-bit light key.
bool RenderLot(LotLightmap*);
struct WallPool { uint8_t opaque[0x4c]; Texture* texture; void* storage; };
// TS3W.exe 0x00689400: Grow the wall-texel storage texture and update storage accounting in original order.
uintptr_t GrowWallPool(WallPool*);
struct RayOwner { uint8_t opaque[0x20]; uint32_t count; };
struct RaySolver { void* caster; RayOwner* owner; uint8_t opaque[0x1c]; uint32_t divisions; };
TS3_X86_OFFSET(RaySolver, divisions, 0x24);
// TS3W.exe 0x00689EA0: Cast both interior ray fans with SSE lane order and EA's asymmetric endpoints.
uint32_t CastRays(RaySolver*, uint32_t, const float*, const float*, uint32_t);
// TS3W.exe 0x0068B010: Resize an AO record vector through the original erase and insert helpers.
uintptr_t ResizeRecords(Vector*, uint32_t);
}
// ABI adapters: hookgen's thiscall manifest expects member methods. These views
// have no native data members; all accessed storage belongs to the sparse types above.
namespace ts3::render {
struct b31 {
    uintptr_t GrowWallPool() {return b31_detail::GrowWallPool(reinterpret_cast<b31_detail::WallPool*>(this));}
    bool RenderLot() {return b31_detail::RenderLot(reinterpret_cast<b31_detail::LotLightmap*>(this));}
    uint32_t Solve(uint32_t arg,float dt) { return b31_detail::Solve(reinterpret_cast<b31_detail::RoofSolver*>(this),arg,dt); }
    void Tick(uint32_t arg, float dt) { b31_detail::Tick(reinterpret_cast<b31_detail::RoofSolver*>(this),arg,dt); }
    void* Reset() { return b31_detail::Reset(reinterpret_cast<b31_detail::RoofSolver*>(this)); }
    bool Notify(uint32_t event,uint32_t value) { return b31_detail::Notify(reinterpret_cast<b31_detail::RoofSolver*>(this),event,value); }
    uintptr_t ResizeRecords(uint32_t n) { return b31_detail::ResizeRecords(reinterpret_cast<b31_detail::Vector*>(this),n); }
    uint32_t CastRays(uint32_t arg,const void* a,const void* b,uint32_t mode) { return b31_detail::CastRays(reinterpret_cast<b31_detail::RaySolver*>(this),arg,static_cast<const float*>(a),static_cast<const float*>(b),mode); }
};
}
