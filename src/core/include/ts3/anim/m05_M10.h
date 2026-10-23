#pragma once
#include "ts3/anim/m05_B01.h"
namespace ts3::anim::m10 {
struct Entry { Entry* next; Entry* previous; uint32_t key; b01::Node* evaluator; };
struct Blend { b01::Node base; Entry* first; Entry* last; Entry* free; };
TS3_X86_OFFSET(Blend,first,0x24);
struct Context { unsigned char opaque[0x40]; void* rig; unsigned char unknown[0x10]; uint8_t flags; };
struct Pose { void** vtable; int32_t refs; unsigned char opaque[0x34]; float* weights; };
TS3_X86_OFFSET(Context,flags,0x54);
TS3_X86_OFFSET(Pose,weights,0x3c);
// TS3W.exe 0x00554980: Evaluate weighted children, stopping after a child with no mask; empty-list EAX is unspecified.
void Evaluate(Blend*,Context*,Pose*);
// TS3W.exe 0x00554D30: Save list count, keys and evaluator payloads in list order.
void Save(Blend*,b01::Cursor*);
// TS3W.exe 0x00554E60: Initialize base and pooled list, preserving sibling links.
Blend* Construct(Blend*,void*);
// TS3W.exe 0x00554E90: Recycle list entries, reset base and optionally free; returns this even after free.
Blend* Destroy(Blend*,uint32_t);
// TS3W.exe 0x00554F60: Append deserialized entries without clearing the list.
void Load(Blend*,b01::Cursor*);
// TS3W.exe 0x00554FF0: Return class identifier 0x87eb0331 (near twin differs only in immediate).
uint32_t Type(void*);
#if defined(_M_IX86)
void __fastcall HookEvaluate(Blend*,void*,Context*,Pose*);
void __fastcall HookSave(Blend*,void*,b01::Cursor*);
Blend* __fastcall HookConstruct(Blend*,void*,void*);
Blend* __fastcall HookDestroy(Blend*,void*,uint32_t);
void __fastcall HookLoad(Blend*,void*,b01::Cursor*);
uint32_t __fastcall HookType(void*,void*);
#endif
}
