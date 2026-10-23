#pragma once
#include "ts3/layout.h"
namespace ts3::anim::m112 {
struct Range { unsigned char* begin; unsigned char* end; };
struct Map { void* opaque; unsigned char** buckets; size_t mask; };
struct Key { uint32_t words[4]; };
struct ClipArgs { void* host; uint32_t opaque[3]; float time; uint8_t invert, flag; uint16_t pad; void* extra; };
struct Output { void* controller; uint8_t mirrored; };
// Only observed builder fields are named; pointer widths widen on native builds.
struct Builder { unsigned char opaque[0x98]; Key key; unsigned char middle[0x14]; uint8_t mirrored; };
TS3_X86_OFFSET(Builder,mirrored,0xbc);
TS3_X86_OFFSET(ClipArgs,time,0x10);
TS3_X86_OFFSET(ClipArgs,extra,0x18);
// TS3W.exe 0x0083DAD0: Build a clip controller, preserving partial-AL status and failure cleanup.
uintptr_t BuildClip(Builder*,const ClipArgs*,Output*);
// TS3W.exe 0x0083DD40: Destroy range elements and reset end; empty ranges preserve incoming EAX.
uintptr_t Clear(Range*,uintptr_t incoming);
// TS3W.exe 0x0083DF10: Forward to the embedded builder and replace only AL with one.
uintptr_t BuildOther(void*,void*,void*);
// TS3W.exe 0x0083DF30: Destroy range elements without resetting end; empty ranges preserve EAX.
uintptr_t Destroy(Range*,uintptr_t incoming);
// TS3W.exe 0x0083DF60: Find or insert an animation name and return the record's interior value.
void* GetName(Map*,const Key*);
// TS3W.exe 0x0083E0B0: Find or insert an extended animation name with zeroed short flags.
void* GetExtendedName(Map*,const Key*);
// Entry adapters retain the otherwise unspecified incoming EAX on empty ranges.
uintptr_t __fastcall BuildOtherEntry(void*,void*,void*,void*);
void* __fastcall GetNameEntry(Map*,void*,const Key*);
void* __fastcall GetExtendedNameEntry(Map*,void*,const Key*);
uintptr_t __fastcall ClearEntry(Range*,void*);
uintptr_t __fastcall DestroyEntry(Range*,void*);
}
