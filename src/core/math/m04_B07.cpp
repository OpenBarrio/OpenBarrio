#include "ts3/math/m04_B07.h"
#include <xmmintrin.h>
#include <cmath>
#include <cstdint>
#include <cstring>
namespace ts3::math::b07 {
float* Reflect(float* out,const float* a,const float* normal) {
    __m128 n=_mm_load_ps(normal), v=_mm_load_ps(a);
    alignas(16) float lanes[4]; _mm_store_ps(lanes,n); lanes[3]=0;
    __m128 p=_mm_mul_ps(v,n);
    __m128 s=_mm_add_ps(_mm_shuffle_ps(p,p,14),p);
    s=_mm_add_ps(_mm_shuffle_ps(s,s,1),s);
    s=_mm_mul_ps(_mm_shuffle_ps(s,s,0),_mm_set1_ps(2));
    _mm_store_ps(out,_mm_sub_ps(v,_mm_mul_ps(s,_mm_load_ps(lanes))));
    return out;
}
float* Decode(float* out,const unsigned char* packed) {
    out[0]=static_cast<float>(packed[2]);
    out[1]=static_cast<float>(packed[1]);
    out[2]=static_cast<float>(packed[0]);
    __m128 v=_mm_load_ps(out);
    v=_mm_div_ps(v,_mm_set1_ps(255));
    v=_mm_mul_ps(v,_mm_set1_ps(2));
    _mm_store_ps(out,_mm_sub_ps(v,_mm_set1_ps(1)));
    return out;
}
#if defined(_M_IX86)
__declspec(naked) void DecodeEntry() {
    __asm {
        push ecx
        push eax
        call Decode
        add esp, 8
        ret
    }
}
#endif
float* Normalize(float* out,const float* a) {
#if defined(_M_IX86)
    const float epsilon=1.e-8f;
    __asm {
        mov ecx, a
        mov eax, out
        fld dword ptr [ecx+8]
        fld dword ptr [ecx+4]
        fld dword ptr [ecx]
        fmul st(0),st(0)
        fld st(1)
        fmulp st(2),st(0)
        faddp st(1),st(0)
        fld st(1)
        fmulp st(2),st(0)
        faddp st(1),st(0)
        fadd epsilon
        fsqrt
        fld1
        fdivrp st(1),st(0)
        fld dword ptr [ecx]
        fmul st(0),st(1)
        fstp dword ptr [eax]
        fld st(0)
        fmul dword ptr [ecx+4]
        fstp dword ptr [eax+4]
        fmul dword ptr [ecx+8]
        fstp dword ptr [eax+8]
    }
#else
    double r=1.0/std::sqrt((double(a[0])*a[0]+double(a[1])*a[1])+double(a[2])*a[2]+double(1.e-8f));
    out[0]=float(a[0]*r); out[1]=float(a[1]*r); out[2]=float(a[2]*r);
#endif
    return out;
}
float* __stdcall NormalizeStd(float* out,const float* a) { return Normalize(out,a); }
float* Copy3(const float* a,float* out) {
    out[0]=a[0]; out[1]=a[1]; out[2]=a[2]; return out;
}
float* Copy4(const float* a,float* out) {
    out[0]=a[0]; out[1]=a[1]; out[2]=a[2]; out[3]=a[3]; return out;
}
int Different(const float* a,const float* b) {
    return a[0]!=b[0] || a[1]!=b[1] || a[2]!=b[2];
}
#if defined(_M_IX86)
int __fastcall DifferentEntry(const float* a,void*,const float* b) { return Different(a,b); }
#endif
double Dot(const float* a,const float* b) {
#if defined(_M_IX86)
    double result;
    __asm {
        mov eax,a
        mov ecx,b
        fld dword ptr [eax+8]
        fmul dword ptr [ecx+8]
        fld dword ptr [eax+4]
        fmul dword ptr [ecx+4]
        faddp st(1),st(0)
        fld dword ptr [eax]
        fmul dword ptr [ecx]
        faddp st(1),st(0)
        fstp result
    }
    return result;
#else
    return (double(a[2])*b[2]+double(a[1])*b[1])+double(a[0])*b[0];
#endif
}
// FLD/FSTP quiets a selected signaling NaN; an SSE move would not.
static void StoreSelected(float* out,const float* source) {
#if defined(_M_IX86)
    __asm {
        mov eax, source
        mov edx, out
        fld dword ptr [eax]
        fstp dword ptr [edx]
    }
#else
    uint32_t bits; std::memcpy(&bits,source,4);
    if((bits & 0x7f800000u)==0x7f800000u && (bits & 0x007fffffu)) bits |= 0x00400000u;
    std::memcpy(out,&bits,4);
#endif
}
// All selections precede stores, but selected values are reloaded in XYZ(W)
// order: partial overlaps must not be flattened into a copied input vector.
float* Min3(float* out,const float* a,const float* b) {
    const float* selected[3];
    for(int i=0;i<3;++i) selected[i]=(a[i]>b[i]) ? b+i : a+i;
    for(int i=0;i<3;++i) StoreSelected(out+i,selected[i]);
    return out;
}
float* Max3(float* out,const float* a,const float* b) {
    const float* selected[3];
    for(int i=0;i<3;++i) selected[i]=(b[i]>a[i]) ? b+i : a+i;
    for(int i=0;i<3;++i) StoreSelected(out+i,selected[i]);
    return out;
}
float* Min4(float* out,const float* a,const float* b) {
    const float* selected[4];
    for(int i=0;i<4;++i) selected[i]=(a[i]>b[i]) ? b+i : a+i;
    for(int i=0;i<4;++i) StoreSelected(out+i,selected[i]);
    return out;
}
float* Max4(float* out,const float* a,const float* b) {
    const float* selected[4];
    for(int i=0;i<4;++i) selected[i]=(b[i]>a[i]) ? b+i : a+i;
    for(int i=0;i<4;++i) StoreSelected(out+i,selected[i]);
    return out;
}
float* __stdcall InverseSquare(float* out,const float* a) {
    __m128 x=_mm_load_ss(a), y=_mm_load_ss(a+1), z=_mm_load_ss(a+2);
    __m128 sum=_mm_add_ss(_mm_mul_ss(x,x),_mm_mul_ss(y,y));
    sum=_mm_add_ss(sum,_mm_mul_ss(z,z));
    __m128 r=_mm_div_ss(_mm_set_ss(1),sum);
    _mm_store_ss(out,_mm_mul_ss(_mm_load_ss(a),r));
    y=_mm_mul_ss(r,_mm_load_ss(a+1));
    z=_mm_mul_ss(r,_mm_load_ss(a+2));
    _mm_store_ss(out+1,y); _mm_store_ss(out+2,z);
    return out;
}
}
