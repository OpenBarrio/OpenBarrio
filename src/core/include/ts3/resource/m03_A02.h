#pragma once
#include "ts3/resource/m03_layouts.h"
namespace ts3::resource::m03::a02 {
// Primary resource prefix: refcount is a scalar, key alignment grows naturally on x64.
struct ResourcePrefix { void* vtable; uint32_t refs; Key key; };
TS3_X86_OFFSET(ResourcePrefix,key,8);
struct Pair { void* object; uintptr_t tag; };
struct Manager { void* vtable; uint8_t opaque[0x9c]; Pair* begin; Pair* end; };
struct TreeNode { TreeNode *left,*right,*parent; uint32_t color; uint32_t key[4]; };
struct Tree { uint32_t unused; TreeNode *sentinelLeft,*sentinelRight,*root; };
struct Hash { uint32_t unused; void** buckets; uint32_t count; uint8_t opaque[16]; Allocator* allocator; unsigned flags; const char* name; };
TS3_X86_OFFSET(Manager,begin,0xa0);
TS3_X86_OFFSET(Tree,root,0xc);
TS3_X86_OFFSET(TreeNode,key,0x10);
TS3_X86_OFFSET(Hash,allocator,0x1c);
// External nonvirtual leaves only; null uses the original x86 bridge.
struct Leaves { void* (*lookup)(Manager*,const void*)=nullptr; void (*lock)(void*)=nullptr; void (*unlock)(void*)=nullptr; void (*rename)(void*,const void*)=nullptr; };
// TS3W.exe 0x004B00C0: route resource notification; direct path preserves raw AL, scan returns 0/1.
uint8_t Notify(Manager*,const void* key,unsigned flags,const Leaves& = {});
// TS3W.exe 0x004B0130: broadcast slot 10, returning the last callback result (self for empty).
uintptr_t Broadcast(Manager*,void* target,void* argument);
// TS3W.exe 0x004B0180: find exact mixed signed/unsigned tree key and write iterator.
TreeNode** Find(Tree*,TreeNode** out,const uint32_t* key);
// TS3W.exe 0x004B0220: release each nonnull adjusted +4 reference in a pair range.
void ReleaseAdjusted(Pair*,Pair*);
// TS3W.exe 0x004B0250: release slot 2 references at pair stride, adapted from 006F5970.
void ReleasePairs(Pair*,Pair*);
// TS3W.exe 0x004B0280: rehash nodes whose chain pointer is at +1C.
void Rehash28(Hash*,uint32_t);
// TS3W.exe 0x004B0330: rehash nodes whose chain pointer is at +14.
void Rehash20(Hash*,uint32_t);
// TS3W.exe 0x004B03E0: copy pairs and AddRef the adjusted +4 interface.
Pair** CopyAdjusted(Pair** out,Pair* begin,Pair* end,Pair* destination);
// TS3W.exe 0x004B0430: copy pairs and AddRef slot 1 on the primary interface.
Pair** CopyPairs(Pair** out,Pair* begin,Pair* end,Pair* destination);
// TS3W.exe 0x004B0480: lock, resolve database/factory, create resource, and optionally notify; preserve raw factory AL.
uint8_t Load(Manager*,unsigned notify,const uint32_t* key,void** out,unsigned flags,void* db,void* factory,const void* alias,const Leaves& = {});
// TS3W.exe 0x004B0810: forward the six arguments to Load with notifications disabled.
uint8_t LoadQuiet(Manager*,const uint32_t*,void**,unsigned,void*,void*,const void*,const Leaves& = {});
// TS3W.exe 0x004B0840: save resource using explicit or embedded key; preserve raw factory AL.
uint8_t Save(Manager*,void* resource,unsigned flags,void* db,void* factory,const uint32_t* key);

// ABI-compatible replace entries (fastcall dummy EDX models thiscall).
// TS3W.exe 0x004B00C0: ABI entry for the corresponding batch implementation.
uint8_t __fastcall NotifyEntry(Manager* m,void*,const void* k,unsigned f);
// TS3W.exe 0x004B0130: ABI entry for the corresponding batch implementation.
uintptr_t __fastcall BroadcastEntry(Manager* m,void*,void* t,void* a);
// TS3W.exe 0x004B0180: ABI entry for the corresponding batch implementation.
TreeNode** __fastcall FindEntry(Tree* t,void*,TreeNode** o,const uint32_t* k);
// TS3W.exe 0x004B0220: ABI entry for the corresponding batch implementation.
void __stdcall ReleaseAdjustedEntry(Pair* b,Pair* e);
// TS3W.exe 0x004B0250: ABI entry for the corresponding batch implementation.
void __stdcall ReleasePairsEntry(Pair* b,Pair* e);
// TS3W.exe 0x004B0280: ABI entry for the corresponding batch implementation.
void __fastcall Rehash28Entry(Hash* h,void*,uint32_t n);
// TS3W.exe 0x004B0330: ABI entry for the corresponding batch implementation.
void __fastcall Rehash20Entry(Hash* h,void*,uint32_t n);
// TS3W.exe 0x004B03E0: ABI entry for the corresponding batch implementation.
Pair** __cdecl CopyAdjustedEntry(Pair** o,Pair* b,Pair* e,Pair* d);
// TS3W.exe 0x004B0430: ABI entry for the corresponding batch implementation.
Pair** __cdecl CopyPairsEntry(Pair** o,Pair* b,Pair* e,Pair* d);
// TS3W.exe 0x004B0480: ABI entry for the corresponding batch implementation.
uint8_t __fastcall LoadEntry(Manager* m,void*,unsigned n,const uint32_t* k,void** o,unsigned f,void* d,void* fac,const void* a);
// TS3W.exe 0x004B0810: ABI entry for the corresponding batch implementation.
uint8_t __fastcall LoadQuietEntry(Manager* m,void*,const uint32_t* k,void** o,unsigned f,void* d,void* fac,const void* a);
// TS3W.exe 0x004B0840: ABI entry for the corresponding batch implementation.
uint8_t __fastcall SaveEntry(Manager* m,void*,void* r,unsigned f,void* d,void* fac,const uint32_t* k);
}
