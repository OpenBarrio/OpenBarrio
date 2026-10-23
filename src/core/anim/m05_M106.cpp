#include "ts3/anim/m05_M106.h"
#include <xmmintrin.h>
#include <cmath>
#include <cstring>
namespace ts3::anim::m106 {
static const char* Address(uintptr_t a) {
#if defined(_M_IX86)
    return reinterpret_cast<const char*>(a);
#else
    (void)a; return "";
#endif
}
Style* Construct(Style* p) {
    for(auto& word:p->zero) word=0;
    #if defined(_M_IX86)
    p->begin=p->end=Address(0x011c7e54); p->capacity=Address(0x011c7e56);
    p->descriptor=Address(0x010043e0);
#else
    static const char empty[3]={};
    p->begin=p->end=empty; p->capacity=empty+2;
    p->descriptor="Locomotion/Manager/AnimationStyleDefinition/mLoopSound";
#endif
    return p;
}
uintptr_t Bind(RenderOwner* p,void* rig) {
    if(rig) {
        void* state=p->state;
        if(!state) {
            state=CallGameFunction<void*,0x004e3f90>(80,Address(0x00ff98ac),0,0,0,0);
            if(state) state=CallGameMethod<void*,0x00574100>(state);
        }
        CallGameVirtual<void,0>(rig,rig);
        CallGameVirtual<void,0>(state);
        if(state!=p->state) CallGameMethod<void,0x006f50e0>(p,state);
        CallGameMethod<void,0x00573320>(state);
    }
    uintptr_t result=CallGameMethod<uintptr_t,0x006db880>(p->binding,rig);
    if(rig) {
        result=CallGameMethod<uintptr_t,0x00575e30>(static_cast<unsigned char*>(rig)+0x28,Address(0x00ff989c));
        p->transformBone=static_cast<int32_t>(result);
    }
    return result;
}
static float ArgumentFloat(float value) {
#if defined(_M_IX86)
    // The dispatchers FLD/FSTP their float arguments, quieting signaling NaNs.
    __asm {
        fld value
        fstp value
    }
#endif
    return value;
}
uint8_t Sample(const Clip* p,b20::Pose* pose,float time,uint32_t mode,uint8_t mirror) {
    time=ArgumentFloat(time);
    if(p->prefix[2]==1) return CallGameFunction<uint8_t,0x00585d80>(p,pose,time,mode,mirror);
    return CallGameFunction<uint8_t,0x00586300>(p,pose,time,mode,mirror);
}
uint8_t Delta(const Clip* p,b20::Pose* pose,float time,float duration,uint8_t mirror) {
    duration=ArgumentFloat(duration); time=ArgumentFloat(time);
    if(p->prefix[2]==1) return CallGameFunction<uint8_t,0x00586880>(p,pose,time,duration,mirror);
    return RootMotion(p,pose,time,duration,mirror);
}
static __m128 Dot(__m128 a,__m128 b) {
    auto v=_mm_mul_ps(a,b); v=_mm_add_ps(_mm_shuffle_ps(v,v,14),v);
    v=_mm_add_ps(_mm_shuffle_ps(v,v,1),v); return _mm_shuffle_ps(v,v,0);
}
static __m128 Normalize(__m128 q,int steps) {
    auto d=Dot(q,q),r=_mm_rsqrt_ps(d),one=_mm_set1_ps(1),half=_mm_set1_ps(.5f);
    for(int i=0;i<steps;++i) r=_mm_add_ps(_mm_mul_ps(_mm_sub_ps(one,_mm_mul_ps(_mm_mul_ps(r,r),d)),_mm_mul_ps(r,half)),r);
    return _mm_mul_ps(r,q);
}
static float Sin(float v) {
#if defined(_M_IX86)
    float result;
    __asm {
        fld v
        fsin
        fstp result
    }
    return result;
#else
    return static_cast<float>(std::sin(double(v)));
#endif
}
static __m128 Slerp(__m128 a,__m128 b,float fraction,bool reverseSum) {
    auto zero=_mm_setzero_ps(),one=_mm_set1_ps(1),t=_mm_set1_ps(fraction),d=Dot(a,b);
    if(_mm_cvtss_f32(d)<0) { a=_mm_sub_ps(zero,a); d=_mm_sub_ps(zero,d); }
    // Original uses FCOS on the binary32 threshold angle.
    float angle=.0872664675f,threshold;
#if defined(_M_IX86)
    __asm {
        fld angle
        fcos
        fstp threshold
    }
#else
    threshold=static_cast<float>(std::cos(double(angle)));
#endif
    if(!(_mm_cvtss_f32(d)>threshold)) {
        float theta=static_cast<float>(std::acos(double(_mm_cvtss_f32(d))));
        float left=_mm_cvtss_f32(_mm_mul_ps(_mm_sub_ps(one,t),_mm_set1_ps(theta)));
        float right=_mm_cvtss_f32(_mm_mul_ps(t,_mm_set1_ps(theta)));
        auto x=_mm_mul_ps(_mm_div_ps(_mm_set1_ps(Sin(left)),_mm_set1_ps(Sin(theta))),a);
        auto y=_mm_mul_ps(_mm_div_ps(_mm_set1_ps(Sin(right)),_mm_set1_ps(Sin(theta))),b);
        return reverseSum?_mm_add_ps(y,x):_mm_add_ps(x,y);
    }
    auto mask=_mm_cmplt_ps(zero,Dot(a,b));
    auto v=_mm_or_ps(_mm_andnot_ps(mask,_mm_sub_ps(a,_mm_mul_ps(_mm_add_ps(a,b),t))),_mm_and_ps(mask,_mm_add_ps(_mm_mul_ps(_mm_sub_ps(b,a),t),a)));
    return Normalize(v,2);
}
static __m128 Cross(__m128 a,__m128 b) {
    return _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(a,a,18),_mm_shuffle_ps(b,b,9)),_mm_mul_ps(_mm_shuffle_ps(a,a,9),_mm_shuffle_ps(b,b,18)));
}
static __m128 Rotate(__m128 v,__m128 q) {
    auto c=_mm_add_ps(Cross(v,q),_mm_mul_ps(v,_mm_shuffle_ps(q,q,255)));
    return _mm_add_ps(_mm_mul_ps(Cross(c,q),_mm_set1_ps(2)),v);
}
static void Frame(float time,float interval,double rate,uint32_t& frame,float& fraction) {
#if defined(_M_IX86)
    int64_t converted,positiveFrame;
    unsigned short saved,truncating;
    __asm fnstcw saved
    truncating=saved|0xc00;
    __asm {
        fld rate
        fmul time
        fldcw truncating
        fistp converted
        fldcw saved
    }
    frame=static_cast<uint32_t>(converted);
    positiveFrame=frame;
    float result;
    __asm {
        fild positiveFrame
        fmul interval
        fld time
        fsubrp st(1),st(0)
        fmul rate
        fstp result
    }
    fraction=result;
#else
    frame=static_cast<uint32_t>(static_cast<int64_t>(rate*time));
    fraction=static_cast<float>((double(time)-double(frame)*interval)*rate);
#endif
}
static __m128 MirrorTranslation(__m128 translation) {
#if defined(_M_IX86)
    auto& flags=*reinterpret_cast<uint32_t*>(uintptr_t(0x011cdbc0));
    auto* mask=reinterpret_cast<float*>(uintptr_t(0x011cdbb0));
    if(!(flags&1)) { _mm_store_ps(mask,_mm_set_ps(-1,1,1,-1)); flags|=1; }
    if(!(flags&2)) { _mm_store_ps(reinterpret_cast<float*>(uintptr_t(0x011cdba0)),_mm_set_ps(1,-1,1,1)); flags|=2; }
    return _mm_mul_ps(translation,_mm_load_ps(mask));
#else
    return _mm_mul_ps(translation,_mm_set_ps(-1,1,1,-1));
#endif
}
uint8_t RootMotion(const Clip* clip,b20::Pose* pose,float time,float duration,uint8_t mirror) {
    float start=time-duration;
    if(start<0) start=0;
    auto* out=reinterpret_cast<unsigned char*>(pose);
    auto zero=_mm_setzero_ps(),identity=_mm_set_ps(1,0,0,0);
    if(start==time) { _mm_store_ps(reinterpret_cast<float*>(out+16),zero); _mm_store_ps(reinterpret_cast<float*>(out+32),identity); return 1; }
    b20::Local(pose);
    double rate=std::floor(1.0/double(clip->interval)+.5);
    uint32_t frame,prior;
    float t,s;
    Frame(time,clip->interval,rate,frame,t);
    Frame(start,clip->interval,rate,prior,s);
    // Branchless source clamps via the sign of -fraction and 1-fraction.
    auto clamp=[](float x) { uint32_t v,n,r; float neg=-x; std::memcpy(&v,&x,4); std::memcpy(&n,&neg,4); v&=uint32_t(int32_t(n)>>31); float y; std::memcpy(&y,&v,4); y=1-y; std::memcpy(&r,&y,4); uint32_t mask=uint32_t(int32_t(r)>>31); v=(mask&0x3f800000)|(v&~mask); std::memcpy(&x,&v,4); return x; };
    t=clamp(t); s=clamp(s);
    auto* base=reinterpret_cast<const unsigned char*>(clip);
    auto* channel=clip->channels==-1?nullptr:base+clip->channels;
    auto* values=clip->values==-1?nullptr:base+clip->values;
    __m128 pt=zero,ct=zero,bindT=zero,pq=identity,cq=identity,bindQ=identity;
    for(uint32_t i=0;i<clip->count;++i,channel+=20) {
        uint32_t hash; std::memcpy(&hash,channel+4,4);
        if(hash!=720414894) continue;
        alignas(16) __m128 a,b,c,d,e,f;
        auto* cursor=values; CallGameFunction<void,0x00585990>(clip,channel,frame,frame+1,&a,&b,&cursor);
        cursor=values; CallGameFunction<void,0x00585990>(clip,channel,prior,prior+1,&c,&d,&cursor);
        cursor=values; CallGameFunction<void,0x00585990>(clip,channel,0u,0u,&e,&f,&cursor);
        if(channel[19]==1) {
            a=_mm_shuffle_ps(a,a,36); b=_mm_shuffle_ps(b,b,36); c=_mm_shuffle_ps(c,c,36); d=_mm_shuffle_ps(d,d,36);
            pt=_mm_add_ps(_mm_mul_ps(_mm_sub_ps(d,c),_mm_set1_ps(s)),c);
            ct=_mm_add_ps(_mm_mul_ps(_mm_sub_ps(b,a),_mm_set1_ps(t)),a); bindT=_mm_shuffle_ps(e,e,36);
        } else if(channel[19]==2) { pq=Slerp(c,d,s,false); cq=Slerp(a,b,t,true); bindQ=Normalize(e,1); }
    }
    __m128 translation=zero,rotation=identity;
    if(time>start) {
        alignas(16) __m128 inv[3]={bindT,bindQ,_mm_set1_ps(1)},p[3]={pt,pq,_mm_set1_ps(1)},c[3]={ct,cq,_mm_set1_ps(1)};
        CallGameFunction<void,0x0054eef0>(inv,inv);
        CallGameFunction<void,0x0054c8d0>(p,inv,p); CallGameFunction<void,0x0054c8d0>(c,inv,c);
        auto q=_mm_xor_ps(p[1],_mm_set_ps(0,-0.0f,-0.0f,-0.0f));
        translation=Rotate(_mm_sub_ps(c[0],p[0]),q);
        auto r=_mm_add_ps(_mm_add_ps(Cross(c[1],q),_mm_mul_ps(_mm_shuffle_ps(q,q,255),c[1])),_mm_mul_ps(_mm_shuffle_ps(c[1],c[1],255),q));
        auto dot=_mm_mul_ps(q,c[1]); dot=_mm_add_ps(_mm_add_ps(_mm_shuffle_ps(dot,dot,2),_mm_shuffle_ps(dot,dot,1)),dot);
        auto w=_mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(c[1],c[1],255),_mm_shuffle_ps(q,q,255)),_mm_shuffle_ps(dot,dot,0));
        rotation=_mm_shuffle_ps(r,_mm_shuffle_ps(r,w,10),132);
        if(mirror) { rotation=_mm_mul_ps(rotation,_mm_set_ps(-1,1,1,-1)); translation=MirrorTranslation(translation); }
    }
    _mm_store_ps(reinterpret_cast<float*>(out+16),translation); _mm_store_ps(reinterpret_cast<float*>(out+32),rotation);
    return 1;
}
#if defined(_M_IX86)
uint8_t __fastcall HookSample(const Clip* c,void*,b20::Pose* p,float t,uint32_t m,uint8_t r) { return Sample(c,p,t,m,r); }
uint8_t __fastcall HookDelta(const Clip* c,void*,b20::Pose* p,float t,float d,uint8_t r) { return Delta(c,p,t,d,r); }
uintptr_t __fastcall HookBind(RenderOwner* p,void*,void* r) { return Bind(p,r); }
Style* __fastcall HookConstruct(Style* p,void*) { return Construct(p); }
#endif
}
