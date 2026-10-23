#pragma once
#include "ts3/render/m07_abi.h"
#include "ts3/render/m07_layouts.h"
#include "ts3/resource/m03_A12.h"
namespace ts3::render::m07::b09 {
using PointerDeque=ts3::resource::m03::a12::PointerDeque;
struct Link { Link* next; Link* previous; };
struct Listener { void** vtable; Link links; };
// Separate native sparse view: pointer-bearing fields expand on x64.
struct Host {
    uint8_t opaque00[8]; void* d3d9; void* device;
    uint32_t word10,adapter,word18,word1c,word20;
    uint8_t flags24[4]; void* window; uint32_t width,height,word34,format,word3c,adapter40,word44;
    float aspect; uint32_t word4c; void* resource50; uint32_t words54[11];
    void* resource80; void* resource84; void* resource88;
    uint8_t sceneActive,lost,flag8e,pad8f;
    void* resource90; void* resource94; void* surface98; uint32_t word9c,worda0;
    PointerDeque fences; void* query; uint32_t padd4; Link otherListeners;
    void* privateData; Link listeners;
    void* callback; void* userdata; void* stateStack; uint32_t stackIndex,stackCapacity;
    uint32_t words100[8]; void* bindings[16]; uint8_t caps[2][0x32C];
    uint32_t words7b8[14]; uint32_t invalidWords[7]; uint32_t word80c;
    uint8_t samplerBytes[0x740]; uint8_t fakeDevice; uint8_t padf51[3]; uint32_t capsResult;
    uint16_t gamma[3][256];
};
TS3_X86_OFFSET(Host,fences,0xA4);
TS3_X86_OFFSET(Host,listeners,0xE4);
TS3_X86_OFFSET(Host,gamma,0xF58);
using Free = void (*)(void*);
void GameFree(void*);
struct MonitorInfo { uint32_t size; int32_t rects[8]; uint32_t flags; char device[32]; };
struct DisplayCalls {
    void* (*monitor)(void*,uint32_t);
    bool (*info)(void*,MonitorInfo*);
    void* (*create)(const char*,const char*);
    int32_t (*caps)(void*,int32_t);
    void (*destroy)(void*);
};
const DisplayCalls& OriginalDisplayCalls();
// TS3W.exe 0x006103B0: computes physical-to-pixel monitor aspect, defaulting to one on lookup failure.
void UpdateAspect(Host*,const DisplayCalls& = OriginalDisplayCalls());
// TS3W.exe 0x00610E20: initializes the host, including pending-fence storage and three identity gamma ramps.
Host* InitializeHost(Host*,const ts3::resource::m03::a12::Boundaries& = ts3::resource::m03::a12::OriginalBoundaries());
// TS3W.exe 0x00610500: appends a listener only when it is not already present.
void AddListener(Host*,Listener*);
// TS3W.exe 0x00610550: unlinks a registered listener, leaving its old links untouched.
void RemoveListener(Host*,Listener*);
// TS3W.exe 0x00610590: invokes slot zero in forward listener order.
void NotifyLost(Host*);
// TS3W.exe 0x006105E0: releases deque pages and map without resetting stale iterator fields.
void DestroyFences(PointerDeque*,Free=GameFree);
// TS3W.exe 0x00610650: adjusts the receiver to its pending-fence deque and destroys it.
void DestroyHostFences(Host*);
// TS3W.exe 0x00610850: releases volatile resources then pending fences in reverse order.
void ReleaseVolatile(Host*,Free=GameFree);
struct ShutdownCalls {
    void* (*manager)(uint32_t);
    void (*flush)(void*);
    void (*finish)(void*);
    Free free;
    void** globalOwner;
    void** globalD3D;
};
struct Rect { int32_t left,top,right,bottom; };
struct SurfaceTemp { void* primary; uint32_t refs; void* secondary; uint32_t words0c[3]; uint32_t width,height,word20; void* surface; };
struct CaptureTexture { uint8_t opaque[0x20]; uint32_t flags; uint8_t opaque24[8]; uint32_t kind; };
TS3_X86_OFFSET(SurfaceTemp,surface,0x24);
struct CaptureCalls {
    void (*construct)(SurfaceTemp*);
    void (*surface2d)(CaptureTexture*,uint32_t,SurfaceTemp*);
    void (*surfaceCube)(CaptureTexture*,uint32_t,uint32_t,SurfaceTemp*);
    void (*destroy)(SurfaceTemp*);
};
struct ResetCalls {
    uint32_t (*clock)();
    void* (*foreground)();
    uint32_t (*windowProcess)(void*);
    uint32_t (*process)();
    uint8_t (*recreate)(Host*);
    void (*aspect)(Host*);
    void (*invalidate)(Host*);
    uint8_t (*resources)(Host*);
    void* (*service)();
};
const ResetCalls& OriginalResetCalls();
// TS3W.exe 0x00610AB0: performs the cooperative-level/reset and resource notification lifecycle; return is AL-only.
uint8_t Reset(Host*,uint8_t,const ResetCalls& = OriginalResetCalls());
// TS3W.exe 0x00611680: ends an active scene, presents (destination precedes source), then handles device loss.
void Present(Host*,const Rect* destination,const Rect* source,void* window);
// TS3W.exe 0x00611760: presents with all three optional arguments null.
void PresentDefault(Host*);
const CaptureCalls& OriginalCaptureCalls();
// TS3W.exe 0x00610660: captures render target zero, retaining EA's target leak on rejected fast-path geometry; return is AL-only.
uint8_t CaptureTarget(Host*,CaptureTexture*,int32_t,int32_t,uint32_t,uint32_t,const Rect*,const CaptureCalls& = OriginalCaptureCalls());
const ShutdownCalls& OriginalShutdownCalls();
// TS3W.exe 0x00610960: drains the manager, releases host resources and clears shared device handles.
void Shutdown(Host*,const ShutdownCalls& = OriginalShutdownCalls());
#if defined(_M_IX86)
inline uint8_t __fastcall HookReset(Host* h,void*,uint8_t force) { return Reset(h,force); }
inline void __fastcall HookPresent(Host* h,void*,const Rect* destination,const Rect* source,void* window) { Present(h,destination,source,window); }
inline void __fastcall HookPresentDefault(Host* h,void*) { PresentDefault(h); }
inline uint8_t __fastcall HookCapture(Host* h,void*,CaptureTexture* t,int32_t x,int32_t y,uint32_t level,uint32_t face,const Rect* r) { return CaptureTarget(h,t,x,y,level,face,r); }
inline void __fastcall HookShutdown(Host* h,void*) { Shutdown(h); }
inline Host* __fastcall HookInitialize(Host* h,void*) { return InitializeHost(h); }
inline void __fastcall HookAspect(Host* h,void*) { UpdateAspect(h); }
inline void __fastcall HookAdd(Host* h,void*,Listener* p) { AddListener(h,p); }
inline void __fastcall HookRemove(Host* h,void*,Listener* p) { RemoveListener(h,p); }
inline void __fastcall HookNotify(Host* h,void*) { NotifyLost(h); }
inline void __fastcall HookDestroy(PointerDeque* q,void*) { DestroyFences(q); }
inline void __fastcall HookHostDestroy(Host* h,void*) { DestroyHostFences(h); }
inline void __fastcall HookRelease(Host* h,void*) { ReleaseVolatile(h); }
#endif
}
