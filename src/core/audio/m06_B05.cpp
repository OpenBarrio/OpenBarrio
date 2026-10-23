#include "ts3/audio/m06_B05.h"
#include <cstring>
namespace ts3::audio::b05 {
namespace {
void X87Payload(float* p) {
#if defined(_M_IX86)
    __asm { mov eax,p }
    __asm { fld dword ptr [eax] }
    __asm { fstp dword ptr [eax] }
#else
    (void)p;
#endif
}
Node* Find(State* s,Key k) {
    for(Node* n=s->hash.buckets[k.low%53u];n;n=n->next)
        if(n->key.low==k.low && n->key.high==k.high) return n;
    return s->hash.sentinel;
}
Node** First(Hash& h) { Node** b=h.buckets; while(!*b) ++b; return b; }
void Recycle(State* s,Node** b,Node* n) {
    Node** link=b; while(*link!=n) link=&(*link)->next;
    *link=n->next; --s->hash.count; n->next=s->free; s->free=n;
}
template<class F> void Groups(State* s,F f) {
    Group** b=s->groups; while(!*b) ++b;
    Group* n=*b; Group* end=s->groups[s->groupCount];
    while(n!=end) { f(n->object); n=n->next; while(!n) n=*++b; }
}
}
// Adapted from verified m03::c04::Insert: modulus 7919 -> 53, count +7BC0 -> +D8.
Result* __fastcall Insert(Hash* h,void*,Result* out,Node* n,uint32_t) {
    Node** b=&h->buckets[n->key.low%53u];
    for(Node* p=*b;p;p=p->next) if(p->key.low==n->key.low && p->key.high==n->key.high) {
        out->node=p; out->bucket=b; out->inserted=0; return out;
    }
    n->next=*b; *b=n; out->node=n; out->bucket=b; ++h->count; out->inserted=1; return out;
}
Node* __fastcall Mark(State* s,void*,Key k) {
    Node* n=Find(s,k); if(n!=s->hash.sentinel) { n->flags|=1; n->key={0,0}; ++s->pending; } return n;
}
Node* __fastcall Vector(State* s,void*,Key k,const float* v) {
    Node* n=Find(s,k); if(n!=s->hash.sentinel) std::memcpy(n->vector,v,16); return n;
}
Node* __fastcall Flags(State* s,void*,Key k,uint32_t bits,uint32_t mask) {
    Node* n=Find(s,k); if(n!=s->hash.sentinel) n->flags=(n->flags&~mask)|bits; return n;
}
uintptr_t __fastcall Stop(State* s,void*) {
    Groups(s,[](void* p){CallGameVirtual<void,10>(p);});
    void* commands=CallGameFunction<void*,0x008B0A70>();
    CallGameVirtual<void,17>(commands,0xEA7FBBA5u);
    return CallGameVirtual<uintptr_t,26>(commands);
}
Node** __fastcall Prune(State* s,void*) {
    Node** b=First(s->hash); Node* n=*b;
    while(n!=s->hash.sentinel) {
        Node* next=n->next;
        if(n->flags&1) { Recycle(s,b,n); --s->pending; }
        n=next; while(!n) n=*++b;
    } return b;
}
Node** __fastcall Clear(State* s,void*) {
    Node** b=First(s->hash); Node* n=*b;
    while(n!=s->hash.sentinel) { Node* next=n->next; Recycle(s,b,n); n=next; while(!n) n=*++b; }
    s->pending=0; return b;
}
void* __cdecl CopyBackward(const void* first,const void* last,void* dest) {
    auto b=static_cast<const uint8_t*>(first); auto e=static_cast<const uint8_t*>(last); auto d=static_cast<uint8_t*>(dest);
    while(e!=b) { e-=500; d-=500; for(size_t i=0;i<500;i+=4) std::memcpy(d+i,e+i,4); } return d;
}
Node** __fastcall Disable(State* s,void*) { s->active=0; Stop(s,nullptr); return Clear(s,nullptr); }
void __fastcall Tick(State* s,void*,float dt) {
    if(!s->active) return;
    Groups(s,[&](void* p){
        CallGameVirtual<void,13>(p,&s->hash,s);
        float payload=dt;
        // The producer reloads through x87 after the callback (including sNaN quieting).
        X87Payload(&payload);
        CallGameVirtual<void,12>(p,payload);
    });
    if(s->pending>0) Prune(s,nullptr);
}
void __fastcall Add(State* s,void*,Key k,uint32_t value) {
    Node* n=s->free;
    if(!n) n=CallGameMethod<Node*,0x008625B0>(&s->free);
    if(!n) return;
    s->free=n->next; n->key=k; n->value=value; n->flags=0; std::memset(n->vector,0,16);
    Result out; Insert(&s->hash,nullptr,&out,n,0);
}
bool __fastcall Apply(State* s,void*,void* target,Key k) {
    Group* n=s->groups[k.low%s->groupCount];
    while(n && (n->key.low!=k.low || n->key.high!=k.high)) n=n->next;
    if(!n || n==s->groups[s->groupCount]) return false;
    void* resource=nullptr;
    auto ok=CallGameVirtual<uint8_t,89>(target,k.low,k.high,&resource,0x1407ECu);
    if(!ok) { if(resource) CallGameVirtual<void,1>(resource); return false; }
    void* object=n->object;
    CallGameVirtual<void,32>(target,object?static_cast<uint8_t*>(object)+4:nullptr);
    bool result=CallGameVirtual<uint8_t,9>(object,resource)!=0;
    if(resource) CallGameVirtual<void,1>(resource);
    return result;
}
}
