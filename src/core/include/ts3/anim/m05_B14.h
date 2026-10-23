#pragma once
#include "ts3/layout.h"
namespace ts3::anim::b14 {
struct Ref { void** vtable; int32_t refs; };
struct State { unsigned char prefix[0x28]; Ref* first; Ref* second; float time,weight; };
struct alignas(16) Record { float vector[4]; uint32_t key; float a,b; uint32_t untouched; };
struct Range { Record *begin,*end,*capacity; void* descriptor; };
struct alignas(16) Node { uint32_t key; unsigned char opaque[12]; float start[4],target[4],current[4]; float duration,elapsed,stamp; uint32_t reserved; Node* next; };
struct Controller { unsigned char prefix[0x28]; uint32_t unknown; Node** buckets; uint32_t count,tag; };
TS3_X86_OFFSET(State,time,0x30); TS3_X86_OFFSET(Node,next,0x50); TS3_X86_OFFSET(Controller,buckets,0x2c);
// TS3W.exe 0x0056AAC0: Returns the transform interface identifier.
uint32_t __fastcall TypeAAC0(void*,void*);
// TS3W.exe 0x0056AAD0: Queries the interface, adjusting this by minus eight.
void* __fastcall QueryAAD0(void*,void*,uint32_t);
// TS3W.exe 0x0056AC20: Returns the derived transform identifier.
uint32_t __fastcall TypeAC20(void*,void*);
// TS3W.exe 0x0056AC30: Queries three identifiers, adjusting this by minus eight.
void* __fastcall QueryAC30(void*,void*,uint32_t);
// TS3W.exe 0x0056B010: Inserts a record, preserving padding and aliased sources.
void* __fastcall Insert(Range*,void*,Record*,const Record*);
// TS3W.exe 0x0056B510: Updates timed vector interpolation in bucket order; ignores its argument.
uint32_t __fastcall Update(Controller*,void*,uint32_t);
// TS3W.exe 0x0056B6C0: Serializes transform nodes and preserves the upper EAX bytes.
uint32_t __fastcall Save(Controller*,void*,void*);
// TS3W.exe 0x0056B810: Returns the timed-vector controller identifier.
uint32_t __fastcall TypeB810(void*,void*);
// TS3W.exe 0x0056B820: Queries two identifiers, adjusting this by minus eight.
void* __fastcall QueryB820(void*,void*,uint32_t);
// TS3W.exe 0x0056B840: Returns the global default duration in ST0.
float __fastcall DefaultDuration(void*,void*);
// TS3W.exe 0x0056BC80: Tests weight against zero, rejecting unordered values.
uint32_t __fastcall Nonnegative(State*,void*);
// TS3W.exe 0x0056BCA0: Tests weight against one, rejecting unordered values.
uint32_t __fastcall Complete(State*,void*);
// TS3W.exe 0x0056BCC0: Returns transition time in ST0.
float __fastcall Time(State*,void*);
// TS3W.exe 0x0056BEA0: Releases both slots in order, clearing before callbacks and restoring last refs.
void __fastcall Reset(State*,void*);
// TS3W.exe 0x0056BF10: Resets then retains new slots and stores time and weight.
Ref* __fastcall Set(State*,void*,Ref*,Ref*,float,float);
}
