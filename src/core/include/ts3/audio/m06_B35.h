#pragma once
#include "ts3/layout.h"
namespace ts3::audio::b35 {
// Offset-indexed observed views. Each pointer-bearing cell remains pointer-sized on native builds;
// byte offsets name x86 cells, not offsets to serialize on x64.
union Cell { uint32_t u; int32_t i; float f; void* p; };
struct Object { Cell cell[604]; };
inline Cell& At(Object* s,unsigned offset) { return s->cell[offset/4]; }
inline Object* View(Object* s,unsigned offset) { return reinterpret_cast<Object*>(s->cell+offset/4); }
inline uint8_t& Byte(Object* s,unsigned offset) { return reinterpret_cast<uint8_t*>(&At(s,offset))[offset%4]; }
struct Triple { Cell a,b,c; };
struct Vector { Triple* begin; Triple* end; Triple* capacity; };
TS3_X86_SIZE(Cell,4);
// TS3W.exe 0x008A1F70: publish spatial controls using three-component distance and x87 azimuth.
uint8_t Spatial(Object*,const float*);
// TS3W.exe 0x008A2320: resolve sound data and read its packed header.
uint8_t Load(Object*);
// TS3W.exe 0x008A2560: release emitter resources in callback-visible order.
uintptr_t Destruct(Object*);
// TS3W.exe 0x008A2640: deleting destructor through the +8 interface.
Object* DeleteAdjusted(Object*,uint32_t);
// TS3W.exe 0x008A2650: evaluate the distance curve with sign-bit clamps (second argument unused).
double Curve(Object*,float,float,float,float);
// TS3W.exe 0x008A2710: insert a triple; full-vector path retains EA's null-destination behavior.
uintptr_t Insert(Vector*,Triple*,const Triple*);
// TS3W.exe 0x008A2820: initialize the emitter, preserving untouched padding.
Object* Construct(Object*);
// TS3W.exe 0x008A29D0: destroy and optionally free an emitter, returning its former address.
Object* Delete(Object*,uint32_t);
// TS3W.exe 0x008A29F0: append a retained resource, incrementing the end before AddRef.
uintptr_t Retain(Object*,void**);
// TS3W.exe 0x008A2A30: instantiate voice descriptors under the RW lock.
uint8_t Voices(Object*,int32_t,Object*);
// TS3W.exe 0x008A2CA0: create and initialize an emitter voice container.
uint8_t CreateVoice(Object*);
// TS3W.exe 0x008A2F30: test the collection's count for zero.
bool Empty(Object*);
// TS3W.exe 0x008A2F40: update nesting depth and notify only on zero/one transitions.
uintptr_t Toggle(Object*,uint8_t);
// TS3W.exe 0x008A2FF0: initialize the fixed-pool hash table, retaining the shared empty bucket sentinel.
Object* InitPool(Object*,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,Object*);
}

#if defined(_M_IX86)
namespace ts3::audio::b35 {
// TS3W.exe 0x008A1F70: ABI entry for Spatial, with thiscall-compatible ECX and callee cleanup.
uint8_t __fastcall HookSpatial(Object* self,void*,const float* point);
// TS3W.exe 0x008A2320: ABI entry for Load, with thiscall-compatible ECX and callee cleanup.
uint8_t __fastcall HookLoad(Object* self,void*);
// TS3W.exe 0x008A2560: ABI entry for Destruct, with thiscall-compatible ECX and callee cleanup.
uintptr_t __fastcall HookDestruct(Object* self,void*);
// TS3W.exe 0x008A2640: ABI entry for DeleteAdjusted, with thiscall-compatible ECX and callee cleanup.
Object* __fastcall HookDeleteAdjusted(Object* self,void*,uint32_t flags);
// TS3W.exe 0x008A2650: ABI entry for Curve, with thiscall-compatible ECX and callee cleanup.
double __fastcall HookCurve(Object* self,void*,float a,float b,float c,float d);
// TS3W.exe 0x008A2710: ABI entry for Insert, with thiscall-compatible ECX and callee cleanup.
uintptr_t __fastcall HookInsert(Vector* self,void*,Triple* position,const Triple* value);
// TS3W.exe 0x008A2820: ABI entry for Construct, with thiscall-compatible ECX and callee cleanup.
Object* __fastcall HookConstruct(Object* self,void*);
// TS3W.exe 0x008A29D0: ABI entry for Delete, with thiscall-compatible ECX and callee cleanup.
Object* __fastcall HookDelete(Object* self,void*,uint32_t flags);
// TS3W.exe 0x008A29F0: ABI entry for Retain, with thiscall-compatible ECX and callee cleanup.
uintptr_t __fastcall HookRetain(Object* self,void*,void** source);
// TS3W.exe 0x008A2A30: ABI entry for Voices, with thiscall-compatible ECX and callee cleanup.
uint8_t __fastcall HookVoices(Object* self,void*,int32_t channels,Object* out);
// TS3W.exe 0x008A2CA0: ABI entry for CreateVoice, with thiscall-compatible ECX and callee cleanup.
uint8_t __fastcall HookCreateVoice(Object* self,void*);
// TS3W.exe 0x008A2F30: ABI entry for Empty, with thiscall-compatible ECX and callee cleanup.
bool __fastcall HookEmpty(Object* self,void*);
// TS3W.exe 0x008A2F40: ABI entry for Toggle, with thiscall-compatible ECX and callee cleanup.
uintptr_t __fastcall HookToggle(Object* self,void*,uint8_t active);
// TS3W.exe 0x008A2FF0: ABI entry for InitPool, with thiscall-compatible ECX and callee cleanup.
Object* __fastcall HookInitPool(Object* self,void*,uint32_t a,uint32_t b,uint32_t c,uint32_t d,uint32_t e,uint32_t f,Object* allocator);
}
#endif
