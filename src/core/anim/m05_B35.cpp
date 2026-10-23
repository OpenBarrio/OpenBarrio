#include "ts3/anim/m05_B35.h"
#include <xmmintrin.h>
#include <cstring>
#include <cfloat>
#include <cmath>
namespace ts3::anim::b35 {
uint32_t __fastcall BaseType(void*,void*) { return 0x74d3f00cu; }
void* __fastcall BaseQuery(void* self,void*,uint32_t id) { return id==0x74d3f00cu?self:nullptr; }
void __fastcall Reset(State* s,void*) { s->y=FLT_MAX; }
float Tuning(uint32_t mask) {
    auto* service=CallGameFunction<void*,0x005EB7C0>();
    return service?CallGameMethod<float,0x005ECBA0>(service,mask):0.0f;
}
float TuningMask(uint32_t bit,uint32_t mask) { return Tuning((1u<<(bit&31))|mask); }
float __fastcall Speed(void* self,void*) {
    alignas(16) float vector[4];
    CallGameVirtual<void,2>(self,static_cast<float*>(vector));
    __m128 v=_mm_load_ps(vector); v=_mm_mul_ps(v,v);
    __m128 z=_mm_shuffle_ps(v,v,2), y=_mm_shuffle_ps(v,v,1);
    __m128 sum=_mm_add_ps(_mm_add_ps(z,y),v);
    sum=_mm_shuffle_ps(sum,sum,0);
    return _mm_cvtss_f32(_mm_sqrt_ps(sum));
}
uint32_t __fastcall Pending(Body* s,void*) { return s->current<s->goal || s->end!=s->start; }
void __fastcall ClearToken(Body* s,void*) {
    if(s->token) {
        if(s->master) CallGameVirtual<void,30>(s->master,s->token,0u);
        s->token=0;
    }
}
static float Flip(float v) {
    uint32_t bits; std::memcpy(&bits,&v,4);
    // COMISS unordered takes JB: NaNs choose +1, then retain their payload through MULSS.
    float sign=(bits==0x80000000u || !(v>=0.0f))?1.0f:-1.0f;
    if(v==0.0f) return sign<0.0f?-0.0f:0.0f;
    bits&=0x7fffffffu; std::memcpy(&v,&bits,4);
    return v*sign;
}
void __fastcall Invert(State* s,void*) { s->dirty=1; s->x=Flip(s->x); s->y=Flip(s->y); }
uint32_t __fastcall Type(void*,void*) { return 0x065e0c92u; }
void* __fastcall Query(void* self,void*,uint32_t id) {
    if(id==0x065e0c92u) return self;
    if(id==0x74d3f00cu) return self;
    return nullptr;
}
void* __fastcall Destroy(void* self,void*,uint32_t flags) {
#if defined(_M_IX86)
    *static_cast<void***>(self)=reinterpret_cast<void**>(0x01026ff8);
#else
    static void* baseTable[1]{};
    *static_cast<void***>(self)=baseTable;
#endif
    if(flags&1) CallGameFunction<void,0x004E3F70>(self);
    return self;
}
void Configure(void* object,float value,uint32_t mode) {
    if(object) {
        auto* controller=CallGameVirtual<void*,1>(object,0xf4c95ed2u);
        if(controller) {
            CallGameVirtual<void,2>(controller,value);
            CallGameVirtual<void,5>(controller,mode);
        }
    }
}
float Curve(float value,uint32_t kind) {
    unsigned index=1;
    if(kind==0x100 || kind==0x10000 || kind==0x900 || kind==0xa00 || kind==0xb00) index=0;
    else if(kind==0x200 || kind==0x600) index=2;
    else if(kind==0x4100 || kind==0x4200 || kind==0x4300) index=4;
    else if(kind==0xf00) index=3;
    const float* table;
#if defined(_M_IX86)
    table=reinterpret_cast<const float*>(0x011663f4)+index*8;
#else
    static const float tables[5][8]={
        {0,.1f,.7853981852531433f,.3f,1.5707963705062866f,.45f,2.356194496154785f,.55f},
        {0,.25f,.7853981852531433f,.75f,1.5707963705062866f,1.125f,2.356194496154785f,1.375f},
        {0,.7f,.7853981852531433f,1.2f,1.5707963705062866f,1.8f,2.356194496154785f,2.2f},
        {0,2,.7853981852531433f,1.66f,1.5707963705062866f,3.32f,2.356194496154785f,4.975f},
        {0,2,.7853981852531433f,2.25f,1.5707963705062866f,4.5f,2.356194496154785f,6.75f}};
    table=tables[index];
#endif
    unsigned lo=0;
    for(unsigned i=0;i<4;++i) { if(!(value>=table[i*2])) break; lo=i; }
    unsigned hi=lo+1>3?3:lo+1;
    float span=table[hi*2]-table[lo*2];
    float fraction=span>0.0f?(value-table[lo*2])/span:0.0f;
    float a=table[lo*2+1], b=table[hi*2+1];
#if defined(_M_IX86)
    // Preserve x87 interpolation rather than introducing float subtraction rounding.
    __asm { fld a }
    __asm { fld b }
    __asm { fsub st(0),st(1) }
    __asm { fmul fraction }
    __asm { faddp st(1),st(0) }
#else
    return static_cast<float>((static_cast<double>(b)-a)*fraction+a);
#endif
}
// ADDSS retains its destination NaN when both operands are NaNs. Do not commute.
static float OrderedAdd(float a,float b) {
#if defined(_M_IX86)
    float result;
    __asm { movss xmm0,a }
    __asm { addss xmm0,b }
    __asm { movss result,xmm0 }
    return result;
#else
    return _mm_cvtss_f32(_mm_add_ss(_mm_set_ss(a),_mm_set_ss(b)));
#endif
}
float __fastcall Total(Body* s,void*,uint8_t forced) {
    float sum=0.0f;
    if((!s->blockFirst || forced) && s->useFirst) sum=s->first;
    if((!s->blockSecond || forced) && s->useSecond) sum=OrderedAdd(s->second,sum);
    if(s->end>s->start && s->values && s->deltas) {
        for(uint32_t i=1;i<=s->end;++i) {
            sum=OrderedAdd(s->deltas[i],sum);
            if(i>s->start) sum=OrderedAdd(s->values[i],sum);
        }
    } else if(s->start && s->values && s->deltas) {
        for(uint32_t i=s->start;i>0;--i) sum=OrderedAdd(s->deltas[i],sum);
    }
    float multiplier=s->scale;
#if defined(_M_IX86)
    if(forced) multiplier=*reinterpret_cast<const float*>(0x0114f628);
#else
    if(forced) multiplier=0.5f;
#endif
    float result=multiplier*sum;
    return s->minimum>result?s->minimum:result;
}
}
