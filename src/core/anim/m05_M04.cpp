#include "ts3/anim/m05_M04.h"
#include <xmmintrin.h>
#include <cstring>
namespace ts3::anim::m04 {
namespace {
void* Address(uint32_t a) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(a);
#else
    (void)a; return nullptr;
#endif
}
Node* Owner(ListLink* p) {return p?reinterpret_cast<Node*>(reinterpret_cast<unsigned char*>(p)-offsetof(Node,link)):nullptr;}
unsigned Count(ListLink* h) { unsigned n=0; for(auto p=h->next;p!=h;p=p->next) ++n; return n; }
Pose* Temporary(Context* c) {
    void* p=CallGameFunction<void*,0x005748a0>(0x19b30u,Address(0x00fc10d8),0u,0u,0u,0u);
    if(!p) return nullptr;
    auto rig=c->rig;
    if(rig) CallGameVirtual<void,0>(rig);
    auto r=CallGameMethod<Pose*,0x00574f50>(p,rig);
    if(r) ++r->refs;
    return r;
}
uintptr_t Release(Pose* p) {
    if(!p) return 0;
    auto r=static_cast<uint32_t>(p->refs)-1u; p->refs=static_cast<int32_t>(r);
    if(r) return r;
    p->refs=1;
    return CallGameVirtual<uintptr_t,0>(p,1u);
}
float Weight(Node* n) {
    uint32_t bits; std::memcpy(&bits,&n->effective,4);
    // The boundary getter uses FLD: signaling NaNs are quieted before an SSE consumer.
    if((bits&0x7f800000u)==0x7f800000u && (bits&0x007fffffu)) bits|=0x00400000u;
    float value;std::memcpy(&value,&bits,4);return value;
}
SrtValue* Srts(Pose* p) {return CallGameMethod<SrtValue*,0x00574190>(p);}
int Bones(Context* c) {return CallGameMethod<int,0x00575650>(&c->rig->skeleton);}
__m128 Sum(__m128 x) {
    auto t=_mm_add_ps(_mm_shuffle_ps(x,x,0x0e),x);
    t=_mm_add_ps(_mm_shuffle_ps(t,t,0x01),t);
    return _mm_shuffle_ps(t,t,0);
}
__m128 Lerp(__m128 a,__m128 b,float w) {return _mm_add_ps(_mm_mul_ps(_mm_sub_ps(b,a),_mm_set1_ps(w)),a);}
void Blend(SrtValue* out,const SrtValue* a,const SrtValue* b,Node* last) {
    auto ap=reinterpret_cast<const float*>(a), bp=reinterpret_cast<const float*>(b);
    auto op=reinterpret_cast<float*>(out);
    _mm_store_ps(op+8,Lerp(_mm_load_ps(ap+8),_mm_load_ps(bp+8),Weight(last)));
    _mm_store_ps(op,Lerp(_mm_load_ps(ap),_mm_load_ps(bp),Weight(last)));
    auto qa=_mm_load_ps(ap+4), qb=_mm_load_ps(bp+4);
    auto mask=_mm_cmplt_ps(_mm_setzero_ps(),Sum(_mm_mul_ps(qb,qa)));
    auto w=_mm_set1_ps(Weight(last));
    auto positive=_mm_add_ps(_mm_mul_ps(_mm_sub_ps(qb,qa),w),qa);
    auto negative=_mm_sub_ps(qa,_mm_mul_ps(_mm_add_ps(qb,qa),w));
    auto q=_mm_or_ps(_mm_and_ps(positive,mask),_mm_andnot_ps(mask,negative));
    auto length=Sum(_mm_mul_ps(q,q));
    auto inv=_mm_rsqrt_ps(length);
    const auto one=_mm_set1_ps(1.f), half=_mm_set1_ps(.5f);
    // Two original rsqrt refinements, preserving every SSE rounding point.
    inv=_mm_add_ps(_mm_mul_ps(_mm_sub_ps(one,_mm_mul_ps(_mm_mul_ps(inv,inv),length)),_mm_mul_ps(half,inv)),inv);
    inv=_mm_add_ps(_mm_mul_ps(_mm_sub_ps(one,_mm_mul_ps(_mm_mul_ps(inv,inv),length)),_mm_mul_ps(half,inv)),inv);
    _mm_store_ps(op+4,_mm_mul_ps(inv,q));
}
float MaskWeight(float a,float b,Node* first,Node* last) {
    // The last product is rounded to float before the first product is added on x87.
    float fw=Weight(first),lw=Weight(last),second,value,negative;
#if defined(_M_IX86)
    // Respect the caller's x87 precision and rounding, including NaN selection.
    __asm {
        fld lw
        fmul b
        fstp second
        fld fw
        fmul a
        fadd second
        fld st(0)
        fchs
        fstp negative
        fstp value
    }
#else
    second=static_cast<float>(static_cast<double>(lw)*b);
    value=static_cast<float>(static_cast<double>(fw)*a+second);
    negative=-value;
#endif
    uint32_t bits,neg; std::memcpy(&bits,&value,4);std::memcpy(&neg,&negative,4);
    // EA clamps with sign-bit selections, not ordered min/max (notably +/-0 and NaNs).
    if(!(neg&0x80000000u)) bits=0;
    std::memcpy(&value,&bits,4);
    float difference=1.f-value; uint32_t d;std::memcpy(&d,&difference,4);
    if(d&0x80000000u) value=1.f;
    return value;
}
}
Node* DestroyWeighted(Node* n,uint32_t flags) {
    b01::Reset(n);
    if(flags&1u) CallGameFunction<void,0x004e3f70>(n);
    return n;
}
Node* DestroyBinary(Node* n,uint32_t flags) {
    b01::Reset(n);
    if(flags&1u) CallGameFunction<void,0x004e3f70>(n);
    return n;
}
ClipEvaluator* ConstructClip(ClipEvaluator* n,void* host) {
    b01::Construct(&n->base,host);
    n->base.vtable=static_cast<void**>(Address(0x00fc11ec));
    for(auto& row:n->bindings) for(auto& b:row) {b.word[0]=b.word[1]=0;b.word[2]=b.word[3]=~0u;}
    n->primary={};n->secondary={};n->primaryResource=nullptr;n->secondaryResource=nullptr;
    n->time=0.f;n->weight=-1.f;n->enabled=0;n->flag=0;
    return n;
}
bool SetResource(ClipEvaluator* n,B03Resource* resource) {
    if(resource) {
        auto old=n->secondaryResource;
        if(resource!=old) {
            CallGameVirtual<void,0>(resource);
            n->secondaryResource=resource;
            if(old) CallGameVirtual<void,1>(old);
        }
        n->secondary=n->secondaryResource->key;
    }
    return true;
}
uintptr_t EvaluateBinary(Node* n,Context* c,Pose* out) {
    auto h=&n->children; unsigned count=Count(h);
    if(count==1) return CallGameVirtual<uintptr_t,2>(Owner(h->next),c,out);
    if(count!=2) return reinterpret_cast<uintptr_t>(h);
    auto a=Temporary(c), b=Temporary(c);
    auto first=Owner(h->next), last=Owner(h->previous);
    CallGameVirtual<void,2>(first,c,a); CallGameVirtual<void,2>(last,c,b);
    if(c->flags&2) {
        auto av=Srts(a), bv=Srts(b), ov=Srts(out); int countBones=Bones(c);
        for(int i=0;i<countBones;++i) Blend(ov+i,av+i,bv+i,last);
        CallGameMethod<void,0x00574d90>(out,a->weights,b->weights,Weight(last));
    }
    uintptr_t result=b?Release(b):reinterpret_cast<uintptr_t>(c);
    if(a) result=Release(a);
    return result;
}
uintptr_t EvaluateWeighted(Node* n,Context* c,Pose* out) {
    auto h=&n->children; unsigned count=Count(h);
    if(count==1) return CallGameVirtual<uintptr_t,2>(Owner(h->next),c,out);
    if(count!=2) return reinterpret_cast<uintptr_t>(h);
    auto a=Temporary(c), b=Temporary(c);
    auto first=Owner(h->next), last=Owner(h->previous);
    CallGameVirtual<void,2>(first,c,a); CallGameVirtual<void,2>(last,c,b);
    CallGameFunction<void,0x0057e500>(out);
    CallGameFunction<void,0x0057e730>(Weight(first),a,out);
    CallGameFunction<void,0x0057e730>(Weight(last),b,out);
    CallGameFunction<void,0x0057cf60>(out,0u);
    float* mask=nullptr;
    if(a->mask || b->mask) {
        mask=CallGameMethod<float*,0x005741b0>(out,1.f);
        for(unsigned i=0;i<static_cast<unsigned>(Bones(c));++i)
            mask[i]=MaskWeight(a->mask?a->mask[i]:1.f,b->mask?b->mask[i]:1.f,first,last);
        if(CallGameMethod<uint32_t,0x0057b4a0>(c->rig))
            mask[300]=MaskWeight(a->mask?a->mask[300]:1.f,b->mask?b->mask[300]:1.f,first,last);
    }
    if(c->flags&2) {
        auto av=Srts(a), bv=Srts(b), ov=Srts(out);
        auto joints=CallGameMethod<Joint*,0x009f0de0>(&c->rig->skeleton);
        int countBones=Bones(c);
        for(int i=0;i<countBones;++i)
            if((joints[i].value60&0x20) && (!mask || mask[i]>0.f)) Blend(ov+i,av+i,bv+i,last);
        auto ow=out->weights, aw=a->weights, bw=b->weights;
        if(ow && aw && bw) {
            auto countWeights=ow->end-ow->begin;
            for(ptrdiff_t i=0;i<countWeights;++i) {
                float w=Weight(last);
                auto v=Lerp(_mm_set1_ps(aw->begin[i]),_mm_set1_ps(bw->begin[i]),w);
                _mm_store_ss(ow->begin+i,v);
            }
        }
    }
    Release(b);
    return Release(a);
}
}
#if defined(_M_IX86)
namespace ts3::anim::m04::hooks {
Node* __fastcall DestroyWeighted(Node* n,void*,uint32_t f){return m04::DestroyWeighted(n,f);}
Node* __fastcall DestroyBinary(Node* n,void*,uint32_t f){return m04::DestroyBinary(n,f);}
ClipEvaluator* __fastcall ConstructClip(ClipEvaluator* n,void*,void* h){return m04::ConstructClip(n,h);}
bool __fastcall SetResource(ClipEvaluator* n,void*,B03Resource* r){return m04::SetResource(n,r);}
uintptr_t __fastcall EvaluateBinary(Node* n,void*,Context* c,Pose* o){return m04::EvaluateBinary(n,c,o);}
uintptr_t __fastcall EvaluateWeighted(Node* n,void*,Context* c,Pose* o){return m04::EvaluateWeighted(n,c,o);}
}
#endif
