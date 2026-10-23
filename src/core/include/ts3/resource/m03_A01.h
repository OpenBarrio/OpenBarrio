#pragma once
#include "ts3/resource/m03_manager_contracts.h"

namespace ts3::resource::m03::a01 {
// Bounded prefix recovered from 004B33B0 and the A01 used-field accesses.
// Native pointer fields widen; opaque spans are not interpreted on either ABI.
struct Manager {
    void* const* vtable;
    uint8_t initialized;
    uint8_t reserved05[3];
    Allocator* allocator;
    uint8_t reserved0c[0x24];
    manager::DatabaseVector databases;
    Mutex mutex;
    uint8_t reserved78[0x28];
    manager::CallbackVector callbacks;
    manager::Controls controls;

};
TS3_X86_OFFSET(Manager,databases,0x30);
TS3_X86_OFFSET(Manager,mutex,0x48);
TS3_X86_OFFSET(Manager,callbacks,0xa0);
TS3_X86_OFFSET(Manager,controls,0xb8);
// TS3W.exe 0x004AFD20: returns the borrowed manager singleton.
Manager* Singleton();
// TS3W.exe 0x004AFD80: sets initialized to one only when zero; only AL is defined.
uint8_t Initialize(Manager*);
// TS3W.exe 0x004AFDA0: delegates slot16 with flags aliasing its stack key-pointer copy.
void* DelegateDatabase(Manager*,const Key*);
// TS3W.exe 0x004AFDC0: locks/unlocks mutex+48, testing only the argument low byte.
int32_t DatabaseLock(Manager*,uint32_t);
// TS3W.exe 0x004AFDE0: adds/removes borrowed controls; removal moves pointers, NOT types.
uint8_t Control(Manager*,uint32_t,void*,uint32_t);
// TS3W.exe 0x004AFE90: finds the first matching low-word type before the null sentinel.
void* FindControl(Manager*,uint32_t);
// TS3W.exe 0x004AFEE0: optionally copies the factor and returns the policy word.
uint32_t ReadPolicy(Manager*,float*);
// TS3W.exe 0x004AFF00: stores policy/factor, replaces +/-0 with .8, returns policy with LAHF AH.
uint32_t SetPolicy(Manager*,uint32_t,float);
// TS3W.exe 0x004AFF30: returns borrowed pointer-array storage at controls+0.
void** PointerSlots(Manager*);
// TS3W.exe 0x004AFF40: forwards parser(out,text,type,group,manager,null); AL result.
uint8_t ParseName(Manager*,ts3::misc::ResourceSortKey*,const char16_t*,uint32_t,uint32_t);
// TS3W.exe 0x004AFF70: searches database vector under mutex; AL boolean result.
uint8_t ContainsDatabase(Manager*,void*);
// TS3W.exe 0x004AFFC0: unlocks around each database slot13 probe; AL predicate, flags on hit only.
void* FindDatabase(Manager*,const Key*,uint32_t*);
// TS3W.exe 0x004B0070: probes callback slot6 in vector order, testing AL only.
manager::RawCallback* FindCallback(Manager*,const uint32_t*);
#if defined(_M_IX86)
// Exact ABI replacement adapters. Explicit fastcall dummy EDX models thiscall
// ECX + stack arguments, retaining all AL bits (no C++ bool normalization).
inline uint8_t __fastcall HookInitialize(Manager* self,void*) { return Initialize(self); }
inline void* __fastcall HookDelegateDatabase(Manager* self,void*,const Key* key) { return DelegateDatabase(self,key); }
inline int32_t __fastcall HookDatabaseLock(Manager* self,void*,uint32_t flag) { return DatabaseLock(self,flag); }
inline uint8_t __fastcall HookControl(Manager* self,void*,uint32_t flag,void* pointer,uint32_t type) { return Control(self,flag,pointer,type); }
inline void* __fastcall HookFindControl(Manager* self,void*,uint32_t type) { return FindControl(self,type); }
inline uint32_t __fastcall HookReadPolicy(Manager* self,void*,float* out) { return ReadPolicy(self,out); }
inline uint32_t __fastcall HookSetPolicy(Manager* self,void*,uint32_t policy,float factor) { return SetPolicy(self,policy,factor); }
inline void** __fastcall HookPointerSlots(Manager* self,void*) { return PointerSlots(self); }
inline uint8_t __fastcall HookParseName(Manager* self,void*,ts3::misc::ResourceSortKey* out,const char16_t* text,uint32_t type,uint32_t group) { return ParseName(self,out,text,type,group); }
inline uint8_t __fastcall HookContainsDatabase(Manager* self,void*,void* database) { return ContainsDatabase(self,database); }
inline void* __fastcall HookFindDatabase(Manager* self,void*,const Key* key,uint32_t* flags) { return FindDatabase(self,key,flags); }
inline manager::RawCallback* __fastcall HookFindCallback(Manager* self,void*,const uint32_t* type) { return FindCallback(self,type); }
#endif
}
