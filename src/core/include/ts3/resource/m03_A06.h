#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::resource::m03::a06 {
struct Record16 { uint32_t words[4]; };
struct Pair32 { uint32_t first,second; };
struct Group { Record16* records; uintptr_t* children; uint32_t count; };
struct Context {
    void* begin; void* end; void* capacity; void* allocator;
    uint32_t count,childCount; Pair32* pairs; uint32_t flags; void* extra;
    Group groups[4];
};
struct Buffer { uint8_t* begin; uint8_t* end; uint8_t* capacity; void* allocator; };
struct InitData {
    void* begin; void* end; void* capacity; void* allocator;
    uint32_t untouched10; void* field14; void* field18; uint8_t flag; uint8_t padding[3]; void* field20;
};
struct LoadPrefix { void* vtable; uint32_t refs; Record16 key,other; };
TS3_X86_OFFSET(Context,count,0x10);
TS3_X86_OFFSET(Context,pairs,0x18);
TS3_X86_OFFSET(Context,groups,0x24);
TS3_X86_OFFSET(InitData,field20,0x20);
TS3_X86_OFFSET(LoadPrefix,key,8);
// Native dispatch debt: allocation families, lazy materialization and borrowed singleton.
struct Dispatch {
    uintptr_t (*ref)(void*,unsigned)=nullptr;
    uintptr_t (*freePairs)(void*)=nullptr;
    uintptr_t (*freeCore)(void*)=nullptr;
    void* (*allocate)(size_t,void*)=nullptr;
    uint8_t (*materialize)(Context*,uint32_t)=nullptr;
    void* singleton=nullptr;
    void* initAllocator=nullptr;
};
Dispatch* SetDispatch(Dispatch*);
// TS3W.exe 0x007260F0: replaces a child, addref before releasing old, inverse low-byte ownership tag.
void SetChild(Context*,uint32_t,void*,uint32_t);
// TS3W.exe 0x00726140: gets a pair; EAX retains output-pointer high bits (index high bits on failure).
uintptr_t GetPair(Context*,uint32_t,uint32_t*,uint32_t*);
// TS3W.exe 0x00726170: decodes a typed one-based pair index; types 2/3 fail after range check.
uintptr_t GetTypedPair(Context*,uint32_t,uint32_t*,uint32_t*);
// TS3W.exe 0x00726270: returns indexed record or the lazily zeroed first fallback.
Record16* GetRecord(Context*,uint32_t,uint32_t);
// TS3W.exe 0x007263C0: initializes only specified fields, leaving +10 and padding untouched.
InitData* Initialize(InitData*);
// TS3W.exe 0x007263E0: releases tagged children only when pair storage exists, frees pairs, resets end.
uintptr_t Clear(Context*);
// TS3W.exe 0x00726430: decodes typed record handle, using a distinct invalid-type fallback.
Record16* GetTypedRecord(Context*,uint32_t);
// TS3W.exe 0x00726670: clears then frees begin; leaves the begin pointer unchanged.
uintptr_t Destroy(Context*);
// TS3W.exe 0x00726690: materializes tagged children and reloads the pointer after successful AL return.
void* GetChild(Context*,uint32_t);
// TS3W.exe 0x00726A60: assigns byte range, reallocating only above capacity; third argument unused.
uint8_t* Assign(Buffer*,const uint8_t*,const uint8_t*,uint32_t);
// TS3W.exe 0x00727220: returns the borrowed mutex at byte offset 0x40.
void* MutexAddress(void*);
// TS3W.exe 0x00727250: returns self for either supported interface ID, otherwise null.
void* QueryInterface(void*,uint32_t);
// TS3W.exe 0x00727280: returns the borrowed async-loader singleton.
void* Singleton();
// TS3W.exe 0x007272A0: copies the first key in load prefix, returning the output pointer.
Record16* CopyKey(LoadPrefix*,Record16*);
// TS3W.exe 0x007272C0: copies the second key in load prefix, returning the output pointer.
Record16* CopyOther(LoadPrefix*,Record16*);
#if defined(_M_IX86)
#define A06_HOOK0(R,N,T) inline R __fastcall Hook##N(T* s,void*) { return N(s); }
A06_HOOK0(InitData*,Initialize,InitData)
A06_HOOK0(uintptr_t,Clear,Context)
A06_HOOK0(uintptr_t,Destroy,Context)
A06_HOOK0(void*,MutexAddress,void)
#undef A06_HOOK0
inline void __fastcall HookSetChild(Context*s,void*,uint32_t i,void*p,uint32_t f){SetChild(s,i,p,f);}
inline uintptr_t __fastcall HookGetPair(Context*s,void*,uint32_t i,uint32_t*a,uint32_t*b){return GetPair(s,i,a,b);}
inline uintptr_t __fastcall HookGetTypedPair(Context*s,void*,uint32_t i,uint32_t*a,uint32_t*b){return GetTypedPair(s,i,a,b);}
inline Record16* __fastcall HookGetRecord(Context*s,void*,uint32_t i,uint32_t t){return GetRecord(s,i,t);}
inline Record16* __fastcall HookGetTypedRecord(Context*s,void*,uint32_t i){return GetTypedRecord(s,i);}
inline void* __fastcall HookGetChild(Context*s,void*,uint32_t i){return GetChild(s,i);}
inline uint8_t* __fastcall HookAssign(Buffer*s,void*,const uint8_t*a,const uint8_t*b,uint32_t c){return Assign(s,a,b,c);}
inline void* __fastcall HookQueryInterface(void*s,void*,uint32_t i){return QueryInterface(s,i);}
inline Record16* __fastcall HookCopyKey(LoadPrefix*s,void*,Record16*out){return CopyKey(s,out);}
inline Record16* __fastcall HookCopyOther(LoadPrefix*s,void*,Record16*out){return CopyOther(s,out);}
#endif
}
