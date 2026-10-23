#pragma once
#include "ts3/layout.h"
namespace ts3::anim::m113 {
struct Ref { void** vtable; int32_t refs; };
struct Result { Ref* value; uint8_t flag; };
struct Entry { Ref* value; float phase; };
struct Node { Node* next; Node* previous; Ref* child; float phase; };
struct Builder { void** vtable; int32_t refs; Node sentinel; unsigned char opaque[0x224-8-16]; float cached; };
struct Range { unsigned char* begin; unsigned char* end; unsigned char* capacity; const char* descriptor; };
struct Owner { void* vtable; unsigned char* data; uint32_t mode; uint32_t cleared; unsigned char opaque[16]; Range records; };
TS3_X86_OFFSET(Builder,cached,0x224);
TS3_X86_OFFSET(Owner,records,0x20);
// TS3W.exe 0x0083E380: Build a phase-selected controller, retaining signed-zero ordering.
uint8_t __fastcall Build(Builder*,void*,void**,Result*);
// TS3W.exe 0x0083E6F0: Allocate a base builder and replace a retained reference.
uint8_t __cdecl Create(Entry*,const float*);
// TS3W.exe 0x0083E780: Update matching child builders or insert a new builder.
uint8_t __fastcall Update(Builder*,void*,void**,uint32_t);
// TS3W.exe 0x0083EB00: Insert a forty-byte record with alias adjustment and geometric growth.
uintptr_t __fastcall Insert(Range*,void*,unsigned char*,unsigned char*);
// TS3W.exe 0x0083ED50: Assign twenty-byte records, retaining the null-destination overflow branch.
uintptr_t __fastcall Assign(Range*,void*,unsigned char*,unsigned char*,uintptr_t);
// TS3W.exe 0x0083EE50: Destroy record and inline ranges in order without clearing dangling pointers.
uintptr_t __fastcall Destroy(Owner*,void*);
}
