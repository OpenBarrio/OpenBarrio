#include "ts3/anim/m05_M118.h"
#include "ts3/swarm/clu_405a50_Swarm.h"
#include <cstring>
#include <cmath>
#if defined(_M_IX86)
#include <intrin.h>
#endif
namespace ts3::anim::m118 {
namespace {
uint32_t Bits(float f) { uint32_t b; std::memcpy(&b,&f,4); return b; }
float Sign(float f) {
    // COMISS/JB treats unordered as negative; -0 is tested by bits first.
    return Bits(f)==0x80000000u || !(f>=0.0f) ? -1.0f : 1.0f;
}
void Retain(Controller* c) {
#if defined(_M_IX86)
    _InterlockedIncrement(reinterpret_cast<volatile long*>(&c->refs));
#else
    ++c->refs;
#endif
}
void Release(Controller* c) {
#if defined(_M_IX86)
    const auto count=_InterlockedDecrement(reinterpret_cast<volatile long*>(&c->refs));
#else
    const auto count=--c->refs;
#endif
    if(count==0) {
#if defined(_M_IX86)
        _InterlockedExchange(reinterpret_cast<volatile long*>(&c->refs),1);
#else
        c->refs=1;
#endif
        CallGameVirtual<void*,0>(c,uint32_t(1));
    }
}
template<unsigned Factory> uint8_t Dispatch(Builder* self,uint32_t arg,uint32_t remember) {
    float phase; std::memcpy(&phase,&arg,4);
    const int mode=CallGameVirtual<int,9>(self,arg,&phase);
    if(static_cast<uint8_t>(remember)) self->current=phase;
    uint8_t accepted=0;
    for(Node* n=self->sentinel.next;n!=&self->sentinel;n=n->next) {
        bool matches=false;
        if(mode==0) matches=phase==n->phase && Sign(phase)==Sign(n->phase);
        else if(mode==1) matches=Sign(phase)==Sign(n->phase);
        else if(mode==2) matches=true;
        if(matches) accepted|=CallGameVirtual<uint8_t,5>(n->controller,arg,uint32_t(0));
    }
    if(accepted) return 1;
    struct Pair { Controller* controller; float phase; } fresh{nullptr};
    CallGameFunction<void,Factory>(&fresh,&phase);
    Controller* c=fresh.controller;
    if(!CallGameVirtual<uint8_t,5>(c,arg,uint32_t(0))) { Release(c); return 0; }
    Node* at=self->sentinel.next;
    while(at!=&self->sentinel) {
        if(at->phase>fresh.phase && Sign(at->phase)>=Sign(fresh.phase)) break;
        at=at->next;
    }
    Node* n=reinterpret_cast<Node*>(self->sentinel.controller);
    if(n) self->sentinel.controller=reinterpret_cast<Controller*>(n->next);
    // EA deliberately does not allocate or check for an exhausted pool.
    n->controller=c;
    Retain(c);
    n->phase=fresh.phase;
    n->next=at;
    n->previous=at->previous;
    at->previous->next=n;
    at->previous=n;
    Release(c);
    return 1;
}
void Destroy(Vector* v) { CallGameMethod<void,0x00912B60>(v,v->begin,v->end); }
void Copy(Vector* v,Effect* begin,Effect* end,uintptr_t context) {
    CallGameMethod<void,0x0083ED50>(v,begin,end,context);
}
void FreeString(String& s) {
    if(s.capacity-s.begin>1 && s.begin) CallGameFunction<void,0x004E3F70>(s.begin);
}
}
uint8_t __fastcall Dispatch4(Builder* s,void*,uint32_t a,uint32_t f) { return Dispatch<0x0083FCB0>(s,a,f); }
uint8_t __fastcall Dispatch5(Builder* s,void*,uint32_t a,uint32_t f) { return Dispatch<0x0083FD50>(s,a,f); }
uint8_t __fastcall Dispatch6(Builder* s,void*,uint32_t a,uint32_t f) { return Dispatch<0x0083FDF0>(s,a,f); }
Vector* __fastcall Assign(Vector* self,void*,Vector* source) {
    if(self!=source) {
        Destroy(self);
        self->end=self->begin;
        Copy(self,source->begin,source->end,reinterpret_cast<uintptr_t>(source));
    }
    return self;
}
Entry* __fastcall Clone(void* self,void*,Entry* source) {
    auto allocator=reinterpret_cast<void**>(self)[7];
    auto p=CallGameFunction<Entry*,0x004E3F90>(sizeof(Entry),allocator,uint32_t(0),uint32_t(0),
#if defined(_M_IX86)
        reinterpret_cast<const char*>(0x00F98980),
#else
        static_cast<const char*>(nullptr),
#endif
        uint32_t(0xe7));
    if(p) {
        p->key=source->key;
        p->effects.begin=p->effects.end=p->storage;
        p->effects.capacity=p->storage+4;
        Copy(&p->effects,source->effects.begin,source->effects.end,reinterpret_cast<uintptr_t>(source));
    }
    p->state=0;
    return p;
}
uint8_t __fastcall Effects(void* self,void*,uint32_t key,uint32_t a,uint32_t b,uint32_t c,uint32_t mask,Vector* out) {
    (void)a; (void)b; (void)c;
#if defined(_M_IX86)
    auto map=static_cast<unsigned char*>(self)+0xb8;
    unsigned char* entry=nullptr;
    CallGameMethod<void,0x0083B960>(map,&entry,&key);
    auto buckets=*reinterpret_cast<unsigned char***>(map+4);
    auto count=*reinterpret_cast<uint32_t*>(map+8);
    if(entry==buckets[count] || !(static_cast<uint16_t>(mask)&*reinterpret_cast<uint16_t*>(entry+32))) return 0;
    unsigned mode;
    switch(static_cast<uint16_t>(mask)) { case 1: mode=0; break; case 2: mode=1; break; case 4: mode=2; break; default: return 0; }
    auto empty=reinterpret_cast<char*>(0x011c7e54);
    String name{empty,empty,empty+1,reinterpret_cast<const char*>(0x01023520)};
    Effect storage[4]; Vector temp{storage,storage,storage+4};
    for(unsigned i=0;i<4;++i) {
        if(!(*reinterpret_cast<uint16_t*>(entry+34)&(1u<<i))) continue;
        if(name.begin!=name.end) { *name.begin=0; name.end=name.begin; }
        CallGameMethod<void,0x00405790>(&name,*reinterpret_cast<char**>(entry+16),*reinterpret_cast<char**>(entry+20));
        auto suffix=reinterpret_cast<const char**>(0x01166204)[mode];
        CallGameMethod<void,0x00405790>(&name,suffix,suffix+std::strlen(suffix));
        suffix=reinterpret_cast<const char**>(0x01166210)[i];
        CallGameMethod<void,0x00405790>(&name,suffix,suffix+std::strlen(suffix));
        Effect value{{empty,empty,empty+1,reinterpret_cast<const char*>(0x01023064)},0};
        // Use the already-verified byte-vector assignment, retaining its grow bridge.
        ts3::swarm::ByteVector valueName{reinterpret_cast<uint8_t*>(empty),reinterpret_cast<uint8_t*>(empty),reinterpret_cast<uint8_t*>(empty+1)};
        valueName.Assign(name.begin,name.end);
        value.name.begin=reinterpret_cast<char*>(valueName.begin);
        value.name.end=reinterpret_cast<char*>(valueName.end);
        value.name.capacity=reinterpret_cast<char*>(valueName.capacity);
        value.kind=reinterpret_cast<uint32_t*>(0x01166220)[i];
        Effect* dest=temp.end++;
        CallGameMethod<void,0x00405AD0>(dest,&value);
        dest->kind=value.kind;
        FreeString(value.name);
    }
    if(out!=&temp) { Destroy(out); out->end=out->begin; Copy(out,temp.begin,temp.end,mode); }
    FreeString(name);
    Destroy(&temp);
    return 1;
#else
    (void)self; (void)key; (void)mask; (void)out;
    AbortNotRewritten("m118 original style-map and string dependencies"); return 0;
#endif
}
}
