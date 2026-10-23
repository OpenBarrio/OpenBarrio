#pragma once
#include "ts3/render/m07_abi.h"
#include <cstddef>
namespace ts3::render::m07::b27 {
struct Node { Node* next; Node* prev; void* value; };
struct alignas(16) Instance { float data[20]; uint8_t enabled; uint8_t tail[15]; };
struct alignas(16) Owner {
    void** table; uint8_t pad04[0x3c]; uint32_t flags40,flags44; uint8_t pad48[8];
    float box[8]; uint8_t pad70[16]; float matrix[16]; uint8_t padC0[0x150];
    uint8_t callbacks[0x78]; float* lodBoxes; uint8_t pad28C[0x44];
    void** secondary; void* instancer; Instance* begin; Instance* end; Instance* capacity; const char* tag;
    uint32_t indices[4]; uint8_t ready; uint8_t pad2F9[7]; float instanceBox[8];
    Node sentinel; void** callbackTable; void* callbackOwner; uint32_t revision;
};
TS3_X86_OFFSET(Owner,box,0x50); TS3_X86_OFFSET(Owner,callbacks,0x210);
TS3_X86_OFFSET(Owner,lodBoxes,0x288); TS3_X86_OFFSET(Owner,secondary,0x2d0);
TS3_X86_OFFSET(Owner,instanceBox,0x300); TS3_X86_OFFSET(Owner,sentinel,0x320);
TS3_X86_OFFSET(Owner,revision,0x334);
struct Dependencies {
    Thiscall<float*,Owner,float*> transform=nullptr;
    Thiscall<uintptr_t,Owner,float,void*,const float*,void*> distance4=nullptr;
    Thiscall<uintptr_t,Owner,float,const float*,void*> distance3=nullptr;
    Thiscall<Owner*,Owner,const float*> select=nullptr;
    Thiscall<uintptr_t,Owner> baseRefresh=nullptr,fixup=nullptr,baseDestroy=nullptr,finalize=nullptr;
    Thiscall<uintptr_t,void> pending=nullptr,instanceDestroy=nullptr,listDestroy=nullptr,addref=nullptr,release=nullptr;
    Thiscall<uintptr_t,Owner,uint32_t> query=nullptr;
    Thiscall<uintptr_t,void,const char*,const char*> name=nullptr;
    void (__cdecl* free)(void*)=nullptr;
    Thiscall<void,void,uint32_t> unsubscribe=nullptr;
    Thiscall<void,Owner,void*,void*> baseConstruct=nullptr;
    Thiscall<void,void,void*,uint32_t,uint32_t> subscribe=nullptr;
    void (__cdecl* find)(Node**,Node*,Node*,void*)=nullptr;
    Stdcall<uintptr_t,Node**,Node*,Node*> erase=nullptr;
    Thiscall<void,void> emptyBox=nullptr;
    Thiscall<void,void,const float*> includePoint=nullptr,includeBox=nullptr;
    Thiscall<void,Owner,const float*> setTransform=nullptr,setBox=nullptr;
    Thiscall<uint32_t,void> lodCount=nullptr;
    Thiscall<const float*,void> lodBox=nullptr;
    Thiscall<void,void,const float*> updateBounds=nullptr;
    void** systemSlot=nullptr; void** hostSlot=nullptr;
    void* (__cdecl* device)()=nullptr;
    Thiscall<uintptr_t,void> supported=nullptr;
    Thiscall<int32_t,void,uint32_t,uint32_t*,uint32_t*,uint8_t**,uint32_t*> allocate=nullptr;
    Thiscall<uintptr_t,void,uint32_t> unlock=nullptr,resize=nullptr;
    Thiscall<uintptr_t,void,uint32_t,void*,uint32_t,uint32_t> stream=nullptr;
    Thiscall<void,void,const void*> descriptor=nullptr;
    Thiscall<uintptr_t,void,const void*> submit=nullptr,fallbackSubmit=nullptr;
    Thiscall<uintptr_t,void,uint32_t,const float*> position=nullptr,color=nullptr;
    Thiscall<float*,void,const float*> convert3=nullptr;
    uintptr_t initialReturn=0;
    float* epsilon=nullptr;
    void** primaryTable=nullptr; void** secondaryTable=nullptr; void** callbackVtable=nullptr; void** baseTable=nullptr;
    const char* instanceTag=nullptr; const char* callbackTag=nullptr;
};
extern Dependencies dependencies;
// TS3W.exe 0x006F8530: forward box distance to the four-argument LOD selector; a0 is ignored.
uintptr_t __fastcall Distance4(Owner*,void*,void*,void*,const float*,void*);
// TS3W.exe 0x006F85D0: forward box distance to the three-argument LOD selector; a0 is ignored.
uintptr_t __fastcall Distance3(Owner*,void*,void*,const float*,void*);
// TS3W.exe 0x006F8930: rebuild instance and model LOD bounds with original SSE lane order.
uintptr_t __fastcall RebuildBounds(Owner*,void*,void*);
// TS3W.exe 0x006F8C70: select alternate bounds using the transformed box center.
Owner* __fastcall SelectBounds(Owner*,void*,float*,float*);
// TS3W.exe 0x006F8D60: refresh fixups and dispatch slot 38 when no pending callbacks remain.
uintptr_t __fastcall Refresh(Owner*,void*);
// TS3W.exe 0x006F8E90: assign the fixup proxy type name to the output string argument.
uintptr_t __fastcall ProxyName(void*,void*,void*);
// TS3W.exe 0x006F8EB0: submit cached or uploaded instance batches, with the original fallback path.
uintptr_t __fastcall Draw(Owner*,void*,void*,const void*,const uint32_t*);
// TS3W.exe 0x006F9290: assign the instance array type name to the output string argument.
uintptr_t __stdcall ArrayName(void*);
// TS3W.exe 0x006F9910: unsubscribe and destroy array-owned storage before the base destructor.
uintptr_t __fastcall Destroy(Owner*,void*);
// TS3W.exe 0x006F9990: adjust the secondary receiver and forward the interface query.
uintptr_t __fastcall QuerySecondary(void*,void*,uint32_t);
// TS3W.exe 0x006F99A0: adjust the secondary receiver and forward the decrement/release helper.
uintptr_t __fastcall AddRefSecondary(void*,void*);
// TS3W.exe 0x006F99B0: adjust the secondary receiver and forward the increment helper.
uintptr_t __fastcall ReleaseSecondary(void*,void*);
// TS3W.exe 0x006F99C0: remove a matching callback; only AL is defined on return.
bool __fastcall RemoveCallback(Owner*,void*,void*);
// TS3W.exe 0x006F9A10: construct the array and register its borrowed fixup callback.
Owner* __fastcall Construct(Owner*,void*,void*,void*);
}
