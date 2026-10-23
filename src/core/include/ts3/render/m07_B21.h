#pragma once
#include "ts3/render/m07_abi.h"
namespace ts3::render::m07::b21 {
struct Resource { void** table; uint8_t prefix[0x24]; void* payload; };
struct Derived { Resource base; uint8_t gap[0x54]; void* extra; };
struct Entry { uint32_t words[4]; void* resource; uint32_t tag; };
struct Interface { void** table; void* resource; uint32_t words[4]; Entry* entries; };
struct Model { void** table; uint8_t prefix[0x3c]; Interface iface; uint8_t vector[16]; };
struct StreamView { uint8_t gap[8]; void* data; };
struct Stream { void** table; StreamView* view; };
struct Temp { void* begin; void* end; void* capacity; uint32_t tag; };
struct Dependencies {
 Thiscall<int32_t,void,uint32_t> find=nullptr;
 Thiscall<int32_t,void> addRef=nullptr,release=nullptr;
 Thiscall<void*,void,uint32_t> query=nullptr;
 Thiscall<int32_t,void> retain=nullptr;
 Thiscall<void,void> baseModel=nullptr,baseDerived=nullptr,baseResource=nullptr,vectorDestroy=nullptr;
 Thiscall<uint8_t,void,void*> enqueue=nullptr;
 Thiscall<void*,Temp,uint32_t,const char**> temp=nullptr;
 void* (__cdecl* factory)()=nullptr;
 Thiscall<uint8_t,void,void*,uint32_t,void**> create=nullptr;
 Thiscall<void,void,uint32_t> notify=nullptr;
 void (__cdecl* free)(void*)=nullptr;
 void (__cdecl* swap)(void*,uint32_t)=nullptr;
 void* manager=nullptr; bool injectedManager=false;
 void** modelTable=nullptr; void** interfaceTable=nullptr; void** baseTable=nullptr;
};
extern Dependencies dependencies;
// TS3W.exe 0x006D8AD0: Copy a default or named four-word model descriptor (AL result).
uint8_t __fastcall Descriptor(Interface*,void*,uint32_t,uint32_t*);
// TS3W.exe 0x006D8B40: Acquire a default or named resource; a failed lookup leaves output untouched.
uint8_t __fastcall Acquire(Interface*,void*,uint32_t,void**);
// TS3W.exe 0x006D8BA0: Queue resource disposal or dispatch immediate cleanup (AL result).
uint8_t __fastcall Dispose(Resource*,void*);
// TS3W.exe 0x006D8C80: Adjust model interface receiver before inherited AddRef.
int32_t __fastcall ModelAddRef(Interface*,void*);
// TS3W.exe 0x006D8C90: Adjust model interface receiver before inherited query.
void* __fastcall ModelQuery(Interface*,void*,uint32_t);
// TS3W.exe 0x006D8CA0: Adjust model interface receiver before deleting destruction.
void* __fastcall ModelDelete(Interface*,void*,uint32_t);
// TS3W.exe 0x006D8CB0: Adjust model interface receiver before inherited release.
int32_t __fastcall ModelRelease(Interface*,void*);
// TS3W.exe 0x006D8CC0: Free derived payload, destruct base, optionally free self, return self.
void* __fastcall DerivedDelete(Derived*,void*,uint32_t);
// TS3W.exe 0x006D8D00: Decode model vertex elements, replace format resource and notify on success.
uint8_t __fastcall Decode(Resource*,void*,Stream*,void*,uint8_t);
// TS3W.exe 0x006D8EA0: Destruct model vector, release resource, destruct base and optionally free self.
void* __fastcall DestroyModel(Model*,void*,uint32_t);
// TS3W.exe 0x006D91F0: Query software vertex-buffer interfaces; no AddRef.
void* __fastcall VertexQuery(Resource*,void*,uint32_t);
// TS3W.exe 0x006D9230: Free software buffer payload, destruct base and optionally free self.
void* __fastcall BufferDelete(Resource*,void*,uint32_t);
// TS3W.exe 0x006D9290: Query software index-buffer interfaces; no AddRef.
void* __fastcall IndexQuery(Resource*,void*,uint32_t);
// TS3W.exe 0x006D92E0: Queue format disposal or clear and release owned resource (AL result).
uint8_t __fastcall FormatDispose(Resource*,void*);
// TS3W.exe 0x006D9310: Release format resource, destruct base and optionally free self.
void* __fastcall FormatDelete(Resource*,void*,uint32_t);
TS3_X86_OFFSET(Resource,payload,0x28);
TS3_X86_OFFSET(Derived,extra,0x80);
TS3_X86_OFFSET(Model,iface,0x40);

TS3_X86_OFFSET(Interface,entries,0x18);
}
