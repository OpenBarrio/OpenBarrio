#include "ts3/runtime/m02_T2.h"
#define NOMINMAX
#include <windows.h>
#include <cstring>
namespace ts3::runtime::m02::t2 {
namespace {
thread_local Dispatch* dispatch=nullptr;
int Close(void* h) { return dispatch ? dispatch->close(dispatch->context,h) : CloseHandle(h); }
uint32_t ProcessorCount() {
    if(dispatch) return dispatch->processors(dispatch->context);
#if defined(_M_IX86)
    auto flags=reinterpret_cast<uint32_t*>(0x11cb744); auto count=reinterpret_cast<uint32_t*>(0x11cb740);
    if(!(*flags&1)) { *flags|=1; InterlockedExchange(reinterpret_cast<LONG*>(count),0); }
    if(!*count) { SYSTEM_INFO info{}; GetSystemInfo(&info); InterlockedExchange(reinterpret_cast<LONG*>(count),info.dwNumberOfProcessors); }
    return *count;
#else
    SYSTEM_INFO info{}; GetSystemInfo(&info); return info.dwNumberOfProcessors;
#endif
}
uintptr_t Recycle(ThreadRecord* r) { return CallGameFunction<uintptr_t,0x4e2da0>(r); }
void NameException(ThreadRecord* r) {
    ULONG_PTR info[4]={0x1000,reinterpret_cast<ULONG_PTR>(r->name),r->id,0};
#if defined(_MSC_VER)
    __try { RaiseException(0x406d1388,0,4,info); } __except(EXCEPTION_EXECUTE_HANDLER) {}
#else
    RaiseException(0x406d1388,0,4,info);
#endif
}
}
Dispatch* SetDispatch(Dispatch* d) { auto old=dispatch; dispatch=d; return old; }
ThreadParameters* ParameterDefaults(ThreadParameters* p) {
    p->reserved=0; p->stackSize=0; p->priority=0; p->idealCpu=-1;
    reinterpret_cast<uint8_t*>(&p->flags)[0]=0;
#if defined(_M_IX86)
    p->name=reinterpret_cast<const char*>(0xf97f7a);
#else
    p->name="";
#endif
    return p;
}
ThreadPool* PoolConstruct(ThreadPool* p,const void* params,bool useDefault) {
    p->active=0; p->minimum=0; p->maximum=0;
    InterlockedExchange(reinterpret_cast<LONG*>(&p->live),0); InterlockedExchange(reinterpret_cast<LONG*>(&p->busy),0);
    p->idleTimeout=0; InterlockedExchange(reinterpret_cast<LONG*>(&p->waiters),0); InterlockedExchange(reinterpret_cast<LONG*>(&p->serial),0);
    ParameterDefaults(&p->parameters);
    CallGameMethod<void,0x4e1420>(&p->condition,static_cast<void*>(nullptr),false);
    CallGameMethod<void,0x4e17e0>(&p->mutex,static_cast<void*>(nullptr),false);
    CallGameMethod<void,0x4e1880>(&p->threads); CallGameMethod<void,0x4e1b00>(&p->work);
    struct Config { int32_t minimum,maximum; uint32_t flags,timeout; ThreadParameters thread; } defaults;
    if(!params && useDefault) { defaults.minimum=0; defaults.maximum=4; defaults.flags=0; defaults.timeout=60000; ParameterDefaults(&defaults.thread); params=&defaults; }
    CallGameMethod<void,0x4e2210>(p,params); return p;
}
bool RWInitialize(RWLock* p,const uint8_t* policy) {
    if(!policy) return false;
    uint8_t parameters[20]; CallGameMethod<void,0x4e1340>(parameters,uint32_t(*policy),static_cast<void*>(nullptr));
    CallGameMethod<void,0x4e1690>(&p->mutex,parameters);
    uint8_t condition[20]; CallGameMethod<void,0x4e1340>(condition,uint32_t(*policy),static_cast<void*>(nullptr));
    CallGameMethod<void,0x4e1380>(&p->readers,condition); CallGameMethod<void,0x4e1380>(&p->writers,condition); return true;
}
RWLock* RWConstruct(RWLock* p,const uint8_t* policy,bool useDefault) {
    p->counters[0]=p->counters[1]=p->counters[2]=0; p->writer=0;
    CallGameMethod<void,0x4e17e0>(&p->mutex,static_cast<void*>(nullptr),false);
    CallGameMethod<void,0x4e1420>(&p->readers,static_cast<void*>(nullptr),true);
    CallGameMethod<void,0x4e1420>(&p->writers,static_cast<void*>(nullptr),true);
    uint8_t defaults[20]; defaults[0]=1; defaults[1]=0;
    RWInitialize(p,(!policy && useDefault)?defaults:policy); return p;
}
uintptr_t RWDestroy(RWLock* p) {
    CallGameMethod<void,0x4e14a0>(&p->writers); CallGameMethod<void,0x4e14a0>(&p->readers);
    return CallGameMethod<uintptr_t,0x4e1670>(&p->mutex);
}
SemaphoreParameters* SemaphoreDefaults(SemaphoreParameters* p,int32_t count,uint8_t local,const char* name) {
    p->count=count; p->local=local;
    if(name) { std::strncpy(p->name,name,15); p->name[15]=0; } else p->name[0]=0;
    return p;
}
uintptr_t SemaphoreDestroy(Semaphore* p) { return p->handle ? Close(p->handle) : 0; }
bool SemaphoreInitialize(Semaphore* p,const SemaphoreParameters* params) {
    if(!params || p->handle) return false;
    p->count=params->count<0 ? 0 : params->count; p->local=params->local;
    int32_t initial=p->local ? 0 : p->count;
    const char* name=p->local || !params->name[0] ? nullptr : params->name;
    p->handle=dispatch ? dispatch->createSemaphore(dispatch->context,initial,0x3fffffff,name) : CreateSemaphoreA(nullptr,initial,0x3fffffff,name);
    return p->handle!=nullptr;
}
Semaphore* SemaphoreConstruct(Semaphore* p,const SemaphoreParameters* params,bool useDefault) {
    p->handle=nullptr; p->count=0; p->waiters=0; p->local=1;
    SemaphoreParameters defaults; defaults.count=0; defaults.local=1; defaults.name[0]=0;
    SemaphoreInitialize(p,(!params && useDefault)?&defaults:params); return p;
}
Semaphore* SemaphoreCount(Semaphore* p,int32_t count) {
    p->handle=nullptr; p->count=0; p->waiters=0; p->local=1;
    SemaphoreParameters defaults; defaults.count=count; defaults.local=1; defaults.name[0]=0;
    SemaphoreInitialize(p,&defaults); return p;
}
uint32_t Poll(Thread* p,uint32_t* result) {
    auto r=p->record; if(!r) return 0;
    if(r->handle) {
        uint32_t code; int ok=dispatch ? dispatch->exitCode(dispatch->context,r->handle,&code) : GetExitCodeThread(r->handle,reinterpret_cast<DWORD*>(&code));
        if(ok) { if(code==259) return 1; Close(r->handle); r->handle=nullptr; }
    }
    if(result) *result=r->result;
    r->state=2; return 2;
}
bool Priority(Thread* p,int32_t priority) {
    if(!p->record) return false;
    auto set=[&](int32_t value) { return dispatch ? dispatch->priority(dispatch->context,p->record->handle,value) : SetThreadPriority(p->record->handle,value); };
    int ok=set(priority);
    while(!ok) {
        if(priority>=15) return set(15)!=0;
        if(priority<=-15) return set(-15)!=0;
        ok=set(priority); ++priority;
    }
    return true;
}
void DebugName(Thread* p,const char* name) {
    if(!p->record || !name) return;
    std::strncpy(p->record->name,name,64); p->record->name[63]=0;
    if(p->record->handle) { if(dispatch) dispatch->debugName(dispatch->context,p->record); else NameException(p->record); }
}
Thread* Allocate() { auto p=CallGameFunction<Thread*,0x5c32d0>(sizeof(Thread)); return Initialize(p); }
Thread* Initialize(Thread* p) { if(p) p->record=nullptr; return p; }
uintptr_t Ideal(Thread* p,int32_t cpu) {
    if(!p->record) return 0;
    auto count=ProcessorCount(); uint32_t target=cpu<0 ? 32 : (uint32_t(cpu)>=count ? uint32_t(cpu)%count : uint32_t(cpu));
    return dispatch ? dispatch->ideal(dispatch->context,p->record->handle,target) : SetThreadIdealProcessor(p->record->handle,target);
}
uintptr_t QueueApc(Thread* p) {
    if(!p->record) return 0;
    if(!p->record->handle) return reinterpret_cast<uintptr_t>(p->record);
    if(dispatch) return dispatch->apc(dispatch->context,p->record->handle);
#if defined(_M_IX86)
    return QueueUserAPC(reinterpret_cast<PAPCFUNC>(0xb9c560),p->record->handle,0);
#else
    AbortNotRewritten("m02 T2 game APC"); return 0;
#endif
}
uintptr_t Release(Thread* p) {
    auto r=p->record; if(!r) return 0;
    // XADD flags describe the SUM, not the old value returned in EDX.
    auto remaining=InterlockedDecrement(reinterpret_cast<LONG*>(&r->refs));
    return remaining==0 ? Recycle(r) : reinterpret_cast<uintptr_t>(r);
}
uintptr_t ReleaseInPlace(Thread* p) { return Release(p); }
uintptr_t Delete(Thread* p) {
    if(!p) return 0;
    Release(p); return CallGameFunction<uintptr_t,0x4e3f70>(p);
}
}

