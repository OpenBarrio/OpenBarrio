#include "ts3/math/m04_B05.h"
#include <cstring>
#include <xmmintrin.h>
namespace ts3::math::b05 {
namespace {
template<class T> T Read(const void* p, unsigned offset=0) {
    T v; std::memcpy(&v, static_cast<const unsigned char*>(p)+offset, sizeof v); return v;
}
void Store(void* p, unsigned lane, float v) {
    std::memcpy(static_cast<unsigned char*>(p)+4*lane, &v, 4);
}
float Mul(float a,float b) { return _mm_cvtss_f32(_mm_mul_ss(_mm_set_ss(a),_mm_set_ss(b))); }
// The flag copy precedes all rereads (including potentially overlapping input).
// Mode priority is zero, signed one, signed XYZ zero + signed W one.
bool Special(const void* src, void* out, uint16_t* flags, float (&sign)[4]) {
    *flags=Read<uint16_t>(src,2);
    const auto f=Read<uint16_t>(src,2);
    for(unsigned i=0;i<4;++i) sign[i]=(f & (1u<<i)) ? -1.0f : 1.0f;
    if(f & 0x10) {
        for(int i=3;i>=0;--i) Store(out,i,0.0f);
        return true;
    }
    if(f & 0x20) {
        for(unsigned i=0;i<4;++i) Store(out,i,sign[i]);
        return true;
    }
    if(f & 0x40) {
        for(unsigned i=0;i<3;++i) Store(out,i,Mul(sign[i],0.0f));
        Store(out,3,sign[3]);
        return true;
    }
    return false;
}
uintptr_t Table(const void* src,void* out,uint16_t* flags,const float* const* table,unsigned count) {
    float sign[4];
    if(Special(src,out,flags,sign)) return reinterpret_cast<uintptr_t>(out);
    const float* base=*table;
    for(unsigned i=0;i<count;++i) {
        const auto index=Read<uint16_t>(src,4+2*i);
        Store(out,i,Mul(Read<float>(base,4*index),sign[i]));
    }
    return reinterpret_cast<uintptr_t>(count==1 ? out : base);
}
uintptr_t Quantized(const void* src,void* out,uint16_t* flags,unsigned count,bool words) {
    float sign[4];
    if(Special(src,out,flags,sign)) return reinterpret_cast<uintptr_t>(out);
    unsigned last=0;
    for(unsigned i=0;i<count;++i) {
        last=words ? Read<uint16_t>(src,4+2*i) : Read<uint8_t>(src,4+i);
        const float scaled=Mul(static_cast<float>(last),sign[i]);
        Store(out,i,Mul(scaled,words ? 0.00024420026f : 0.0039215689f));
    }
    return (count==1 || count==4) ? last : reinterpret_cast<uintptr_t>(out);
}
}
uintptr_t MirrorSigns(const void* source,void* output,char alternate) {
    const __m128 signs=alternate ? _mm_set_ps(1,-1,1,1) : _mm_set_ps(-1,1,1,-1);
    const __m128 input=_mm_load_ps(static_cast<const float*>(source));
    _mm_store_ps(static_cast<float*>(output),_mm_mul_ps(signs,input));
    return reinterpret_cast<uintptr_t>(source);
}
uintptr_t MirrorSignsShuffled(const void* source,void* output,char alternate) {
    const __m128 input=_mm_load_ps(static_cast<const float*>(source));
    // Match the original shuffle/multiply sequence: false passes Z through
    // without any arithmetic (including no extra sNaN exception on that lane).
    const __m128 positive=_mm_set1_ps(1.0f), negative=_mm_set1_ps(-1.0f);
    const __m128 x=_mm_mul_ps(_mm_shuffle_ps(input,input,0),alternate ? positive : negative);
    const __m128 y=_mm_mul_ps(_mm_shuffle_ps(input,input,0x55),positive);
    const __m128 z=alternate ? _mm_mul_ps(_mm_shuffle_ps(input,input,0xaa),negative)
                             : _mm_shuffle_ps(input,input,0xaa);
    const __m128 w=_mm_mul_ps(_mm_shuffle_ps(input,input,0xff),negative);
    const __m128 xy=_mm_shuffle_ps(x,y,0x10), zw=_mm_shuffle_ps(z,w,0x32);
    _mm_store_ps(static_cast<float*>(output),_mm_shuffle_ps(xy,zw,0x88));
    return reinterpret_cast<uintptr_t>(alternate ? source : output);
}
uintptr_t Decode10(const void* source,void* output,uint16_t* flags) {
    float sign[4];
    if(Special(source,output,flags,sign)) return reinterpret_cast<uintptr_t>(output);
    unsigned last=0;
    for(unsigned i=0;i<3;++i) {
        last=(Read<uint32_t>(source,4)>>(i*10)) & 1023;
        // x87 keeps the unsigned integer * sign exact, then uses a binary32 literal.
        const double v=static_cast<double>(last)*sign[i];
        Store(output,i,static_cast<float>(v*static_cast<double>(0.00097751711f)));
    }
    return last;
}
uintptr_t DecodeTable1(const void* s,void* o,uint16_t* f,const float* const* t) { return Table(s,o,f,t,1); }
uintptr_t DecodeTable2(const void* s,void* o,uint16_t* f,const float* const* t) { return Table(s,o,f,t,2); }
uintptr_t DecodeTable3(const void* s,void* o,uint16_t* f,const float* const* t) { return Table(s,o,f,t,3); }
uintptr_t DecodeTable4(const void* s,void* o,uint16_t* f,const float* const* t) { return Table(s,o,f,t,4); }
uintptr_t DecodeByte1(const void* s,void* o,uint16_t* f) { return Quantized(s,o,f,1,false); }
uintptr_t DecodeByte2(const void* s,void* o,uint16_t* f) { return Quantized(s,o,f,2,false); }
uintptr_t DecodeByte3(const void* s,void* o,uint16_t* f) { return Quantized(s,o,f,3,false); }
uintptr_t DecodeByte4(const void* s,void* o,uint16_t* f) { return Quantized(s,o,f,4,false); }
uintptr_t DecodeWord4(const void* s,void* o,uint16_t* f) { return Quantized(s,o,f,4,true); }
uintptr_t PackNormal(const void* source,void* output) {
    const __m128 input=_mm_load_ps(static_cast<const float*>(source));
    const __m128 v=_mm_add_ps(_mm_mul_ps(_mm_set1_ps(127.0f),input),_mm_set1_ps(128.0f));
    auto* bytes=static_cast<uint8_t*>(output);
    bytes[2]=static_cast<uint8_t>(_mm_cvttss_si32(v));
    bytes[1]=static_cast<uint8_t>(_mm_cvttss_si32(_mm_shuffle_ps(v,v,0x55)));
    bytes[0]=static_cast<uint8_t>(_mm_cvttss_si32(_mm_shuffle_ps(v,v,0xaa)));
    bytes[3]=0;
    return reinterpret_cast<uintptr_t>(output);
}
}
