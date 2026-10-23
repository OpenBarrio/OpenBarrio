#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::m85 {
struct Chain { int32_t* begin; int32_t* end; int32_t* capacity; void* descriptor; unsigned char opaque10[0x38]; int32_t length; };
TS3_X86_OFFSET(Chain,length,0x48);
TS3_X86_SIZE(Chain,0x4c);
// TS3W.exe 0x0057B5C0: Fill uncached IK lengths, testing only AL of descendant results.
uintptr_t Update(RigResource* self);
// TS3W.exe 0x0057B870: Construct a rig resource without clearing opaque bytes.
RigResource* Construct(RigResource* self);
// TS3W.exe 0x0057B970: Delete the enclosing rig from its skeleton interface.
RigResource* DeleteSecondary(Skeleton* self,uint32_t flags);
// TS3W.exe 0x0057B980: Destroy chains, poses, skeleton and resource base in order.
uintptr_t Destroy(RigResource* self);
// TS3W.exe 0x0057BBE0: Allocate and construct a rig, returning null on allocation failure.
RigResource* Create();
// TS3W.exe 0x0057BC10: Destroy and optionally free a rig, returning its former address.
RigResource* Delete(RigResource* self,uint32_t flags);
uintptr_t __fastcall HookUpdate(RigResource* self,void*);
RigResource* __fastcall HookConstruct(RigResource* self,void*);
RigResource* __fastcall HookDeleteSecondary(Skeleton* self,void*,uint32_t flags);
uintptr_t __fastcall HookDestroy(RigResource* self,void*);
RigResource* __fastcall HookDelete(RigResource* self,void*,uint32_t flags);
}
