#pragma once
#include "ts3/layout.h"
namespace ts3::resource {
struct ThrottlePath { ThrottlePath* next; char text[1]; };
struct ThrottleHandle { void* file; uint32_t magic; };
// RWFileSys base header plus the driver's shared (not per-handle) cursor.
struct ThrottledDriver {
    void** vtable;
    ThrottlePath* paths;
    uint32_t base8, baseC;
    uint64_t position;
    float latency, perByte;
    // TS3W.exe 0x00739480: forward base path unregistration, preserving its incidental EAX.
    uintptr_t Unregister();
    // TS3W.exe 0x00739490: seek shared cursor; only negative relative seeks saturate at zero.
    uint64_t Seek(ThrottleHandle*,int64_t,uint32_t,uint32_t,uint32_t);
    // TS3W.exe 0x00739570: return underlying file size without checking handle magic.
    uint64_t Size(ThrottleHandle*);
    // TS3W.exe 0x00739590: forward file resize and preserve the full helper result.
    uint32_t Resize(ThrottleHandle*,uint32_t,uint32_t);
    // TS3W.exe 0x007395B0: forward underlying file status without validating magic.
    uint32_t Status(ThrottleHandle*);
    // TS3W.exe 0x007395D0: configure timer then start rounded single-precision byte delay.
    uintptr_t StartDelay(void*,uint32_t);
    // TS3W.exe 0x00739620: construct base and reciprocal bandwidth; cursor remains uninitialized.
    ThrottledDriver* Construct(void*,float,float);
    // TS3W.exe 0x00739660: strip first matching prefix and open, closing on wrapper allocation failure.
    ThrottleHandle* Open(const char*,uint32_t,uint32_t);
    // TS3W.exe 0x00739750: close valid handle then poison magic and free wrapper.
    uintptr_t Close(ThrottleHandle*);
    // TS3W.exe 0x00739860: scalar deleting destructor returns this, freeing iff flags bit zero is set.
    ThrottledDriver* Destroy(uint32_t);
    // TS3W.exe 0x00739890: synchronous throttled read; cursor advances by full 64-bit completion.
    uint32_t Read(ThrottleHandle*,void*,uint32_t,uint32_t,uint32_t);
    // TS3W.exe 0x00739940: synchronous throttled write; invalid magic returns zero without side effects.
    uint32_t Write(ThrottleHandle*,void*,uint32_t,uint32_t,uint32_t);
};
TS3_X86_OFFSET(ThrottledDriver,position,0x10);
TS3_X86_OFFSET(ThrottledDriver,perByte,0x1c);
TS3_X86_OFFSET(ThrottleHandle,magic,4);
}

#if defined(_M_IX86)
namespace ts3::resource::c03_abi {
// Fastcall adapters occupy ECX/EDX and pop exactly the original thiscall stack.
// TS3W.exe 0x00739480: ABI adapter for ThrottledDriver::Unregister.
inline uintptr_t __fastcall Unregister(ThrottledDriver* self,void*) { return self->Unregister(); }
// TS3W.exe 0x00739490: ABI adapter for ThrottledDriver::Seek.
inline uint64_t __fastcall Seek(ThrottledDriver* self,void*,ThrottleHandle* h,int64_t offset,uint32_t origin,uint32_t a,uint32_t b) { return self->Seek(h,offset,origin,a,b); }
// TS3W.exe 0x00739570: ABI adapter for ThrottledDriver::Size.
inline uint64_t __fastcall Size(ThrottledDriver* self,void*,ThrottleHandle* h) { return self->Size(h); }
// TS3W.exe 0x00739590: ABI adapter for ThrottledDriver::Resize.
inline uint32_t __fastcall Resize(ThrottledDriver* self,void*,ThrottleHandle* h,uint32_t lo,uint32_t hi) { return self->Resize(h,lo,hi); }
// TS3W.exe 0x007395B0: ABI adapter for ThrottledDriver::Status.
inline uint32_t __fastcall Status(ThrottledDriver* self,void*,ThrottleHandle* h) { return self->Status(h); }
// TS3W.exe 0x007395D0: ABI adapter for ThrottledDriver::StartDelay.
inline uintptr_t __fastcall StartDelay(ThrottledDriver* self,void*,void* timer,uint32_t bytes) { return self->StartDelay(timer,bytes); }
// TS3W.exe 0x00739620: ABI adapter for ThrottledDriver::Construct.
inline ThrottledDriver* __fastcall Construct(ThrottledDriver* self,void*,void* name,float latency,float bandwidth) { return self->Construct(name,latency,bandwidth); }
// TS3W.exe 0x00739660: ABI adapter for ThrottledDriver::Open.
inline ThrottleHandle* __fastcall Open(ThrottledDriver* self,void*,const char* path,uint32_t mode,uint32_t a) { return self->Open(path,mode,a); }
// TS3W.exe 0x00739750: ABI adapter for ThrottledDriver::Close.
inline uintptr_t __fastcall Close(ThrottledDriver* self,void*,ThrottleHandle* h) { return self->Close(h); }
// TS3W.exe 0x00739860: ABI adapter for ThrottledDriver::Destroy.
inline ThrottledDriver* __fastcall Destroy(ThrottledDriver* self,void*,uint32_t flags) { return self->Destroy(flags); }
// TS3W.exe 0x00739890: ABI adapter for ThrottledDriver::Read.
inline uint32_t __fastcall Read(ThrottledDriver* self,void*,ThrottleHandle* h,void* buffer,uint32_t n,uint32_t a,uint32_t b) { return self->Read(h,buffer,n,a,b); }
// TS3W.exe 0x00739940: ABI adapter for ThrottledDriver::Write.
inline uint32_t __fastcall Write(ThrottledDriver* self,void*,ThrottleHandle* h,void* buffer,uint32_t n,uint32_t a,uint32_t b) { return self->Write(h,buffer,n,a,b); }
}
#endif
