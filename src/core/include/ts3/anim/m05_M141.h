#pragma once
#include "ts3/layout.h"
#include <cstdint>
#include <xmmintrin.h>
#include "ts3/anim/m05_M107.h"
namespace ts3::anim::m141 {
struct alignas(16) MotionRecord {
    uint32_t words[10];
    unsigned char untouched28[8];
    __m128 vectors[4];
    uint32_t tail[4];
};
TS3_X86_SIZE(MotionRecord,0x80);
struct alignas(16) MotionState {
    unsigned char opaque00[0x14];
    void* transform;
    unsigned char opaque18[0x60];
    void* controller;
    unsigned char opaque7c[7];
    uint8_t flag83,flag84;
    unsigned char opaque85[0x1b];
    MotionRecord record;
    unsigned char opaque120[0x28];
    int32_t mode;
    void* host;
    int32_t remaining;
};
TS3_X86_OFFSET(MotionState,controller,0x78);
TS3_X86_OFFSET(MotionState,record,0xa0);
TS3_X86_OFFSET(MotionState,host,0x14c);
// TS3W.exe 0x00854060: copy the motion record, leaving its eight padding bytes untouched, and return destination.
MotionRecord* CopyRecord(MotionRecord*,const MotionRecord*);
// TS3W.exe 0x008540F0: refresh the root-motion target and mirror transition state; preserve SSE reciprocal refinement and duplicated X/W output.
uint8_t RefreshMotion(MotionState*);
struct ControllerRef { void** vtable; int32_t refs; };
struct Link { Link* next; Link* previous; };
struct OwnedNode { Link links; ControllerRef* controller; };
struct StyleRange { unsigned char* begin; unsigned char* end; unsigned char opaque08[0xb4]; };
struct Body {
    void** vtable;
    void** refVtable;
    int32_t refs;
    unsigned char opaque0c[8];
    void* host;
    void** secondary;
    Link controllers;
    void* descriptor;
    m107::Style style;
    Link effects;
    void* effectsDescriptor;
    m107::Style* slaveStyles;
    void** slaves;
    unsigned char opaqueB8[12];
    void* master;
    int32_t slaveCount;
    uint8_t defer;
    unsigned char opaqueCD[0x23];
    unsigned char embeddedF0[0x20];
    unsigned char opaque110[0x14];
    void* actor;
    unsigned char opaque128[4];
    float selectedWeight;
    unsigned char opaque130[8];
    float rate;
    unsigned char opaque13c[0x24];
    float multiplier;
    unsigned char opaque164[0x10];
    uint32_t completions;
    uint32_t handle;
    unsigned char opaque17c[0xc8];
    StyleRange* ranges;
    uint32_t nesting;
    unsigned char opaque24c[4];
    uint8_t flag250,flag251;
    unsigned char opaque252[2];
    uint8_t enabled;
};
TS3_X86_OFFSET(Body,style,0x28);
TS3_X86_OFFSET(Body,slaveStyles,0xb0);
TS3_X86_OFFSET(Body,actor,0x124);
TS3_X86_OFFSET(Body,rate,0x138);
TS3_X86_OFFSET(Body,handle,0x178);
TS3_X86_OFFSET(Body,ranges,0x244);
// TS3W.exe 0x008538B0: remove a completed controller and dispatch the body state transition while retaining the parent; preserve low-byte predicates and release order.
uintptr_t Complete(void* secondary,ControllerRef* const* controller,int32_t event);
#if defined(_M_IX86)
uintptr_t __fastcall HookComplete(void*,void*,ControllerRef* const*,int32_t);
MotionRecord* __fastcall HookCopyRecord(MotionRecord*,void*,const MotionRecord*);
uint8_t __fastcall HookRefreshMotion(MotionState*,void*);
#endif
}
