#include "ts3/anim/m05_B36.h"
#include "ts3/layout.h"
#include <cstring>
#include <intrin.h>
namespace ts3::anim::b36 {
namespace {
struct Ref { void* vtable; volatile long refs; uint64_t key; };
void Add(void* p) { if(p) _InterlockedIncrement(&static_cast<Ref*>(p)->refs); }
void Release(void* p) {
    if(p && _InterlockedDecrement(&static_cast<Ref*>(p)->refs)==0) {
        _InterlockedExchange(&static_cast<Ref*>(p)->refs,1);
        CallGameVirtual<void,0>(p,1u);
    }
}
void* Query(Body* s) { return s->interface124 ? CallGameVirtual<void*,1>(s->interface124,0x549b548bu) : nullptr; }
uint32_t Global(uintptr_t a) {
#if defined(_M_IX86)
    return *reinterpret_cast<uint32_t*>(a);
#else
    (void)a; AbortNotRewritten("m05_B36 global"); return 0;
#endif
}
void* Address(uintptr_t a) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(a);
#else
    (void)a; AbortNotRewritten("m05_B36 address"); return nullptr;
#endif
}
void Free(void* p) { CallGameFunction<void,0x4e3f70>(p); }
struct EffectNode { EffectNode* next; EffectNode* previous; void* effect; };
struct Vector { void* begin; void* end; void* capacity; void* allocator; };
void FreeVector(Vector& v,size_t stride) {
    if(v.begin) {
        auto bytes=static_cast<unsigned char*>(v.capacity)-static_cast<unsigned char*>(v.begin);
        CallGameVirtual<void,3>(v.allocator,v.begin,uint32_t(bytes/stride*stride));
    }
}
}
void GetVector(Body* s,Float4Value* out) {
    std::memset(out,0,16);
    if(auto q=Query(s)) CallGameVirtual<void,5>(q,out);
}
bool TryVector(Body* s,Float4Value* out) {
    if(auto q=Query(s)) return CallGameVirtual<bool,18>(q,out);
    std::memset(out,0,16); return false;
}
int32_t IsComplete(Body* s) {
    return s->current<s->limit && s->upper>=s->current && s->value24C==s->value248;
}
void SetToken(Body* s,void** range,uint32_t flags) {
    if(s->token) {
        if(s->master) CallGameVirtual<void,30>(s->master,s->token,0u);
        s->token=nullptr;
    }
    if(range[0]!=range[1] && s->master) CallGameVirtual<void,29>(s->master,&s->token,range[0],flags);
}
void Dispatch(void* s,void* event,uint32_t) {
    auto payload=*reinterpret_cast<unsigned char**>(static_cast<unsigned char*>(event)+sizeof(void*)*4);
    if(payload && *reinterpret_cast<uint32_t*>(payload+8)==0xc1e40004u) {
        auto host=*(static_cast<void**>(s)-1);
        if(auto q=CallGameVirtual<void*,1>(host,0x14cfd2f8u)) CallGameVirtual<void,15>(q,*reinterpret_cast<uint32_t*>(payload+12));
    }
}
Motion* Construct(Motion* s,uint32_t kind) {
    s->kind=kind;
    for(auto& f:s->values) f=0;
    s->flag14=0; s->flag15=0; s->resource=nullptr;
    return s;
}
void SelectDisplacement(void* controller,Float4Value* out,void* context) {
    float duration=CallGameVirtual<float,75>(controller);
    uint64_t key=CallGameVirtual<uint64_t,4>(context);
    float time=CallGameVirtual<float,41>(controller,key);
    int count=CallGameVirtual<int,18>(controller);
    if(count>0) {
        alignas(16) unsigned char temporary[16]; int index=-1;
        CallGameVirtual<void,71>(controller,temporary,time,0u,&index);
        if(index<0) index=0;
        for(;index<count;++index) {
            auto p=CallGameVirtual<unsigned char*,52>(controller,index);
            std::memcpy(out,p+16,16);
            p=CallGameVirtual<unsigned char*,52>(controller,index);
            float weight=*reinterpret_cast<float*>(p+32);
            // COMISS/JAE treats NaN as the small-weight path.
            if(!(weight>=0.005f)) {
                p=CallGameVirtual<unsigned char*,52>(controller,index);
                float delta=duration-*reinterpret_cast<float*>(p+36);
                if(!(delta>=0.15f)) continue;
            }
            float v[4]; std::memcpy(v,out,16);
            float squared=(v[2]*v[2]+v[1]*v[1])+v[0]*v[0];
            if(squared>0.0f) break;
        }
    }
    Release(controller);
}
bool RoutingKind(void* controller) {
    if(!controller) return false;
    auto route=CallGameVirtual<void*,3>(controller,0u,0x2000u,0u);
    if(CallGameMethod<bool,0x81c880>(route)) {
        auto parent=CallGameVirtual<void*,35>(controller);
        if(!parent) {
            CallGameMethod<void,0x7460f0>(&controller); return true;
        }
        auto a=CallGameVirtual<uint32_t*,12>(controller);
        uint32_t kind=*a;
        parent=CallGameVirtual<void*,35>(controller);
        uint32_t other=*CallGameVirtual<uint32_t*,12>(parent);
        if(kind==0x8d43950au && other!=0x8d43950au) {
            CallGameMethod<void,0x7460f0>(&controller); return true;
        }
    }
    Release(controller); return false;
}
float Remaining(void* controller,uint64_t key,uint32_t index,uint32_t flags,bool adjust) {
    float total=0;
    while(controller) {
        if(CallGameVirtual<bool,39>(controller)) {
            float result=CallGameVirtual<float,75>(controller)*10.0f;
            Release(controller); return result;
        }
        float duration=CallGameVirtual<float,75>(controller);
        double time=CallGameVirtual<double,41>(controller,key);
        total=static_cast<float>((static_cast<double>(duration)-time)+total);
        Add(controller);
        if(!CallGameFunction<bool,0x81f550>(controller)) break;
        auto next=CallGameVirtual<void*,31>(controller);
        if(next!=controller) { Add(next); Release(controller); controller=next; }
    }
    float correction=0;
    if(adjust) {
        Add(controller);
        if(RoutingKind(controller)) {
            auto state=CallGameFunction<void*,0x5eb7c0>();
            if(state) correction=CallGameMethod<float,0x5ecba0>(state,(1u<<(index&31))|flags);
        }
    }
    float delta=total-correction;
    float result=delta>0 ? delta : 0;
    Release(controller); return result;
}
void Destroy(Motion* s) { Release(s->resource); }
Motion* Copy(Motion* s,const Motion* from) {
    s->kind=from->kind;
    for(unsigned i=0;i<3;++i) std::memcpy(&s->values[i],&from->values[i],4);
    uint32_t last; std::memcpy(&last,&from->values[3],4);
    // The last float alone passes through FLD/FSTP: quiet an SNaN, unlike the raw first three lanes.
    if((last&0x7f800000u)==0x7f800000u && (last&0x007fffffu)) last|=0x00400000u;
    std::memcpy(&s->values[3],&last,4);
    s->flag14=from->flag14; s->flag15=from->flag15; s->resource=from->resource;
    Add(s->resource); return s;
}
int32_t Mode(Body* s) {
    if(!s->cachedMode) {
        auto controller=CallGameMethod<void*,0xc93970>(s->object14);
        Add(controller);
        if(CallGameFunction<bool,0x81f550>(controller)) s->cachedMode=2+(s->flag17E!=0);
        else s->cachedMode=s->flag17E ? 4 : 1;
    }
    return s->cachedMode;
}
void CreateEffect(Body* s,uint32_t name,uint32_t slot,bool force) {
    auto world=CallGameFunction<void*,0x938b50>();
    if(!world) return;
    struct Parameters { Vector first; uint32_t gap[2]; Vector second; } parameters{};
    uint32_t key[3];
    auto service=CallGameFunction<void*,0x931020>();
    CallGameMethod<void,0x93b020>(&parameters,service);
    CallGameMethod<void,0x93b680>(&parameters,0x8cdb555bu,name);
    uint32_t mask=Global(0x11e8e90)|Global(0x11e9158);
    CallGameMethod<void,0x93b5a0>(&parameters,key);
    uint32_t id=CallGameFunction<uint32_t,0x4fa590>(name,0x811c9dc5u,0u);
    uint32_t descriptor[4]={id,0,0x2da18f83,1};
    auto effect=CallGameMethod<void*,0x93a1e0>(world,key,mask,descriptor,0u,0u);
    if(effect) {
        Add(effect);
        auto component=CallGameMethod<void*,0x935770>(effect,Global(0x11e9158));
        if(component) {
            auto binding=CallGameVirtual<void*,9>(s->interface124,slot,0u,0u);
            CallGameMethod<void,0x95b460>(component,binding);
        }
        component=CallGameMethod<void*,0x935770>(effect,Global(0x11e8e90));
        if(component) {
            auto owner=Address(Global(0x11ecbc4));
            if(!force && owner) {
                CallGameMethod<void,0xc64920>(owner,static_cast<Ref*>(effect)->key);
                CallGameMethod<void,0x944bf0>(component,0u,3u);
            } else {
                CallGameMethod<void,0x944bf0>(component,0u,0u);
                auto node=CallGameFunction<EffectNode*,0x4e3f90>(uint32_t(sizeof(EffectNode)),s->effectsDescriptor,0u,0u,Address(0xf98980),231u);
                node->effect=effect; Add(effect);
                auto sentinel=reinterpret_cast<EffectNode*>(&s->effects);
                node->next=sentinel; node->previous=sentinel->previous;
                sentinel->previous->next=node; sentinel->previous=node;
            }
        }
        Release(effect);
    }
    FreeVector(parameters.second,4); FreeVector(parameters.first,16);
}
void ClearEffects(Body* s,bool remove) {
    auto world=CallGameFunction<void*,0x938b50>();
    if(world) {
        auto sentinel=reinterpret_cast<EffectNode*>(&s->effects);
        for(auto n=sentinel->next;n!=sentinel;n=n->next) {
            auto effect=n->effect;
            if(effect) {
                Add(effect);
                auto component=CallGameMethod<void*,0x935770>(effect,Global(0x11e8e90));
                if(component) CallGameMethod<void,0x944c60>(component,uint32_t(remove));
            }
            // EA dereferences a null payload here when remove is set.
            if(remove) CallGameMethod<void,0x939420>(world,static_cast<Ref*>(effect)->key);
            Release(effect);
        }
    }
    if(remove) {
        CallGameMethod<void,0x866e90>(&s->effects);
        s->effects.next=s->effects.previous=&s->effects;
    }
}
void* DeleteRanges(Range* s,uint32_t flags) {
    if(flags&2) {
        auto cookie=reinterpret_cast<uint32_t*>(s)-1;
        uint32_t count=*cookie;
        struct Child { void* begin; void* end; void* capacity; uint32_t descriptor; void* inlineBegin; unsigned char opaque[8]; };
        // EA decrements the cookie in 32 bits, then uses JNS. In particular,
        // cookies above 0x80000000 skip destruction rather than walking storage.
        for(uint32_t i=count;static_cast<int32_t>(i-1)>=0;--i) {
            auto& r=s[i-1];
            for(auto p=static_cast<Child*>(r.begin);p<static_cast<Child*>(r.end);++p)
                if(p->begin && p->begin!=p->inlineBegin) Free(p->begin);
            if(r.begin && r.begin!=r.inlineBegin) Free(r.begin);
        }
        if(flags&1) Free(cookie);
        return cookie;
    }
    CallGameMethod<void,0x759410>(s,s->begin,s->end);
    if(s->begin && s->begin!=s->inlineBegin) Free(s->begin);
    if(flags&1) Free(s);
    return s;
}
}
