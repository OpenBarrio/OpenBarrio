#include "ts3/anim/m05_M87.h"
#include <cstring>
#include <cmath>
#include <xmmintrin.h>
namespace ts3::anim::m87 {
static void* Address(uint32_t n) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(uintptr_t(n));
#else
    (void)n; AbortNotRewritten("m87 game data"); return nullptr;
#endif
}
Mask* Mask::Destroy(uint32_t flags) {
    base.vtable=Address(0xfc33a8);
    if(values.begin) CallGameFunction<void,0x4e3f70>(values.begin);
    CallGameMethod<void,0x731cf0>(this);
    if(flags&1) CallGameFunction<void,0x4e3f70>(this);
    return this;
}
bool Mask::Load(void* stream,uint32_t unused,bool swap) {
    (void)unused;
    struct Source { void* table; struct Link { void* a; void* b; unsigned char* data; }* link; };
    auto* source=static_cast<Source*>(stream);
    unsigned char* data=source->link?source->link->data:nullptr;
    uint32_t size=CallGameVirtual<uint32_t,7>(source);
    CallGameMethod<void,0x731cc0>(this,size);
    if(swap) CallGameFunction<void,0x739270>(data,size>>2);
    uint32_t version; std::memcpy(&version,data+4,4);
    if(version<0x200 || version>0x2ff) return false;
    if(version>0x200) std::memcpy(&weight,data+0x18,4); else weight=1.0f;
    uint32_t count; std::memcpy(&count,data+0x38,4);
    values.end=values.begin;
    float zero=0;
    CallGameMethod<void,0x57c870>(&values,count,&zero);
    for(uint32_t i=0;i<count;++i) if(i<size_t(values.end-values.begin)) {
        float* destination=values.begin+i;
        const unsigned char* source=data+0x3c+4*i;
#if defined(_M_IX86)
        // FLD/FSTP is observable for signaling NaNs (MOVSS/memcpy is not equivalent).
        __asm {
            mov eax, source
            fld dword ptr [eax]
            mov edx, destination
            fstp dword ptr [edx]
        }
#else
        float value; std::memcpy(&value,source,4);
        volatile double extended=value;
        *destination=static_cast<float>(extended);
#endif
    }
    return true;
}
static uintptr_t Append(void* s,uint32_t text) { return CallGameMethod<uintptr_t,0x405960>(s,Address(text)); }
static uintptr_t Range(void* s,uint32_t first,uint32_t last) { return CallGameMethod<uintptr_t,0x405790>(s,Address(first),Address(last)); }
uintptr_t Describe(Controller* c,void* s) {
    if(!c) return Range(s,0xfc1b24,0xfc1b2a);
    const uint32_t keys[]={0xb489fc2c,0x34f49f94,0x073b9d8c,0x074a5f9d,0x07a11ba8,0x06b26363,0x075e4c89,0x0664fb68,0x075e3ecb,0x0681c688};
    void* q[10];
    for(unsigned i=0;i<10;++i) q[i]=CallGameVirtual<void*,1>(&c->secondary8,keys[i]);
    if(q[0]) {
        void* resource=static_cast<Clip*>(q[0])->resource;
        if(resource) {
            const char* name=CallGameMethod<const char*,0x57a7a0>(resource);
            CallGameMethod<uintptr_t,0x405790>(s,name,name+std::strlen(name));
        }
    } else if(q[1]) {
        auto* b=static_cast<Binary*>(q[1]); Range(s,0xfc347c,0xfc3487);
        Describe(b->left,s); Range(s,0xfb64cc,0xfb64ce); Describe(b->right,s);
        CallGameFunction<uintptr_t,0x567460>(s,Address(0xfc342c),double(b->weight));
    } else if(q[4]) {
        Append(s,0xfc3470); Describe(static_cast<Unary28*>(q[4])->child,s); Append(s,0xf99d24);
    } else if(q[5]) {
        Append(s,0xfc3464); Describe(static_cast<Unary50*>(q[5])->child,s); Append(s,0xf99d24);
    } else if(q[2]) {
        Append(s,0xfc3454); Describe(static_cast<Unary30*>(q[2])->child,s); Append(s,0xf99d24);
    } else if(q[3]) {
        Append(s,0xf9eb70);
    } else if(q[6]) {
        Append(s,0xfc3444); Describe(static_cast<Unary2C*>(q[6])->child,s); Append(s,0xf99d24);
    } else if(q[9]) {
        Append(s,0xfc3438);
        Describe(CallGameMethod<Controller*,0x580420>(q[9]),s); Append(s,0xfc1b2c);
        Describe(CallGameMethod<Controller*,0x444710>(q[9]),s);
        float w=CallGameMethod<float,0x56f640>(q[9]);
        CallGameFunction<uintptr_t,0x567460>(s,Address(0xfc342c),double(w));
    } else if(q[8]) {
        auto* w=static_cast<Weighted*>(q[8]); Append(s,0xfc3420); Describe(w->child,s);
        CallGameFunction<uintptr_t,0x567460>(s,Address(0xfc3414),double(w->weight));
    } else Append(s,q[7]?0xfc340c:0xfc33f0);
    return CallGameFunction<uintptr_t,0x567460>(s,Address(0xfc33e4),c->uniqueId);
}
uintptr_t Clear(b20::Pose* p) {
    auto* local=b20::Local(p);
    std::memset(local,0,size_t(p->count)*48);
    float* weights=b20::ZeroWeights(p);
    auto* vertices=static_cast<FloatRange*>(p->vertexWeights);
    if(!vertices) return reinterpret_cast<uintptr_t>(weights);
    for(auto* v=vertices->begin;v!=vertices->end;++v) *v=0.0f;
    return reinterpret_cast<uintptr_t>(vertices->end);
}
void AddVertex(b20::Pose* source,b20::Pose* destination,float weight) {
    auto* src=static_cast<FloatRange*>(source->vertexWeights);
    if(!src) return;
    CallGameMethod<void,0x574cd0>(destination);
    float* out=static_cast<FloatRange*>(destination->vertexWeights)->begin;
    for(auto* p=src->begin;p!=src->end;++p,++out) if(*p!=0.0f) {
        float product=*p*weight;
        *out=product+*out;
    }
}
void AddRoot(Root* p,const Float4Value* in,float weight) {
    float old=p->weight;
    __m128 a=_mm_load_ps(reinterpret_cast<const float*>(&p->translation));
    __m128 b=_mm_load_ps(reinterpret_cast<const float*>(&p->rotation));
    __m128 t=_mm_load_ps(reinterpret_cast<const float*>(in));
    __m128 q=_mm_load_ps(reinterpret_cast<const float*>(in+1));
    __m128 w=_mm_set1_ps(weight);
    t=_mm_add_ps(_mm_mul_ps(w,t),a);
    if(old==0.0f) q=_mm_mul_ps(q,w);
    else {
        __m128 dot=_mm_mul_ps(q,b);
        __m128 sum=_mm_add_ps(_mm_shuffle_ps(dot,dot,0x0e),dot);
        __m128 final=_mm_add_ps(_mm_shuffle_ps(sum,sum,1),sum);
        float d=_mm_cvtss_f32(final);
        q=_mm_mul_ps(q,w);
        if(d<0 || std::isnan(d)) q=_mm_sub_ps(b,q); else q=_mm_add_ps(q,b);
    }
    p->weight=old+weight;
    _mm_store_ps(reinterpret_cast<float*>(&p->translation),t);
    _mm_store_ps(reinterpret_cast<float*>(&p->rotation),q);
}
}
