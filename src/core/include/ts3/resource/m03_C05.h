#pragma once
#include <cstdint>
#include <cstddef>
namespace ts3::resource::m03::c05 {
struct Pool;
struct PoolVtable {
    void* unused;
    void* (__fastcall* allocate5)(Pool*, void*, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t);
    void* (__fastcall* allocate3)(Pool*, void*, uintptr_t, uintptr_t, uintptr_t);
};
struct Pool { PoolVtable* vtable; };
struct CallbackOwner { uint8_t prefix[0x74]; void* callback; };
struct Loader { uint8_t prefix[0x7c]; void* value; };
struct Cache { uint8_t prefix[0x44]; void* value; };
struct RefObject { void* vtable; int32_t packed; };
struct Callback {
    void* vtable; uintptr_t reserved;
    uintptr_t (__fastcall* function)(void*,void*);
    uint8_t* context;
};
#if defined(_M_IX86)
static_assert(offsetof(CallbackOwner,callback)==0x74);
static_assert(offsetof(Loader,value)==0x7c);
static_assert(offsetof(Cache,value)==0x44);
static_assert(offsetof(RefObject,packed)==4);
static_assert(offsetof(Callback,function)==8 && offsetof(Callback,context)==12);
#endif
// x64 service owner supplies the native table; never truncate a native pointer.
extern const void* nativeRegistrationTable;
// TS3W.exe 0x00402010: Forwards only arguments 1, 2 and 5 to allocator slot2.
void* __fastcall Allocate5(Pool* self, void* dummy, uintptr_t a, uintptr_t b, uintptr_t ignoredC, uintptr_t ignoredD, uintptr_t e);
// TS3W.exe 0x00402030: Forwards arguments 1, 2, 5, 6 and 7 to allocator slot1.
void* __fastcall Allocate7(Pool* self, void* dummy, uintptr_t a, uintptr_t b, uintptr_t ignoredC, uintptr_t ignoredD, uintptr_t e, uintptr_t f, uintptr_t g);
// TS3W.exe 0x004026B0: Returns zero in EDX:EAX.
uint64_t __fastcall Zero64(void*, void*, uintptr_t);
// TS3W.exe 0x0044DB00: Returns false in AL; upper EAX is unspecified.
uint8_t __fastcall DriverFalse(void*, void*, uintptr_t, uintptr_t, uintptr_t);
// TS3W.exe 0x00504220: Stores and returns the callback pointer at +74.
void* __fastcall SetCallback(CallbackOwner* self, void*, void* value);
// TS3W.exe 0x00533560: Returns the borrowed pointer at +7C.
void* __fastcall LoaderValue(Loader* self, void*);
// TS3W.exe 0x00548EB0: Returns false in AL; upper EAX is unspecified.
uint8_t __fastcall FactoryFalse(void*, void*, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t);
// TS3W.exe 0x0057A960: Returns signed packed reference state arithmetic-shifted by eight.
int32_t __fastcall RefCount(RefObject* self, void*);
// TS3W.exe 0x0058D900: Forwards to slot1 with alignment 4 and offset zero.
void* __fastcall Allocate3(Pool* self, void*, uintptr_t a, uintptr_t b, uintptr_t c);
// TS3W.exe 0x005FB4E0: Returns the interior address +2C without dereferencing.
void* __fastcall KeyAddress(void* self, void*);
// TS3W.exe 0x006119F0: Returns full EAX zero.
uint32_t __fastcall DriverZero(void*, void*, uintptr_t, uintptr_t);
// TS3W.exe 0x00619EF0: Returns full EAX one.
uint32_t __fastcall DatabaseOne(void*, void*);
// TS3W.exe 0x006C2940: Invokes the stored thiscall function with context plus argument; returns its EAX.
uintptr_t __fastcall Invoke(Callback* self, void*, intptr_t offset);
// TS3W.exe 0x006C93D0: Returns the borrowed pointer at +44.
void* __fastcall CacheValue(Cache* self, void*);
// TS3W.exe 0x00743780: Returns the localized-string native registration table.
const void* __fastcall RegistrationTable(void*, void*);
}
