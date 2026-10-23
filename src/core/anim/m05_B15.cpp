#include "ts3/anim/m05_B15.h"
#include <cstring>
namespace ts3::anim::b15 {
float Sentinel() { return 3.4028234663852886e+38f; }
float Time(Blend* s) {
    Child* c=s->second?s->second:s->first;
    return c?CallGameVirtual<float,7>(c):Sentinel();
}
uint32_t BlendType() { return 0x0681c688; }
void* BlendQuery(void* s,uint32_t id) {
    return id==0x0681c688 || id==0x7489d72b ? static_cast<unsigned char*>(s)-2*sizeof(void*):nullptr;
}
uint32_t TransformType() { return 0x0664fb68; }
void* TransformQuery(void* s,uint32_t id) {
    return id==0x0664fb68 || id==0x7489d72b ? static_cast<unsigned char*>(s)-2*sizeof(void*):nullptr;
}
namespace {
uint32_t Bits(float x) { uint32_t u; std::memcpy(&u,&x,4); return u; }
float Float(uint32_t u) { float x; std::memcpy(&x,&u,4); return x; }
uintptr_t Release(Child* c) {
    uint32_t r=--c->refs;
    if(r) return r;
    c->refs=1;
    return CallGameVirtual<uintptr_t,0>(c,1u);
}
}
uintptr_t Tick(Blend* s,float delta) {
    ++s->refs;
    if(s->first) CallGameVirtual<void,2>(s->first,delta);
    if(s->second) CallGameVirtual<void,2>(s->second,delta);
    if(s->duration>0.0f) {
        // SUBSS sign selection, NOT a numerical min/max. NaN payloads and -0 matter.
        float v=delta/s->duration; v+=s->phase;
        float a=-0.0f-v;
        v=Float(Bits(v)&(0u-(Bits(a)>>31)));
        a=1.0f-v;
        uint32_t mask=0u-(Bits(a)>>31);
        s->phase=Float((Bits(v)&~mask)|(Bits(1.0f)&mask));
    } else s->phase=1.0f;
    return Release(reinterpret_cast<Child*>(s));
}
void Clear(Blend* s) {
    auto clear=[](Child*& slot) {
        if(slot) {
            CallGameVirtual<void,1>(slot);
            Child* old=slot; slot=nullptr;
            if(old) Release(old);
        }
    };
    clear(s->first); clear(s->second);
}
bool ReadPositive(void*,Reader* r) {
    uint32_t value=0xffffffff;
    bool ok=CallGameFunction<bool,0x004dd700>(r->stream,&value,r->endian);
    CallGameMethod<void,0x007d77c0>(r,ok);
    return (r->good & (value<=1 ? 0xff:0))!=0;
}
void __stdcall FreeChains(Node** buckets,uint32_t count) {
    for(uint32_t i=0;i<count;++i) {
        Node* p=buckets[i];
        while(p) { Node* next=p->next; CallGameFunction<void,0x004e3f70>(p); p=next; }
        buckets[i]=nullptr;
    }
}
InsertResult* Insert(Table* s,InsertResult* out,const uint32_t* key,void*) {
    uint32_t hash=*key, bucket=hash%s->capacity;
    for(Node* p=s->buckets[bucket];p;p=p->next) if(p->key==hash) {
        out->node=p; out->bucket=&s->buckets[bucket]; out->inserted=0; return out;
    }
    struct Decision { uint8_t grow; unsigned char pad[3]; uint32_t capacity; } decision;
    CallGameMethod<void*,0x004e09b0>(s->policy,&decision,s->capacity,s->count,1u);
#if defined(_M_IX86)
    constexpr uintptr_t source=0x00f98980;
#else
    constexpr uintptr_t source=0;
#endif
    Node* p=CallGameFunction<Node*,0x004e3fb0>(0x40u,0x10u,0u,s->allocator,0u,0u,source,0xfbu);
    if(p) p->key=*key;
    p->next=nullptr; // EA allocation failure intentionally faults here.
    if(decision.grow) {
        bucket=hash%decision.capacity;
        CallGameMethod<void,0x005c9fc0>(s,decision.capacity);
    }
    p->next=s->buckets[bucket]; s->buckets[bucket]=p; ++s->count;
    out->node=p; out->bucket=&s->buckets[bucket]; out->inserted=1;
    return out;
}
void ForwardTick(Forward* s,float t) { if(s->child) CallGameVirtual<void,2>(s->child,t); }
void ForwardSet(Forward* s,float t) { if(s->child) CallGameVirtual<void,4>(s->child,t); }
float ForwardTime(Forward* s) { return s->child?CallGameVirtual<float,5>(s->child):0.0f; }
float ForwardDuration(Forward* s) { return s->child?CallGameVirtual<float,6>(s->child):0.0f; }
}
