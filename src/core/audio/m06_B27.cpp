#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "ts3/audio/m06_B27.h"
#include "ts3/misc/clu_4a6c30_x.h"
#include <algorithm>
#include <cstring>
namespace ts3::audio::m06_B27 {
namespace {
void* Allocate(size_t n,const char* label,bool debug=true) {
    return CallGameFunction<void*,0x4E3F90>(n,label,0u,0u,debug?reinterpret_cast<const char*>(uintptr_t(0xF98980)):nullptr,debug?0xE7u:0u);
}
uintptr_t Free(void* p) { return CallGameFunction<uintptr_t,0x4E3F70>(p); }
CRITICAL_SECTION* Mutex(SoundArray* s) { return reinterpret_cast<CRITICAL_SECTION*>(s->mutex); }
void Retain(Data* d) { if (d) InterlockedExchangeAdd(&d->refs,1); }
void Release(Data* d) {
    if (d && InterlockedDecrement(&d->refs)==0) {
        InterlockedExchange(&d->refs,1);
        CallGameVirtual<void,0>(d,1u);
    }
}
uintptr_t ClearHash(Hash* h) {
    auto result=CallGameMethod<uintptr_t,0x8689B0>(h,h->buckets,h->count);
    h->size=0; return result;
}
void StopSounds(SoundArray* s,bool full) {
    auto bucket=s->hash.buckets;
    while (!*bucket) ++bucket;
    auto n=*bucket; auto end=s->hash.buckets[s->hash.count];
    while (n!=end) {
        auto sentinel=&n->value.tree.sentinel;
        auto entry=sentinel->right;
        while (entry!=sentinel) {
            auto sound=reinterpret_cast<SoundEntry*>(entry)->sound;
            if (sound) {
                CallGameVirtual<void,0>(sound);
                if ((static_cast<uint8_t*>(sound)[0xADC]&1)!=0) CallGameVirtual<void,6>(sound,2u,full?1u:0u);
                CallGameVirtual<void,1>(sound);
            }
            entry=CallGameFunction<Link*,0x4E0B20>(entry);
        }
        n=n->next;
        while (!n) n=*++bucket;
    }
    ClearHash(&s->hash);
}
}
Pair* MakePair(Pair* out,uint32_t low,uint32_t high,Tree source) {
    out->key[1]=high; out->key[0]=low;
    CallGameMethod<void,0x871C40>(&out->tree,&source);
    CallGameMethod<void,0x8715D0>(&source,source.sentinel.parent);
    return out;
}
Pair* CopyPair(Pair* out,const Pair* source) {
    out->key[0]=source->key[0]; out->key[1]=source->key[1];
    CallGameMethod<void,0x871C40>(&out->tree,&source->tree);
    return out;
}
Node* AllocateNode(Hash* self,const Pair* source) {
    auto n=static_cast<Node*>(Allocate(sizeof(Node),self->label));
    if (n) CopyPair(&n->value,source);
    n->next=nullptr; // EA writes through null after allocation failure.
    return n;
}
void Clear(SoundArray* self) {
    EnterCriticalSection(Mutex(self));
    if (self->active) {
        self->active=0;
        StopSounds(self,true);
        CallGameMethod<void,0x866E90>(&self->queue);
        self->queue.next=self->queue.prev=&self->queue;
    }
    LeaveCriticalSection(Mutex(self));
}
void Remove(SoundArray* self,uint32_t handle) {
    EnterCriticalSection(Mutex(self));
    if (CallGameMethod<uint8_t,0x871310>(self,handle) && self->active) {
        Iterator it{};
        CallGameMethod<void,0x626610>(&self->hash,&it);
        auto end=self->hash.buckets[self->hash.count];
        while (it.node!=end) {
            auto n=it.node; auto& t=n->value.tree;
            auto sentinel=&t.sentinel; auto candidate=sentinel;
            auto p=sentinel->parent;
            while (p) {
                if (reinterpret_cast<SoundEntry*>(p)->key<handle) p=p->left;
                else { candidate=p; p=p->right; }
            }
            if (candidate!=sentinel && handle<reinterpret_cast<SoundEntry*>(candidate)->key) candidate=sentinel;
            if (candidate!=sentinel) {
                --t.size;
                CallGameFunction<Link*,0x4E0B20>(candidate);
                ts3::misc::EraseResourceTree(reinterpret_cast<ts3::misc::ResourceTreeNode*>(candidate),reinterpret_cast<ts3::misc::ResourceTreeNode*>(sentinel));
                auto entry=reinterpret_cast<SoundEntry*>(candidate);
                if (entry->sound) CallGameVirtual<void,1>(entry->sound);
                candidate->left=reinterpret_cast<Link*>(t.free); t.free=entry;
            }
            if (!t.size) {
                Iterator next;
                CallGameMethod<Iterator*,0x871CD0>(&self->hash,&next,it);
                it=next;
            } else {
                it.node=n->next;
                while (!it.node) it.node=*++it.bucket;
            }
        }
    }
    LeaveCriticalSection(Mutex(self));
}
void Stop(SoundArray* self) {
    EnterCriticalSection(Mutex(self));
    if (self->active) StopSounds(self,false);
    LeaveCriticalSection(Mutex(self));
}
InsertResult* Insert(Hash* self,InsertResult* out,const Pair* value,uint32_t) {
    auto low=value->key[0]; auto index=low%self->count;
    auto n=self->buckets[index];
    while (n && (n->value.key[0]!=value->key[0] || n->value.key[1]!=value->key[1])) n=n->next;
    if (n) { out->node=n; out->bucket=&self->buckets[index]; out->inserted=0; return out; }
    struct Rehash { uint8_t needed; uint8_t pad[3]; uint32_t count; } rehash{};
    CallGameMethod<void,0x4E09B0>(self->policy,&rehash,self->count,self->size,1u);
    n=AllocateNode(self,value);
    if (rehash.needed) {
        index=low%rehash.count;
        CallGameMethod<void,0x871040>(self,rehash.count);
    }
    n->next=self->buckets[index]; self->buckets[index]=n; ++self->size;
    out->node=n; out->bucket=&self->buckets[index]; out->inserted=1;
    return out;
}
uintptr_t Destroy(SoundArray* self) {
#if defined(_M_IX86)
    self->table=reinterpret_cast<void*>(0x01028D3C);
#else
    self->table=nullptr;
#endif
    Clear(self);
    if (reinterpret_cast<intptr_t>(self->modeCap)-reinterpret_cast<intptr_t>(self->mode)>1 && self->mode) Free(self->mode);
    DeleteCriticalSection(Mutex(self));
    CallGameMethod<void,0x866E90>(&self->queue);
    auto result=ClearHash(&self->hash);
    if (self->hash.count>1) result=Free(self->hash.buckets);
#if defined(_M_IX86)
    self->table=reinterpret_cast<void*>(0x0107396C);
#endif
    return result;
}
InsertResult* Find(Hash* self,InsertResult* out,const Pair* value) {
    // Exact InsertAdapter twin: delegate with low-byte zero, return caller's output.
    Insert(self,out,value,0);
    return out;
}
void Process(SoundArray* self) {
    EnterCriticalSection(Mutex(self));
    if (self->queue.next!=&self->queue) {
        auto data=reinterpret_cast<QueueNode*>(self->queue.next)->data;
        Retain(data);
        float update=self->update;
#if defined(_M_IX86)
        __asm { fld update }
        __asm { fstp update }
#endif
        if (CallGameFunction<uint8_t,0x8722C0>(&self->hash,data,self->mode,update)) {
            if (self->queue.next!=&self->queue) CallGameMethod<void,0x759310>(&self->queue);
            self->active=1;
        }
        Release(data);
    }
    LeaveCriticalSection(Mutex(self));
}
uint8_t Enqueue(SoundArray* self,uint32_t count,void* bytes,uint32_t size) {
    EnterCriticalSection(Mutex(self));
    auto memory=Allocate(sizeof(Data),"Audio/SoundArray/data",false);
    auto data=memory?CallGameMethod<Data*,0x871210>(memory,count,bytes,size):nullptr;
    Retain(data);
    auto node=static_cast<QueueNode*>(Allocate(sizeof(QueueNode),self->label));
    if (reinterpret_cast<uintptr_t>(node)!=uintptr_t(0)-offsetof(QueueNode,data)) {
        node->data=data; Retain(data);
    }
    node->link.next=&self->queue; node->link.prev=self->queue.prev;
    self->queue.prev->next=&node->link; self->queue.prev=&node->link;
    if (count>self->hash.size && self->hash.size) Stop(self);
    Process(self);
    Release(data);
    LeaveCriticalSection(Mutex(self));
    return 1;
}
void Poll(SoundArray* self,uint32_t,uint32_t) {
    EnterCriticalSection(Mutex(self));
    if (self->queue.next!=&self->queue) Process(self);
    LeaveCriticalSection(Mutex(self));
}
void* Relocate(Deque* self,uint64_t request) {
    uint32_t amount=static_cast<uint32_t>(request), side=static_cast<uint32_t>(request>>32);
    auto old=self->begin;
    uint32_t used=static_cast<uint32_t>(self->end-old)+1;
    uint32_t front=static_cast<uint32_t>(old-self->map);
    uint32_t back=self->capacity-used-front;
    void** destination;
    if (side==1 && amount<=front) {
        amount=std::max(amount,front>>1); destination=self->map+(front-amount);
        std::memmove(destination,old,used*sizeof(void*));
    } else if (side==0 && amount<=back) {
        amount=std::max(amount,back>>1); destination=old+amount;
        std::memmove(destination,old,used*sizeof(void*));
    } else {
        auto capacity=std::max(self->capacity,amount)+self->capacity+2;
        auto map=static_cast<void**>(Allocate(capacity*sizeof(void*),self->label));
        destination=map+front+(side==0?amount:0);
        if (self->map) std::memcpy(destination,old,used*sizeof(void*));
        if (self->map) Free(self->map);
        self->map=map; self->capacity=capacity;
    }
    self->begin=destination; self->first=*destination;
    self->last=static_cast<uint8_t*>(self->first)+208;
    self->end=destination+used-1; self->firstEnd=*self->end;
    self->lastEnd=static_cast<uint8_t*>(self->firstEnd)+208;
    return self->last;
}
}