namespace ts3::runtime::m02::t2 {
ThreadPool* __fastcall PoolConstructEntry(ThreadPool* p,void*,const void* a,bool b) { return PoolConstruct(p,a,b); }
bool __fastcall RWInitializeEntry(RWLock* p,void*,const uint8_t* a) { return RWInitialize(p,a); }
RWLock* __fastcall RWConstructEntry(RWLock* p,void*,const uint8_t* a,bool b) { return RWConstruct(p,a,b); }
uintptr_t __fastcall RWDestroyEntry(RWLock* p,void*) { return RWDestroy(p); }
SemaphoreParameters* __fastcall SemaphoreDefaultsEntry(SemaphoreParameters* p,void*,int32_t a,uint8_t b,const char* c) { return SemaphoreDefaults(p,a,b,c); }
uintptr_t __fastcall SemaphoreDestroyEntry(Semaphore* p,void*) { return SemaphoreDestroy(p); }
bool __fastcall SemaphoreInitializeEntry(Semaphore* p,void*,const SemaphoreParameters* a) { return SemaphoreInitialize(p,a); }
Semaphore* __fastcall SemaphoreConstructEntry(Semaphore* p,void*,const SemaphoreParameters* a,bool b) { return SemaphoreConstruct(p,a,b); }
Semaphore* __fastcall SemaphoreCountEntry(Semaphore* p,void*,int32_t a) { return SemaphoreCount(p,a); }
ThreadParameters* __fastcall ParameterDefaultsEntry(ThreadParameters* p,void*) { return ParameterDefaults(p); }
uint32_t __fastcall PollEntry(Thread* p,void*,uint32_t* a) { return Poll(p,a); }
bool __fastcall PriorityEntry(Thread* p,void*,int32_t a) { return Priority(p,a); }
void __fastcall DebugNameEntry(Thread* p,void*,const char* a) { return DebugName(p,a); }
uintptr_t __fastcall IdealEntry(Thread* p,void*,int32_t a) { return Ideal(p,a); }
uintptr_t __fastcall QueueApcEntry(Thread* p,void*) { return QueueApc(p); }
uintptr_t __fastcall ReleaseEntry(Thread* p,void*) { return Release(p); }
}
