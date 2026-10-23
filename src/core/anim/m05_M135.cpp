#include "ts3/anim/m05_M135.h"
#include "ts3/route/clu_848d20_Route.h"
#include <utility>
#include <intrin.h>
#include <cstring>
namespace ts3::anim::m135 {
namespace {
// The original uses FLD/FSTP, including signaling-NaN quieting and x87 exceptions.
void CopySpeed(float* destination,const float* source) {
#if defined(_M_IX86)
    __asm { mov eax, source }
    __asm { mov edx, destination }
    __asm { fld dword ptr [eax] }
    __asm { fstp dword ptr [edx] }
#else
    uint32_t bits; std::memcpy(&bits,source,4);
    if((bits&0x7f800000u)==0x7f800000u && (bits&0x007fffffu)) bits|=0x00400000u;
    std::memcpy(destination,&bits,4);
#endif
}
struct Controller { void* table; int32_t refs; void* interface8; void* interfaceC; unsigned char gap[12]; void* host; uint32_t id; uint8_t finished; };
struct Node : Link { Controller* controller; uint32_t kind; };
void Detach(Body* b,Node* n) { CallGameMethod<void,0x00566590>(&n->controller->interfaceC,&b->secondary); }
void Fix(Link* s,Link* other) {
    if(s->next==other) s->next=s->previous=s;
    else { s->previous->next=s; s->next->previous=s; }
}
void Swap(Link* a,Link* b) { std::swap(a->next,b->next); std::swap(a->previous,b->previous); Fix(a,b); Fix(b,a); }
void Completed(Body* b,uint32_t kind) {
    if(kind==3) ++b->counters[2];
    else if(kind==10) { --b->count; b->flags[0]=0; }
    else if(kind==9) { ++b->count; b->flags[1]=0; }
}
void Remove(Node* n) {
    n->previous->next=n->next; n->next->previous=n->previous;
    auto c=n->controller;
    if(c) {
        auto previous=static_cast<uint32_t>(c->refs);
        c->refs=static_cast<int32_t>(previous-1u);
        if(previous==1) { c->refs=1; CallGameVirtual<void,0>(c,1); }
    }
    CallGameFunction<void,0x004E3F70>(n);
}
struct Resource { void* table; volatile long refs; };
void AddRef(Resource* r) { if(r) _InterlockedIncrement(&r->refs); }
void Release(Resource* r) { if(r && !_InterlockedDecrement(&r->refs)) { _InterlockedExchange(&r->refs,1); CallGameVirtual<void,0>(r,1); } }
uint32_t StyleWord(Body* b,size_t offset) { return *reinterpret_cast<uint32_t*>(b->style+offset-0x28); }
}
uint32_t Type(void*) { return 0xf4cfd7da; }
void* Query(void* self,uint32_t id) { return id==0x54cfb94f || id==0xf4cfd7da ? self : nullptr; }
uint8_t Export(Body* b,State* s) {
    CallGameMethod<void,0x0084AF20>(b);
    if(b->stateF0[0x21]) return 0;
    for(auto p=b->controllers.next;p!=&b->controllers;p=p->next) Detach(b,static_cast<Node*>(p));
    Swap(&b->controllers,&s->controllers);
    CopySpeed(&s->speed,&b->speed);
    for(unsigned i=0;i<3;++i) s->counters[i]=b->counters[i];
    s->count=b->count; s->target=b->target;
    s->flags[0]=b->flags17c[2]; s->flags[1]=b->flags17c[3];
    s->flags[2]=b->flags[0]; s->flags[3]=b->flags[1];
    s->flags[5]=b->flags[2]; s->flags[6]=b->flags[3];
    return 1;
}
uint8_t Restore(Body* b,State* s) {
    CallGameFunction<void,0x0050ABE0>(&b->controllers,&s->controllers);
    CopySpeed(&b->speed,&s->speed);
    for(unsigned i=0;i<3;++i) b->counters[i]=s->counters[i];
    b->count=s->count; b->target=s->target;
    b->flags17c[2]=s->flags[0]; b->flags17c[3]=s->flags[1];
    b->flags[0]=s->flags[2]; b->flags[1]=s->flags[3];
    b->flags[2]=s->flags[5]; b->flags[3]=s->flags[6]; b->flags[4]=s->flags[7];
    unsigned active=0;
    for(auto p=b->controllers.next;p!=&b->controllers;) {
        auto n=static_cast<Node*>(p); auto c=n->controller;
        if(c && CallGameVirtual<uint8_t,4>(c->host,c)) {
            c=n->controller; // the host callback may replace the retained controller
            auto special=CallGameVirtual<void*,1>(&c->interface8,0x073b9d8c);
            if(special) {
                auto hostInterface=CallGameVirtual<void*,1>(b->host,0x14cfd2f8);
                void* resource=hostInterface?CallGameMethod<void*,0x00955EF0>(hostInterface):nullptr;
                auto canvas=CallGameMethod<void*,0x00C93970>(b->host);
                CallGameMethod<void,0x0056E640>(special,canvas);
                CallGameMethod<void,0x0056E690>(special,resource);
                auto data=static_cast<unsigned char*>(special);
                *reinterpret_cast<uint32_t*>(data+60)=StyleWord(b,0x7c);
                *reinterpret_cast<uint32_t*>(data+64)=StyleWord(b,0x84);
            }
            c=n->controller;
            if(!c->finished) {
                ++active;
                CallGameMethod<void,0x00566170>(&c->interfaceC,&b->secondary,n->kind);
                p=p->next; continue;
            }
            Completed(b,n->kind);
        } else {
            c=n->controller;
            if(c) { Detach(b,n); if(n->controller->finished) Completed(b,n->kind); }
        }
        // EA follows next->previous after detach callbacks, not a cached node.
        auto next=p->next; Remove(static_cast<Node*>(next->previous)); p=next;
    }
    if(!b->flags17c[3]) {
        auto resource=CallGameMethod<Resource*,0x00C93970>(b->host); AddRef(resource);
        auto key=CallGameVirtual<uint64_t,4>(b->master);
        AddRef(resource);
        float threshold=CallGameFunction<float,0x00849810>(resource,key,StyleWord(b,0x7c),StyleWord(b,0x84),1);
        while(b->target) {
            float time=CallGameMethod<float,0x00848890>(b,0);
            if(!(time>threshold)) break; // unordered x87 compare also breaks
            --b->target;
        }
        int mode=static_cast<int32_t>(b->counters[1])>0 || b->count ? 3:2;
        if(active) { b->flags17c[6]=1; b->flags17c[7]=mode==2 || s->flags[4]; }
        else {
            if(s->flags[4]) {
                b->count=b->target;
                alignas(16) unsigned char location[16];
                CallGameVirtual<void,7>(b->host,location);
                float value=static_cast<float>(CallGameVirtual<double,70>(resource,location));
                auto hostKey=CallGameVirtual<uint64_t,4>(b->host);
                if(value>CallGameVirtual<double,41>(resource,hostKey)) {
                    hostKey=CallGameVirtual<uint64_t,4>(b->host);
                    CallGameVirtual<void,42>(resource,value,hostKey);
                }
            }
            reinterpret_cast<ts3::route::RouteTuner*>(b)->Update84AF90(mode,static_cast<int8_t>(mode==2 || s->flags[4]));
        }
        if(b->count==b->target) {
            CallGameMethod<void,0x0084CE30>(b,0);
            struct Actor { void* actor; uint32_t gap[3],id; };
            Actor storage[4]; struct Range { Actor* begin; Actor* end; Actor* capacity; } range{storage,storage,storage+4};
#if defined(_M_IX86)
            auto manager=*reinterpret_cast<void**>(0x011d9140);
#else
            void* manager=nullptr;
#endif
            CallGameMethod<void,0x00844820>(manager,StyleWord(b,0x78),&range);
            for(auto p=range.begin;p!=range.end;++p) CallGameMethod<void,0x0084C3B0>(b,p->actor,p->id,1);
            CallGameMethod<void,0x00912B60>(&range,range.begin,range.end);
        }
        Release(resource);
    }
    return 1;
}
Progress* Reset(Body* b) {
    for(auto p=b->controllers.next;p!=&b->controllers;p=p->next) Detach(b,static_cast<Node*>(p));
    CallGameMethod<void,0x0081F320>(&b->controllers);
    b->controllers.next=b->controllers.previous=&b->controllers;
    for(auto& c:b->counters) c=0xffffffff;
    b->reset128=0; b->speed=1;
    for(auto& w:b->weights) w=1;
    for(unsigned i:{0u,1u,2u,3u,4u,6u,7u}) b->flags17c[i]=0;
    auto end=b->end; // EA snapshots the range before callbacks/free operations
    for(auto p=b->begin;p<end;++p) if(p->begin && p->begin!=p->inlineBegin) CallGameFunction<void,0x004E3F70>(p->begin);
    b->end=b->begin; b->count=b->target=0;
    return b->begin;
}
Body* Construct(Body* b,void* host,uint32_t arg) {
    CallGameMethod<void,0x008397E0>(b,host);
    auto address=[](uintptr_t x)->void* {
#if defined(_M_IX86)
        return reinterpret_cast<void*>(x);
#else
        (void)x; AbortNotRewritten("AnimatedSimBody game descriptors"); return nullptr;
#endif
    };
    b->secondary=address(0x00fc2104);
    b->vtable=address(0x01027294); b->interface4=address(0x01027290); b->secondary=address(0x0102727c);
    b->controllerDescriptor=address(0x0102724c); b->controllers.next=b->controllers.previous=&b->controllers;
    CallGameMethod<void,0x007513F0>(b->style);
    b->effectsDescriptor=address(0x0102721c); b->effects.next=b->effects.previous=&b->effects;
    b->b0=0; b->slaves=b->slavesEnd=b->slavesCapacity=nullptr; b->slaveDescriptor=address(0x010271ec);
    b->master=b->host; b->c8=0; b->cc[0]=0;
    CallGameMethod<void,0x0085A7F0>(b->stateD0); CallGameMethod<void,0x0085A900>(b->stateF0);
    b->maxTime=3.402823466e38f; b->brainTable=address(0x0102700c); b->argument=arg; b->reset128=0;
    for(auto& v:b->values12c) v=0;
    for(auto& v:b->values13c) v=0;
    for(auto& w:b->weights) w=1;
    b->speed=b->speed168=1;
    for(auto& c:b->counters) c=0xffffffff;
    b->value178=0; for(auto& f:b->flags17c) f=0;
    b->inlineBegin=b->begin=b->end=b->storage; b->capacity=b->storage+6;
    b->descriptor=address(0x010271b8); b->value244=b->count=b->target=0;
    b->flags[0]=b->flags[1]=b->flags[4]=0; b->flags[2]=b->flags[3]=1;
    CallGameMethod<void,0x008399E0>(b,b->stateD0);
    CallGameMethod<void,0x008399E0>(b,b->stateF0);
    CallGameMethod<void,0x008399E0>(b,&b->brainTable);
    return b;
}
}
namespace ts3::anim::m135::hooks {
uint8_t __fastcall Export(Body* b,void*,State* s) { return m135::Export(b,s); }
uint8_t __fastcall Restore(Body* b,void*,State* s) { return m135::Restore(b,s); }
Progress* __fastcall Reset(Body* b,void*) { return m135::Reset(b); }
Body* __fastcall Construct(Body* b,void*,void* h,uint32_t a) { return m135::Construct(b,h,a); }
uint32_t __fastcall Type(void* b,void*) { return m135::Type(b); }
void* __fastcall Query(void* b,void*,uint32_t id) { return m135::Query(b,id); }
}
