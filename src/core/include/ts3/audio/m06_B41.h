#pragma once
#include "ts3/layout.h"
namespace ts3::audio::b41 {
struct Ref { void* vtable; };
struct RefVector {
 Ref** begin; Ref** end; Ref** capacity; const char* label; Ref** storage;
 Ref* inlineStorage[10];
};
struct SceneList {
 void* vtable; void* secondary; void* tertiary; uint32_t state;
 RefVector entries;
 uint8_t active; uint8_t pad4d[3]; uint32_t key[2]; uint32_t a,b; float gain;
 uint8_t flag; uint8_t pad65[3]; void* owner;
};
struct Primitive {
 void* vtable; void* secondary; void* tertiary; uint32_t state;
 uint8_t enabled; uint8_t pad11[3]; void* context; Ref* resource; void* tail; uint32_t hash;
};
struct GainView { void* vtable; uint32_t state; float gain; uint8_t flag; };
struct RefCountView { void* vtable; uint32_t state[3]; uint32_t refs; };
TS3_X86_OFFSET(SceneList,entries,0x10);
TS3_X86_OFFSET(SceneList,active,0x4c);
TS3_X86_OFFSET(SceneList,owner,0x68);
TS3_X86_OFFSET(Primitive,resource,0x18);
TS3_X86_OFFSET(RefCountView,refs,0x10);
// TS3W.exe 0x008A69B0: Adjust tertiary interface by -8 and delete the scene list.
SceneList* __fastcall F8A69B0(void*,void*,uint32_t);
// TS3W.exe 0x008A69C0: Adjust secondary interface by -4 and forward interface query.
void* __fastcall F8A69C0(void*,void*,uint32_t);
// TS3W.exe 0x008A69D0: Construct the inline retained-interface scene list.
SceneList* __fastcall F8A69D0(SceneList*,void*,void*);
// TS3W.exe 0x008A6A50: Destroy list entries and optionally link object into the free pool.
SceneList* __fastcall F8A6A50(SceneList*,void*,uint32_t);
// TS3W.exe 0x008A6AA0: Insert a retained pointer, including aliased source and growth.
void __fastcall F8A6AA0(RefVector*,void*,Ref**,Ref**);
// TS3W.exe 0x008A6BF0: Remove the first matching retained pointer; only AL is a result.
uint8_t __fastcall F8A6BF0(void*,void*,Ref*);
// TS3W.exe 0x008A6C50: Add a unique retained pointer; only AL is a result.
uint8_t __fastcall F8A6C50(void*,void*,Ref*);
// TS3W.exe 0x008A6CE0: Unsubscribe the secondary interface, returning AL=1.
uint8_t __fastcall F8A6CE0(Primitive*,void*);
// TS3W.exe 0x008A6D10: Read the enabled byte without normalizing it.
uint8_t __fastcall F8A6D10(Primitive*,void*);
// TS3W.exe 0x008A6D20: Query either supported interface ID, returning this or null.
void* __fastcall F8A6D20(void*,void*,uint32_t);
// TS3W.exe 0x008A6D50: On matching notification and key invoke the update virtual.
uint8_t __fastcall F8A6D50(void*,void*,uint32_t,const void*);
// TS3W.exe 0x008A6D90: Subscribe, clear/release the resource, resolve it, then update.
uint8_t __fastcall F8A6D90(Primitive*,void*);
// TS3W.exe 0x008A6E10: Invalidate then store a changed gain; unordered compares also update.
void __fastcall F8A6E10(GainView*,void*,float);
// TS3W.exe 0x008A6E40: Store and return the argument's low byte.
uint8_t __fastcall F8A6E40(GainView*,void*,uint32_t);
// TS3W.exe 0x008A6E50: Increment and return the non-atomic reference word with wraparound.
uint32_t __fastcall F8A6E50(RefCountView*,void*);
}
