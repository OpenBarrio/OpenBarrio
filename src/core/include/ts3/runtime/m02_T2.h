#pragma once
#include "ts3/runtime/m02_threads.h"
namespace ts3::runtime::m02::t2 {
struct SemaphoreParameters { int32_t count; uint8_t local; char name[16]; uint8_t pad[3]; };
struct Semaphore { void* handle; int32_t count, waiters; uint8_t local, pad[3]; };
// Optional per-thread platform seam; nullptr restores the production Win32/bridge path.
struct Dispatch {
    void* context;
    void* (*createSemaphore)(void*,int32_t,int32_t,const char*);
    int (*close)(void*,void*);
    int (*exitCode)(void*,void*,uint32_t*);
    int (*priority)(void*,void*,int32_t);
    uint32_t (*ideal)(void*,void*,uint32_t);
    uint32_t (*processors)(void*);
    uintptr_t (*apc)(void*,void*);
    void (*debugName)(void*,ThreadRecord*);
};
Dispatch* SetDispatch(Dispatch*);
// TS3W.exe 0x004E2320: construct a pool, optionally using the default configuration.
ThreadPool* PoolConstruct(ThreadPool*,const void*,bool);
// TS3W.exe 0x004E23E0: initialize RW mutex and both conditions from the policy byte.
bool RWInitialize(RWLock*,const uint8_t*);
// TS3W.exe 0x004E25B0: construct the RW members then initialize the selected policy.
RWLock* RWConstruct(RWLock*,const uint8_t*,bool);
// TS3W.exe 0x004E2620: destroy writer condition, reader condition, then mutex.
uintptr_t RWDestroy(RWLock*);
// TS3W.exe 0x004E26A0: construct semaphore parameters, truncating names to 15 characters.
SemaphoreParameters* SemaphoreDefaults(SemaphoreParameters*,int32_t,uint8_t,const char*);
// TS3W.exe 0x004E26E0: close a nonnull handle without clearing it.
uintptr_t SemaphoreDestroy(Semaphore*);
// TS3W.exe 0x004E26F0: initialize a semaphore; local mode creates zero kernel permits.
bool SemaphoreInitialize(Semaphore*,const SemaphoreParameters*);
// TS3W.exe 0x004E28C0: construct a semaphore with optional default parameters.
Semaphore* SemaphoreConstruct(Semaphore*,const SemaphoreParameters*,bool);
// TS3W.exe 0x004E2920: construct a local semaphore with an explicit initial count.
Semaphore* SemaphoreCount(Semaphore*,int32_t);
// TS3W.exe 0x004E2960: initialize thread parameters, writing only the flag's low byte.
ThreadParameters* ParameterDefaults(ThreadParameters*);
// TS3W.exe 0x004E2A70: poll native completion, closing finished handles and publishing result.
uint32_t Poll(Thread*,uint32_t*);
// TS3W.exe 0x004E2AF0: try priority then walk upward or clamp to the extreme priority.
bool Priority(Thread*,int32_t);
// TS3W.exe 0x004E2B70: copy a debug name and raise the debugger naming exception under SEH.
void DebugName(Thread*,const char*);
// TS3W.exe 0x004E2C10: allocate a nullable zeroed Thread facade.
Thread* Allocate();
// TS3W.exe 0x004E2C30: zero a nullable caller-owned Thread facade.
Thread* Initialize(Thread*);
// TS3W.exe 0x004E2C50: set ideal CPU modulo cached processor count; negatives map to 32.
uintptr_t Ideal(Thread*,int32_t);
// TS3W.exe 0x004E2D10: queue the game's APC on a nonnull native handle.
uintptr_t QueueApc(Thread*);
// TS3W.exe 0x004E2E10: decrement the record reference, recycling when the xadd result is zero.
uintptr_t Release(Thread*);
// TS3W.exe 0x004E3310: release a caller-owned facade without clearing its record pointer.
uintptr_t ReleaseInPlace(Thread*);
// TS3W.exe 0x004E3330: release an allocated nullable facade and free the facade itself.
uintptr_t Delete(Thread*);
}

namespace ts3::runtime::m02::t2 {
// TS3W.exe 0x004E2320: ABI adapter for PoolConstruct.
ThreadPool* __fastcall PoolConstructEntry(ThreadPool* p,void*,const void* a,bool b);
// TS3W.exe 0x004E23E0: ABI adapter for RWInitialize.
bool __fastcall RWInitializeEntry(RWLock* p,void*,const uint8_t* a);
// TS3W.exe 0x004E25B0: ABI adapter for RWConstruct.
RWLock* __fastcall RWConstructEntry(RWLock* p,void*,const uint8_t* a,bool b);
// TS3W.exe 0x004E2620: ABI adapter for RWDestroy.
uintptr_t __fastcall RWDestroyEntry(RWLock* p,void*);
// TS3W.exe 0x004E26A0: ABI adapter for SemaphoreDefaults.
SemaphoreParameters* __fastcall SemaphoreDefaultsEntry(SemaphoreParameters* p,void*,int32_t a,uint8_t b,const char* c);
// TS3W.exe 0x004E26E0: ABI adapter for SemaphoreDestroy.
uintptr_t __fastcall SemaphoreDestroyEntry(Semaphore* p,void*);
// TS3W.exe 0x004E26F0: ABI adapter for SemaphoreInitialize.
bool __fastcall SemaphoreInitializeEntry(Semaphore* p,void*,const SemaphoreParameters* a);
// TS3W.exe 0x004E28C0: ABI adapter for SemaphoreConstruct.
Semaphore* __fastcall SemaphoreConstructEntry(Semaphore* p,void*,const SemaphoreParameters* a,bool b);
// TS3W.exe 0x004E2920: ABI adapter for SemaphoreCount.
Semaphore* __fastcall SemaphoreCountEntry(Semaphore* p,void*,int32_t a);
// TS3W.exe 0x004E2960: ABI adapter for ParameterDefaults.
ThreadParameters* __fastcall ParameterDefaultsEntry(ThreadParameters* p,void*);
// TS3W.exe 0x004E2A70: ABI adapter for Poll.
uint32_t __fastcall PollEntry(Thread* p,void*,uint32_t* a);
// TS3W.exe 0x004E2AF0: ABI adapter for Priority.
bool __fastcall PriorityEntry(Thread* p,void*,int32_t a);
// TS3W.exe 0x004E2B70: ABI adapter for DebugName.
void __fastcall DebugNameEntry(Thread* p,void*,const char* a);
// TS3W.exe 0x004E2C50: ABI adapter for Ideal.
uintptr_t __fastcall IdealEntry(Thread* p,void*,int32_t a);
// TS3W.exe 0x004E2D10: ABI adapter for QueueApc.
uintptr_t __fastcall QueueApcEntry(Thread* p,void*);
// TS3W.exe 0x004E2E10: ABI adapter for Release.
uintptr_t __fastcall ReleaseEntry(Thread* p,void*);
}
