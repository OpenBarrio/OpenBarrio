#pragma once
#include "ts3/render/m07_abi.h"
#include "ts3/render/m07_layouts.h"
#include "ts3/ui/clu_6e8410_UI.h"
namespace ts3::render::m07::b28 {
struct Link { Link* next; Link* prev; };
using Ref = ResourceHeader;
struct Objects {
 void** table; int32_t refs; uint8_t pad08[8]; void* owner; uint32_t d14;
 Link children; void* begin; void* end; void* capacity; const char* tag;
 uint32_t selector,d34,d38,d3c; uint64_t handle; uint32_t d48,d4c; Ref* resource;
};
struct alignas(16) SceneObject {
 void** table; int32_t refs; uint8_t pad08[8]; void* context; void* owner;
 Link link18,link20; uint32_t d28,d2c; void* p30; void* p34; void* p38; uint32_t d3c;
 uint32_t d40,d44,d48,d4c; uint8_t bounds[0x30];
 float matrix[16],transform[16]; ts3::ui::UiIdentity identity; uint8_t pad150[0x40];
 SceneObject* parent; Link children; uint32_t d19c,d1a0; uint8_t pad1a4[12];
 uint32_t a,b,c; float f1bc,f1c0,f1c4; uint8_t pad1c8[8];
 float color[4],alpha,scale,inherited; void* registration; uint32_t d1f0,d1f4;
 void* rigsBegin; void* rigsEnd; void* rigsCapacity; const char* rigsTag; uint8_t flags,flags2,pad20a[6];
};
TS3_X86_OFFSET(Objects,resource,0x50);
TS3_X86_OFFSET(SceneObject,matrix,0x80);
TS3_X86_OFFSET(SceneObject,parent,0x190);
TS3_X86_OFFSET(SceneObject,flags,0x208);
struct Node { Node* next; Node* prev; void* value; };
struct Instance { void** table; void* emulator; uint8_t* begin; uint8_t* end; void* capacity; uint32_t d14; uint32_t names[4]; uint8_t pad28[0x28]; Node list; };
struct BindOwner { void** table; uint8_t opaque[0x31c]; Node list; };
TS3_X86_OFFSET(Instance,list,0x50); TS3_X86_OFFSET(BindOwner,list,0x320);
struct Dependencies {
 void* (__cdecl* manager)()=nullptr;
 Thiscall<uint64_t,void,Objects*> insert=nullptr;
 Thiscall<void,void,uint64_t> erase=nullptr;
 uintptr_t (__cdecl* free)(void*)=nullptr;
 Thiscall<void,void,void*> flush=nullptr;
 void (__cdecl* inverse)(float*,const float*,void*)=nullptr;
 Thiscall<uintptr_t,SceneObject> notify=nullptr;
 Thiscall<uintptr_t,SceneObject,float,float> opacity=nullptr;
 Thiscall<void,SceneObject> update=nullptr;
 Thiscall<void*,SceneObject,void*> getMatrix=nullptr;
 Thiscall<uintptr_t,void,void*> setBounds=nullptr;
 Thiscall<void,SceneObject,int,ts3::ui::UiIdentity*> detach=nullptr;
 Thiscall<void,void,SceneObject*> unregister=nullptr;
 Thiscall<uintptr_t,void> clearRigs=nullptr;
 void (__cdecl* find)(Node**,Node*,Node*,uint32_t)=nullptr;
 Thiscall<void,Node> append=nullptr;
 uintptr_t (__cdecl* finish)(Node*,Node*,Node*,Node*)=nullptr;
};
extern Dependencies dependencies;
struct InstanceModel { Model base; uint8_t pad30[0x10]; void** meshes; uint8_t pad44[0x2e]; uint8_t instanced; };
TS3_X86_OFFSET(InstanceModel,meshes,0x40); TS3_X86_OFFSET(InstanceModel,instanced,0x72);
// TS3W.exe 0x006F9C50: prepare an instanced model and run retained fixups or the instance emulator.
uint8_t __fastcall Prepare(Instance*,void*,InstanceModel*,InstanceModel**,int,int);
// TS3W.exe 0x006F9B80: replace a retained material-fixup callback in the intrusive list.
uintptr_t __fastcall Bind(BindOwner*,void*,void*,uint32_t);
// TS3W.exe 0x006FA880: initialize a scene-node object collection, leaving padding untouched.
Objects* __fastcall ConstructObjects(Objects*,void*);
// TS3W.exe 0x006FA8F0: destroy a scene-node collection and release its last resource reference.
uintptr_t __fastcall DestroyObjects(Objects*,void*);
// TS3W.exe 0x006FAAD0: flush and visit objects; EAX is undefined when owner is null.
uintptr_t __fastcall VisitObjects(Objects*,void*);
// TS3W.exe 0x006FAB30: destroy and optionally free a collection, returning this.
Objects* __fastcall DeleteObjects(Objects*,void*,uint32_t);
// TS3W.exe 0x006FABF0: query the two scene-object interface identifiers.
void* __fastcall Query(SceneObject*,void*,uint32_t);
// TS3W.exe 0x006FB280: set a pair of identifiers and invalidate only when changed.
uint32_t __fastcall SetPair(SceneObject*,void*,uint32_t,uint32_t);
// TS3W.exe 0x006FB2C0: set the third identifier and invalidate only when changed.
uint32_t __fastcall SetThird(SceneObject*,void*,uint32_t);
// TS3W.exe 0x006FCFC0: rebase the local matrix using ordered SSE multiplication.
uintptr_t __fastcall Rebase(SceneObject*,void*,const float*);
// TS3W.exe 0x006FD320: recompute inherited opacity, stopping at the first non-inheriting ancestor.
uintptr_t __fastcall Opacity(SceneObject*,void*);
// TS3W.exe 0x006FD6B0: update bounds and notify children in intrusive order.
uintptr_t __fastcall Update(SceneObject*,void*);
// TS3W.exe 0x006FD710: initialize a scene object, preserving uninitialized fields.
SceneObject* __fastcall ConstructScene(SceneObject*,void*,void*,void*);
// TS3W.exe 0x006FD930: detach children, unregister and unlink a scene object.
uintptr_t __fastcall DestroyScene(SceneObject*,void*);
}
