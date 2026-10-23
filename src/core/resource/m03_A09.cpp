#include "ts3/resource/m03_A09.h"
#include "ts3/resource/m03_A01.h"
#include "ts3/runtime/m02_T1.h"
#include <algorithm>
#include <atomic>
#include <cstring>
#include <stdexcept>
#include <type_traits>
namespace ts3::resource::m03::a09 {
namespace {
thread_local Boundary boundary=nullptr;
template<class T> uintptr_t Bits(T v) { if constexpr(std::is_pointer_v<T>) return reinterpret_cast<uintptr_t>(v); else return static_cast<uintptr_t>(v); }
template<unsigned A,class S,class... T> uintptr_t M(S* s,T... v) {
    if(boundary) return boundary(A,s,{Bits(v)...});
    return CallGameMethod<uintptr_t,A>(s,v...);
}
template<unsigned A,class... T> uintptr_t F(T... v) {
    if(boundary) return boundary(A,nullptr,{Bits(v)...});
    return CallGameFunction<uintptr_t,A>(v...);
}
template<unsigned N,class S,class... T> uintptr_t V(S* s,T... v) {
    if(boundary) return boundary(0x80000000u|N,s,{Bits(v)...});
    return CallGameVirtual<uintptr_t,N>(s,v...);
}
void Lock(Loader* s) {
#if defined(_M_IX86)
    M<0x4e16f0>(s->mutex,reinterpret_cast<void*>(0xffd2dc));
#else
    M<0x4e16f0>(s->mutex,static_cast<void*>(nullptr));
#endif
}
uintptr_t Unlock(Loader* s) { return M<0x4e17b0>(s->mutex); }
uintptr_t Status(uintptr_t raw,bool b) { return (raw&~uintptr_t(255))|uintptr_t(b); }
Client* Tagged(void* p) { return reinterpret_cast<Client*>(reinterpret_cast<uintptr_t>(p)-1); }
void Link(Client* c,LoadInfo* i) {
    c->next=reinterpret_cast<Client*>(&i->next); c->previous=i->previous;
    i->previous=c; c->previous->next=c;
}
void Complete(void* cb,void* ctx,void* resource) { Key zero{}; V<0>(cb,&zero,ctx,resource); }
void Advance(Iterator& i) { if(++i.current==i.last) { ++i.node; i.first=*i.node; i.last=i.first+32; i.current=i.first; } }
void Retreat(Iterator& i) { if(i.current==i.first) { --i.node; i.first=*i.node; i.last=i.first+32; i.current=i.last; } --i.current; }
ptrdiff_t Distance(Iterator a,Iterator b) { return 32*(b.node-a.node-1)+(a.last-a.current)+(b.current-b.first); }
Iterator At(Iterator i,ptrdiff_t n) { while(n>0) { Advance(i); --n; } while(n<0) { Retreat(i); ++n; } return i; }
void* const* RangeTable() {
#if defined(_M_IX86)
    return reinterpret_cast<void* const*>(0xffd528);
#else
    static void* table[2]{}; return table;
#endif
}
void* const* BaseTable() {
#if defined(_M_IX86)
    return reinterpret_cast<void* const*>(0xfb2cbc);
#else
    static void* table[1]{}; return table;
#endif
}
}
Boundary SetBoundary(Boundary b) { auto old=boundary; boundary=b; return old; }
uintptr_t Replace(Loader* s,void** dest,void** source,RefCallback acquire,RefCallback release,uint32_t inherit,void* cb,void* ctx) {
    Lock(s); uintptr_t old=Bits(*dest), value=Bits(*source);
    if(old==value) return Status(Unlock(s),true);
    if(old&1) { M<0x728760>(s,dest,release); value=Bits(*source); }
    else if(old && !(old&3)) release(*dest);
    if(!(value&1)) {
        if(value&~uintptr_t(3)) { acquire(reinterpret_cast<void*>(value)); *dest=reinterpret_cast<void*>(value); if(!(inherit&255) && cb) Complete(cb,ctx,*dest); }
        // EA deliberately leaves the old destination value when source is 0 or 2.
        return Status(Unlock(s),true);
    }
    Client* c=s->free;
    if(!c) c=reinterpret_cast<Client*>(M<0x7272e0>(&s->free));
    if(!c) { *dest=nullptr; if(cb) Complete(cb,ctx,nullptr); return Status(Unlock(s),false); }
    s->free=c->next; c->job=nullptr;
    Client* from=Tagged(reinterpret_cast<void*>(value)); c->cell=dest; c->resource=from->resource; c->info=from->info;
    if(inherit&255) { c->completion=from->completion; c->context=from->context; }
    else { c->completion=cb; c->context=ctx; }
    Link(c,c->info); *dest=reinterpret_cast<void*>(Bits(c)+1);
    return Status(Unlock(s),true);
}
uintptr_t Release(Loader* s,void** cell,RefCallback cb) { Lock(s); M<0x728760>(s,cell,cb); return Unlock(s); }
Iterator* Insert(Deque* s,Iterator* out,Iterator where,const Entry* value) {
    if(where.current==s->end.current) {
        if(s->end.current+1==s->end.last) M<0xaa2f20>(s,value);
        else { Entry* p=s->end.current++; if(p) *p=*value; }
        *out=s->end; Retreat(*out);
    } else if(where.current==s->begin.current) {
        if(s->begin.current==s->begin.first) M<0xaa2eb0>(s,value);
        else { --s->begin.current; if(s->begin.current) *s->begin.current=*value; }
        *out=s->begin;
    } else {
        // The unreconstructed middle-insert dependency takes its iterator by value.
        if(boundary) boundary(0x728990,s,{Bits(out),Bits(&where),Bits(value)});
        else CallGameMethod<void,0x728990>(s,out,where,value);
    }
    return out;
}
Iterator* Erase(Deque* s,Iterator* out,Iterator where) {
    const ptrdiff_t index=Distance(s->begin,where);
    const uint32_t half=static_cast<uint32_t>(Distance(s->begin,s->end))>>1;
    Iterator next=where; Advance(next);
    if(index>=half) {
        for(Iterator dst=where,src=next;src.current!=s->end.current;Advance(dst),Advance(src)) *dst.current=*src.current;
        if(s->end.current==s->end.first) { if(s->end.first) F<0x4e3f70>(s->end.first); --s->end.node; s->end.first=*s->end.node; s->end.last=s->end.first+32; s->end.current=s->end.last-1; }
        else --s->end.current;
    } else {
        for(Iterator dst=where;dst.current!=s->begin.current;) { Iterator src=dst; Retreat(src); *dst.current=*src.current; dst=src; }
        Entry* p=s->begin.current+1;
        if(p==s->begin.last) { if(s->begin.first) F<0x4e3f70>(s->begin.first); ++s->begin.node; s->begin.first=*s->begin.node; s->begin.last=s->begin.first+32; s->begin.current=s->begin.first; }
        else s->begin.current=p;
    }
    *out=At(s->begin,index); return out;
}
uintptr_t Cancel(Loader* s,void** cell) {
    LoadInfo* info=nullptr; void* job=nullptr; bool wait=false; Lock(s);
    if(Bits(*cell)&1) {
        info=Tagged(*cell)->info;
        if(info) {
            V<0>(info); info->cancelled=1;
            if(info->pending && s->queue.begin.current!=s->queue.end.current) {
                M<0x727c50>(s,info->request,500);
                for(Iterator i=s->queue.begin;i.current!=s->queue.end.current;Advance(i)) if(i.current->request==info->request) { void* request=i.current->request; Iterator out{}; Erase(&s->queue,&out,i); M<0x59a990>(request); break; }
            }
            job=info->job; if(job) M<0x599460>(job); wait=info->wait!=0;
        }
    }
    Unlock(s);
    if(job) { if(wait) M<0x401230>(info->pad3c+12,500); M<0x59a820>(job,500); M<0x59a500>(job); }
    uintptr_t raw=Bits(*cell); const bool ready=!(raw&3)&&raw;
    if(info) raw=V<1>(info);
    if(job) raw=M<0x59a990>(job);
    return Status(raw,ready);
}
uintptr_t Preload(Loader* s,const Key* key) {
    Lock(s); auto manager=boundary?reinterpret_cast<void*>(F<0x4afd20>()):static_cast<void*>(a01::Singleton());
    auto factory=reinterpret_cast<void*>(V<22>(manager,key));
    if(!factory || !(V<6>(factory,key,0)&255)) return Status(Unlock(s),true);
    Client* c=s->free; if(!c) c=reinterpret_cast<Client*>(M<0x7272e0>(&s->free));
    if(!c) return Status(Unlock(s),false);
    s->free=c->next; c->job=nullptr;
    auto info=reinterpret_cast<LoadInfo*>(M<0x7297e0>(s,key,1,&c->job,0,0,1));
    if(!info) { M<0x727dd0>(s,key,3,0u); M<0x728530>(s,c); return Status(Unlock(s),false); }
    c->cell=nullptr; c->resource=nullptr; c->info=info; M<0x6e9540>(&c->job,info->job);
    c->completion=nullptr; c->context=nullptr; c->state=0; Link(c,info);
    return Status(Unlock(s),true);
}
uint8_t Children(Loader* s,LoadInfo* info) {
    if(info->children) {
        auto record=reinterpret_cast<void*>(V<6>(info->factory));
        // Record's second native word is the backing-data view; its +8 pointer is borrowed.
        auto backing=static_cast<void**>(record)[1];
        void* data=backing?static_cast<void**>(backing)[2]:nullptr;
        uintptr_t size=V<7>(record);
        if(!(M<0x726ba0>(info->context,data,size,0)&255)) { info->failed=1; return 0; }
        uint32_t count=static_cast<uint32_t>(M<0x7260e0>(info->context,2));
        if(count) {
            Lock(s); uint32_t sum=info->size;
            for(uint32_t i=0;i<count;++i) {
                auto key=reinterpret_cast<const Key*>(M<0x726270>(info->context,i,2));
                auto child=reinterpret_cast<LoadInfo*>(M<0x7297e0>(s,key,info->mode,0,info->child,uint32_t(info->cancelled),1));
                if(!child) { info->failed=1; Unlock(s); return 0; }
                sum+=child->size; M<0x59a870>(info->job,child->job);
            }
            if(s->budget<sum) s->budget=sum;
            Unlock(s);
        }
    }
    return 1;
}
void* Singleton() {
    if(boundary) return reinterpret_cast<void*>(boundary(0x11d3c88,nullptr,{}));
#if defined(_M_IX86)
    void* p; std::memcpy(&p,reinterpret_cast<void*>(0x11d3c88),sizeof(p)); return p;
#else
    throw std::logic_error("m03_A09 singleton requires native dispatch");
#endif
}
CallbackManager* ConstructCallbacks(CallbackManager* s) {
    s->enabled=0;
    std::atomic_ref<int32_t>(s->refs).exchange(1); std::atomic_ref<int32_t>(s->active).exchange(0);
    s->requestName="ResourceSystem/CallbackMan/RequestQueue";
    s->requestNext=s->requestPrevious=&s->requestNext;
    if(boundary) M<0x4e17e0>(s->mutex,0,1);
    else runtime::m02::t1::ConstructMutex(reinterpret_cast<runtime::m02::Mutex*>(s->mutex),nullptr,1);
    s->processName="ResourceSystem/CallbackMan/ProcessQueue";
    s->processNext=s->processPrevious=&s->processNext; return s;
}
uint8_t Any(Ranges* s,const Key* key) { for(Range *p=s->begin,*end=s->end;p!=end;++p) if(V<1>(p,key)&255) return 1; return 0; }
Ranges* Construct(Ranges* s) { s->vtable=RangeTable(); s->begin=s->end=s->capacity=nullptr; s->name="ResourceSystem/KeyRanges"; return s; }
uintptr_t Destroy(Ranges* s) {
    s->vtable=RangeTable(); Range* end=s->end;
    for(Range* p=s->begin;p<end;++p) V<0>(p,0);
    uintptr_t result=Bits(s->begin); if(result) result=F<0x4e3f70>(s->begin);
    s->vtable=BaseTable(); return result;
}
Ranges* DeletingDestroy(Ranges* s,uint32_t flags) { Destroy(s); if(flags&1) F<0x4e3f70>(s); return s; }
Ranges* Swap(Ranges* s,Ranges* other) { std::swap(s->begin,other->begin); std::swap(s->end,other->end); std::swap(s->capacity,other->capacity); return other; }
}
