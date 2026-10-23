#pragma once
#include "ts3/layout.h"
namespace ts3::resource::m04 {
struct WaitTimer { unsigned char prefix[20]; float factor; int64_t deadline; };
struct WaitDispatch {
    int64_t (*counter)();
    void (*sleep)(uint32_t);
};
// TS3W.exe 0x007397C0: wait until the signed deadline difference is negative, sleeping 1..100 ms; returns the low difference word.
uint32_t WaitUntil(WaitTimer*,const WaitDispatch* = nullptr);
struct CacheRegistration { void** vtable; void* database; };
// TS3W.exe 0x0073C610: stop, close and release the database ref view, then clear the singleton; null-database EAX is undefined.
uintptr_t StopCache(CacheRegistration*,void** singleton = nullptr);
// TS3W.exe 0x00738670: initialize the main manager's devices, configured databases, caches and async loader; preserve cleanup's upper EAX.
uintptr_t StartSystem(void* manager);
}
#if defined(_M_IX86)
namespace ts3::resource::m04::abi {
inline uint32_t __stdcall WaitUntil(WaitTimer* p) { return m04::WaitUntil(p); }
inline uintptr_t __fastcall StopCache(CacheRegistration* p,void*) { return m04::StopCache(p); }
inline uintptr_t __fastcall StartSystem(void* p,void*) { return m04::StartSystem(p); }
}
#endif
