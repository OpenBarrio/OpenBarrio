#include "ts3/resource/m03_G13.h"
#include <cstring>
#if defined(_M_IX86)
#include <windows.h>
#endif
namespace ts3::resource::m03::g13 {
#if defined(_M_IX86)
namespace {
using B=uint8_t;
B* At(void* p,ptrdiff_t n) { return static_cast<B*>(p)+n; }
template<class T> T& F(void* p,size_t n) { return *reinterpret_cast<T*>(At(p,n)); }
void*& P(void* p,size_t n) { return F<void*>(p,n); }
void Lock(void* s) { CallGameMethod<uintptr_t,0x004E16F0>(At(s,0x40),reinterpret_cast<void*>(0x00FFD2DC)); }
uintptr_t Unlock(void* s) { return CallGameMethod<uintptr_t,0x004E17B0>(At(s,0x40)); }
void Release(void* p) { CallGameMethod<uintptr_t,0x0059A990>(p); }
void Remove(void* s,void* i) { CallGameMethod<void,0x007276C0>(At(s,0x80),At(i,8)); }
uintptr_t BoolReturn(void* s,bool b) { return (Unlock(s)&~uintptr_t(255))|uintptr_t(b); }
void Notify(void* cb,void* key,uintptr_t ctx,void* value) { if(cb) CallGameVirtual<void,0>(cb,key,ctx,value); }
struct Iterator { void* current; void* first; void* last; void* node; };
}
#endif
uintptr_t __cdecl Complete(void* request) {
#if defined(_M_IX86)
    void* job=P(request,0x14);
    F<B>(P(job,0x10),0xa0)=0;
    // Tail target is thiscall but returns through the callback's cdecl stack.
    return CallGameMethod<uintptr_t,0x0059A350>(job);
#else
    (void)request; NotRewritten("m03_G13 Complete");
#endif
}
void* __fastcall Construct(void* self,void*) {
#if defined(_M_IX86)
    P(self,0)=reinterpret_cast<void*>(0x00FFD388);
    std::memset(At(self,4),0,40);
    P(self,0x2c)=reinterpret_cast<void*>(0x00FFD424);
    CallGameMethod<void,0x006AD790>(At(self,4),0u);
    F<uint32_t>(self,0x38)=0x01400000u;
    CallGameMethod<void,0x004E17E0>(At(self,0x40),0u,1u);
    F<uint32_t>(self,0x380)=0;
    std::memset(At(self,0x80),0,0x2fc);
    P(self,0x37c)=reinterpret_cast<void*>(~uintptr_t(0));
    P(self,0x394)=reinterpret_cast<void*>(0x00FFD3F4);
    P(self,0x38c)=At(self,0x38c); P(self,0x390)=At(self,0x38c);
    P(self,0x398)=nullptr; P(self,0x39c)=nullptr;
    P(self,0x3a0)=reinterpret_cast<void*>(0x00FFD3D0);
    F<uint32_t>(self,0x3a4)=454;
    return self;
#else
    (void)self; NotRewritten("m03_G13 Construct");
#endif
}
uintptr_t __fastcall Drain(void* self,void*) {
#if defined(_M_IX86)
    void* manager=CallGameFunction<void*,0x00599160>();
    if(P(self,0x388)) { CallGameVirtual<void,6>(manager,P(self,0x388)); P(self,0x388)=nullptr; }
    Lock(self);
    while(F<uint32_t>(self,0x380)) {
        size_t bucket=0x80; while(!P(self,bucket)) bucket+=4;
        void* info=At(P(self,bucket),-8);
        void* job=P(info,0xa8);
        if(job) CallGameMethod<void,0x00599460>(job);
        F<B>(info,0x98)=0; F<B>(info,0x9e)=1;
        Remove(self,info); Unlock(self);
        // Even a null job is passed to the wait helper.
        CallGameMethod<void,0x0059AB40>(job,1u);
        Lock(self); if(job) Release(job);
    }
    while(P(self,0xc)!=P(self,0x1c)) {
        void* job=P(P(self,0xc),0);
        void* next=At(P(self,0xc),8);
        if(next==P(self,0x14)) {
            if(P(self,0x10)) CallGameFunction<void,0x004E3F70>(P(self,0x10));
            P(self,0x18)=At(P(self,0x18),4);
            P(self,0x10)=P(P(self,0x18),0);
            P(self,0x14)=At(P(self,0x10),256);
            next=P(self,0x10);
        }
        P(self,0xc)=next; Unlock(self);
        CallGameMethod<void,0x0059AB40>(job,1u); Release(job); Lock(self);
    }
    return Unlock(self);
#else
    (void)self; NotRewritten("m03_G13 Drain");
#endif
}
uintptr_t __fastcall Cancel(void* self,void*,void* info) {
#if defined(_M_IX86)
    Lock(self);
    if(F<B>(info,0x9c)) { F<B>(info,0x9c)=0; CallGameVirtual<void,21>(P(info,0x2c)); }
    if(F<B>(info,0x98)) { Remove(self,info); F<B>(info,0x98)=0; }
    if(F<B>(info,0x99)) {
        F<B>(info,0x99)=0;
        Iterator it{P(self,0xc),P(self,0x10),P(self,0x14),P(self,0x18)};
        while(it.current!=P(self,0x1c)) {
            void* job=P(it.current,0);
            if(job==P(info,0xa4)) {
                Iterator out{};
                CallGameMethod<void,0x00728EE0>(At(self,4),&out,it);
                Release(job); break;
            }
            it.current=At(it.current,8);
            if(it.current==it.last) { it.node=At(it.node,4); it.first=P(it.node,0); it.last=At(it.first,256); it.current=it.first; }
        }
    }
    if(!F<B>(info,0xa1)) {
        if(!F<B>(info,0x9e)) F<B>(info,0x9e)=1;
        float elapsed=0;
        if(F<int64_t>(info,0xb0)) {
            LARGE_INTEGER now;
            auto qpc=*reinterpret_cast<BOOL (__stdcall**)(LARGE_INTEGER*)>(uintptr_t(0x00F953B8));
            qpc(&now);
            elapsed=static_cast<float>(CallGameFunction<double,0x004F3480>(4u)*static_cast<double>(now.QuadPart-F<int64_t>(info,0xb0)));
        }
        CallGameMethod<void,0x00727DD0>(self,At(info,0x10),3u,elapsed);
        if(P(info,0x28)) {
            CallGameVirtual<void,8>(P(info,0x28));
            if(P(info,0x28)) { void* r=P(info,0x28); P(info,0x28)=nullptr; CallGameVirtual<void,2>(r); }
        }
        void* sentinel=At(info,0xb8);
        while(P(sentinel,4)!=sentinel) {
            void* client=P(sentinel,4);
            if(P(client,8)) {
                P(P(client,8),0)=reinterpret_cast<void*>(2);
                Notify(P(client,24),At(info,16),F<uintptr_t>(client,28),nullptr);
            }
            // Reload links after callback: reentrancy is observable.
            P(P(P(sentinel,4),4),0)=sentinel;
            P(sentinel,4)=P(P(sentinel,4),4);
            if(P(client,20)) Release(P(client,20));
            P(client,0)=P(self,0x398); P(self,0x398)=client;
        }
    }
    Unlock(self); CallGameMethod<void,0x007263E0>(At(info,0xc0));
    return CallGameMethod<uintptr_t,0x00727D20>(self,1u);
#else
    (void)self; (void)info; NotRewritten("m03_G13 Cancel");
#endif
}
uintptr_t __fastcall Request(void* self,void*,void* key,uint32_t type,void** handle,void* callback,uintptr_t context,uint32_t cancelMode,uint32_t flags,uint32_t priority) {
#if defined(_M_IX86)
    Lock(self);
    uint32_t resolved[4]; CallGameFunction<void,0x007D80E0>(resolved,key);
    if(*handle) CallGameMethod<void,0x00728760>(self,handle,cancelMode);
    void* manager=CallGameFunction<void*,0x004AFD20>();
    void* factory=CallGameVirtual<void*,22>(manager,resolved);
    void* record;
    if(factory && CallGameVirtual<uint8_t,6>(factory,resolved,&record)) {
        void* value=CallGameVirtual<void*,3>(record,type);
        if(!value) CallGameVirtual<void,1>(record);
        *handle=value; Notify(callback,key,context,value);
        return BoolReturn(self,value!=nullptr);
    }
    void* client=P(self,0x398);
    if(!client) client=CallGameMethod<void*,0x007272E0>(At(self,0x398));
    if(!client) { Notify(callback,key,context,nullptr); return BoolReturn(self,false); }
    P(self,0x398)=P(client,0);
    // EA overwrites the old job pointer without releasing it.
    P(client,20)=nullptr;
    void* info=CallGameMethod<void*,0x007297E0>(self,resolved,0u,At(client,20),flags,0u,priority);
    if(info) {
        P(client,24)=callback; F<uint32_t>(client,12)=type; P(client,16)=info;
        F<uint32_t>(client,32)=flags; P(client,8)=handle; F<uintptr_t>(client,28)=context;
        P(client,4)=P(info,188); P(client,0)=At(info,184);
        P(info,188)=client; P(P(client,4),0)=client;
        *handle=reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(client)|1);
        return BoolReturn(self,true);
    }
    *handle=reinterpret_cast<void*>(2);
    CallGameMethod<void,0x00727DD0>(self,key,3u,0.0f);
    Notify(callback,key,context,nullptr);
    if(P(client,20)) Release(P(client,20));
    P(client,0)=P(self,0x398); P(self,0x398)=client;
    return BoolReturn(self,false);
#else
    (void)self;(void)key;(void)type;(void)handle;(void)callback;(void)context;(void)cancelMode;(void)flags;(void)priority;
    NotRewritten("m03_G13 Request");
#endif
}
}
