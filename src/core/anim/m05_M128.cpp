#include "ts3/anim/m05_M128.h"
#include <intrin.h>
#include <xmmintrin.h>
namespace ts3::anim::m128_impl {
namespace {
void Retain(Ref* p) { if(p) ++p->refs; }
void Release(Ref* p) { if(p && --p->refs==0) { p->refs=1; CallGameVirtual<void,0>(p,uint32_t(1)); } }
Ref* Emit(Style* s,Ref* r,uint32_t index) {
    Ref* item=nullptr;
    for(;;) {
        Ref* old=item; item=nullptr; Release(old);
        if(!CallGameVirtual<uint8_t,85>(r,index,&item)) break;
        CallGameVirtual<void,21>(s->host,item);
    }
    return item;
}
}
void* DeleteStyle(Style* self,uint32_t flags) {
#if defined(_M_IX86)
    self->vtable=reinterpret_cast<void**>(0x01026ee8);
#else
    self->vtable=nullptr;
#endif
    CallGameMethod<void,0x008396b0>(self);
    if(flags&1) CallGameFunction<void,0x004e3f70>(self);
    return self;
}
uintptr_t UpdateStyle(Style* self,uint32_t) {
    auto* r=CallGameMethod<Ref*,0x00c93970>(self->host);
    if(!r) return 0;
    _InterlockedExchangeAdd(reinterpret_cast<volatile long*>(&r->refs),1);
    if(CallGameVirtual<uint32_t,18>(r)>0) {
        Release(Emit(self,r,0));
        if(!CallGameVirtual<uint8_t,47>(r) && CallGameVirtual<int32_t,45>(r)==-1) {
            CallGameVirtual<void,23>(self->host,int32_t(-1),uint32_t(0));
            CallGameVirtual<void,48>(r,uint32_t(1));
        }
        auto key=CallGameVirtual<uint64_t,4>(self->host);
        float time=static_cast<float>(CallGameVirtual<double,41>(r,uint32_t(key),uint32_t(key>>32)));
        uint32_t last=CallGameVirtual<uint32_t,18>(r)-1;
        alignas(16) unsigned char scratch[16]; int32_t section=0;
        CallGameVirtual<void,71>(r,scratch,time,uint32_t(0),&section);
        if(section>=0) {
            int32_t count=CallGameVirtual<int32_t,29>(r,section);
            for(int32_t i=0;i<count;++i) {
                if(CallGameVirtual<int32_t,45>(r)<i) {
                    Ref* item=Emit(self,r,uint32_t(i+1));
                    CallGameVirtual<void,23>(self->host,i,uint32_t(section)==last?int32_t(-1):i+1);
                    CallGameVirtual<void,46>(r,i);
                    Release(item);
                }
            }
        }
    }
    uintptr_t result=reinterpret_cast<uintptr_t>(&r->refs);
    if(!_InterlockedDecrement(reinterpret_cast<volatile long*>(&r->refs))) {
        _InterlockedExchange(reinterpret_cast<volatile long*>(&r->refs),1);
        result=CallGameVirtual<uintptr_t,0>(r,uint32_t(1));
    }
    return result;
}
uint32_t CanProgress(Body* self) {
    if(CallGameMethod<uint8_t,0x0085a8f0>(self->state) && (self->mode==0 || self->mode==1)) return 0;
    return self->progress<self->limit && self->limit>0 && !self->flag250 && !self->flag251 && !self->flag17f;
}
Controller** FindController(Controller** out,Controller* source,int32_t depth) {
    if(depth<=0) { *out=nullptr; Release(source?&source->base:nullptr); return out; }
    void* query=&source->queryVtable;
    auto* found=CallGameVirtual<Controller*,1>(query,uint32_t(0xb489fc2c));
    if(found) { Retain(&found->base); *out=found; Retain(&found->base); Release(&found->base); Release(&source->base); return out; }
    auto* first=CallGameVirtual<Controller*,1>(query,uint32_t(0x073b9d8c));
    if(first) {
        Retain(&first->base); auto* child=first->child; Retain(child?&child->base:nullptr);
        FindController(out,child,depth-1); Release(&first->base); Release(&source->base); return out;
    }
    auto* second=CallGameVirtual<Controller*,1>(query,uint32_t(0x34f49f94));
    if(second) {
        Retain(&second->base); auto* child=second->child; Retain(child?&child->base:nullptr);
        FindController(out,child,depth-1);
    } else {
        auto* third=CallGameVirtual<Controller*,1>(query,uint32_t(0x075e3ecb));
        if(third) {
            Retain(&third->base);
            auto* child=reinterpret_cast<Controller*>(third->indirect); Retain(child?&child->base:nullptr);
            FindController(out,child,depth-1);
        } else *out=nullptr;
        Release(third?&third->base:nullptr);
    }
    Release(second?&second->base:nullptr); Release(&source->base); return out;
}
double RootMotion(RootController* self,ts3::math::Float4Value* translation,ts3::math::Float4Value* rotation,float* speed) {
    if(!self) return 0.0;
    alignas(16) unsigned char workspace[17216];
    CallGameMethod<void,0x0054c720>(workspace);
    float old=static_cast<float>(CallGameVirtual<double,5>(self));
    float end=static_cast<float>(CallGameVirtual<double,8>(self));
    CallGameVirtual<void,4>(self,end);
    auto node=CallGameVirtual<uintptr_t,3>(self,workspace);
    CallGameVirtual<void,4>(self,old);
    Evaluation evaluation;
    CallGameMethod<void,0x0083b080>(&evaluation);
    for(int i=0;i<16;++i) evaluation.matrix[i]=(i%5==0)?1.0f:0.0f;
    evaluation.duration=static_cast<float>(CallGameVirtual<double,8>(self));
    evaluation.rig=CallGameVirtual<void*,8>(self->model);
    evaluation.zero=0; evaluation.mode=1;
    CallGameMethod<void,0x0054c5c0>(workspace,node);
    if(!evaluation.rig) { CallGameMethod<void,0x0054c6e0>(workspace); return 0.0; }
    auto* storage=CallGameFunction<void*,0x005748a0>(uint32_t(105264),"Animation/RigPose",uint32_t(0),uint32_t(0),uint32_t(0),uint32_t(0));
    Pose* pose=nullptr;
    if(storage) {
        CallGameVirtual<void,0>(evaluation.rig);
        pose=CallGameMethod<Pose*,0x00574f50>(storage,evaluation.rig);
        Retain(pose?&pose->base:nullptr);
    }
    CallGameMethod<void,0x0054c570>(workspace,&evaluation,pose);
    *translation=pose->translation; *rotation=pose->rotation;
    __m128 v=_mm_load_ps(pose->translation.lane); v=_mm_mul_ps(v,v);
    __m128 sum=_mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v,v,2),_mm_shuffle_ps(v,v,1)),v);
    float distance=_mm_cvtss_f32(_mm_sqrt_ps(_mm_shuffle_ps(sum,sum,0)));
    *speed=1.0f;
    if(evaluation.duration>0.0f) *speed=distance/evaluation.duration;
    Release(&pose->base);
    CallGameMethod<void,0x0054c6e0>(workspace);
    return distance;
}
double RootDistance(RootController* self,ts3::math::Float4Value* t,ts3::math::Float4Value* r) {
    float discarded;
    return RootMotion(self,t,r,&discarded);
}
}
