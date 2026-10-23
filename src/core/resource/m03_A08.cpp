#include "ts3/resource/m03_A08.h"
#include "ts3/layout.h"
#include <atomic>

namespace ts3::resource::m03::a08 {
namespace {
void* Mutex(Loader* s) { return s->prefix+0x40; }
uintptr_t Lock(Loader* s) {
#if defined(_M_IX86)
    return CallGameMethod<uintptr_t,0x004e16f0>(Mutex(s),reinterpret_cast<void*>(0x00ffd2dc));
#else
    (void)s; ts3::NotRewritten("A08 native mutex provider");
#endif
}
uintptr_t Unlock(Loader* s) { return CallGameMethod<uintptr_t,0x004e17b0>(Mutex(s)); }
uintptr_t Release(void* job) { return CallGameMethod<uintptr_t,0x0059a990>(job); }
}
void* ForwardA(Quad* out,Quad first,Quad last,Quad dest) {
    CallGameFunction<void*,0x00727850>(out,first,last,dest);
    return out;
}
void* ForwardB(Quad* out,Quad first,Quad last,Quad dest) {
    CallGameFunction<void*,0x007278d0>(out,first,last,dest);
    return out;
}
LoadInfo* ConstructInfo(LoadInfo* s) {
    for(auto& word:s->key) word=0;
#if defined(_M_IX86)
    s->vtable=reinterpret_cast<void*>(0x00ffd374);
#else
    // Native vtable installation belongs to the unreconstructed constructor provider.
    ts3::NotRewritten("A08 LoadInfo native vtable provider");
#endif
    s->p24=s->p28=s->p2c=nullptr;
    CallGameMethod<void,0x00401000>(s->request);
    std::atomic_ref<int32_t>(s->state).exchange(0);
    for(auto& flag:s->flags) flag=0;
    s->pb0=s->pb4=nullptr;
    s->clients.next=s->clients.prev=&s->clients;
    CallGameMethod<void,0x007263c0>(s->context);
    return s;
}
uint32_t Shutdown(Loader* s,void*) {
    auto r=CallGameMethod<uint32_t,0x00727d20>(s,1);
    return (r&0xffffff00u)|1u;
}
uintptr_t CancelJob(Loader* s,const void* owner,void* key) {
    Lock(s);
    // 00727EF0 writes BOTH words of a hash iterator, not just its node.
    struct Iterator { Links* node; void* bucket; } result{};
    // Only low bytes of the two false flags are observed by the callee.
    CallGameMethod<void,0x00727ef0>(s->prefix+0x80,&result,key,uint32_t(0),uint32_t(0));
    Links* found=result.node;
    void* job=nullptr;
    if(found!=s->sentinel) {
        auto* info=found ? reinterpret_cast<LoadInfo*>(reinterpret_cast<uint8_t*>(found)-2*sizeof(void*)) : nullptr;
        if(info->p2c==owner) {
            CallGameMethod<void,0x00599460>(info->job); // unconditional, even null
            job=info->job; // ref helper may reenter and replace this field
            if(job) CallGameMethod<void,0x00599460>(job);
        }
    }
    auto r=Unlock(s);
    if(job) {
        CallGameMethod<void,0x0059ab40>(job,1);
        r=Release(job);
    }
    return r;
}
uintptr_t RemoveClient(Loader* s,void* client) {
    Lock(s);
    CallGameMethod<void,0x00717030>(&s->clients,&client);
    return Unlock(s);
}
Child* Recycle(Loader* s,Child* child) {
    if(child->job) Release(child->job);
    Child* old=s->freeChildren;
    child->next=old;
    s->freeChildren=child;
    return old;
}
void* Find(void* map,void* out,const void* key) {
    CallGameMethod<void,0x00727e80>(map,out,key,uint32_t(0));
    return out;
}
void* ForwardC(Quad* out,Quad first,Quad last,Quad dest) { return ForwardA(out,first,last,dest); }
void* ForwardD(Quad* out,Quad first,Quad last,Quad dest) { return ForwardB(out,first,last,dest); }
uintptr_t Dispose(Loader* s,uintptr_t* handle,uintptr_t(*callback)(void*)) {
    uintptr_t bits=*handle;
    uintptr_t r=reinterpret_cast<uintptr_t>(handle);
    if(!bits) return r;
    if(bits&1) {
        auto* child=reinterpret_cast<Child*>(bits-1);
        auto* parent=child->parent;
        child->prev->next=child->next;
        child->next->prev=child->prev;
        Recycle(s,child);
        r=reinterpret_cast<uintptr_t>(&parent->clients);
        if(parent->clients.prev==&parent->clients) {
            float elapsed=CallGameMethod<float,0x00727470>(parent);
            r=CallGameMethod<uintptr_t,0x00727dd0>(s,parent->key,4,elapsed);
        }
    } else if(!(bits&3)) {
        Unlock(s);
        callback(reinterpret_cast<void*>(bits));
        r=Lock(s);
    }
    *handle=0;
    return r;
}
uintptr_t AddClient(Loader* s,void* client) {
    Lock(s);
#if defined(_M_IX86)
    auto* node=CallGameFunction<Client*,0x004e3f90>(uint32_t(12),s->clients.value,uint32_t(0),uint32_t(0),reinterpret_cast<void*>(0x00f98980),uint32_t(0xe7));
#else
    auto* node=CallGameFunction<Client*,0x004e3f90>(sizeof(Client),s->clients.value,uint32_t(0),uint32_t(0),"ResourceSystem",uint32_t(0xe7));
#endif
    // EA tests node+8, not node. Null allocation still faults while linking.
    if(reinterpret_cast<uintptr_t>(node)+2*sizeof(void*)) node->value=client;
    node->next=&s->clients;
    node->prev=s->clients.prev;
    s->clients.prev->next=node;
    s->clients.prev=node;
    return Unlock(s);
}
Loader* CreateLoader() {
#if defined(_M_IX86)
    auto* p=CallGameFunction<Loader*,0x004e3f90>(uint32_t(0x3a8),reinterpret_cast<void*>(0x00ffd450),0,0,0,0);
#else
    auto* p=CallGameFunction<Loader*,0x004e3f90>(sizeof(Loader),"ResourceSystem/AsyncLoader",0,0,0,0);
#endif
    return p ? CallGameMethod<Loader*,0x00728690>(p) : nullptr;
}
void* Assign(Quad* self,Quad* out,const Quad* first,const Quad* last,void*) {
    return ForwardC(out,*first,*last,*self);
}
uintptr_t AssignDiscard(Quad* self,const Quad* first,const Quad* last,void*) {
    Quad temporary;
    return reinterpret_cast<uintptr_t>(ForwardD(&temporary,*first,*last,*self));
}
}
