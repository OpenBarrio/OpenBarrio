#include "ts3/anim/m05_B14.h"
#include <cstring>
#include <limits>
#include <initializer_list>
namespace ts3::anim::b14 {
uint32_t __fastcall TypeAAC0(void*,void*) { return 0x060d99b1; }
void* __fastcall QueryAAD0(void* p,void*,uint32_t id) { return id==0x060d99b1 || id==0x7489d72b ? static_cast<unsigned char*>(p)-8 : nullptr; }
uint32_t __fastcall TypeAC20(void*,void*) { return 0x060caeef; }
void* __fastcall QueryAC30(void* p,void*,uint32_t id) { return id==0x060caeef || id==0x060d99b1 || id==0x7489d72b ? static_cast<unsigned char*>(p)-8 : nullptr; }
uint32_t __fastcall TypeB810(void*,void*) { return 0x0664ed68; }
void* __fastcall QueryB820(void* p,void*,uint32_t id) { return id==0x0664ed68 || id==0x7489d72b ? static_cast<unsigned char*>(p)-8 : nullptr; }
float __fastcall DefaultDuration(void*,void*) {
#if defined(_M_IX86)
    return *reinterpret_cast<const float*>(0x0114f594);
#else
    return -1.0f;
#endif
}
uint32_t __fastcall Nonnegative(State* p,void*) { return p->weight>=0.0f; }
uint32_t __fastcall Complete(State* p,void*) { return p->weight>=1.0f; }
float __fastcall Time(State* p,void*) { return p->time; }
static void Copy(Record* d,const Record* s) {
    std::memcpy(d->vector,s->vector,16); d->key=s->key;
    // FLD/FSTP quiets signalling NaNs in these two scalar fields only.
    uint32_t a,b; std::memcpy(&a,&s->a,4); std::memcpy(&b,&s->b,4);
    if((a&0x7f800000)==0x7f800000 && (a&0x7fffff)) a|=0x400000;
    if((b&0x7f800000)==0x7f800000 && (b&0x7fffff)) b|=0x400000;
    std::memcpy(&d->a,&a,4); std::memcpy(&d->b,&b,4);
}
void* __fastcall Insert(Range* p,void*,Record* pos,const Record* value) {
    if(p->end!=p->capacity) {
        auto* src=value;
        if(src>=pos && src<p->end) ++src;
        if(p->end) Copy(p->end,p->end-1);
        auto* eax=p->end;
        for(auto* q=p->end-1;q!=pos;) { --q; --eax; Copy(eax,q); }
        Copy(pos,src); ++p->end; return eax;
    }
    size_t n=p->end-p->begin,capacity=n ? n*2 : 1;
    Record* storage=nullptr;
#if defined(_M_IX86)
    storage=CallGameFunction<Record*,0x004e3fb0>(uint32_t(capacity*32),16,0,p->descriptor,0,0,reinterpret_cast<const char*>(0x00f98980),0xfb);
#else
    AbortNotRewritten("m05 B14 aligned allocator");
#endif
    auto* out=storage;
    for(auto* q=p->begin;q!=pos;++q,++out) if(out) Copy(out,q);
    if(out) Copy(out,value);
    ++out;
    for(auto* q=pos;q!=p->end;++q,++out) if(out) Copy(out,q);
    if(p->begin) CallGameFunction<void,0x004e3f70>(p->begin);
    p->begin=storage; p->end=out; p->capacity=storage+capacity;
    return p->capacity;
}
uint32_t __fastcall Update(Controller* p,void*,uint32_t) {
    float now=0;
#if defined(_M_IX86)
    auto* clock=*reinterpret_cast<unsigned char**>(0x011ecbc4);
    if(clock) now=*reinterpret_cast<float*>(clock+0x124)*0.01666666753590107f;
#endif
    Node** bucket=p->buckets; while(!*bucket) ++bucket;
    Node* node=*bucket; Node* end=p->buckets[p->count];
    uint32_t result=p->count;
    while(node!=end) {
        float duration=node->duration;
        // LAHF overwrites AH of the count held in EAX, including on skipped nodes.
        uint32_t flags=duration!=duration?0x47u:duration==0.0f?0x42u:duration<0.0f?0x03u:0x02u;
        result=(result&~0xff00u)|(flags<<8);
        if(duration!=0.0f) {
            float delta=now-node->stamp;
            if(node->stamp>now) delta=delta+1440.0f;
            float elapsed=delta+node->elapsed;
            node->stamp=now; node->elapsed=elapsed;
            if(elapsed>=duration) std::memcpy(node->current,node->target,16);
            else { float t=elapsed/duration; for(int i=0;i<4;++i) { float d=node->target[i]-node->start[i]; float v=d*t; node->current[i]=v+node->start[i]; } }
        }
        node=node->next;
        if(!node) { do { ++bucket; node=*bucket; } while(!node); }
    }
    return result;
}
uint32_t __fastcall Save(Controller* p,void*,void* stream) {
    auto* s=static_cast<unsigned char*>(stream);
    auto integer=[&](uint32_t value) { auto r=CallGameFunction<uint32_t,0x004ddbd0>(*reinterpret_cast<void**>(s+8),value,*reinterpret_cast<uint32_t*>(s+16)); return CallGameMethod<uint32_t,0x007d77c0>(s,r); };
    uint32_t r=integer(2); if(!s[0x14]) return r&~255u;
    r=integer(p->tag); if(!s[0x14]) return r&~255u;
    struct Iterator { Node* node; Node** bucket; } it;
    CallGameMethod<void*,0x00626610>(&p->unknown,&it);
    Node* end=p->buckets[p->count];
    r=static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end));
    while(it.node!=end) {
        auto* n=it.node;
        r=CallGameFunction<uint32_t,0x004ddcc0>(*reinterpret_cast<void**>(s+8),n->key,*reinterpret_cast<uint32_t*>(s+16));
        r=CallGameMethod<uint32_t,0x007d77c0>(s,r);
        r=CallGameMethod<uint32_t,0x007d7960>(s,n->start);
        r=CallGameMethod<uint32_t,0x007d7960>(s,n->target);
        r=CallGameMethod<uint32_t,0x007d7960>(s,n->current);
        for(float f:{n->duration,n->elapsed,n->stamp}) { r=CallGameFunction<uint32_t,0x004ddff0>(*reinterpret_cast<void**>(s+8),f,*reinterpret_cast<uint32_t*>(s+16)); r=CallGameMethod<uint32_t,0x007d77c0>(s,r); }
        it.node=n->next; if(!it.node) { do { ++it.bucket; it.node=*it.bucket; } while(!it.node); }
    }
    return (r&~255u)|s[0x14];
}
static void Release(Ref* p) { if(p && --p->refs==0) { p->refs=1; CallGameVirtual<void,0>(p,1); } }
void __fastcall Reset(State* p,void*) { auto* a=p->first; if(a) { p->first=nullptr; Release(a); } auto* b=p->second; if(b) { p->second=nullptr; Release(b); } p->weight=-1.0f; p->time=-1.0f; }
Ref* __fastcall Set(State* p,void*,Ref* a,Ref* b,float time,float weight) { Reset(p,nullptr); if(a!=p->first) { if(a) ++a->refs; auto* old=p->first; p->first=a; Release(old); } if(b!=p->second) { if(b) ++b->refs; auto* old=p->second; p->second=b; Release(old); } p->time=time; p->weight=weight; return b; }
}
