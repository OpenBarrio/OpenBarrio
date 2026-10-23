#include "ts3/anim/m05_M05.h"
#include "ts3/anim/m05_B20.h"
#include "ts3/anim/m05_B21.h"
#include "ts3/anim/m05_B24.h"
#include "ts3/anim/m05_B27.h"
#include "ts3/anim/m05_B19.h"
#include "ts3/math/m04_B11.h"
#include <cstring>
#include <xmmintrin.h>
namespace ts3::anim {
namespace {
template<unsigned A> void* Table() {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(uintptr_t(A));
#else
    AbortNotRewritten("M05 evaluator game table"); return nullptr;
#endif
}
}
const void* __fastcall PayloadE480(void* self,void*,const void* input) {
    auto src=static_cast<const float*>(input);
    auto dst=reinterpret_cast<float*>(static_cast<unsigned char*>(self)+16);
    _mm_store_ps(dst,_mm_load_ps(src));
    _mm_store_ps(dst+4,_mm_load_ps(src+4));
    return input;
}
B03Evaluator* __fastcall DeleteE4A0(B03Evaluator* self,void*,uint32_t flags) {
    self->vtable=Table<0x00FC11EC>();
    if(self->secondResource) CallGameVirtual<void,1>(self->secondResource);
    if(self->resource) CallGameVirtual<void,1>(self->resource);
    b01::Reset(reinterpret_cast<b01::Node*>(self));
    if(flags&1) CallGameFunction<void,0x004E3F70>(self);
    return self;
}
void __fastcall SampleE4F0(B03Evaluator* self,void*,void* context,void* pose) {
    // COMISS/jae: unordered falls back to the context, just as a negative value does.
    float weight=self->weight;
    if(!(weight>=0.0f)) weight=static_cast<M05Context*>(context)->weight;
    if(!self->resource) return;
    void* clip=b23::Data(reinterpret_cast<b23::Resource*>(self->resource),nullptr);
    if(static_cast<M05Context*>(context)->flags&2) {
        void* binding=CallGameMethod<void*,0x0057ADF0>(self->resource,static_cast<M05SamplePose*>(pose)->rig);
        CallGameMethod<void,0x005886E0>(clip,pose,self->time,binding,uint32_t(self->enabled));
    }
    auto view=reinterpret_cast<M05ClipResource*>(self->resource)->view;
    if((reinterpret_cast<uintptr_t>(view->name)&2) && (static_cast<M05Context*>(context)->flags&1)) {
        CallGameMethod<void,0x00588730>(clip,pose,self->time,weight,uint32_t(self->enabled));
    } else {
        _mm_store_ps(static_cast<M05SamplePose*>(pose)->root,_mm_setzero_ps());
        _mm_store_ps(static_cast<M05SamplePose*>(pose)->root+4,_mm_set_ps(1,0,0,0));
    }
    static_cast<M05SamplePose*>(pose)->weight=1.0f;
}
namespace {
struct IKChain {
    uint32_t opaque[4]; int32_t target[12]; uint32_t flags; int32_t bone; uint32_t tail;
};
TS3_X86_SIZE(IKChain,0x4c);
struct alignas(16) IKRecord {
    float rotation[4],translation[4]; uint32_t low,high; int32_t bone,target; float weight;
};
TS3_X86_SIZE(IKRecord,64);
static bool NearZero(float v) {
    __m128 x=_mm_set1_ps(v);
    x=_mm_max_ps(_mm_mul_ps(x,_mm_set1_ps(-1.0f)),x);
    return _mm_movemask_ps(_mm_cmple_ps(x,_mm_set1_ps(1.52587890625e-5f)))==15;
}
static void AddDelta(SrtValue* s,const SrtValue* delta) {
    auto p=reinterpret_cast<float*>(s); auto d=reinterpret_cast<const float*>(delta);
    _mm_store_ps(p,_mm_add_ps(_mm_load_ps(d),_mm_load_ps(p)));
    __m128 q=_mm_add_ps(_mm_load_ps(d+4),_mm_load_ps(p+4));
    __m128 n=_mm_mul_ps(q,q);
    __m128 sum=_mm_add_ps(_mm_shuffle_ps(n,n,0x0e),n);
    n=_mm_add_ps(_mm_shuffle_ps(sum,sum,1),sum);
    n=_mm_shuffle_ps(n,n,0);
    if(_mm_cvtss_f32(n)>1.0e-12f) {
        __m128 r=_mm_rsqrt_ps(n);
        __m128 c=_mm_sub_ps(_mm_set1_ps(1),_mm_mul_ps(_mm_mul_ps(r,r),n));
        c=_mm_add_ps(_mm_mul_ps(c,_mm_mul_ps(_mm_set1_ps(0.5f),r)),r);
        _mm_store_ps(p+4,_mm_mul_ps(c,q));
    }
    _mm_store_ps(p+8,_mm_add_ps(_mm_load_ps(d+8),_mm_load_ps(p+8)));
}
static void EvaluateIK(B03Evaluator* self,M05Context* context,b20::Pose* pose) {
    auto rig=context->rig;
    if(rig->ikBegin==rig->ikEnd) return;
    SrtValue* srt=b20::Local(pose);
    b20::Auxiliary(pose);
    auto groups=static_cast<b19::Groups*>(pose->workspace);
    // The names remain original game data, reached only by x86 dependency bridges.
    int32_t world=CallGameMethod<int32_t,0x00575E30>(&rig->skeleton,Table<0x00FC1240>());
    int32_t special[3]={-1,-1,-1};
    alignas(16) SrtValue relative[3];
    if(world!=-1) {
        SrtValue offset=srt[world];
        special[0]=CallGameMethod<int32_t,0x00575E30>(&rig->skeleton,Table<0x00FC1230>());
        special[1]=CallGameMethod<int32_t,0x00575E30>(&rig->skeleton,Table<0x00FC1224>());
        special[2]=CallGameMethod<int32_t,0x00575E30>(&rig->skeleton,Table<0x00FC1218>());
        for(unsigned i=0;i<3;++i) if(special[i]!=-1) {
            if(context->deltas) AddDelta(srt+special[i],static_cast<SrtValue*>(context->deltas)+special[i]);
        }
        for(unsigned i=0;i<3;++i) if(special[i]!=-1)
            ts3::math::b11::ComposePose(reinterpret_cast<__m128*>(srt+special[i]),reinterpret_cast<__m128*>(&offset),reinterpret_cast<__m128*>(&relative[i]));
    }
    auto first=static_cast<IKChain*>(rig->ikBegin),last=static_cast<IKChain*>(rig->ikEnd);
    unsigned row=0;
    for(auto chain=first;chain!=last;++chain,++row) {
        b19::Group group; // Only initialized fields are consumed; record tail padding stays opaque.
        group.range={group.storage,group.storage,reinterpret_cast<b19::Record*>(group.vectors)};
        _mm_store_ps(reinterpret_cast<float*>(group.vectors),_mm_set_ps(1,0,0,0));
        _mm_store_ps(reinterpret_cast<float*>(group.vectors+16),_mm_setzero_ps());
        group.weight=0; group.contribution=1; group.flags=chain->flags;
        if(chain->bone==-1) group.contribution=0;
        else {
            auto root=reinterpret_cast<float*>(srt+chain->bone);
            _mm_store_ps(reinterpret_cast<float*>(group.vectors),_mm_load_ps(root+4));
            _mm_store_ps(reinterpret_cast<float*>(group.vectors+16),_mm_load_ps(root));
            for(unsigned col=0;col<11;++col) {
                const auto& b=self->binding[row][col];
                if(!(b.word[0]|b.word[1]) || int32_t(b.word[2])==-1) continue;
                int32_t target=chain->target[b.word[3]];
                if(target==-1) continue;
                auto source=reinterpret_cast<float*>(srt+target);
                IKRecord record;
                _mm_store_ps(record.rotation,_mm_set_ps(1,0,0,0));
                _mm_store_ps(record.translation,_mm_setzero_ps());
                record.low=b.word[0]; record.high=b.word[1]; record.bone=int32_t(b.word[2]); record.target=target;
                record.weight=source[2];
                if(self->secondResource) {
                    float factor=CallGameMethod<float,0x0057C780>(self->secondResource,target);
                    record.weight=static_cast<float>(static_cast<double>(factor)*static_cast<double>(record.weight));
                }
                if(record.weight>0 && !NearZero(record.weight)) {
                    // Original deliberately faults at absolute 8/12 for a missing actor.
                    auto address=reinterpret_cast<uintptr_t>(context->actor)+(context->actor?0x10:8);
                    auto key=reinterpret_cast<const volatile uint32_t*>(address);
                    if(record.low!=key[0] || record.high!=key[1]) {
                        if(!CallGameVirtual<void*,0>(context->resolver,record.low,record.high)) continue;
                    }
                }
                _mm_store_ps(record.rotation,_mm_load_ps(source+4));
                int which=-1;
                for(int k=0;k<3;++k) if(record.bone==special[k]) { which=k; break; }
                if(world!=-1 && which!=-1 && CallGameVirtual<void*,0>(context->resolver,record.low,record.high)==context->actor) {
                    auto r=reinterpret_cast<float*>(&relative[which]);
                    _mm_store_ps(record.rotation,_mm_load_ps(r+4));
                    _mm_store_ps(record.translation,_mm_load_ps(r));
                    record.low=record.high=0; record.bone=-1;
                }
                CallGameMethod<void,0x00574ED0>(&group,&record);
            }
        }
        if(group.weight>0) b19::Append(groups+row,&group);
    }
}
}
void __fastcall EvaluateE630(B03Evaluator* self,void*,void* context,void* pose) {
    if(!self->resource) return;
    b20::Reset(static_cast<b20::Pose*>(pose));
    if(b23::Data(reinterpret_cast<b23::Resource*>(self->resource),nullptr)) SampleE4F0(self,nullptr,context,pose);
    if(static_cast<M05Context*>(context)->flags&2) EvaluateIK(self,static_cast<M05Context*>(context),static_cast<b20::Pose*>(pose));
    if(!self->secondResource) return;
    auto out=reinterpret_cast<float*>(b20::Fill(static_cast<b20::Pose*>(pose),1.0f));
    auto weights=static_cast<float*>(b27::GetIndex(reinterpret_cast<b27::IndexOwner*>(self->secondResource)));
    auto count=CallGameMethod<uint32_t,0x0057C770>(self->secondResource);
    auto rig=static_cast<M05Context*>(context)->rig;
    auto skeleton=&rig->skeleton;
    uint32_t bones=static_cast<uint32_t>(b21::Count(reinterpret_cast<b21::SkeletonView*>(skeleton)));
    if(bones<count) count=bones;
    for(uint32_t i=0;i<count;++i) {
        int32_t source=static_cast<int32_t>(i);
        if(self->enabled) {
            source=b21::MappedIndex(reinterpret_cast<b21::SkeletonView*>(skeleton),static_cast<int32_t>(i));
            if(source<0) source=static_cast<int32_t>(i);
        }
        // Scalar load/store, not memcpy: the original permits overlapping weight arrays.
        out[i]=weights[source];
    }
    if(b24::ChannelCount(rig,nullptr)) {
        out[0x4b0/sizeof(float)]=b24::Weight(self->secondResource,nullptr);
    }
}
M05Node* __fastcall ConstructEE20(M05Node* self,void*,void* argument) {
    // Adapted from the verified ConstructName twin: same base-call/install/return pattern.
    b01::Construct(self,argument);
    self->vtable=static_cast<void**>(Table<0x00FC1250>());
    return self;
}
uint32_t __fastcall TypeEE40(void*,void*) { return 0x9419c8ab; }
}
