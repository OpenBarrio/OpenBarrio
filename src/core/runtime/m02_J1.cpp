#include "ts3/runtime/m02_J1.h"
#include <windows.h>
#include <cstring>
namespace ts3::runtime::m02::j1 {
namespace {
uintptr_t Lock(Scheduler* s) { return CallGameMethod<uintptr_t,0x4e16f0>(&s->mutex,reinterpret_cast<void*>(uintptr_t(0xfd9744))); }
uintptr_t Unlock(Scheduler* s) { return CallGameMethod<uintptr_t,0x4e17b0>(&s->mutex); }
void Reset(Link& l) { l.next=l.prev=&l; }
void Before(Link* at,Link* node) { auto prev=at->prev; at->prev=node; prev->next=node; node->prev=prev; node->next=at; }
void Unlink(Link* l) { l->next->prev=l->prev; l->prev->next=l->next; }
Dependency* Dep(Link* l) { return reinterpret_cast<Dependency*>(reinterpret_cast<uint8_t*>(l)-sizeof(Link)); }
int32_t Effective(Job* j) { return static_cast<int32_t>(j->reserved48); }
int32_t Base(Job* j) { int32_t p; std::memcpy(&p,j->reserved14+12,4); return p; }
void Base(Job* j,int32_t p) { std::memcpy(j->reserved14+12,&p,4); }
void Recycle(Job* j) { CallGameMethod<void,0x59a650>(j->scheduler,j); }
}
bool Initialize(Scheduler* s,uint32_t mask) {
    if(!s->reservedJob) { auto j=CallGameVirtual<Job*,5>(s,uint32_t(0x80000000),uint32_t(0),int32_t(-1)); s->reservedJob=j; j->state=0; }
    for(unsigned i=0;i<32;++i) if(mask&(uint32_t(1)<<i)) CallGameVirtual<Job*,5>(s,uint32_t(0x80000000),uint32_t(1),i);
    return true;
}
uintptr_t Stamp(Scheduler* s,Job* j) { Lock(s); j->state=CallGameFunction<uint32_t,0x4e1270>(); return Unlock(s); }
uint32_t State(void* view) { auto j=JobRecord(view); auto s=j->scheduler; Lock(s); auto state=j->state; Unlock(s); return state==5?4:state; }
void SetCancel(void* view,uint8_t flag) { JobRecord(view)->flags[1]=flag; }
Dependency* AllocateLink(Scheduler* s) {
    auto h=&s->lists[0]; if(h->prev==h) return CallGameFunction<Dependency*,0x4e3f90>(uint32_t(24),"Services/JobManager/JobLink",uint32_t(0),uint32_t(0),uint32_t(0),uint32_t(0));
    auto l=h->prev; l->prev->next=h; h->prev=l->prev; return reinterpret_cast<Dependency*>(l);
}
uintptr_t RetainExternal(void* view) {
    auto j=JobRecord(view); auto s=j->scheduler; Lock(s); auto result=Unlock(s);
    if(!j->flags[2]) { j->flags[2]=1; result=reinterpret_cast<uintptr_t>(&j->refs); InterlockedExchangeAdd(reinterpret_cast<LONG*>(&j->refs),1); }
    return result;
}
Scheduler* Construct(Scheduler* s) {
#if defined(_M_IX86)
    s->vtable=reinterpret_cast<void**>(0xfd97a4);
#endif
    CallGameMethod<void,0x4e17e0>(&s->mutex,static_cast<void*>(nullptr),true);
    for(auto& q:s->queues) Reset(q);
    s->reservedJob=nullptr; for(auto& l:s->lists) Reset(l); s->state=s->trailing=0; return s;
}
Scheduler* Destroy(Scheduler* s,uint32_t flags) {
#if defined(_M_IX86)
    s->vtable=reinterpret_cast<void**>(0xfd97a4);
#endif
    CallGameMethod<void,0x4e1670>(&s->mutex);
#if defined(_M_IX86)
    s->vtable=reinterpret_cast<void**>(0xfd9748);
#endif
    if(flags&1) CallGameFunction<void,0x4e3f70>(s); return s;
}
Link* __stdcall Insert(Link* head,Job* j) {
    auto at=head;
    if(head->next!=head) { do { at=at->prev; if(Effective(j)<=Effective(reinterpret_cast<Job*>(at))) { at=at->next; break; } } while(head->next!=at); }
    Before(at,&j->links[0]); return at;
}
JobService* ConstructService(JobService* s,uint8_t flag) {
    CallGameMethod<void,0x59ecc0>(s,uint32_t(44618123),uint32_t(0));
#if defined(_M_IX86)
    s->vtable=reinterpret_cast<void**>(0xfd9810); s->secondary=reinterpret_cast<void**>(0xfd980c);
#endif
    Construct(&s->scheduler); s->flag=flag; s->current=nullptr; s->config=5; return s;
}
uintptr_t Update(JobService* s,uint32_t,uint32_t) {
    if(s->current) { DeadlineTimer timer; CallGameMethod<void,0x4f35b0>(&timer,uint32_t(4),uint32_t(0)); CallGameMethod<void,0x4f34f0>(&timer,s->config,uint32_t(1)); return CallGameVirtual<uintptr_t,9>(&s->scheduler,s->current,&timer,uint32_t(1)); }
    // The original empty path leaves EAX unspecified; no caller may consume it.
    return 0;
}
Link** LowerRaw(Link** out,Link* first,Link* last,Dependency* value) {
    int n=0; for(auto p=first;p!=last;p=p->next) ++n;
    while(n>0) { int half=n/2; auto middle=first; for(int i=0;i<half;++i) middle=middle->next;
        if(Effective(Dep(middle)->target)<=Effective(value->target)) n=half;
        else { first=middle->next; n-=half+1; }
    } *out=first; return out;
}
Link** LowerView(Link** out,Link* first,Link* last,Link* value) { return LowerRaw(out,first,last,Dep(value)); }
Job* Find(Scheduler* s,uint32_t mask) {
    Lock(s); auto h=&s->queues[10]; Job* found=nullptr;
    for(auto p=h->next;p!=h;p=p->next) { auto j=reinterpret_cast<Job*>(p); if(j->reserved2c&mask) { found=j; break; } }
    Unlock(s); return found;
}
uintptr_t Invoke(void* view) { auto j=JobRecord(view); return CallGameMethod<uintptr_t,0x59a190>(j->scheduler,j); }
bool Reorder(Link* head,Link* node,const uint8_t*) {
    Link* at;
    if(node!=head->next && Effective(Dep(node->prev)->target)<=Effective(Dep(node)->target)) LowerView(&at,head->next,node,node);
    else { if(node->next==head || Effective(Dep(node)->target)>Effective(Dep(node->next)->target)) return false; LowerView(&at,node->next,head,node); }
    if(at!=node) { Unlink(node); Before(at,node); } return true;
}
uintptr_t Configure(Scheduler* s,Job* j,void* timer,uint32_t flag) { Lock(s); auto result=CallGameMethod<uint8_t,0x599e60>(s,j,timer,flag,uint32_t(0)); return (Unlock(s)&~uintptr_t(255))|result; }
void Propagate(Scheduler* s,Job* j) {
    auto priority=Base(j); auto h=&j->dependencies[1];
    if(h->prev!=h) { auto p=Effective(Dep(h->next)->target); if(priority<p) priority=p; }
    if(priority==Effective(j)) return;
    j->reserved48=uint32_t(priority); if(j->state==3) CallGameMethod<void,0x599fc0>(s,j);
    auto deps=&j->dependencies[0]; uint8_t comparator=0;
    for(auto p=deps->next;p!=deps;p=p->next) { auto d=reinterpret_cast<Dependency*>(p); auto owner=d->owner; Reorder(&owner->dependencies[1],&d->links[1],&comparator); Propagate(s,owner); }
}
uintptr_t Operate(void* view) { auto j=JobRecord(view); auto s=j->scheduler; Lock(s); CallGameMethod<void,0x59a220>(s,j); return Unlock(s); }
int32_t ReleaseRaw(Job* j) { auto result=InterlockedDecrement(reinterpret_cast<LONG*>(&j->refs)); if(!result) Recycle(j); return result; }
uintptr_t SetPriority(void* view,int32_t priority) { auto j=JobRecord(view); auto s=j->scheduler; Lock(s); if(Base(j)!=priority) { Base(j,priority); Propagate(s,j); } return Unlock(s); }
uintptr_t Relate(void* first,void* second) { auto j=JobRecord(first); return CallGameMethod<uintptr_t,0x59a6e0>(j->scheduler,j,JobRecord(second)); }
int32_t Release(void* view) { auto j=JobRecord(view); auto result=InterlockedDecrement(reinterpret_cast<LONG*>(&j->refs)); if(!result) { auto s=j->scheduler; Lock(s); Recycle(j); Unlock(s); } return result; }

bool __fastcall InitializeEntry(Scheduler* s,void*,uint32_t mask) { return Initialize(s,mask); }
uintptr_t __fastcall StampEntry(Scheduler* s,void*,Job* j) { return Stamp(s,j); }
uint32_t __fastcall StateEntry(void* view,void*) { return State(view); }
void __fastcall SetCancelEntry(void* view,void*,uint8_t flag) { SetCancel(view,flag); }
Dependency* __fastcall AllocateLinkEntry(Scheduler* s,void*) { return AllocateLink(s); }
uintptr_t __fastcall RetainExternalEntry(void* view,void*) { return RetainExternal(view); }
Scheduler* __fastcall ConstructEntry(Scheduler* s,void*) { return Construct(s); }
Scheduler* __fastcall DestroyEntry(Scheduler* s,void*,uint32_t flags) { return Destroy(s,flags); }
JobService* __fastcall ConstructServiceEntry(JobService* s,void*,uint8_t flag) { return ConstructService(s,flag); }
uintptr_t __fastcall UpdateEntry(JobService* s,void*,uint32_t a,uint32_t b) { return Update(s,a,b); }
Job* __fastcall FindEntry(Scheduler* s,void*,uint32_t mask) { return Find(s,mask); }
uintptr_t __fastcall InvokeEntry(void* view,void*) { return Invoke(view); }
uintptr_t __fastcall ConfigureEntry(Scheduler* s,void*,Job* j,void* timer,uint32_t flag) { return Configure(s,j,timer,flag); }
void __fastcall PropagateEntry(Scheduler* s,void*,Job* j) { Propagate(s,j); }
uintptr_t __fastcall OperateEntry(void* view,void*) { return Operate(view); }
int32_t __fastcall ReleaseRawEntry(Job* j,void*) { return ReleaseRaw(j); }
uintptr_t __fastcall SetPriorityEntry(void* view,void*,int32_t p) { return SetPriority(view,p); }
uintptr_t __fastcall RelateEntry(void* a,void*,void* b) { return Relate(a,b); }
int32_t __fastcall ReleaseEntry(void* view,void*) { return Release(view); }
}
