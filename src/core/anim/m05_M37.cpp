#include "ts3/anim/m05_M37.h"
#include "ts3/anim/m05_B09.h"
#include <cstring>
namespace ts3::anim::m37 {
namespace {
void* Address(uint32_t a) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(uintptr_t(a));
#else
    (void)a; ts3::NotRewritten("M37 game data");
#endif
}
void Tables(Controller& p,uint32_t primary,uint32_t secondary8,uint32_t secondaryC) {
    p.vtable=Address(primary); p.secondary8=Address(secondary8); p.secondaryC=Address(secondaryC);
}
float FloatStore(float value) {
#if defined(_M_IX86)
    __asm { fld value }
    __asm { fstp value }
#else
    uint32_t bits; std::memcpy(&bits,&value,4);
    if((bits&0x7f800000u)==0x7f800000u && (bits&0x7fffffu)) { bits|=0x400000u; std::memcpy(&value,&bits,4); }
#endif
    return value;
}
}
Transform* DestroySecondary(void* p,uint32_t flags) {
    // Verified twin 004A3A90, with this adjustment 12 rather than 4.
    return DestroyTransform(reinterpret_cast<Transform*>(static_cast<unsigned char*>(p)-offsetof(Controller,secondaryC)),flags);
}
Transform* ConstructTransform(Transform* p,void* host) {
    b08::Construct(&p->base,host);
    Tables(p->base,0x00fc1d18,0x00fc1d10,0x00fc1d0c);
    p->map.maxLoad=1.0f; p->map.growth=2.0f; p->map.allocator=Address(0x00fc1d5c);
    p->map.size=0; p->map.hash=0; p->map.bucketCount=1; p->map.buckets=static_cast<BoneEntry**>(Address(0x0114c900));
    return p;
}
Transform* DestroyTransform(Transform* p,uint32_t flags) {
    Tables(p->base,0x00fc1d18,0x00fc1d10,0x00fc1d0c);
    CallGameMethod<void,0x005c9d90>(&p->map,p->map.buckets,p->map.bucketCount);
    p->map.size=0;
    if(p->map.bucketCount>1) CallGameFunction<void,0x004e3f70>(p->map.buckets);
    CallGameMethod<void,0x00565b40>(&p->base);
    if(flags&1) CallGameFunction<void,0x004e3f70>(p);
    return p;
}
void SetTarget(Transform* p,uint32_t key,const Float4Value* value,float duration) {
    auto* rig=CallGameVirtual<void*,8>(p->base.hostSlot);
    if(!rig) return;
    auto* skeleton=reinterpret_cast<b21::SkeletonView*>(static_cast<unsigned char*>(rig)+sizeof(ResourcePrefix));
    int32_t index=b21::FindIndex(skeleton,key);
    if(index==-1) return;
    InsertResult result;
    // Only the low byte of this unused insertion hint is defined by EA.
    CallGameMethod<void,0x0056b860>(&p->map,&result,&index,0u);
    auto* entry=result.node;
    Float4Value previous{};
    if(!result.inserted) previous=entry->target;
    entry->current=previous; entry->start=previous;
    entry->target=*value; entry->duration=duration; entry->elapsed=0.0f;
    float stamp=0.0f;
#if defined(_M_IX86)
    auto* clock=*reinterpret_cast<unsigned char**>(0x011ecbc4);
    if(clock) { float factor; uint32_t bits=0x3c888889; std::memcpy(&factor,&bits,4); stamp=*reinterpret_cast<float*>(clock+0x124)*factor; }
#else
    ts3::NotRewritten("M37 clock global");
#endif
    entry->stamp=stamp;
    // UCOMISS + LAHF/TEST/JP: only ordered equality (including -0) snaps.
    if(duration==0.0f) entry->current=entry->target;
}
uint8_t SaveTime(Time* p,b08::StreamState* s) {
    auto status=CallGameFunction<uint32_t,0x004ddbd0>(s->handle,1u,s->extra);
    CallGameMethod<void,0x007d77c0>(s,status);
    if(!s->good) return 0;
    b08::SaveOptional(p->first,s);
    if(!s->good) return 0;
    b08::SaveOptional(p->second,s);
    if(!s->good) return 0;
    status=CallGameFunction<uint32_t,0x004ddff0>(s->handle,FloatStore(p->scale),s->extra);
    CallGameMethod<void,0x007d77c0>(s,status);
    if(!s->good) return 0;
    // Reader and StreamState have the same proven stream prefix.
    b09impl::WriteFloat(reinterpret_cast<b09impl::Reader*>(s),p->offset);
    return s->good;
}
Time* ConstructTime(Time* p,void* host) {
    b08::Construct(&p->base,host);
    Tables(p->base,0x00fc1d98,0x00fc1d8c,0x00fc1d88);
    p->first=nullptr; p->second=nullptr; p->scale=-1.0f; p->offset=-1.0f;
    return p;
}
}
