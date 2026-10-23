#include "ts3/anim/m05_M105.h"
#include "ts3/math/m04_B11.h"
#include "ts3/layout.h"
#include <cmath>
#include <cstring>
namespace ts3::anim::m105 {
namespace {
__m128 Identity() { return _mm_set_ps(1,0,0,0); }
__m128 Dot(__m128 a,__m128 b) {
    auto p=_mm_mul_ps(a,b);
    p=_mm_add_ps(_mm_shuffle_ps(p,p,14),p);
    p=_mm_add_ps(_mm_shuffle_ps(p,p,1),p);
    return _mm_shuffle_ps(p,p,0);
}
__m128 Normalize(__m128 q,int refinements) {
    auto n=Dot(q,q), r=_mm_rsqrt_ps(n);
    for(int i=0;i<refinements;++i)
        r=_mm_add_ps(_mm_mul_ps(_mm_sub_ps(_mm_set1_ps(1),_mm_mul_ps(_mm_mul_ps(r,r),n)),_mm_mul_ps(r,_mm_set1_ps(0.5f))),r);
    return _mm_mul_ps(r,q);
}
__m128 Interpolate(__m128 a,__m128 b,float fraction,bool current) {
    auto zero=_mm_setzero_ps(),one=_mm_set1_ps(1),t=_mm_set1_ps(fraction);
    auto d=Dot(a,b);
    if(_mm_cvtss_f32(d)<0) { a=_mm_sub_ps(zero,a); d=_mm_sub_ps(zero,d); }
    float angleLimit;
    uint32_t bits=0x3db2b8c3; std::memcpy(&angleLimit,&bits,4);
    if(_mm_cvtss_f32(d)<=float(std::cos(double(angleLimit)))) {
        float angle=float(std::acos(double(_mm_cvtss_f32(d))));
        float first=_mm_cvtss_f32(_mm_mul_ps(_mm_sub_ps(one,t),_mm_set1_ps(angle)));
        float second=_mm_cvtss_f32(_mm_mul_ps(_mm_set1_ps(angle),t));
        auto wa=_mm_div_ps(_mm_set1_ps(float(std::sin(double(first)))),_mm_set1_ps(float(std::sin(double(angle)))));
        auto wb=_mm_div_ps(_mm_set1_ps(float(std::sin(double(second)))),_mm_set1_ps(float(std::sin(double(angle)))));
        // The two original interpolation blocks have different ADDPS operand order.
        return current ? _mm_add_ps(_mm_mul_ps(wb,b),_mm_mul_ps(wa,a)) : _mm_add_ps(_mm_mul_ps(wa,a),_mm_mul_ps(wb,b));
    }
    auto positive=_mm_cmplt_ps(zero,Dot(a,b));
    auto q=_mm_or_ps(_mm_andnot_ps(positive,_mm_sub_ps(a,_mm_mul_ps(_mm_add_ps(a,b),t))),
                     _mm_and_ps(_mm_add_ps(_mm_mul_ps(_mm_sub_ps(b,a),t),a),positive));
    return Normalize(q,2);
}
// Clamp is implemented by sign bits in EA, not min/max: retain NaN and signed-zero behavior.
float ClampFraction(float value,float negative) {
    uint32_t v,n; std::memcpy(&v,&value,4); std::memcpy(&n,&negative,4);
    uint32_t lo=(n&0x80000000)?v:0;
    float f; std::memcpy(&f,&lo,4);
    float rem=1.0f-f; uint32_t r; std::memcpy(&r,&rem,4);
    uint32_t result=(r&0x80000000)?0x3f800000:lo;
    std::memcpy(&f,&result,4); return f;
}
void Frame(double rate,float time,float interval,uint32_t& index,float& fraction) {
    float value,negative;
#if defined(_M_IX86)
    // FISTP to int64 followed by low-word use and unsigned conversion; preserve x87 residual precision.
    int64_t wide;
    uint16_t cw,trunc;
    __asm { fnstcw cw }
    trunc=uint16_t(cw|0xc00);
    __asm {
        fld rate
        fmul time
        fldcw trunc
        fistp wide
        fldcw cw
    }
    index=uint32_t(wide);
    double frame=double(index);
    __asm {
        fld time
        fld interval
        fmul frame
        fsubp st(1),st(0)
        fmul rate
        fld st(0)
        fchs
        fstp negative
        fstp value
    }
#else
    index=uint32_t(int64_t(rate*double(time)));
    double residual=(double(time)-double(index)*double(interval))*rate;
    value=float(residual); negative=float(-residual);
#endif
    fraction=ClampFraction(value,negative);
}
__m128 Cross(__m128 a,__m128 q9,__m128 q18) {
    return _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(a,a,18),q9),_mm_mul_ps(_mm_shuffle_ps(a,a,9),q18));
}
}
bool SampleRoot(const ClipView* clip,b20::Pose* pose,float time,float elapsed,bool mirror) {
    float start=time-elapsed;
    // COMISS chooses zero only for an ordered negative difference; unordered keeps the NaN.
    if(start<0) start=0;
    auto* out=reinterpret_cast<__m128*>(pose->prefix);
    if(start==time) {
        out[1]=_mm_setzero_ps(); out[2]=Identity();
        return true;
    }
    b20::Local(pose);
    double rate;
#if defined(_M_IX86)
    double roundedInput;
    float interval=clip->interval;
    const float half=0.5f;
    __asm {
        fld1
        fdiv interval
        fadd half
        fstp roundedInput
    }
    rate=std::floor(roundedInput);
#else
    rate=std::floor(1.0/double(clip->interval)+0.5);
#endif
    uint32_t endFrame,startFrame; float endFraction,startFraction;
    Frame(rate,time,clip->interval,endFrame,endFraction);
    Frame(rate,start,clip->interval,startFrame,startFraction);
    auto* base=reinterpret_cast<const unsigned char*>(clip);
    auto* channel=clip->channelsOffset==-1?nullptr:reinterpret_cast<const Channel*>(base+clip->channelsOffset);
    auto* values=clip->valuesOffset==-1?nullptr:base+clip->valuesOffset;
    auto t0=_mm_setzero_ps(),t1=t0,bindT=t0;
    auto q0=Identity(),q1=q0,bindQ=q0;
    for(uint32_t i=0;i<clip->channelCount;++i,++channel) {
        if(channel->identifier!=0x2af0a8ae) continue;
        alignas(16) __m128 ea,eb,sa,sb,bind,unused;
        auto* cursor=values;
        CallGameFunction<void,0x005855F0>(clip,channel,endFrame,endFrame+1,&ea,&eb,&cursor);
        cursor=values;
        CallGameFunction<void,0x005855F0>(clip,channel,startFrame,startFrame+1,&sa,&sb,&cursor);
        cursor=values;
        CallGameFunction<void,0x005855F0>(clip,channel,uint32_t(0),uint32_t(0),&bind,&unused,&cursor);
        if(channel->component==1) {
            // Serialized XYZ expands to XYZX, not XYZ0.
            sa=_mm_shuffle_ps(sa,sa,_MM_SHUFFLE(0,2,1,0)); sb=_mm_shuffle_ps(sb,sb,_MM_SHUFFLE(0,2,1,0));
            ea=_mm_shuffle_ps(ea,ea,_MM_SHUFFLE(0,2,1,0)); eb=_mm_shuffle_ps(eb,eb,_MM_SHUFFLE(0,2,1,0));
            t0=_mm_add_ps(_mm_mul_ps(_mm_sub_ps(sb,sa),_mm_set1_ps(startFraction)),sa);
            t1=_mm_add_ps(_mm_mul_ps(_mm_sub_ps(eb,ea),_mm_set1_ps(endFraction)),ea);
            bindT=_mm_shuffle_ps(bind,bind,_MM_SHUFFLE(0,2,1,0));
        } else if(channel->component==2) {
            q0=Interpolate(sa,sb,startFraction,false);
            q1=Interpolate(ea,eb,endFraction,true);
            bindQ=Normalize(bind,1);
        }
    }
    auto translation=_mm_setzero_ps(),rotation=Identity();
    if(time>start) {
        alignas(16) __m128 inverse[]={bindT,bindQ,_mm_set1_ps(1)};
        math::b11::InvertPose(inverse,inverse);
        alignas(16) __m128 sampled[]={t0,q0,_mm_set1_ps(1)};
        math::b11::ComposePose(sampled,inverse,sampled);
        auto previousQ=sampled[1]; t0=sampled[0];
        sampled[0]=t1; sampled[1]=q1;
        math::b11::ComposePose(sampled,inverse,sampled);
        auto delta=_mm_sub_ps(sampled[0],t0);
        auto conjugate=_mm_xor_ps(_mm_set_ps(0,-0.0f,-0.0f,-0.0f),previousQ);
        auto q9=_mm_shuffle_ps(conjugate,conjugate,9),q18=_mm_shuffle_ps(conjugate,conjugate,18),w=_mm_shuffle_ps(conjugate,conjugate,255);
        auto cross=_mm_add_ps(Cross(delta,q9,q18),_mm_mul_ps(delta,w));
        translation=_mm_add_ps(_mm_mul_ps(Cross(cross,q9,q18),_mm_set1_ps(2)),delta);
        auto q=sampled[1]; auto qw=_mm_shuffle_ps(q,q,255);
        auto xyz=_mm_add_ps(_mm_add_ps(Cross(q,q9,q18),_mm_mul_ps(w,q)),_mm_mul_ps(qw,conjugate));
        auto product=_mm_mul_ps(conjugate,q);
        auto dot=_mm_add_ps(_mm_add_ps(_mm_shuffle_ps(product,product,2),_mm_shuffle_ps(product,product,1)),product);
        auto ww=_mm_sub_ps(_mm_mul_ps(qw,w),_mm_shuffle_ps(dot,dot,0));
        rotation=_mm_shuffle_ps(xyz,_mm_shuffle_ps(xyz,ww,10),132);
        if(mirror) {
            rotation=_mm_mul_ps(rotation,_mm_set_ps(-1,1,1,-1));
            translation=_mm_mul_ps(translation,_mm_set_ps(-1,1,1,-1));
        }
    }
    out[1]=translation; out[2]=rotation;
    return true;
}
}
