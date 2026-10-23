#include "ts3/anim/m05_M01.h"
#include <cstring>
namespace ts3::anim::m01 {
namespace {
template<unsigned A> void* Address() {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(uintptr_t(A));
#else
    AbortNotRewritten("M01 game address"); return nullptr;
#endif
}
uintptr_t TrueAL(uintptr_t r) { return (r & ~uintptr_t(255)) | 1; }
// The bool-only B01 serialization API does not expose the payload's upper EAX.
// Mirror its four-byte cursor operation here to retain that raw ABI contract.
bool Advance(Cursor* s,bool writing) {
    s->position=static_cast<int32_t>(static_cast<uint32_t>(s->position)+4u);
    if(s->position>=0) {
        if(writing) CallGameMethod<void,0x00594140>(s,4);
        else CallGameMethod<void,0x00593ff0>(s,4);
    }
    return s->position<0;
}
void Recycle(b02::Pool* pool) {
    auto* end=reinterpret_cast<b02::PoolNode*>(pool);
    auto* p=reinterpret_cast<b02::PoolNode*>(pool->next);
    while(p!=end) {
        auto* next=p->next;
        p->next=pool->free; pool->free=p; p=next;
    }
}
}
uintptr_t EraseFactory(uint32_t key) {
    return TrueAL(b01::Erase(static_cast<b01::Registry*>(Address<0x011cd120>()),&key));
}
uintptr_t SaveCurrent(Registry* self,Cursor* s) {
    auto* n=static_cast<Node*>(self->current);
    uint32_t type=CallGameVirtual<uint32_t,0>(n);
    if(Advance(s,true)) std::memcpy(s->output+static_cast<ptrdiff_t>(s->position)-3,&type,4);
    return TrueAL(CallGameVirtual<uintptr_t,9>(n,s));
}
uintptr_t LoadCurrent(Registry* self,Cursor* s) {
    uint32_t type=static_cast<uint32_t>(reinterpret_cast<uintptr_t>(s));
    if(Advance(s,false)) std::memcpy(&type,s->input+static_cast<ptrdiff_t>(s->position)-3,4);
    auto* n=CallGameMethod<Node*,0x0054c770>(self,type);
    self->current=n;
    return TrueAL(CallGameVirtual<uintptr_t,10>(n,s));
}
uintptr_t Clear(Registry* self) {
    auto* pool=&self->pool;
    auto* end=reinterpret_cast<b02::PoolNode*>(pool);
    auto* p=reinterpret_cast<b02::PoolNode*>(pool->next);
    while(p!=end) {
        auto* n=static_cast<Node*>(p->payload[1]);
        auto type=CallGameVirtual<uint32_t,0>(n);
        auto* factory=b01::Find(type);
        CallGameVirtual<void,1>(factory,n);
        p=p->next; // callbacks precede the next-link read
    }
    self->current=nullptr;
    Recycle(pool);
    pool->next=pool->previous=pool;
    CallGameMethod<void,0x004e87f0>(&self->heap);
    return CallGameMethod<uintptr_t,0x004e85b0>(&self->heap,self->workspace,0x4000,
        Address<0x005bfb90>(),Address<0x00c0fd60>(),0);
}
uintptr_t Destroy(Registry* self) {
    Clear(self);
    Recycle(&self->pool);
    return CallGameMethod<uintptr_t,0x004e87f0>(&self->heap);
}
Node* Construct(Node* self,void* host) {
    b01::Construct(self,host);
    self->vtable=static_cast<void**>(Address<0x00fc10ac>());
    return self;
}
}
#if defined(_M_IX86)
namespace ts3::anim::m01::hooks {
uintptr_t __cdecl EraseFactory(uint32_t key) {return m01::EraseFactory(key);}
uintptr_t __fastcall SaveCurrent(Registry* s,void*,Cursor* c) {return m01::SaveCurrent(s,c);}
uintptr_t __fastcall LoadCurrent(Registry* s,void*,Cursor* c) {return m01::LoadCurrent(s,c);}
uintptr_t __fastcall Clear(Registry* s,void*) {return m01::Clear(s);}
uintptr_t __fastcall Destroy(Registry* s,void*) {return m01::Destroy(s);}
Node* __fastcall Construct(Node* s,void*,void* h) {return m01::Construct(s,h);}
}
#endif
