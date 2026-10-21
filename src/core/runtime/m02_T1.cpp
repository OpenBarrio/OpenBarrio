#include "ts3/runtime/m02_T1.h"
#include <cstring>
#include <intrin.h>
namespace ts3::runtime::m02::t1 {
namespace {
// Use the game's import cells, not host imports: the exehost Scope interposes
// these cells and production uses the original loader-resolved Win32 APIs.
template<class R, unsigned A, class... Args> R Import(Args... args) {
#if defined(_M_IX86)
    using Fn=R(__stdcall*)(Args...);
    return (*reinterpret_cast<Fn*>(uintptr_t(A)))(args...);
#else
    ((void)args,...); AbortNotRewritten("m02 T1 Win32 import"); return R{};
#endif
}
void Close(void* h) { Import<int,0xF95398>(h); }
void Enter(void* p) { Import<void,0xF9512C>(p); }
void Leave(void* p) { Import<void,0xF95130>(p); }
void* Alloc(size_t n) { return CallGameFunction<void*,0x4E3F90>(n,"Threads/Contruct",0); }
void Free(void* p) { CallGameFunction<void,0x4E3F70>(p); }
template<class N> N*& Head(List* p) { return reinterpret_cast<N*&>(p->sentinel.next); }
template<class N> N*& Tail(List* p) { return reinterpret_cast<N*&>(p->sentinel.prev); }
long Exchange(uint32_t* p,long x) { return _InterlockedExchange(reinterpret_cast<volatile long*>(p),x); }
long Add(uint32_t* p,long x) { return _InterlockedExchangeAdd(reinterpret_cast<volatile long*>(p),x); }
}
void Reap(Registry* p,uint8_t force) {
    Enter(p->critical);
    for(auto& h:p->handles) if(h) {
        uint32_t code=0;
        if(force || !Import<int,0xF951FC>(h,&code) || code!=259) { Close(h); h=nullptr; }
    }
    Leave(p->critical);
}
void RegisterHandle(Registry* p,void* h,uint8_t add) {
    if(!h) return;
    Enter(p->critical);
    unsigned i=0;
    if(add) {
        while(i<300 && p->handles[i]) ++i;
        if(i==300) Close(h); else p->handles[i]=h;
    } else {
        while(i<300 && p->handles[i]!=h) ++i;
        if(i!=300) { Close(h); p->handles[i]=nullptr; }
    }
    Leave(p->critical);
}
uint32_t SetTls(void* value,uint8_t own) {
#if defined(_M_IX86)
    auto& index=*reinterpret_cast<uint32_t*>(0x114C908);
    auto registry=reinterpret_cast<Registry*>(0x11C7E60);
    if(index==~0u) { index=Import<uint32_t,0xF95200>(); if(index==~0u) return index; }
    Reap(registry,0);
    if(own) {
        if(value) RegisterHandle(registry,value,1);
        else if(auto previous=Import<void*,0xF95208>(index)) RegisterHandle(registry,previous,0);
    }
    return Import<int,0xF95204>(index,value);
#else
    (void)value; (void)own; AbortNotRewritten("m02 T1 TLS globals"); return 0;
#endif
}
uint32_t Sleep(const uint32_t* ticks) { return Import<uint32_t,0xF951F0>(*ticks,1); }
Parameters* InitParameters(Parameters* p,uint8_t mode,const char* name) {
    p->mode=mode;
    if(name) { std::strncpy(p->name,name,15); p->name[15]=0; } else p->name[0]=0;
    return p;
}
bool InitMutex(Mutex* p,const Parameters* param) {
    if(!param) return false;
    p->recursion=0; p->csMode=param->mode;
    if(p->csMode) { Import<void,0xF95124>(p); return true; }
    p->criticalSection[0]=Import<void*,0xF95214>(nullptr,0,param->name[0]?param->name:nullptr);
    return p->criticalSection[0]!=nullptr;
}
Mutex* ConstructMutex(Mutex* p,const Parameters* param,uint8_t defaults) {
    p->recursion=0; p->auxiliary28=0; p->csMode=1;
    for(auto& v:p->criticalSection) v=nullptr;
    p->auxiliary18=p->auxiliary1c=0;
    Parameters def; InitParameters(&def,1,nullptr);
    InitMutex(p,(!param && defaults)?&def:param); return p;
}
void DestroyMutex(Mutex* p) {
    if(p->csMode) Import<void,0xF95128>(p); else Close(p->criticalSection[0]);
}
bool InitCondition(Condition* p,const Parameters* param) {
    if(!param) return false;
    const char* name=param->name[0]?param->name:nullptr;
    // SemaphoreParameters is an opaque 24-byte dependency owned by T2.
    alignas(void*) unsigned char a[24], b[24];
    CallGameMethod<void*,0x4E26A0>(a,0,param->mode,name);
    CallGameMethod<void*,0x4E26A0>(b,1,param->mode,name);
    Parameters m; InitParameters(&m,param->mode,name);
    return CallGameMethod<uint8_t,0x4E26F0>(&p->semaphores[0],a) &&
           CallGameMethod<uint8_t,0x4E26F0>(&p->semaphores[1],b) && InitMutex(&p->mutex,&m);
}
Condition* ConstructCondition(Condition* p,const Parameters* param,uint8_t defaults) {
    Exchange(&p->counters[0],0); p->counters[1]=p->counters[2]=0;
    for(auto& s:p->semaphores) CallGameMethod<void*,0x4E28C0>(&s,nullptr,0);
    ConstructMutex(&p->mutex,nullptr,0);
    Parameters def; InitParameters(&def,1,nullptr);
    InitCondition(p,(!param && defaults)?&def:param); return p;
}
void DestroyCondition(Condition* p) {
    DestroyMutex(&p->mutex);
    CallGameMethod<void,0x4E26E0>(&p->semaphores[1]);
    CallGameMethod<void,0x4E26E0>(&p->semaphores[0]);
}
bool Signal(Condition* p,uint8_t all) {
    const void* forever=reinterpret_cast<const void*>(uintptr_t(0xFB895C));
    if(CallGameMethod<int,0x4E16F0>(&p->mutex,forever)<0) return false;
    bool post=false; long n=0;
    if(p->counters[1]) {
        if(p->counters[0]) {
            if(all) { n=Exchange(&p->counters[0],0); p->counters[1]+=n; }
            else { n=1; ++p->counters[1]; Add(&p->counters[0],-1); }
            post=true;
        }
    } else if(int32_t(p->counters[0])>int32_t(p->counters[2])) {
        if(CallGameMethod<int,0x4E2760>(&p->semaphores[1],forever)==-1) {
            CallGameMethod<void,0x4E17B0>(&p->mutex); return false;
        }
        if(p->counters[2]) { Add(&p->counters[0],-long(p->counters[2])); p->counters[2]=0; }
        if(all) { n=Exchange(&p->counters[0],0); p->counters[1]=n; }
        else { n=1; p->counters[1]=1; Add(&p->counters[0],-1); }
        post=true;
    }
    CallGameMethod<void,0x4E17B0>(&p->mutex);
    if(post) CallGameMethod<void,0x4E2810>(&p->semaphores[0],n);
    return true;
}
PoolParameters* InitPoolParameters(PoolParameters* p) {
    p->minimum=0; p->maximum=4; p->initial=0; p->timeout=60000;
    CallGameMethod<void*,0x4E2960>(&p->thread); return p;
}
List* ConstructThreads(List* p) {
    p->count=0; auto h=static_cast<ThreadNode*>(Alloc(sizeof(ThreadNode))); Head<ThreadNode>(p)=h;
    auto t=static_cast<ThreadNode*>(Alloc(sizeof(ThreadNode))); Tail<ThreadNode>(p)=t;
    h->next=h->prev=t; t->next=t->prev=h; return p;
}
List* ConstructWork(List* p) {
    p->count=0; auto h=static_cast<WorkNode*>(Alloc(sizeof(WorkNode))); Head<WorkNode>(p)=h;
    if(h) h->value.function=h->value.argument=h->value.context=nullptr;
    auto t=static_cast<WorkNode*>(Alloc(sizeof(WorkNode))); Tail<WorkNode>(p)=t;
    if(t) t->value.function=t->value.argument=t->value.context=nullptr;
    h->next=h->prev=t; t->next=t->prev=h; return p;
}
WorkNode* AppendWork(List* p,const Payload* value) {
    auto n=static_cast<WorkNode*>(Alloc(sizeof(WorkNode)));
    // The original has no allocation-failure recovery: its null path also faults.
    n->value=*value; auto tail=Tail<WorkNode>(p);
    n->prev=tail->prev; n->next=tail; tail->prev->next=n; tail->prev=n; ++p->count; return n;
}
WorkNode* ClearWork(List* p) {
    auto n=Head<WorkNode>(p)->next; auto tail=Tail<WorkNode>(p);
    if(n!=tail) { do { auto next=n->next; next->prev=n->prev; n->prev->next=next; Free(n); n=next; } while(n!=tail); p->count=0; }
    return n;
}
ThreadNode* ClearThreads(List* p) {
    auto n=Head<ThreadNode>(p)->next; auto tail=Tail<ThreadNode>(p);
    if(n!=tail) { do { auto next=n->next; next->prev=n->prev; n->prev->next=next; Free(n); n=next; } while(n!=tail); p->count=0; }
    return n;
}
uintptr_t Restart(ThreadPool* p) {
    auto h=Head<ThreadNode>(&p->threads), tail=Tail<ThreadNode>(&p->threads);
    uintptr_t result=reinterpret_cast<uintptr_t>(h);
    for(auto n=h->next;n!=tail;n=n->next) {
        auto info=n->value;
        result=CallGameMethod<uintptr_t,0x4E2A70>(info->thread,nullptr);
        if(result==2) {
            auto wrapper=CallGameFunction<void*,0x4E2980>();
            result=CallGameMethod<uintptr_t,0x4E2F30>(info->thread,reinterpret_cast<void*>(uintptr_t(0x4E1DF0)),info,&p->parameters,wrapper);
        }
    }
    return result;
}
int32_t Submit(ThreadPool* p,void* function,void* context,void* out,int32_t flags) {
    auto serial=uint32_t(Add(&p->serial,1))+1;
    Payload work{serial,function,nullptr,context};
    return CallGameMethod<int32_t,0x4E1F90>(p,&work,out,flags)==-1?-1:int32_t(serial);
}
}
