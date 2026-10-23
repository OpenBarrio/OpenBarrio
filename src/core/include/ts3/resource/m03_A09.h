#pragma once
#include "ts3/resource/m03_layouts.h"
#include "ts3/runtime/m02_layouts.h"
#include <initializer_list>
namespace ts3::resource::m03::a09 {
// Local dependency injection: addresses identify boundary operations, not owned bodies.
using Boundary = uintptr_t(*)(unsigned,void*,std::initializer_list<uintptr_t>);
Boundary SetBoundary(Boundary);
struct Entry { void* request; uintptr_t value; bool operator==(const Entry&) const=default; };
struct Iterator { Entry* current; Entry* first; Entry* last; Entry** node; };
struct Deque { const char* name; uint32_t mapSize; Iterator begin,end; };
struct Client;
struct LoadInfo {
    void* const* vtable; uint8_t pad04[0x24]; void* factory;
    uint8_t pad2c[12]; uint32_t size; uint8_t pad3c[0x58];
    uint8_t children,flag95,flag96,mode,flag98,pending,cancelled,wait,flag9c,flag9d,failed;
    uint8_t pad9f[5]; void* request; void* job; void* child; uint8_t padb0[8];
    Client* next; Client* previous; uint8_t context[64];
};
struct Client { Client* next; Client* previous; void** cell; void* resource; LoadInfo* info; void* job; void* completion; void* context; uintptr_t state; };
struct Loader {
    void* const* vtable; Deque queue; uint8_t pad2c[8]; uint32_t budget; uint8_t pad38[8];
    uint8_t mutex[0x40]; uint8_t buckets[0x318]; Client* free;
};
struct Range { void* const* vtable; uint8_t payload[36]; };
struct Ranges { void* const* vtable; Range* begin; Range* end; Range* capacity; const char* name; };
struct CallbackManager {
    uint8_t enabled; uint8_t padding[3]; int32_t refs,active;
    void* requestNext; void* requestPrevious; const char* requestName;
    uint8_t mutex[sizeof(runtime::m02::Mutex)]; void* processNext; void* processPrevious; const char* processName;
};
TS3_X86_OFFSET(Loader,mutex,0x40); TS3_X86_OFFSET(Loader,free,0x398);
TS3_X86_OFFSET(LoadInfo,request,0xa4); TS3_X86_OFFSET(LoadInfo,next,0xb8);
TS3_X86_SIZE(Range,40);
using RefCallback=void(*)(void*);
// TS3W.exe 0x00728C30: replace a tagged client cell, retaining callback order and AL-only status.
uintptr_t Replace(Loader*,void**,void**,RefCallback,RefCallback,uint32_t,void*,void*);
// TS3W.exe 0x00728DB0: release a client cell under the loader mutex; return unlock EAX.
uintptr_t Release(Loader*,void**,RefCallback);
// TS3W.exe 0x00728DE0: insert a two-word request at a deque iterator.
Iterator* Insert(Deque*,Iterator*,Iterator,const Entry*);
// TS3W.exe 0x00728EE0: erase the nearer side of a segmented request deque.
Iterator* Erase(Deque*,Iterator*,Iterator);
// TS3W.exe 0x00729060: cancel and wait for a tagged load; preserve incidental EAX above AL.
uintptr_t Cancel(Loader*,void**);
// TS3W.exe 0x00729CD0: preload a supported resource, linking a client to its load.
uintptr_t Preload(Loader*,const Key*);
// TS3W.exe 0x0072A020: schedule child resources and raise the queue budget (wrapping u32 sum).
uint8_t Children(Loader*,LoadInfo*);
// TS3W.exe 0x0072A510: borrow the callback manager singleton.
void* Singleton();
// TS3W.exe 0x0072A5D0: initialize callback-manager queues and synchronization state.
CallbackManager* ConstructCallbacks(CallbackManager*);
// TS3W.exe 0x0072A8B0: short-circuit the range predicates, testing AL only.
uint8_t Any(Ranges*,const Key*);
// TS3W.exe 0x0072A9D0: construct an empty key-range vector.
Ranges* Construct(Ranges*);
// TS3W.exe 0x0072A9F0: destroy range elements, free backing storage and restore the base vptr.
uintptr_t Destroy(Ranges*);
// TS3W.exe 0x0072AA30: destroy ranges and optionally free self when flags bit zero is set.
Ranges* DeletingDestroy(Ranges*,uint32_t);
// TS3W.exe 0x0072AC90: exchange vector pointers in order, returning the other object.
Ranges* Swap(Ranges*,Ranges*);
#if defined(_M_IX86)
inline uintptr_t __fastcall HookReplace(Loader* s,void*,void** d,void** v,RefCallback a,RefCallback r,uint32_t i,void* c,void* x) { return Replace(s,d,v,a,r,i,c,x); }
inline uintptr_t __fastcall HookRelease(Loader* s,void*,void** c,RefCallback r) { return Release(s,c,r); }
inline Iterator* __fastcall HookInsert(Deque* s,void*,Iterator* o,Iterator i,const Entry* v) { return Insert(s,o,i,v); }
inline Iterator* __fastcall HookErase(Deque* s,void*,Iterator* o,Iterator i) { return Erase(s,o,i); }
inline uintptr_t __fastcall HookCancel(Loader* s,void*,void** c) { return Cancel(s,c); }
inline uintptr_t __fastcall HookPreload(Loader* s,void*,const Key* k) { return Preload(s,k); }
inline uint8_t __fastcall HookChildren(Loader* s,void*,LoadInfo* i) { return Children(s,i); }
inline CallbackManager* __fastcall HookConstructCallbacks(CallbackManager* s,void*) { return ConstructCallbacks(s); }
inline uint8_t __fastcall HookAny(Ranges* s,void*,const Key* k) { return Any(s,k); }
inline Ranges* __fastcall HookConstruct(Ranges* s,void*) { return Construct(s); }
inline uintptr_t __fastcall HookDestroy(Ranges* s,void*) { return Destroy(s); }
inline Ranges* __fastcall HookDeletingDestroy(Ranges* s,void*,uint32_t f) { return DeletingDestroy(s,f); }
inline Ranges* __fastcall HookSwap(Ranges* s,void*,Ranges* o) { return Swap(s,o); }
#endif
}
