#include "ts3/anim/m05_M136.h"
#include "ts3/layout.h"
#include <cstring>
#if defined(_M_IX86)
#include <intrin.h>
#endif
namespace ts3::anim::m136 {
float GetScale(const Body* self) { return self->scale; }
uint32_t CountControllers(const Body* self) {
    uint32_t count=0;
    for(auto* p=self->controllers.next;p!=&self->controllers;p=p->next) ++count;
    return count;
}
namespace {
void Free(void* p) { CallGameFunction<void,0x004E3F70>(p); }
#if defined(_M_IX86)
template<class T> T& At(void* p,ptrdiff_t offset) { return *reinterpret_cast<T*>(static_cast<unsigned char*>(p)+offset); }
void DestroyStyles(Style* p) {
    if(!p) return;
    int32_t n=At<int32_t>(p,-4);
    while(--n>=0) CallGameMethod<void,0x00754060>(p+n);
    Free(reinterpret_cast<unsigned char*>(p)-4);
}
uintptr_t Release(void* p,bool atomic) {
    auto* refs=&At<long>(p,4);
    if(atomic ? _InterlockedDecrement(refs)==0 : --*refs==0) {
        if(atomic) _InterlockedExchange(refs,1); else *refs=1;
        return CallGameVirtual<uintptr_t,0>(p,1u);
    }
    return reinterpret_cast<uintptr_t>(refs);
}
#endif
}
Body* DeleteSecondary(void* self,uint32_t flags) {
    auto* body=reinterpret_cast<Body*>(static_cast<unsigned char*>(self)-sizeof(void*));
    Destroy(body);
    if(flags&1) Free(body);
    return body;
}
uintptr_t Destroy(Body* self) {
#if defined(_M_IX86)
    self->vtable=reinterpret_cast<void*>(0x01027294);
    self->secondaryBase=reinterpret_cast<void*>(0x01027290);
    self->secondary=reinterpret_cast<void*>(0x0102727C);
    if(self->retained) {
        if(self->master) CallGameVirtual<void,30>(self->master,self->retained,0u);
        self->retained=nullptr;
    }
    if(self->slaveStyles) { DestroyStyles(self->slaveStyles); self->slaveStyles=nullptr; }
    if(self->host) {
        auto* actor=CallGameVirtual<unsigned char*,1>(self->host,0x749c6a98u);
        CallGameVirtual<void,11>(actor+8,0u);
    }
    if(self->slaveProgressions) {
        CallGameMethod<void,0x0084D700>(self->slaveProgressions,3u);
        self->slaveProgressions=nullptr;
    }
    if(self->buffer13C) { Free(self->buffer13C); self->buffer13C=nullptr; }
    if(self->buffer140) { Free(self->buffer140); self->buffer140=nullptr; }
    for(int i=0;i<self->slaveCount;++i) {
        if(self->slaveBegin[i]) {
            auto* actor=CallGameVirtual<unsigned char*,1>(self->slaveBegin[i],0x749c6a98u);
            if(actor) {
                At<void*>(actor,0x30)=nullptr; At<void*>(actor,0x34)=nullptr;
                CallGameVirtual<void,11>(actor+8,0u);
                self->slaveBegin[i]=nullptr;
            }
        }
    }
    self->slaveEnd=self->slaveBegin;
    struct Node { Node *next,*previous; void* controller; };
    for(auto* n=reinterpret_cast<Node*>(self->controllers.next);
        n!=reinterpret_cast<Node*>(&self->controllers);n=n->next) {
        auto* delegates=static_cast<unsigned char*>(n->controller)+0xc;
        auto* sentinel=delegates+4;
        auto* p=At<unsigned char*>(sentinel,0);
        while(p!=sentinel) {
            if(p && At<void*>(p,8)==&self->secondary) {
                if(At<uint32_t>(delegates,0xc)) {
                    At<void*>(p,8)=nullptr; At<void*>(p,0xc)=nullptr;
                } else {
                    auto* prev=At<unsigned char*>(p,4); auto* next=At<unsigned char*>(p,0);
                    At<void*>(prev,0)=next; At<void*>(next,4)=prev; Free(p);
                }
                break;
            }
            p=At<unsigned char*>(p,0);
        }
    }
    self->slaveCount=0;
    CallGameMethod<void,0x0081F320>(&self->controllers);
    self->controllers.next=self->controllers.previous=&self->controllers;
    CallGameMethod<void,0x0084CE30>(self,1u);
    CallGameMethod<void,0x00839A10>(self,self->stateD0);
    CallGameMethod<void,0x00839A10>(self,self->stateF0);
    CallGameMethod<void,0x00839A10>(self,&self->state114Vtable);
    for(auto* p=self->styleBegin;p<self->styleEnd;++p)
        if(p->begin && p->begin!=p->inlinePointer) Free(p->begin);
    if(self->styleBegin && self->styleBegin!=self->inlinePointer) Free(self->styleBegin);
    self->state114Vtable=reinterpret_cast<void*>(0x01026ff8);
    CallGameMethod<void,0x0085A7D0>(self->stateF0);
    CallGameMethod<void,0x0085A7D0>(self->stateD0);
    if(self->slaveBegin) Free(self->slaveBegin);
    CallGameMethod<void,0x00866E90>(&self->effects);
    CallGameMethod<void,0x00754060>(&self->style);
    auto result=CallGameMethod<uintptr_t,0x0081F320>(&self->controllers);
    // The tail jump 839750 changes fields but leaves EAX from the final list cleanup.
    self->vtable=reinterpret_cast<void*>(0x01022de4);
    self->entity=nullptr;
    self->secondaryBase=reinterpret_cast<void*>(0x0107396c);
    return result;
#else
    (void)self; NotRewritten("m136 body destruction game dependencies");
#endif
}
uintptr_t SetSlaves(Body* self,Style* styles,const uint64_t* ids,int32_t count,uint8_t flag) {
#if defined(_M_IX86)
    uintptr_t result=*reinterpret_cast<uintptr_t*>(0x011d9140);
    void* const bindingContext=reinterpret_cast<void*>(result);
    if(!styles || !ids) return result;
    result=static_cast<uintptr_t>(count);
    if(count<=0) return result;
    self->slaveCount=count; self->slaveFlag=flag;
    DestroyStyles(self->slaveStyles);
    bool hadProgressions=self->slaveProgressions!=nullptr;
    self->slaveStyles=styles;
    CallGameMethod<void,0x00AEF830>(&self->slaveBegin,self->slaveCount);
    auto distance=static_cast<int32_t>(reinterpret_cast<uintptr_t>(self->styleEnd)-reinterpret_cast<uintptr_t>(self->styleBegin));
    result=0x92492493u*static_cast<uint32_t>(distance);
    bool progressions=distance/28!=0 && (self->styleKeyHigh || self->styleKeyLow);
    if(!hadProgressions && progressions) {
        uint64_t bytes=uint64_t(uint32_t(self->slaveCount))*188;
        uint32_t size=bytes>0xfffffffbULL?0xffffffffu:static_cast<uint32_t>(bytes)+4;
        auto* allocation=CallGameFunction<unsigned char*,0x004E3F90>(size,reinterpret_cast<void*>(0x010272bc),0u,0u,0u,0u);
        result=reinterpret_cast<uintptr_t>(allocation);
        if(allocation) {
            At<uint32_t>(allocation,0)=self->slaveCount;
            self->slaveProgressions=allocation+4;
            // Compiler vector-constructor helper is stdcall (ret 0x10), not cdecl.
            using Construct=uintptr_t(__stdcall*)(void*,uint32_t,uint32_t,void*);
            result=reinterpret_cast<Construct>(0x004466D0)(allocation+4,188u,self->slaveCount,reinterpret_cast<void*>(0x0075fcd0));
        } else self->slaveProgressions=nullptr;
    }
    for(int i=0;i<self->slaveCount;++i) {
        auto* manager=CallGameFunction<void*,0x00938B50>(); result=reinterpret_cast<uintptr_t>(manager);
        uint32_t lo=static_cast<uint32_t>(ids[i]),hi=static_cast<uint32_t>(ids[i]>>32);
        if(lo || hi) {
            auto* resource=CallGameMethod<void*,0x00939560>(manager,lo,hi); result=reinterpret_cast<uintptr_t>(resource);
            if(resource) {
                _InterlockedIncrement(&At<long>(resource,4));
                auto* actor=CallGameMethod<unsigned char*,0x00935770>(resource,*reinterpret_cast<uint32_t*>(0x011dc484));
                auto* source=CallGameVirtual<unsigned char*,9>(self->host);
                At<uint32_t>(actor,0x30)=At<uint32_t>(source,8);
                At<uint32_t>(actor,0x34)=At<uint32_t>(source,0xc);
                self->slaveBegin[i]=actor+8;
                result=reinterpret_cast<uintptr_t>(self->slaveBegin);
            }
            if(progressions) {
                auto* style=self->slaveStyles+i;
                result=CallGameMethod<uintptr_t,0x00844C10>(bindingContext,
                    At<uint32_t>(style,0x50),At<uint32_t>(style,0x54)|At<uint32_t>(style,0x5c),
                    static_cast<unsigned char*>(self->slaveProgressions)+188*i);
            }
            if(resource) result=Release(resource,true);
        } else self->slaveBegin[i]=nullptr;
    }
    float time=self->time;
    if(self->slaveCount==1) {
        float delta=0;
        auto* object=CallGameMethod<void*,0x00C93970>(self->entity);
        if(object) _InterlockedIncrement(&At<long>(object,4));
        auto* component=CallGameVirtual<void*,3>(object,0u,0x2000u,0u);
        if(CallGameMethod<uint8_t,0x0081C880>(component)) {
            auto key=At<uint32_t>(self->slaveStyles,0x54)|At<uint32_t>(self->slaveStyles,0x5c);
            auto* service=CallGameFunction<void*,0x005EB7C0>();
            if(service) delta=CallGameMethod<float,0x005ECBA0>(service,key);
        }
        auto type=At<uint32_t>(self->slaveStyles,0x5c);
        if(type!=0x400 && type!=0x500) delta=0;
        self->time=delta+time;
        result=Release(object,true);
    }
    return result;
#else
    (void)self; (void)styles; (void)ids; (void)count; (void)flag;
    NotRewritten("m136 slave binding game dependencies");
#endif
}
bool BuildControllers(Body* self) {
#if defined(_M_IX86)
    struct Parameters { void* host; float values[4]; uint8_t flag,mirror; uint16_t pad; void* resource; };
    struct ControllerRef { void* pointer; uint8_t flag; unsigned char pad[3]; };
    struct Vector { void **begin,**end,**capacity; void* descriptor; };
    Parameters parameters{self->host,{0,0,0,0},0,0,0,nullptr};
    CallGameMethod<void,0x00849BF0>(self,&parameters);
    if(self->mirrored) parameters.mirror=1;
    ControllerRef controller{nullptr,0,{}};
    auto cleanup=[&] { if(parameters.resource) Release(parameters.resource,true); };
    if(self->style.controller) {
        if(CallGameVirtual<uint8_t,3>(self->style.controller,&parameters,&controller)) {
            if(controller.pointer) {
                Vector slaves{nullptr,nullptr,nullptr,reinterpret_cast<void*>(0x010272f4)};
                CallGameMethod<void,0x00AEF830>(&slaves,self->slaveCount);
                Parameters slaveParameters{nullptr,{0,0,0,0},0,static_cast<uint8_t>(self->mirrored!=0),0,nullptr};
                if(self->slaveCount>0 && self->slaveStyles) {
                    // 8499E0 is the copy constructor for the 28-byte by-value argument.
                    Parameters copied;
                    CallGameMethod<void,0x008499E0>(&copied,&slaveParameters);
                    if(!CallGameMethod<uint8_t,0x00849D40>(self,&slaves,0u,copied,0u)) {
                        CallGameMethod<void,0x008499B0>(&slaveParameters);
                        if(slaves.begin) Free(slaves.begin);
                        CallGameMethod<void,0x0055CFA0>(&controller);
                        CallGameMethod<void,0x008499B0>(&parameters);
                        return false;
                    }
                }
                // Exact caller has seven stack arguments, callee pops 0x1c.
                CallGameMethod<void,0x0084B590>(self,controller.pointer,self->slaveCount<=0?nullptr:slaves.begin,
                    self->slaveCount,0u,2u,0.0f,1.0f);
                for(int i=0;i<self->slaveCount;++i) if(self->slaveBegin[i]) {
                    Release(slaves.begin[i],false); slaves.begin[i]=nullptr;
                }
                if(slaves.begin) Free(slaves.begin);
                if(controller.pointer) Release(controller.pointer,false);
                cleanup(); return true;
            }
        } else if(controller.pointer) Release(controller.pointer,false);
    }
    cleanup(); return false;
#else
    (void)self; NotRewritten("m136 controller building game dependencies");
#endif
}
}
