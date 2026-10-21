#pragma once
#include "ts3/runtime/m02_threads.h"
namespace ts3::runtime::m02::t1 {
struct Parameters { uint8_t mode; char name[16]; };
struct Registry { void* handles[300]; void* critical[6]; };
struct PoolParameters { int32_t minimum, maximum, initial; uint32_t timeout; ThreadParameters thread; };
struct Payload { uint32_t serial; void* function; void* argument; void* context; };
struct ThreadNode { ThreadInfo* value; ThreadNode* prev; ThreadNode* next; };
struct WorkNode { Payload value; WorkNode* prev; WorkNode* next; };
// TS3W.exe 0x004E10E0: Reap completed duplicated handles, or all handles when forced.
void Reap(Registry*, uint8_t);
// TS3W.exe 0x004E1160: Add or remove the first matching duplicated handle.
void RegisterHandle(Registry*, void*, uint8_t);
// TS3W.exe 0x004E1200: Lazily allocate TLS and update registry ownership.
uint32_t SetTls(void*, uint8_t);
// TS3W.exe 0x004E1320: Perform an alertable relative sleep (not an absolute deadline).
uint32_t Sleep(const uint32_t*);
// TS3W.exe 0x004E1340: Initialize mode and a truncated name; null leaves the name tail untouched.
Parameters* InitParameters(Parameters*, uint8_t, const char*);
// TS3W.exe 0x004E1380: Initialize both condition semaphores and its mutex, without rollback.
bool InitCondition(Condition*, const Parameters*);
// TS3W.exe 0x004E1420: Construct a condition, optionally using default parameters.
Condition* ConstructCondition(Condition*, const Parameters*, uint8_t);
// TS3W.exe 0x004E14A0: Destroy condition mutex then semaphores in reverse order.
void DestroyCondition(Condition*);
// TS3W.exe 0x004E1590: Signal one or all condition waiters, preserving signed counter comparisons.
bool Signal(Condition*, uint8_t);
// TS3W.exe 0x004E1670: Destroy the selected mutex representation.
void DestroyMutex(Mutex*);
// TS3W.exe 0x004E1690: Initialize a CS or named kernel mutex; null parameters do nothing.
bool InitMutex(Mutex*, const Parameters*);
// TS3W.exe 0x004E17E0: Construct a mutex, optionally initializing the default CS.
Mutex* ConstructMutex(Mutex*, const Parameters*, uint8_t);
// TS3W.exe 0x004E1850: Initialize thread-pool defaults.
PoolParameters* InitPoolParameters(PoolParameters*);
// TS3W.exe 0x004E1880: Allocate and cross-link two thread-list sentinels.
List* ConstructThreads(List*);
// TS3W.exe 0x004E18E0: Unlink/free pending work; an empty list retains its count.
WorkNode* ClearWork(List*);
// TS3W.exe 0x004E1920: Unlink/free thread nodes, without destroying their payloads.
ThreadNode* ClearThreads(List*);
// TS3W.exe 0x004E1B00: Allocate work-list sentinels, leaving their serials untouched.
List* ConstructWork(List*);
// TS3W.exe 0x004E1B80: Append a copied work payload before the tail sentinel.
WorkNode* AppendWork(List*, const Payload*);
// TS3W.exe 0x004E1F30: Restart workers whose join state is exactly two.
uintptr_t Restart(ThreadPool*);
// TS3W.exe 0x004E2020: Increment the work serial before submitting, including failed submissions.
int32_t Submit(ThreadPool*, void*, void*, void*, int32_t);
}

namespace ts3::runtime::m02::t1 {
// ABI entry adapters: dummy EDX makes these compatible with x86 thiscall.
inline void __fastcall ReapEntry(Registry* p, void*, uint8_t a) { return Reap(p,a); }
inline void __fastcall RegisterHandleEntry(Registry* p, void*, void* h, uint8_t a) { return RegisterHandle(p,h,a); }
inline Parameters* __fastcall InitParametersEntry(Parameters* p, void*, uint8_t a, const char* b) { return InitParameters(p,a,b); }
inline bool __fastcall InitConditionEntry(Condition* p, void*, const Parameters* a) { return InitCondition(p,a); }
inline Condition* __fastcall ConstructConditionEntry(Condition* p, void*, const Parameters* a, uint8_t b) { return ConstructCondition(p,a,b); }
inline void __fastcall DestroyConditionEntry(Condition* p, void*) { return DestroyCondition(p); }
inline bool __fastcall SignalEntry(Condition* p, void*, uint8_t a) { return Signal(p,a); }
inline void __fastcall DestroyMutexEntry(Mutex* p, void*) { return DestroyMutex(p); }
inline bool __fastcall InitMutexEntry(Mutex* p, void*, const Parameters* a) { return InitMutex(p,a); }
inline Mutex* __fastcall ConstructMutexEntry(Mutex* p, void*, const Parameters* a, uint8_t b) { return ConstructMutex(p,a,b); }
inline PoolParameters* __fastcall InitPoolParametersEntry(PoolParameters* p, void*) { return InitPoolParameters(p); }
inline List* __fastcall ConstructThreadsEntry(List* p, void*) { return ConstructThreads(p); }
inline WorkNode* __fastcall ClearWorkEntry(List* p, void*) { return ClearWork(p); }
inline ThreadNode* __fastcall ClearThreadsEntry(List* p, void*) { return ClearThreads(p); }
inline List* __fastcall ConstructWorkEntry(List* p, void*) { return ConstructWork(p); }
inline WorkNode* __fastcall AppendWorkEntry(List* p, void*, const Payload* a) { return AppendWork(p,a); }
inline uintptr_t __fastcall RestartEntry(ThreadPool* p, void*) { return Restart(p); }
inline int32_t __fastcall SubmitEntry(ThreadPool* p, void*, void* a, void* b, void* c, int32_t d) { return Submit(p,a,b,c,d); }
}
