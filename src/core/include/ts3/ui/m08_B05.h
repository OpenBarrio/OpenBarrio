#pragma once
#include <cstdint>
#include <cstddef>
namespace ts3::ui::m08::b05 {
struct Factory { void** table; uint32_t word; uint8_t opaque[0x20]; volatile long exchange; };
struct Channel { void* focus; uint8_t opaque004[4]; void* capture; uint8_t opaque00C[0x20]; };
struct Manager {
    void** table; uint8_t opaque004[0xDC]; uint32_t flags;
    uint8_t opaque0E4[0x5CC]; Channel channels[10]; uint8_t opaque868[12]; void* value874;
};
struct TreeWindow {
    uint8_t opaque000[0x2C]; TreeWindow* parent; uint8_t opaque030[0x40];
    uint8_t flags070; uint8_t opaque071[0x187]; uint8_t flags1F8,flags1F9;
};
struct PendingVector { uintptr_t *begin,*end,*cap; const char* name; };
struct Pending { uint8_t mutex[0x30]; PendingVector vector; };
// Injectable native boundary; defaults use verified dependencies or typed original bridges.
struct PendingOps {
    void (*construct)(Pending*); void (*atExit)(); void (*lock)(Pending*); void (*unlock)(Pending*);
    void (*reserve)(PendingVector*,size_t); void (*free)(void*);
};
struct PendingEnvironment { Pending** slot; uint32_t* flags; Pending* storage; const PendingOps* ops; const char* name; const char* tempName; };
#if defined(_M_IX86)
static_assert(offsetof(Factory,exchange)==0x28);
static_assert(sizeof(Channel)==0x2C && offsetof(Manager,channels)==0x6B0 && offsetof(Manager,value874)==0x874);
static_assert(offsetof(TreeWindow,flags1F8)==0x1F8);
static_assert(offsetof(Pending,vector)==0x30);
#endif
// TS3W.exe 0x0050A2A0: Return the factory's constant interface identifier.
uint32_t ClassId();
// TS3W.exe 0x0050A520: Install the factory table and exchange its reference word to zero.
Factory* Construct(Factory*);
// TS3W.exe 0x0050A6A0: Lazily initialize the pending texture singleton; test only bit zero.
Pending* GetPending();
Pending* GetPending(PendingEnvironment&);
// TS3W.exe 0x0050A860: Swap pending storage under the lock, then dispatch tagged callbacks outside it.
uint8_t Drain(Pending*);
uint8_t Drain(Pending*,const PendingOps&,const char*);
// TS3W.exe 0x0050A920: Obtain and drain the pending texture singleton.
uint8_t DrainGlobal();
// TS3W.exe 0x0050A990: Forward one argument to slot six, prepending two zero arguments.
uintptr_t Forward(Manager*,uintptr_t);
// TS3W.exe 0x0050A9B0: Return a channel's adjusted focus interface, or null.
void* Focus(Manager*,int32_t);
// TS3W.exe 0x0050A9D0: Return all ten focus match bits, including null matches.
uint32_t FocusMask(Manager*,void*);
// TS3W.exe 0x0050AA70: Return a channel's adjusted capture interface, or null.
void* Capture(Manager*,int32_t);
// TS3W.exe 0x0050AA90: Clear capture via slot 25 only when the unadjusted pointer matches.
uintptr_t ClearMatch(Manager*,int32_t,void*,uintptr_t);
// TS3W.exe 0x0050AAD0: Clear mask then optionally set it, using only the low byte of the boolean.
uint32_t SetFlags(Manager*,uint32_t,uint32_t);
// TS3W.exe 0x0050AB00: Test whether any requested flag is set.
uint32_t HasFlags(Manager*,uint32_t);
// TS3W.exe 0x0050AB20: Propagate dirty state to ancestors, stopping on an already marked ancestor.
void Dirty(TreeWindow*);
// TS3W.exe 0x0050AB60: Return the borrowed pointer at manager offset 874.
void* Value(Manager*);
// TS3W.exe 0x0050AB70: Return the manager singleton without creating it.
Manager* Singleton();
#if defined(_M_IX86)
inline Factory* __fastcall HookConstruct(Factory* p,void*) { return Construct(p); }
inline uint8_t __fastcall HookDrain(Pending* p,void*) { return Drain(p); }
inline uintptr_t __fastcall HookForward(Manager* p,void*,uintptr_t a) { return Forward(p,a); }
inline void* __fastcall HookFocus(Manager* p,void*,int32_t i) { return Focus(p,i); }
inline uint32_t __fastcall HookFocusMask(Manager* p,void*,void* t) { return FocusMask(p,t); }
inline void* __fastcall HookCapture(Manager* p,void*,int32_t i) { return Capture(p,i); }
inline uintptr_t __fastcall HookClearMatch(Manager* p,void*,int32_t i,void* t,uintptr_t a) { return ClearMatch(p,i,t,a); }
inline uint32_t __fastcall HookSetFlags(Manager* p,void*,uint32_t m,uint32_t a) { return SetFlags(p,m,a); }
inline uint32_t __fastcall HookHasFlags(Manager* p,void*,uint32_t m) { return HasFlags(p,m); }
inline void __stdcall HookDirty(TreeWindow* p) { Dirty(p); }
inline void* __fastcall HookValue(Manager* p,void*) { return Value(p); }
inline Pending* HookGetPending() { return GetPending(); }
#endif
}
