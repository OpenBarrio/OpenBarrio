#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::m155 {
struct Component {
    void** vtable;
    void* host;
    uintptr_t opaque08[6];
    void* pending;
    void* transform;
    uintptr_t opaque28;
    uint32_t flags;
};
TS3_X86_OFFSET(Component,pending,0x20);
TS3_X86_OFFSET(Component,transform,0x24);
TS3_X86_OFFSET(Component,flags,0x2c);
struct LocalPose {
    void** vtable;
    unsigned char opaque[0x3850-sizeof(void*)];
    void** weights;
};
TS3_X86_OFFSET(LocalPose,weights,0x3850);
struct Ref { void** vtable; volatile long refs; };
// TS3W.exe 0x00941230: draw rig bone lines, axes and labels with optional pose-difference overlay; overlay axes read stale stack bytes.
void DrawPose(void*,void*,const Matrix44Value*,uint32_t,void*,const void*,int32_t,const uint32_t*,const uint32_t*,const uint32_t*,const char* const*,const Matrix44Value*,const Matrix44Value* staleAxes);
// TS3W.exe 0x00941B30: update component notification and copy the optional host transform; returns AL=1.
bool Update(Component*,uintptr_t);
// TS3W.exe 0x00941BC0: latch bit 0x800 then tail-call the component refresh, preserving EAX.
uintptr_t Invalidate(Component*);
// TS3W.exe 0x00941C00: destroy local-pose weights and optionally free this; returns this even after free.
LocalPose* DeletePose(LocalPose*,uint32_t);
// TS3W.exe 0x00941FC0: find/create render state, attach retained inputs and update it; preserves release-path EAX.
uintptr_t UpdateRender(uint32_t,uint32_t,void*,Ref*);
#if defined(_M_IX86)
// TS3W.exe 0x00941230: preserve the original cdecl frame and its stale overlay-axes matrix.
void DebugPose(void*,void*,const Matrix44Value*,uint32_t,void*,const void*,int32_t,const uint32_t*,const uint32_t*,const uint32_t*,const char* const*,const Matrix44Value*);
namespace hooks {
bool __fastcall Update(Component*,void*,uintptr_t);
uintptr_t __fastcall Invalidate(Component*,void*);
LocalPose* __fastcall DeletePose(LocalPose*,void*,uint32_t);
}
#endif
}
