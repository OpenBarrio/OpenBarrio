#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::b21 {
struct Hash { Joint* buckets[13]; Joint* sentinel; uint32_t count; };
struct SkeletonView { void* vtable; Joint* begin; Joint* end; Joint* capacity; void* descriptor; Hash hash; };
struct Vector { Joint* begin; Joint* end; Joint* capacity; void* descriptor; };
struct Iterator { Joint* node; Joint** bucket; };
struct InsertResult { Joint* node; Joint** bucket; uint8_t inserted; };
struct Name { char* begin; char* end; char* capacity; void* descriptor; };
TS3_X86_OFFSET(SkeletonView,hash,0x14);
TS3_X86_OFFSET(Hash,sentinel,0x34);
TS3_X86_SIZE(Hash,0x3c);
// TS3W.exe 0x00575650: Count the skeleton's 0x70-byte joints using signed division.
int32_t Count(const SkeletonView*);
// TS3W.exe 0x00575670: Return a joint's borrowed name pointer without bounds checks.
char* JointName(const SkeletonView*,int32_t);
// TS3W.exe 0x00575690: Read the joint hash at +5C.
uint32_t JointHash(const SkeletonView*,int32_t);
// TS3W.exe 0x005756B0: Read the joint parent-chain index at +58.
int32_t ChainIndex(const SkeletonView*,int32_t);
// TS3W.exe 0x005756D0: Read +54, mapping any signed-negative input or result to -1.
int32_t MappedIndex(const SkeletonView*,int32_t);
// TS3W.exe 0x005756F0: Follow +58 until the target or a negative index, retaining upper EAX bits.
uint32_t IsAncestor(const SkeletonView*,int32_t,int32_t);
// TS3W.exe 0x00575720: Initialize a joint, leaving opaque leading and trailing bytes untouched.
Joint* ConstructJoint(Joint*);
// TS3W.exe 0x005757A0: Find a hash in one of thirteen chains, returning the sentinel iterator on miss.
Iterator* Find(Hash*,Iterator*,const uint32_t*);
// TS3W.exe 0x00575800: Insert a unique joint hash at the head of its chain; duplicates are not linked.
InsertResult* InsertHash(Hash*,InsertResult*,Joint*,uint32_t);
// TS3W.exe 0x00575B00: Find a joint hash and return its +50 index, or -1.
int32_t FindIndex(SkeletonView*,uint32_t);
// TS3W.exe 0x00575B40: Construct the opposite-side joint name with EA's case-sensitive prefix rules.
Name* MirrorName(Name*,const char*,uint32_t,int32_t);
// TS3W.exe 0x00575CA0: Destroy names in a joint range; signed capacity <=1 is not freed.
uintptr_t __stdcall FreeNames(Joint*,const Joint*);
// TS3W.exe 0x005766D0: Erase a range using forward assignment, then destroy the vacated names.
Joint* Erase(Vector*,Joint*,Joint*);
// TS3W.exe 0x00576720: Insert repeated joints, retaining the alias-safe temporary and EA growth policy.
void Insert(Vector*,Joint*,uint32_t,const Joint*);
// TS3W.exe 0x00576930: Resize a joint vector by erasing or inserting default joint records.
void Resize(Vector*,uint32_t);
}
#if defined(_M_IX86)
namespace ts3::anim::b21 {
inline int32_t __fastcall HookCount(const SkeletonView* p,void*) { return Count(p); }
inline char* __fastcall HookJointName(const SkeletonView* p,void*,int32_t n) { return JointName(p,n); }
inline uint32_t __fastcall HookJointHash(const SkeletonView* p,void*,int32_t n) { return JointHash(p,n); }
inline int32_t __fastcall HookChainIndex(const SkeletonView* p,void*,int32_t n) { return ChainIndex(p,n); }
inline int32_t __fastcall HookMappedIndex(const SkeletonView* p,void*,int32_t n) { return MappedIndex(p,n); }
inline uint32_t __fastcall HookIsAncestor(const SkeletonView* p,void*,int32_t a,int32_t b) { return IsAncestor(p,a,b); }
inline Joint* __fastcall HookConstructJoint(Joint* p,void*) { return ConstructJoint(p); }
inline Iterator* __fastcall HookFind(Hash* p,void*,Iterator* out,const uint32_t* key) { return Find(p,out,key); }
inline InsertResult* __fastcall HookInsertHash(Hash* p,void*,InsertResult* out,Joint* j,uint32_t x) { return InsertHash(p,out,j,x); }
inline int32_t __fastcall HookFindIndex(SkeletonView* p,void*,uint32_t key) { return FindIndex(p,key); }
inline Joint* __fastcall HookErase(Vector* p,void*,Joint* a,Joint* b) { return Erase(p,a,b); }
inline void __fastcall HookInsert(Vector* p,void*,Joint* a,uint32_t n,const Joint* j) { Insert(p,a,n,j); }
inline void __fastcall HookResize(Vector* p,void*,uint32_t n) { Resize(p,n); }
}
#endif
