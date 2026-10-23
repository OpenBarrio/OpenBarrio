#pragma once
#include "ts3/layout.h"
namespace ts3::anim::b33 {
struct Builder {
    char16_t *begin,*end,*capacity; unsigned char gap[4]; char16_t storage[64];
    uint32_t values[9]; uint8_t flag,enabled;
};
TS3_X86_OFFSET(Builder,storage,0x10);
TS3_X86_OFFSET(Builder,values,0x90);
TS3_X86_OFFSET(Builder,flag,0xb4);
TS3_X86_OFFSET(Builder,enabled,0xb5);
struct BuilderObject { void** vtable; int32_t refs; Builder builder; };
TS3_X86_OFFSET(BuilderObject,builder,8);
struct Effect { char *begin,*end,*capacity; void* descriptor; uint32_t hash; };
TS3_X86_OFFSET(Effect,hash,0x10);
struct Ref { void** vtable; int32_t refs; };
struct Node { uint32_t key; Ref* value; Node* next; };
struct Table { void* opaque; Node** buckets; uint32_t count,size; unsigned char policy[16]; };
struct Result { Node* node; Node** bucket; uint8_t inserted; };
struct Link { Link* next; Link* previous; Ref* value; };
struct Pool { Link* first; Link* previous; Link* free; };
struct Owner { unsigned char prefix[0x8c]; Table table; };
TS3_X86_OFFSET(Ref,refs,4);
TS3_X86_OFFSET(Node,value,4);
TS3_X86_OFFSET(Node,next,8);
TS3_X86_OFFSET(Table,buckets,4);
TS3_X86_OFFSET(Table,count,8);
TS3_X86_OFFSET(Table,size,0xc);
TS3_X86_OFFSET(Table,policy,0x10);
TS3_X86_OFFSET(Result,inserted,8);
TS3_X86_OFFSET(Pool,free,8);
TS3_X86_OFFSET(Owner,table,0x8c);
struct StringNode { uint32_t key; unsigned char gap[12]; char* begin; char* end; char* capacity; void* descriptor; StringNode* next; };
TS3_X86_OFFSET(StringNode,next,0x20);
// TS3W.exe 0x0083CDC0: Write a byte in temporary stack storage and return the index.
uint32_t __fastcall StackIndex(void*,void*,uint32_t);
// TS3W.exe 0x0083CDE0: Return the original type identifier.
uint32_t __fastcall TypeA(void*,void*);
// TS3W.exe 0x0083CDF0: Read effect name and hash from a description.
uint8_t __fastcall ReadEffect(void*,void*,void*,Effect*);
// TS3W.exe 0x0083CE90: Find an entry and return its value address, or null.
void* __fastcall FindValue(Table*,void*,uint32_t);
// TS3W.exe 0x0083D630: Insert a unique hash node, retaining the policy and rehash ordering.
Result* __fastcall Insert(Table*,void*,Result*,const uint32_t*,uintptr_t);
// TS3W.exe 0x0083D7C0: Release list payloads and recycle links without resetting the sentinel.
void __fastcall Recycle(Pool*,void*);
// TS3W.exe 0x0083D920: Assign a range of 0x7c-byte records and return the destination end.
void* Copy(const void*,const void*,void*);
// TS3W.exe 0x0083D960: Initialize an empty inline builder, preserving uninitialized bytes.
Builder* __fastcall Init(Builder*,void*);
// TS3W.exe 0x0083D9D0: Initialize a named builder from four words and a byte flag.
Builder* __fastcall Construct(Builder*,void*,const uint32_t*,const char16_t*,uint8_t);
// TS3W.exe 0x0083DAB0: Return the byte at offset 0xbd (AL only).
uint8_t __fastcall Flag(const BuilderObject*,void*);
// TS3W.exe 0x0083DAC0: Return the original builder type identifier.
uint32_t __fastcall TypeB(void*,void*);
// TS3W.exe 0x0083DBD0: Assign a builder, including both state bytes; allow self assignment.
Builder* __fastcall Assign(Builder*,void*,const Builder*);
// TS3W.exe 0x0083DDE0: Free string nodes in every bucket and clear the buckets.
void __fastcall Clear(void*,void*,StringNode**,uint32_t);
// TS3W.exe 0x0083E020: Find or insert a null-valued node and return its value address.
Ref** __fastcall Get(Table*,void*,const uint32_t*);
// TS3W.exe 0x0083E2F0: Add a named reference only when the name is not already present.
uint8_t __fastcall Add(Owner*,void*,const char16_t*,Ref*);
}
