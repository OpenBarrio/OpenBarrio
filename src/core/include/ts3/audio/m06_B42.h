#pragma once
#include "ts3/layout.h"
namespace ts3::audio {
struct B42Value {
    void* table; uint32_t state; float value; uint8_t enabled; uint8_t pad[3]; int32_t refs; void* owner;
};
struct B42Curve { void* begin; void* end; void* capacity; uint8_t rest[0x360-12]; };
struct B42Scene {
    void* table; uint32_t reserved[2]; B42Curve curve; B42Scene* linked;
    uint8_t enabled; uint8_t pad[3]; uint32_t stamp; float value; void* context; uint8_t registered;
};
TS3_X86_OFFSET(B42Value, refs, 0x10);
TS3_X86_OFFSET(B42Scene, linked, 0x36c);
TS3_X86_OFFSET(B42Scene, context, 0x37c);
// TS3W.exe 0x008A6E60: Release the signed reference count, deleting through slot two at counts <= one.
int32_t __fastcall B42Release(B42Value*, void*);
// TS3W.exe 0x008A6E80: Allocate a 24-byte pooled value or forward other sizes to the heap.
void* __cdecl B42AllocateValue(uint32_t);
// TS3W.exe 0x008A6EC0: Initialize the value interface, preserving padding and setting its owner.
B42Value* __fastcall B42Construct(B42Value*, void*, void*);
// TS3W.exe 0x008A6EF0: Install the base table and optionally push the value onto its free list.
B42Value* __fastcall B42Delete(B42Value*, void*, uint32_t);
// TS3W.exe 0x008A6F10: Reset the response curve to the two default points and return AL=true.
uint32_t __fastcall B42Reset(B42Scene*, void*);
// TS3W.exe 0x008A6F50: Return the cached scalar through x87.
float __fastcall B42ValueGet(B42Scene*, void*);
// TS3W.exe 0x008A6F60: Return the borrowed context pointer.
void* __fastcall B42Context(B42Scene*, void*);
// TS3W.exe 0x008A6F70: Return the raw enabled byte, without boolean normalization.
uint8_t __fastcall B42Enabled(B42Scene*, void*);
// TS3W.exe 0x008A6F80: Store and return the low byte of the argument.
uint8_t __fastcall B42SetEnabled(B42Scene*, void*, uint32_t);
// TS3W.exe 0x008A6F90: Register on every nonzero request; unregister only when already registered.
void __fastcall B42Register(B42Scene*, void*, uint32_t);
// TS3W.exe 0x008A6FE0: Forward point assignment to the embedded response curve.
uintptr_t __fastcall B42SetCurve(B42Scene*, void*, const float*, int32_t);
// TS3W.exe 0x008A6FF0: Allocate a 0x388-byte pooled scene or forward other sizes to the heap.
void* __cdecl B42AllocateScene(uint32_t);
// TS3W.exe 0x008A7030: Disable registration, conditionally stop the linked scene, then release it; only AL becomes one.
uint8_t __fastcall B42Stop(B42Scene*, void*);
// TS3W.exe 0x008A7090: Forward updates on changed or zero stamps, propagate enabled state and evaluate the curve.
void __fastcall B42Update(B42Scene*, void*, float, uint32_t);
// TS3W.exe 0x008A7120: Retain the new link before publishing it and releasing the old link.
void __fastcall B42Link(B42Scene*, void*, B42Scene*);
}
