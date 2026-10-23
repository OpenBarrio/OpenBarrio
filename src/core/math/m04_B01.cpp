#include "ts3/math/m04_B01.h"
#include "ts3/layout.h"
#include <cmath>
#include <cstring>
#include <xmmintrin.h>
namespace ts3::math::b01 {
namespace {
uint32_t Bits(float x) { uint32_t u; std::memcpy(&u,&x,4); return u; }
float Float(uint32_t u) { float x; std::memcpy(&x,&u,4); return x; }
float Add(float a,float b) { return _mm_cvtss_f32(_mm_add_ss(_mm_set_ss(a),_mm_set_ss(b))); }
float Sub(float a,float b) { return _mm_cvtss_f32(_mm_sub_ss(_mm_set_ss(a),_mm_set_ss(b))); }
float Mul(float a,float b) { return _mm_cvtss_f32(_mm_mul_ss(_mm_set_ss(a),_mm_set_ss(b))); }
float Div(float a,float b) { return _mm_cvtss_f32(_mm_div_ss(_mm_set_ss(a),_mm_set_ss(b))); }
// Selection by the subtraction's raw sign bit is observable for zero and NaN.
float Select(float diff,float negative,float positive) {
    return (Bits(diff)>>31) ? negative : positive;
}
int Sign(float x) { return Bits(x)==0x80000000u || !(x>=0.0f) ? -1:1; }
#if defined(_M_IX86)
struct Extended { unsigned char bytes[10]; };
// MSVC has no 80-bit C++ value type. Tail-return these helpers so the caller
// sees the complete ST0 value, including when x87 precision control is 64 bits.
__declspec(naked) double LoadExtended(const Extended*) {
    __asm {
        mov eax, [esp+4]
        fld tbyte ptr [eax]
        ret
    }
}
__declspec(naked) double AddExtended(float,float) {
    __asm {
        fld dword ptr [esp+4]
        fadd dword ptr [esp+8]
        ret
    }
}
__declspec(noinline) double RandomUnit() {
    return ts3::CallGameMethod<double,0x004FA870>(reinterpret_cast<int*>(0x011DC060));
}
#endif
}
#if defined(_M_IX86)
__declspec(naked) double Exp(float) {
    __asm {
        fld dword ptr [esp+4]
        fldl2e
        fmulp st(1), st(0)
        fld st(0)
        frndint
        fxch st(1)
        fsub st(0), st(1)
        f2xm1
        fld1
        faddp st(1), st(0)
        fscale
        fstp st(1)
        ret
    }
}
#else
double Exp(float x) { return std::exp(static_cast<double>(x)); }
#endif
int InRange(float x,float r) { return r>=x && x>=Sub(-0.0f,r); }
double Approach(float from,float to,float rate,float divisor,const float* scale) {
    const float v=Mul(Div(rate,divisor),*scale);
    const float delta=Sub(to,from), low=Sub(-0.0f,v);
    const float a=Select(Sub(low,delta),delta,low);
    const float b=Select(Sub(v,a),v,a);
#if defined(_M_IX86)
    return AddExtended(b,from);
#else
    return static_cast<double>(b)+static_cast<double>(from);
#endif
}
double Power(float x,int exponent) {
    uint32_t n=exponent<0 ? 0u-static_cast<uint32_t>(exponent):static_cast<uint32_t>(exponent);
    float value=1.0f;
    for (;;) {
        if (n&1) value=Mul(value,x);
        n>>=1;
        if (!n) break;
        x=Mul(x,x);
    }
    return exponent<0 ? Div(1.0f,value):value;
}
float* Gaussian(float* out,uint32_t count,float sigma) {
    const uint32_t half=(count-1)>>1;
    if (!count) return reinterpret_cast<float*>(static_cast<uintptr_t>(half));
    const float s2=Mul(sigma,sigma), denom=Mul(s2,2.0f), norm=Mul(s2,6.2831854820251465f);
    float* cursor=out;
    for (uint32_t col=0;col<count;++col) {
        const int32_t y=static_cast<int32_t>(half-col);
        const float yy=Mul(static_cast<float>(y),static_cast<float>(y));
        cursor=out+col;
        for (uint32_t row=0;row<count;++row) {
            int32_t x=static_cast<int32_t>(half-row);
            float result;
#if defined(_M_IX86)
            __asm {
                fild x
                fmul st(0), st(0)
                fadd yy
                fdiv denom
                fchs
                fldl2e
                fmulp st(1), st(0)
                fld st(0)
                frndint
                fxch st(1)
                fsub st(0), st(1)
                f2xm1
                fld1
                faddp st(1), st(0)
                fscale
                fstp st(1)
                fdiv norm
                fstp result
            }
#else
            result=static_cast<float>(std::exp(-(static_cast<double>(x)*x+yy)/denom)/norm);
#endif
            *cursor=result;
            cursor+=count;
        }
    }
    return cursor;
}
float MaxRatio(const float* a,float n) {
    float maximum=a[0];
    if (a[1]>maximum) maximum=a[1];
    if (a[2]>maximum) maximum=a[2];
    if (n>maximum) maximum=n;
    return Div(n,maximum);
}
double __stdcall Response(float x) {
    if (x<0.001f) return 51.0;
    float value=Div(1.0f,Add(Mul(x,0.9800000190734863f),0.02f));
    return value<1.0f ? 1.0f:value;
}
double __stdcall InverseResponse(float x) {
    float value=Mul(Sub(Div(50.0f,x),1.0f),0.020408162847161293f);
    value=Select(Sub(-0.0f,value),value,0.0f);
    return Select(Sub(1.0f,value),1.0f,value);
}
double Legendre(int degree,int order,double x) {
#if defined(_M_IX86)
    Extended p{},q{},root{},temp{};
    double one=1.0, two=2.0;
    __asm {
        fld1
        fstp tbyte ptr p
    }
    if (order>0) {
        __asm {
            fld1
            fsub x
            fld x
            fadd one
            fmulp st(1), st(0)
            fsqrt
            fstp tbyte ptr root
        }
        for (int i=0;i<order;++i) {
            double odd=2.0*i+1.0;
            __asm {
                fld odd
                fchs
                fld tbyte ptr root
                fmulp st(1), st(0)
                fld tbyte ptr p
                fmulp st(1), st(0)
                fstp tbyte ptr p
            }
        }
    }
    if (degree==order) return LoadExtended(&p);
    double m=static_cast<double>(order);
    __asm {
        fld m
        fmul two
        fadd one
        fld tbyte ptr p
        fmulp st(1), st(0)
        fmul x
        fstp tbyte ptr q
    }
    if (degree==order+1) return LoadExtended(&q);
    if (degree<order+2) return 0.0;
    for (int n=order+2;n<=degree;++n) {
        double a=2.0*n-1.0,b=static_cast<double>(n)+order-1.0,c=static_cast<double>(n)-order;
        __asm {
            fld a
            fld tbyte ptr q
            fmulp st(1), st(0)
            fmul x
            fld tbyte ptr p
            fmul b
            fsubp st(1), st(0)
            fdiv c
            fstp tbyte ptr temp
        }
        p=q; q=temp;
    }
    return LoadExtended(&q);
#else
    double p=1.0;
    if (order>0) {
        const double root=std::sqrt((1.0-x)*(x+1.0));
        for (int i=0;i<order;++i) p*=-(2.0*i+1.0)*root;
    }
    if (degree==order) return p;
    double q=(2.0*order+1.0)*p*x;
    if (degree==order+1) return q;
    if (degree<order+2) return 0.0;
    for (int n=order+2;n<=degree;++n) {
        double next=((2.0*n-1.0)*q*x-p*(static_cast<double>(n)+order-1.0))/(static_cast<double>(n)-order);
        p=q; q=next;
    }
    return q;
#endif
}
void ScaleMagnitude(float* self,float factor) {
    const float x=*self;
    if (x==0.0f) {
        if (factor<0.0f) { *self=-0.0f; return; }
        if (factor>0.0f) { *self=0.0f; return; }
    }
    *self=Mul(Float(Bits(x)&0x7fffffffu),factor);
}
int SignedGreater(const float* self,const float* other) {
    return *self>*other && Sign(*self)>=Sign(*other);
}
int SignedLessEqual(const float* self,const float* other) {
    return *other>=*self && Sign(*other)>=Sign(*self);
}
float* __stdcall Degrees(float* out,const float* radians) {
    const float sign=static_cast<float>(Sign(*radians));
    float value=Mul(*radians,57.295780181884766f);
    *out=value;
    if (value==0.0f) *out=sign<0.0f ? -0.0f:0.0f;
    else *out=Mul(Float(Bits(value)&0x7fffffffu),sign);
    return out;
}
static float* SinCosRaw(float angle,float* sine,float* cosine,float scale,float step,const float* pairs) {
    float quant=Add(Mul(scale,angle),12582912.0f);
    uint32_t raw=Bits(quant),index=(raw&15)*2;
    const int32_t whole=static_cast<int32_t>(raw-0x4b400000u);
    float v=Sub(angle,Mul(static_cast<float>(whole),step));
    const float s=pairs[index], c=pairs[index+1];
    *sine=Add(Mul(Sub(c,Mul(Mul(s,v),0.5f)),v),s);
    // Deliberate reload: output can overlap the table.
    *cosine=Sub(pairs[index+1],Mul(Add(Mul(Mul(pairs[index+1],v),0.5f),pairs[index]),v));
    return cosine;
}
float* SinCos(float angle,float* sine,float* cosine,const TrigState& state) {
    return SinCosRaw(angle,sine,cosine,state.scale,state.step,state.pairs);
}
void __fastcall ScaleMagnitudeEntry(float* self,void*,float factor) { ScaleMagnitude(self,factor); }
int __fastcall SignedGreaterEntry(const float* self,void*,const float* other) { return SignedGreater(self,other); }
int __fastcall SignedLessEqualEntry(const float* self,void*,const float* other) { return SignedLessEqual(self,other); }
float* SinCosEntry(float angle,float* sine,float* cosine) {
#if defined(_M_IX86)
    return SinCosRaw(angle,sine,cosine,*reinterpret_cast<const float*>(0x011DA248),
        *reinterpret_cast<const float*>(0x011DA24C),reinterpret_cast<const float*>(0x011DC068));
#else
    (void)angle; (void)sine; (void)cosine;
    ts3::AbortNotRewritten("external runtime trig data for m04_B01::SinCosEntry");
    return nullptr;
#endif
}
#if defined(_M_IX86)
__declspec(naked) void MaxRatioEntry() {
    __asm {
        push ecx
        push edx
        push eax
        push dword ptr [esp+16]
        push eax
        call MaxRatio
        add esp, 8
        sub esp, 4
        fstp dword ptr [esp]
        movss xmm0, dword ptr [esp]
        add esp, 4
        pop eax
        pop edx
        pop ecx
        ret
    }
}
#endif
#if defined(_M_IX86)
__declspec(naked) double RandomRange(float,float) {
    __asm {
        sub esp, 16
        fld dword ptr [esp+20]
        fld st(0)
        fld dword ptr [esp+24]
        fsub st(1), st(0)
        fxch st(1)
        fstp qword ptr [esp+8]
        faddp st(1), st(0)
        fstp qword ptr [esp]
        call RandomUnit
        fld qword ptr [esp]
        fld st(0)
        fld qword ptr [esp+8]
        fsub st(1), st(0)
        fxch st(3)
        fmulp st(1), st(0)
        fadd st(0), st(2)
        fcomi st(0), st(1)
        jb belowHigh
        fstp st(2)
        fstp st(1)
        add esp, 16
        ret
    belowHigh:
        fstp st(1)
        fxch st(1)
        fcomi st(0), st(1)
        ja belowLow
        fstp st(0)
        add esp, 16
        ret
    belowLow:
        fstp st(1)
        add esp, 16
        ret
    }
}
#else
double RandomRange(float center,float radius) {
    (void)center; (void)radius;
    ts3::AbortNotRewritten("external game RNG dependency for m04_B01::RandomRange");
    return 0.0;
}
#endif
}
