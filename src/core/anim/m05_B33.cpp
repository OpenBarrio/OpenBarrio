#include "ts3/anim/m05_B33.h"
#include "ts3/scratch/clu_4e3f70_Scratch.h"
#include "ts3/swarm/clu_405a50_Swarm.h"
#include <cstring>
#include <atomic>
namespace ts3::anim::b33 {
uint32_t __fastcall StackIndex(void*,void*,uint32_t index) {
    volatile unsigned char temporary[56]; temporary[index*4]=0; return index;
}
uint32_t __fastcall TypeA(void*,void*) { return 0x00f9eb70; }
uint32_t __fastcall TypeB(void*,void*) { return 0x010231e0; }
uint8_t __fastcall Flag(const BuilderObject* self,void*) { return self->builder.enabled; }
static void Release(Ref* ref) {
    if (!ref) return;
    // Original last-release restores one before invoking the deleting virtual.
    auto& count=*reinterpret_cast<std::atomic<int32_t>*>(&ref->refs);
    if (count.fetch_sub(1)==1) { count.exchange(1); CallGameVirtual<void,0>(ref,1u); }
}
uint8_t __fastcall ReadEffect(void*,void*,void* description,Effect* output) {
    auto name=CallGameMethod<const char16_t*,0x00471070>(description,uintptr_t(0x00fc0c6c));
    auto end=name; while (*end) ++end;
    struct Text { char* begin; char* end; char* capacity; void* descriptor; } text;
    auto converted=CallGameFunction<Text*,0x004f38c0>(&text,name,uint32_t(end-name),uintptr_t(0x01023064));
    if (static_cast<void*>(converted)!=output)
        reinterpret_cast<ts3::swarm::ByteVector*>(output)->Assign(converted->begin,converted->end);
    if (reinterpret_cast<intptr_t>(text.capacity)-reinterpret_cast<intptr_t>(text.begin)>1 && text.begin)
        ts3::scratch::Free4E3F70(text.begin);
    auto hashName=CallGameMethod<const char16_t*,0x00471070>(description,uintptr_t(0x01023194));
    auto hash=CallGameFunction<uint32_t,0x007cfda0>(hashName);
    output->hash=hash;
    return 1;
}
void* __fastcall FindValue(Table* t,void*,uint32_t key) {
    struct Iterator {Node* node; Node** bucket;} it;
    CallGameMethod<void,0x00d4e550>(t,&it,&key);
    return it.node==t->buckets[t->count]?nullptr:&it.node->value;
}
Result* __fastcall Insert(Table* t,void*,Result* result,const uint32_t* pair,uintptr_t extra) {
    uint32_t bucket=*pair%t->count;
    for (auto n=t->buckets[bucket];n;n=n->next) if(n->key==*pair) {
        result->node=n; result->bucket=t->buckets+bucket; result->inserted=0; return result;
    }
    struct Policy { uint8_t grow; unsigned char pad[3]; uint32_t count; } policy;
    CallGameMethod<void,0x004e09b0>(t->policy,&policy,t->count,t->size,1u);
    Node* node=CallGameMethod<Node*,0x00b96bb0>(t,pair);
    if(policy.grow) { bucket=*pair%policy.count; CallGameMethod<void,0x00769ad0>(t,policy.count); }
    node->next=t->buckets[bucket]; t->buckets[bucket]=node; ++t->size;
    result->node=node; result->bucket=t->buckets+bucket; result->inserted=1;
    (void)extra; return result;
}
void __fastcall Recycle(Pool* pool,void*) {
    auto sentinel=reinterpret_cast<Link*>(pool);
    auto next=pool->first;
    while(next!=sentinel) { auto node=next; next=node->next; Release(node->value); node->next=pool->free; pool->free=node; }
}
void* Copy(const void* begin,const void* end,void* destination) {
    auto src=static_cast<const unsigned char*>(begin); auto dst=static_cast<unsigned char*>(destination);
    while(src!=end) { CallGameMethod<void,0x007578b0>(dst,src); src+=0x7c; dst+=0x7c; }
    return dst;
}
static void Empty(Builder* b) { b->begin=b->end=b->storage; b->capacity=b->storage+64; *b->begin=0; }
Builder* __fastcall Init(Builder* b,void*) {
    Empty(b); std::memset(b->values,0,sizeof(b->values)); b->flag=0; b->enabled=1; return b;
}
Builder* __fastcall Construct(Builder* b,void*,const uint32_t* words,const char16_t* name,uint8_t flag) {
    Empty(b); auto end=name; while(*end) ++end;
    CallGameMethod<void,0x0083b590>(b,name,end);
    std::memcpy(b->values,words,16); std::memset(b->values+4,0,20); b->flag=flag; b->enabled=1; return b;
}
Builder* __fastcall Assign(Builder* b,void*,const Builder* src) {
    if(b!=src) {
        if(b->begin!=b->end) { *b->begin=0; b->end=b->begin; }
        CallGameMethod<void,0x0083b590>(b,src->begin,src->end);
    }
    std::memcpy(b->values,src->values,sizeof(b->values)); b->flag=src->flag; b->enabled=src->enabled; return b;
}
void __fastcall Clear(void*,void*,StringNode** buckets,uint32_t count) {
    for(uint32_t i=0;i<count;++i) {
        auto node=buckets[i]; while(node) { auto old=node; node=node->next;
            if(reinterpret_cast<intptr_t>(old->capacity)-reinterpret_cast<intptr_t>(old->begin)>1 && old->begin)
                ts3::scratch::Free4E3F70(old->begin);
            ts3::scratch::Free4E3F70(old);
        } buckets[i]=nullptr;
    }
}
Ref** __fastcall Get(Table* t,void*,const uint32_t* key) {
    struct Iterator {Node* node; Node** bucket;} it;
    CallGameMethod<void,0x00d38b00>(t,&it,key);
    if(it.node!=t->buckets[t->count]) return &it.node->value;
    struct Pair {uint32_t key; Ref* value;} pair{*key,nullptr}; Result result;
    Insert(t,nullptr,&result,reinterpret_cast<const uint32_t*>(&pair),reinterpret_cast<uintptr_t>(key)&~uintptr_t(255));
    auto value=&result.node->value; Release(pair.value); return value;
}
uint8_t __fastcall Add(Owner* owner,void*,const char16_t* name,Ref* value) {
    if(!name) return 0;
    auto key=CallGameFunction<uint32_t,0x007cfda0>(name);
    struct Iterator {Node* node; Node** bucket;} it;
    auto t=&owner->table; CallGameMethod<void,0x00d38b00>(t,&it,&key);
    if(it.node!=t->buckets[t->count]) return 0;
    auto slot=Get(t,nullptr,&key); CallGameMethod<void,0x00b96150>(slot,value); return 1;
}
}
