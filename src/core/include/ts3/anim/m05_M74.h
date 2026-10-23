#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::m74 {
// Typed refinement of the scaffold's opaque skeleton hash/name records.
struct Name {
    uint32_t untouched;
    char* begin;
    char* end;
    char* capacity;
    void* descriptor;
};
struct Skeleton {
    void* vtable;
    Joint* begin;
    Joint* end;
    Joint* capacity;
    void* descriptor;
    Joint* buckets[13];
    Joint* sentinel;
    uint32_t count;
    Name name;
};
TS3_X86_OFFSET(Skeleton,buckets,0x14);
TS3_X86_OFFSET(Skeleton,sentinel,0x48);
TS3_X86_OFFSET(Skeleton,count,0x4c);
TS3_X86_OFFSET(Skeleton,name,0x50);
TS3_X86_OFFSET(Name,begin,4);
TS3_X86_SIZE(Skeleton,0x64);
// TS3W.exe 0x00575470: Hash a nullable joint name, returning zero for null.
uint32_t __cdecl HashName(const char* name);
// TS3W.exe 0x00575490: Find the first left/right token and return its byte offset; FLn/FRn return the second byte.
int32_t __cdecl ClassifyName(const char* name, int32_t* side);
// TS3W.exe 0x00575E30: Find a joint by its case-folded hash, returning its index or -1.
int32_t __fastcall FindJoint(Skeleton* self, void*, const char* name);
// TS3W.exe 0x00575E80: Rebuild intrusive hash buckets and joint indices; duplicate hashes keep the first joint.
void __fastcall Rebuild(Skeleton* self, void*);
// TS3W.exe 0x00575F50: Initialize a skeleton, preserving the first name word and installing EA's sentinels.
Skeleton* __fastcall Construct(Skeleton* self, void*);
// TS3W.exe 0x00575FB0: Destroy names and joint storage without resetting the vector or string pointers.
void __fastcall Destruct(Skeleton* self, void*);
}
