#include "ts3/math/m04_B02.h"
#include <cmath>
#include <cstring>
#include <xmmintrin.h>
#include <emmintrin.h>
// Inline-assembly return paths deliberately leave the result in ST0.
// A C++ double temporary here would add a spill absent from the EA contract.
#if defined(_M_IX86)
#pragma warning(push)
#pragma warning(disable:4035 4716)
#endif
namespace ts3::math::b02 {
namespace {
uint32_t Bits(float x) { uint32_t b; std::memcpy(&b,&x,4); return b; }
int32_t Round(float x) {
#if defined(_M_IX86)
    int32_t n;
    __asm { fld x }
    __asm { fistp n }
    return n;
#else
    // The native API follows the current FP rounding mode, not C++ truncation.
    return _mm_cvtss_si32(_mm_set_ss(x));
#endif
}
}
double ApproxCos(float x,const CosineTable& t) {
    const float biased = x*t.scale + 12582912.0f;
    const uint32_t bits = Bits(biased);
    const int32_t n = static_cast<int32_t>(bits-1262485504u);
    const float c=t.pairs[2*(bits&15)], s=t.pairs[2*(bits&15)+1];
#if defined(_M_IX86)
    const float step=t.step, half=0.5f;
    __asm {
        fild n
        fmul step
        fsubr x
        fld c
        fld st(0)
        fmul st(0),st(2)
        fmul half
        fsubr s
        fmulp st(2),st(0)
        faddp st(1),st(0)
    }
#else
    const double r=double(x)-double(n)*t.step;
    return r*(s-double(c)*r*0.5)+c;
#endif
}
#if defined(_M_IX86)
double GameApproxCos(float x) {
    // Runtime-owned globals: do not snapshot them at static initialization.
    CosineTable t;
    t.scale=*reinterpret_cast<const float*>(0x011DA7C8);
    t.step=*reinterpret_cast<const float*>(0x011DA7CC);
    std::memcpy(t.pairs,reinterpret_cast<const void*>(0x011DC068),sizeof(t.pairs));
    return ApproxCos(x,t);
}
#endif
double ApproxAtan(float y,float x) {
    const float ay=static_cast<float>(std::fabs(double(y))+double(1.0e-10f));
    const float ax=std::fabs(x);
    const float r=(ay-ax)/(ax+ay);
    const float p=(0.9817f-(r*r)*0.1963f)*r;
    // MULSS occurs at runtime: folding 3*pi/4 would ignore MXCSR rounding.
    volatile float quarterPi=0.7853981852531433f;
    const float angle=!(x>=0.0f)?quarterPi*3.0f-p:quarterPi+p;
    return !(y>=0.0f)?-0.0f-angle:angle;
}
double Clamp(float x,float lo,float hi) {
#if defined(_M_IX86)
    const float half=0.5f, three=3.0f;
    __asm {
        fld x
        fld lo
        fsub st(1), st(0)
        fld st(1)
        fabs
        faddp st(2),st(0)
        fld half
        fmul st(2), st(0)
        fxch st(2)
        faddp st(1),st(0)
        fld hi
        fld st(0)
        fsubrp st(2),st(0)
        fld st(1)
        fabs
        faddp st(2),st(0)
        fxch st(1)
        fmulp st(2),st(0)
        fsubrp st(1),st(0)
    }
#else
    double d=double(x)-lo; double v=double(hi)-(lo+(d+std::fabs(d))*0.5); return hi-0.5*(v+std::fabs(v));
#endif
}
double Smooth(float x,float lo,float hi) {
#if defined(_M_IX86)
    const float half=0.5f, three=3.0f;
    __asm {
        fld x
        fld lo
        fsub st(1), st(0)
        fld st(1)
        fabs
        faddp st(2),st(0)
        fld half
        fmul st(2), st(0)
        fxch st(2)
        fadd st(0),st(1)
        fld hi
        fld st(0)
        fsubrp st(2),st(0)
        fld st(0)
        fsub st(0),st(3)
        fld st(2)
        fabs
        faddp st(3),st(0)
        fxch st(2)
        fmulp st(4),st(0)
        fsubrp st(3),st(0)
        fxch st(2)
        fsub st(0),st(1)
        fdiv st(0),st(2)
        fld st(0)
        fadd st(0),st(1)
        fsubr three
        fmulp st(3),st(0)
        fmul st(2), st(0)
        fmulp st(2),st(0)
        faddp st(1),st(0)
    }
#else
    double d=double(x)-lo; double v=double(hi)-((d+std::fabs(d))*0.5+lo); double w=double(hi)-lo; double z=(hi-0.5*(v+std::fabs(v))-lo)/w; return w*(3.0-(z+z))*z*z+lo;
#endif
}
double Ellipse(uint32_t a,uint32_t b) {
#if defined(_M_IX86)
    const float three=3.0f,ten=10.0f,k=0.7853981852531433f,two32=4294967296.0f;
    __asm {
        mov eax,a
        fild a
        test eax,eax
        jns loadB
        fadd two32
    loadB:
        mov eax,b
        fild b
        test eax,eax
        jns evaluate
        fadd two32
    evaluate:
        fld st(0)
        fadd st(0),st(2)
        fld three
        fmul st(1),st(0)
        fld st(2)
        fmul st(0),st(3)
        fld st(4)
        fmul st(0),st(5)
        faddp st(1),st(0)
        fmulp st(1),st(0)
        fxch st(2)
        fmulp st(3),st(0)
        fxch st(2)
        fmul ten
        faddp st(1),st(0)
        fsqrt
        fsubp st(1),st(0)
        fmul k
    }
#else
    return ((double(a)+b)*3.0-std::sqrt(3.0*(double(b)*b+double(a)*a)+double(a)*b*10.0))*double(0.7853981852531433f);
#endif
}
uint8_t* ByteWeights(int32_t a,int32_t b,int32_t c,int32_t d,uint8_t* p,uint8_t* q,uint8_t* r,uint8_t* s) {
    // Unsigned arithmetic preserves the original dword overflow before CVTSI2SS.
    const uint32_t aa=a,bb=b,cc=c,dd=d;
    *p=uint8_t(Round(float(int32_t(aa-dd+bb+cc))*0.5f));
    *q=uint8_t(Round(float(int32_t(aa-cc+bb+dd))*0.5f));
    *r=uint8_t(Round(float(int32_t(aa-bb+cc+dd))*0.5f));
    *s=uint8_t(Round(float(int32_t(bb-aa+cc+dd))*0.5f));
    return s;
}
float* Weights(float a,float b,float c,float d,float* p,float* q,float* r,float* s) {
    const float ab=a+b;
    *p=((ab+c)-d)*0.5f;
    *q=((ab-c)+d)*0.5f;
    *r=(((a-b)+c)+d)*0.5f;
    *s=(((b-a)+c)+d)*0.5f;
    return s;
}
double Octant(float x) {
    const int32_t n=Round(x/0.7853981256484985f);
    const int32_t octant=n%8;
#if defined(_M_IX86)
    const float step=0.7853981256484985f;
    __asm { fild octant }
    __asm { fmul step }
#else
    return double(octant)*double(0.7853981256484985f);
#endif
}
int Near(float a,float b) {
    return a+0.0001f>b && b>a-0.0001f;
}
double __stdcall Snap(char enabled,char up,float x) {
    const int32_t n=Round(x);
    const float rounded=float(n);
    if (!enabled || std::fabs(x-rounded)<0.05f) return rounded;
    // Compare raw sign/magnitude, including negative zero, just like CMP/SBB/ADC.
    if (up) {
        const float delta=float(double(n)-double(x));
        return double(int32_t(uint32_t(n)+(Bits(delta)<0x80000001u?0u:1u)));
    }
    const float delta=float(double(x)-double(n));
    return double(int32_t(uint32_t(n)+(Bits(delta)<0x80000001u?0u:0xffffffffu)));
}
int Within(const float* xyz,float margin) {
    const float low=-0.0f-margin;
    return margin>=xyz[0] && xyz[0]>=low && margin>=xyz[1] && xyz[1]>=low && margin>=xyz[2] && xyz[2]>=low;
}
double TurnStep(float x) {
    const int32_t n=Round(x*159.15493774414062f);
#if defined(_M_IX86)
    const float step=0.006283185910433531f;
    __asm { fild n }
    __asm { fmul step }
#else
    return double(n)*double(0.006283185910433531f);
#endif
}
double AngleDistance(float a,float b) {
    const float d=std::fabs(a-b);
    const int32_t n=_mm_cvttss_si32(_mm_set_ss(d*0.15915494f));
    const float r=d-float(n)*6.2831855f;
    if (3.1415927f>r) return r;
#if defined(_M_IX86)
    const float turn=6.2831855f;
    __asm { fld turn }
    __asm { fsub r }
#else
    return double(6.2831855f)-double(r);
#endif
}
char __stdcall Heading(int,float x,float y,float* out) {
#if defined(_M_IX86)
    const float threshold=0.01f,angleOffset=1.5707964897155762f;
    double sum;
    __asm {
        fld x
        fabs
        fld y
        fabs
        faddp st(1),st(0)
        fstp sum
    }
    if (!(sum>=threshold)) { *out=0.0f; return 0; }
    float angle;
    unsigned negative;
    __asm {
        fld y
        fchs
        fld x
        fpatan
        fadd angleOffset
        fst angle
        fldz
        fcomip st(0),st(1)
        fstp st(0)
        seta al
        movzx eax,al
        mov negative,eax
    }
    *out=negative?angle+6.283185958862305f:angle;
#else
    if (!(std::fabs(double(x))+std::fabs(double(y))>=double(0.01f))) { *out=0.0f; return 0; }
    const double a=std::atan2(-double(y),double(x))+double(1.5707964897155762f);
    const float angle=float(a);
    *out=a<0?angle+6.283185958862305f:angle;
#endif
    return 1;
}
}

#if defined(_M_IX86)
#pragma warning(pop)
#endif
