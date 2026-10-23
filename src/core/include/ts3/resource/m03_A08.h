#pragma once
#include "ts3/resource/m03_layouts.h"
#include <cstdint>

namespace ts3::resource::m03::a08 {
struct Links { Links* next; Links* prev; };
struct Client { Client* next; Client* prev; void* value; };
struct Child { Child* next; Child* prev; void* reserved08; void* reserved0c; struct LoadInfo* parent; void* job; };
struct LoadInfo {
    void* vtable; uint8_t reserved04[12];
    uint32_t key[4]; uint32_t reserved20;
    void* p24; void* p28; void* p2c;
    uint8_t reserved30[24]; uint8_t request[0x48];
    int32_t state; uint8_t reserved94[7]; uint8_t flags[7]; uint8_t reserveda2[6];
    void* job; uint32_t reservedac; void* pb0; void* pb4;
    Links clients; uint8_t context[32];
};
struct Loader {
    uint8_t prefix[0x37c]; Links* sentinel; uint8_t reserved380[8];
    void* active; Client clients; Child* freeChildren;
};
TS3_X86_OFFSET(LoadInfo,request,0x48);
TS3_X86_OFFSET(LoadInfo,flags,0x9b);
TS3_X86_OFFSET(LoadInfo,job,0xa8);
TS3_X86_OFFSET(LoadInfo,clients,0xb8);
TS3_X86_OFFSET(LoadInfo,context,0xc0);
TS3_X86_OFFSET(Loader,clients,0x38c);
TS3_X86_OFFSET(Loader,freeChildren,0x398);
// An iterator consists of four POINTER-sized fields on either ABI.
struct Quad { void* cur; void* begin; void* end; void* node; };
// TS3W.exe 0x00728090: forwards three iterator quads to 00727850 and returns out.
void* ForwardA(Quad* out,Quad first,Quad last,Quad dest);
// TS3W.exe 0x00728110: forwards three iterator quads to 007278D0 and returns out.
void* ForwardB(Quad* out,Quad first,Quad last,Quad dest);
// TS3W.exe 0x00728190: initializes only the explicitly written LoadInfo fields and embedded helpers.
LoadInfo* ConstructInfo(LoadInfo*);
// TS3W.exe 0x00728440: invokes shutdown with one and replaces only AL with one.
uint32_t Shutdown(Loader*,void* ignored);
// TS3W.exe 0x00728450: finds a matching record under lock, double-retains its job, then cancels outside lock.
uintptr_t CancelJob(Loader*,const void* owner,void* key);
// TS3W.exe 0x00728500: removes a client under the mutex, inheriting unlock's return.
uintptr_t RemoveClient(Loader*,void* client);
// TS3W.exe 0x00728530: releases a child job and pushes the child onto the free list (job is not cleared).
Child* Recycle(Loader*,Child*);
// TS3W.exe 0x00728560: forwards lookup with a false low byte, returning the output iterator.
void* Find( void* map,void* out,const void* key);
// TS3W.exe 0x00728590: forwards three iterator quads through ForwardA.
void* ForwardC(Quad* out,Quad first,Quad last,Quad dest);
// TS3W.exe 0x00728610: forwards three iterator quads through ForwardB.
void* ForwardD(Quad* out,Quad first,Quad last,Quad dest);
// TS3W.exe 0x00728760: disposes a tagged context and clears its handle, unlocking around raw callbacks.
uintptr_t Dispose(Loader*,uintptr_t* handle,uintptr_t(*callback)(void*));
// TS3W.exe 0x00728810: allocates and links a client before the sentinel under lock; no allocation null check.
uintptr_t AddClient(Loader*,void* client);
// TS3W.exe 0x00728880: allocates 0x3A8 bytes and invokes the loader constructor only on success.
Loader* CreateLoader();
// TS3W.exe 0x007288B0: splats destination/first/last iterator fields, ignoring the fourth stack argument.
void* Assign(Quad*,Quad* out,const Quad* first,const Quad* last,void* unused);
// TS3W.exe 0x00728920: forwards using a temporary output; EAX is a dangling stack-temporary address.
uintptr_t AssignDiscard(Quad*,const Quad* first,const Quad* last,void* unused);
#if defined(_M_IX86)
inline LoadInfo* __fastcall HookConstructInfo(LoadInfo* s,void*) { return ConstructInfo(s); }
inline uint32_t __fastcall HookShutdown(Loader* s,void*,void* a) { return Shutdown(s,a); }
inline uintptr_t __fastcall HookCancelJob(Loader* s,void*,const void* k,void* o) { return CancelJob(s,k,o); }
inline uintptr_t __fastcall HookRemoveClient(Loader* s,void*,void* c) { return RemoveClient(s,c); }
inline Child* __fastcall HookRecycle(Loader* s,void*,Child* c) { return Recycle(s,c); }
inline void* __fastcall HookFind(void* s,void*,void* o,const void* k) { return Find(s,o,k); }
inline uintptr_t __fastcall HookDispose(Loader* s,void*,uintptr_t* h,uintptr_t(*c)(void*)) { return Dispose(s,h,c); }
inline uintptr_t __fastcall HookAddClient(Loader* s,void*,void* c) { return AddClient(s,c); }
inline void* __fastcall HookAssign(Quad* s,void*,Quad* o,const Quad* a,const Quad* b,void* u) { return Assign(s,o,a,b,u); }
inline uintptr_t __fastcall HookAssignDiscard(Quad* s,void*,const Quad* a,const Quad* b,void* u) { return AssignDiscard(s,a,b,u); }
#endif
}
