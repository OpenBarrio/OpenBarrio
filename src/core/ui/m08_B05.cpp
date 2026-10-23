#include "ts3/ui/m08_B05.h"
#include "ts3/ui/m08_abi.h"
#include "ts3/runtime/m02_T1.h"
#include "ts3/scratch/clu_4e3f70_Scratch.h"
#include <intrin.h>
namespace ts3::ui::m08::b05 {
namespace {
void ConstructPending(Pending* p) { ts3::runtime::m02::t1::ConstructMutex(reinterpret_cast<ts3::runtime::m02::Mutex*>(p),nullptr,1); }
void AtExit() {
#if defined(_M_IX86)
    ts3::CallGameFunction<int,0x00F7E9A1>(reinterpret_cast<void(*)()>(0x00F90F40));
#else
    ts3::NotRewritten("m08 B05 pending exit registration");
#endif
}
void Lock(Pending* p) {
    const uint32_t timeout=0xFFFFFFFF;
    ts3::CallGameMethod<void,0x004E16F0>(p,&timeout);
}
void Unlock(Pending* p) { ts3::CallGameMethod<void,0x004E17B0>(p); }
void Reserve(PendingVector* v,size_t n) { ts3::CallGameMethod<void,0x004D4FD0>(v,n); }
void Free(void* p) { ts3::scratch::Free4E3F70(p); }
const PendingOps ops{ConstructPending,AtExit,Lock,Unlock,Reserve,Free};
PendingEnvironment Environment() {
#if defined(_M_IX86)
    return {reinterpret_cast<Pending**>(0x011CD034),reinterpret_cast<uint32_t*>(0x011CD078),reinterpret_cast<Pending*>(0x011CD038),&ops,reinterpret_cast<const char*>(0x00FBA4FC),reinterpret_cast<const char*>(0x00FBA568)};
#else
    ts3::NotRewritten("m08 B05 pending globals require a native environment");
#endif
}
void* Adjust(void* p) { return p?static_cast<uint8_t*>(p)+sizeof(void*):nullptr; }
}
uint32_t ClassId() { return 0x01BE6B15; }
Factory* Construct(Factory* p) {
#if defined(_M_IX86)
    p->table=reinterpret_cast<void**>(0x00FBA518);
#else
    // Image table is an opaque token on native builds, never a truncated pointer.
    static void* nativeTable[6]{};
    p->table=nativeTable;
#endif
    _InterlockedExchange(&p->exchange,0);
    p->word=0;
    return p;
}
Pending* GetPending(PendingEnvironment& e) {
    if (!*e.slot) {
        if (!(*e.flags & 1)) {
            *e.flags |= 1;
            e.ops->construct(e.storage);
            e.storage->vector={nullptr,nullptr,nullptr,e.name};
            e.ops->atExit();
        }
        *e.slot=e.storage;
    }
    return *e.slot;
}
Pending* GetPending() { auto e=Environment(); return GetPending(e); }
uint8_t Drain(Pending* p,const PendingOps& o,const char* name) {
    PendingVector temp{nullptr,nullptr,nullptr,name};
    uintptr_t *begin=nullptr,*end=nullptr;
    o.lock(p);
    if (p->vector.begin != p->vector.end) {
        const size_t capacity=(reinterpret_cast<uintptr_t>(p->vector.cap)-reinterpret_cast<uintptr_t>(p->vector.begin))/sizeof(uintptr_t);
        o.reserve(&temp,capacity);
        begin=p->vector.begin; end=p->vector.end;
        p->vector.begin=temp.begin; p->vector.end=temp.end; p->vector.cap=temp.cap;
    }
    o.unlock(p);
    for (auto it=begin;it!=end;++it) {
        auto object=reinterpret_cast<void*>(*it & ~uintptr_t(1));
        auto table=*static_cast<void***>(object);
        const auto method=reinterpret_cast<Thiscall<void,void>>(table[(*it & 1)?4:5]);
        CallThis(method,object);
        // Reload the table after the first callback: it may have changed.
        table=*static_cast<void***>(object);
        CallThis(reinterpret_cast<Thiscall<void,void>>(table[1]),object);
    }
    if (begin) o.free(begin);
    return 0;
}
uint8_t Drain(Pending* p) { auto e=Environment(); return Drain(p,*e.ops,e.tempName); }
uint8_t DrainGlobal() { return Drain(GetPending()); }
uintptr_t Forward(Manager* p,uintptr_t arg) {
    return CallThis(reinterpret_cast<Thiscall<uintptr_t,Manager,uintptr_t,uintptr_t,uintptr_t>>(p->table[6]),p,uintptr_t(0),uintptr_t(0),arg);
}
void* Focus(Manager* p,int32_t index) { return Adjust(p->channels[index].focus); }
uint32_t FocusMask(Manager* p,void* target) {
    uint32_t result=0;
    for (unsigned i=0;i<10;++i) if (Adjust(p->channels[i].focus)==target) result |= 1u<<i;
    return result;
}
void* Capture(Manager* p,int32_t index) { return Adjust(p->channels[index].capture); }
uintptr_t ClearMatch(Manager* p,int32_t index,void* target,uintptr_t arg) {
    void* base=target?static_cast<uint8_t*>(target)-sizeof(void*):nullptr;
    if (p->channels[index].capture==base)
        return CallThis(reinterpret_cast<Thiscall<uintptr_t,Manager,int32_t,void*,uintptr_t>>(p->table[25]),p,index,static_cast<void*>(nullptr),arg);
    return reinterpret_cast<uintptr_t>(base);
}
uint32_t SetFlags(Manager* p,uint32_t mask,uint32_t enabled) {
    p->flags &= ~mask;
    if (static_cast<uint8_t>(enabled)) p->flags |= mask;
    return p->flags;
}
uint32_t HasFlags(Manager* p,uint32_t mask) { return (p->flags & mask)?1u:0u; }
void Dirty(TreeWindow* p) {
    for (;;) {
        if (!(p->flags1F8 & 0x20) && !(p->flags070 & 1)) { p->flags1F9 |= 0x10; return; }
        p=p->parent;
        if (!p || (p->flags1F8 & 0x10)) return;
        p->flags1F8 |= 0x10;
    }
}
void* Value(Manager* p) { return p->value874; }
Manager* Singleton() {
#if defined(_M_IX86)
    return *reinterpret_cast<Manager**>(0x011CD080);
#else
    ts3::NotRewritten("m08 B05 manager singleton native provider");
#endif
}
}
