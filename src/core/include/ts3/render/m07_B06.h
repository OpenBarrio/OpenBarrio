#pragma once
#include "ts3/render/m07_interfaces.h"
namespace ts3::render::b06 {
// Batch-local sparse view: pointers hidden in scaffold opaque ranges are native-sized.
struct Host {
    uint8_t opaque00[8]; void* d3d9; void* device;
    uint32_t word10,word14,draws; uint8_t opaque1c[8];
    uint8_t flag24,flag25,flag26,flag27; uint8_t opaque28[0x30];
    uint32_t vertexCount,indexCount; uint8_t opaque60[0x20];
    void* vertex; void* index; uint8_t opaque88[8];
    void* color; void* depth; void* texture; uint8_t opaque9c[0x44];
    void* privateData; uint8_t opaquee4[0x60];
    uint32_t cache[(0x1558-0x144)/4];
    uint32_t& Word(uint32_t offset) { return cache[(offset-0x144)/4]; }
};
TS3_X86_OFFSET(Host,draws,0x18);
TS3_X86_OFFSET(Host,vertex,0x80);
TS3_X86_OFFSET(Host,privateData,0xe0);
TS3_X86_OFFSET(Host,cache,0x144);
// TS3W.exe 0x0060F1D0: draw indexed instanced primitives and restore stream frequencies.
int32_t __fastcall Draw(Host*,void*,uint32_t,int32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t);
// TS3W.exe 0x0060F350: create immediate texture and buffers, keeping partial state on failure.
bool __fastcall CreateImmediate(Host*,void*);
// TS3W.exe 0x0060F430: detach and reacquire color/depth surfaces in order.
bool __fastcall AcquireSurfaces(Host*,void*);
// TS3W.exe 0x0060F4C0: query and duplicate render/sampler caches; ignore HRESULTs.
uint32_t __fastcall ReadStates(Host*,void*);
// TS3W.exe 0x0060F5D0: expose the private-data mode block.
void* __fastcall Mode(Host*,void*);
// TS3W.exe 0x0060F5F0: query device format support using private adapter settings.
bool __fastcall CheckFormat(Host*,void*,uint32_t,uint32_t,uint32_t);
// TS3W.exe 0x0060F630: capture gamma only for fullscreen hardware gamma mode.
void __fastcall CaptureGamma(Host*,void*);
// TS3W.exe 0x0060F670: expose the three contiguous gamma ramps; return third output argument.
void** __fastcall GammaRamps(Host*,void*,void**,void**,void**);
// TS3W.exe 0x0060F710: unsigned test of the private maximum index against 65535.
bool __fastcall WideIndices(Host*,void*);
// TS3W.exe 0x0060F740: return renderer auxiliary singleton (exact singleton-load twin).
void* __cdecl Singleton();
// TS3W.exe 0x0060F750: forward state 168 without boolean normalization.
void __fastcall State168(Host*,void*,uint32_t);
// TS3W.exe 0x0060F770: translate cull enum and forward state 22.
void __fastcall Cull(Host*,void*,uint32_t);
// TS3W.exe 0x0060F790: forward state 7 using only the argument's low byte.
void __fastcall ZEnable(Host*,void*,uint32_t);
}
