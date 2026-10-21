// M02 native views. Pointer fields grow on x64; offsets describe only the x86 image.
#pragma once
#include "ts3/layout.h"
#include <array>
namespace ts3::runtime::m02 {
struct Link { Link* next; Link* prev; };
struct List { void* reserved; Link sentinel; uint32_t count; };
struct Mutex {
    void* criticalSection[6]; uint32_t auxiliary18, auxiliary1c, recursion;
    uint8_t csMode, pad25[3]; uint32_t auxiliary28, pad2c;
};
struct Semaphore { void* handle; uint32_t count, maximum, reserved; };
struct Condition { uint32_t counters[3]; Semaphore semaphores[2]; uint32_t pad2c; Mutex mutex; };
struct RWLock { uint32_t counters[3]; uintptr_t writer; Mutex mutex; Condition readers, writers; };
struct ThreadRecord {
    void* handle; uint32_t id, state, result; void* function; void* argument;
    void* tlsHandle; void* wrapper; uint32_t refs; char name[64];
};
struct Thread { ThreadRecord* record; };
struct ThreadParameters { uint32_t reserved, stackSize; int32_t priority, idealCpu; uint32_t flags; const char* name; };
struct Work { void* function; void* argument; void* context; uint32_t flags; };
struct ThreadInfo { uint8_t running, stop, pad[2]; Thread* thread; void* pool; Work work; };
struct ThreadPool {
    uint8_t active, pad[3]; int32_t minimum, maximum, live, busy;
    uint32_t idleTimeout, waiters, serial; ThreadParameters parameters;
    Condition condition; Mutex mutex; List threads, work;
};
struct Scheduler {
    void** vtable; uint32_t reserved04; Mutex mutex;
    Link queues[11]; void* reservedJob; Link lists[2]; uint32_t reservedA4, state, trailing;
};
struct Job {
    Link links[2]; void* callback; uint8_t reserved14[16]; uint32_t state;
    uintptr_t worker; uint32_t reserved2c, refs; Scheduler* scheduler;
    Link dependencies[2]; uint32_t reserved48; uint8_t flags[3], pad4f;
};
// Public handles point at the SECOND link, not at the allocation base.
inline void* JobView(Job* p) { return p ? static_cast<void*>(&p->links[1]) : nullptr; }
inline Job* JobRecord(void* p) { return p ? reinterpret_cast<Job*>(static_cast<uint8_t*>(p)-offsetof(Job,links)-sizeof(Link)) : nullptr; }
struct JobService {
    void** vtable; uint8_t reserved04[12]; void** secondary;
    uint32_t reserved14; Scheduler scheduler; Job* current; uint32_t config;
    uint8_t flag, pad[3];
};
struct StopWatch { uint64_t start, accumulated; uint32_t mode; float factor; };
struct DeadlineTimer { StopWatch watch; uint64_t deadline; };
struct Registration { void** vtable; };
TS3_X86_SIZE(Mutex,0x30); TS3_X86_OFFSET(Mutex,recursion,0x20); TS3_X86_OFFSET(Mutex,csMode,0x24);
TS3_X86_SIZE(Condition,0x60); TS3_X86_OFFSET(Condition,semaphores,0x0c); TS3_X86_OFFSET(Condition,mutex,0x30);
TS3_X86_SIZE(RWLock,0x100); TS3_X86_OFFSET(RWLock,mutex,0x10); TS3_X86_OFFSET(RWLock,readers,0x40); TS3_X86_OFFSET(RWLock,writers,0xa0);
TS3_X86_SIZE(ThreadRecord,100); TS3_X86_OFFSET(ThreadRecord,refs,0x20); TS3_X86_OFFSET(ThreadRecord,name,0x24);
TS3_X86_OFFSET(ThreadRecord,function,0x10); TS3_X86_OFFSET(ThreadRecord,tlsHandle,0x18);
TS3_X86_SIZE(ThreadParameters,0x18); TS3_X86_OFFSET(ThreadParameters,name,0x14);
TS3_X86_SIZE(ThreadInfo,0x1c); TS3_X86_OFFSET(ThreadInfo,work,0xc);
TS3_X86_SIZE(ThreadPool,0xe8); TS3_X86_OFFSET(ThreadPool,parameters,0x20); TS3_X86_OFFSET(ThreadPool,condition,0x38);
TS3_X86_OFFSET(ThreadPool,mutex,0x98); TS3_X86_OFFSET(ThreadPool,threads,0xc8); TS3_X86_OFFSET(ThreadPool,work,0xd8);
TS3_X86_SIZE(Scheduler,0xb0); TS3_X86_OFFSET(Scheduler,mutex,8); TS3_X86_OFFSET(Scheduler,queues,0x38);
TS3_X86_OFFSET(Scheduler,reservedJob,0x90); TS3_X86_OFFSET(Scheduler,lists,0x94); TS3_X86_OFFSET(Scheduler,state,0xa8);
TS3_X86_SIZE(Job,0x50); TS3_X86_OFFSET(Job,callback,0x10); TS3_X86_OFFSET(Job,state,0x24); TS3_X86_OFFSET(Job,refs,0x30);
TS3_X86_OFFSET(Job,scheduler,0x34); TS3_X86_OFFSET(Job,dependencies,0x38); TS3_X86_OFFSET(Job,flags,0x4c);
TS3_X86_SIZE(JobService,0xd4); TS3_X86_OFFSET(JobService,secondary,0x10); TS3_X86_OFFSET(JobService,scheduler,0x18); TS3_X86_OFFSET(JobService,current,0xc8);
TS3_X86_SIZE(StopWatch,0x18); TS3_X86_OFFSET(StopWatch,accumulated,8); TS3_X86_OFFSET(StopWatch,mode,0x10); TS3_X86_OFFSET(StopWatch,factor,0x14);
TS3_X86_SIZE(DeadlineTimer,0x20);
// Explicit dispatch: no implicit fallback to a live OS/game on an unsupported call.
struct AllocatorDispatch {
    void* context;
    void* (*allocate)(void*,size_t,size_t,unsigned family);
    void (*free)(void*,void*,unsigned family);
};
struct PlatformDispatch {
    void* context;
    uint32_t (*tick)(void*);
    bool (*counter)(void*,int64_t*);
    bool (*frequency)(void*,int64_t*);
    uint64_t (*rdtsc)(void*);
    uint32_t (*wait)(void*,void*,uint32_t,bool);
};
}
