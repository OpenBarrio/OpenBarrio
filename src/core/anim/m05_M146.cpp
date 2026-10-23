#include "ts3/anim/m05_M146.h"
#include "ts3/anim/m05_B37.h"
#include <cstring>
#include <cmath>
namespace ts3::anim::m146 {
namespace {
void* Manager() {
#if defined(_M_IX86)
    return *reinterpret_cast<void**>(0x011d9140);
#else
    AbortNotRewritten("route manager global 011D9140"); return nullptr;
#endif
}
uintptr_t Release(Child* c) {
    uint32_t n=--c->refs;
    if(n) return n;
    c->refs=1;
    return CallGameVirtual<uintptr_t,0>(c,1u);
}
}
uint32_t Type(void* secondary) {
    return b37::TypeOtherVehicle(static_cast<unsigned char*>(secondary)-offsetof(Controller,interfaceTable),nullptr);
}
void* Query(void* secondary,uint32_t id) {
    // Same adjustment pattern as ResourceBaseDestroySecondary, but this interface is +18.
    auto primary=static_cast<unsigned char*>(secondary)-offsetof(Controller,interfaceTable);
    auto result=b37::QueryOtherVehicle(primary,nullptr,id);
    // The verified primary has a different opaque-prefix representation on x64.
    // Remap its secondary result into this controller's widened native layout.
    return id==0xf4da7c8du && result ? secondary : result;
}
Controller* Destroy(Controller* self,uint32_t flags) {
    // Adapted from the verified RouteManagerObject::Destruct twin: only tables differ.
    self->mainTable=reinterpret_cast<void*>(0x010275d4);
    self->secondaryTable=reinterpret_cast<void*>(0x010275d0);
    self->interfaceTable=reinterpret_cast<void*>(0x010275c0);
    CallGameMethod<void,0x00856f10>(self);
    if(self->child) Release(self->child);
    CallGameMethod<void,0x00859890>(self);
    if(flags&1) CallGameFunction<void,0x004e3f70>(self);
    return self;
}
uintptr_t Initialize(Controller* self) {
    uintptr_t result=CallGameMethod<uintptr_t,0x008591a0>(self);
    self->state=1;
    if(!self->host) return result;
    auto rig=CallGameVirtual<unsigned char*,8>(self->host);
    if(!rig) return 0;
    const uintptr_t names[]={0x00ff989c,0x01027498,0x01027488,0x01027478,0x01027638,0x01027620,0x01027608};
    for(unsigned i=0;i<7;++i)
        self->joints[i]=CallGameMethod<int32_t,0x00575e30>(rig+0x28,reinterpret_cast<const char*>(names[i]));
    self->first=1.0f; self->second=0.5f;
    auto memory=CallGameFunction<void*,0x004e3f90>(0x5cu,reinterpret_cast<const char*>(0x01027408),0u,0u,0u,0u);
    Child* child=memory?CallGameMethod<Child*,0x0056abb0>(memory,self->host):nullptr;
    if(child) ++child->refs;
    if(self->joints[0]!=-1) CallGameVirtual<void,20>(child,self->joints[0]);
    if(self->joints[1]!=-1) CallGameMethod<void,0x0056b230>(child,self->joints[1]);
    for(unsigned i=2;i<7;++i) {
        if(self->joints[i]==-1) continue;
        if(i==4) CallGameMethod<void,0x0056b230>(child,self->joints[i]);
        else {
            alignas(16) ts3::math::Float4Value axis{{-1.0f,0,0,-1.0f}};
            CallGameMethod<void,0x0056b2e0>(child,self->joints[i],0.0f,0.2857142984867096f,&axis);
        }
    }
    CallGameMethod<void,0x0056a8e0>(child,self->angle*0.30000001192092896f);
    CallGameMethod<void,0x0056a900>(child,0.6159499883651733f);
    alignas(16) ts3::math::Float4Value rotation{{0,0,0,1}};
    CallGameVirtual<void,19>(child,&rotation);
    CallGameMethod<void,0x005665f0>(&self->child,&child);
    CallGameVirtual<void,5>(self->host,0x2711u,self->child,2u,-1.0f,-1.0f,0u,0u,0u,0u);
    return Release(child);
}
void Set(Controller* self,uint8_t index,float time) {
    if(auto manager=Manager()) self->segment=CallGameMethod<Segment*,0x0083ce90>(manager,static_cast<uint32_t>(index));
    self->index=index;
    // The +D4 word is a time in this derived controller, not a joint index.
    std::memcpy(&self->joints[0],&time,4);
}
void Select(Controller* self,uint32_t start,uint32_t end) {
    auto manager=Manager(); if(!manager) return;
    uint32_t count=CallGameMethod<uint32_t,0x0089f6c0>(manager);
    if(start>=count) start=count-1;
    if(end>=count) end=count-1;
    // FILD unsigned conversion and subtraction are exact before the final float store.
    float difference=static_cast<float>(static_cast<double>(end)-static_cast<double>(start));
    float half=static_cast<float>(static_cast<int32_t>(count))*0.5f;
    bool near=half>=std::fabs(difference);
    self->reverse=near?(start>=end):(start<end);
    uint32_t index;
    if(!self->reverse) {
        index=start; self->destination=end?end-1:count-1;
    } else {
        self->destination=end; index=start?start-1:count-1;
    }
    if(auto current=Manager()) self->segment=CallGameMethod<Segment*,0x0083ce90>(current,index);
    self->joints[0]=0; self->index=index;
    if(self->segment) {
        auto source=self->segment->begin;
        if(self->reverse) {
            auto delta=self->segment->end-source;
            source+=(delta>>5)*32-32;
        }
        std::memcpy(&self->position,source,16);
    }
}
}
#if defined(_M_IX86)
namespace ts3::anim::m146::hooks {
uint32_t __fastcall Type(void* s,void*) {return m146::Type(s);}
void* __fastcall Query(void* s,void*,uint32_t i) {return m146::Query(s,i);}
uintptr_t __fastcall Initialize(Controller* s,void*) {return m146::Initialize(s);}
Controller* __fastcall Destroy(Controller* s,void*,uint32_t f) {return m146::Destroy(s,f);}
void __fastcall Set(Controller* s,void*,uint8_t i,float t) {m146::Set(s,i,t);}
void __fastcall Select(Controller* s,void*,uint32_t a,uint32_t b) {m146::Select(s,a,b);}
}
#endif
