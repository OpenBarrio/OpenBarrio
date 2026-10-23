#include "ts3/audio/m06_B49.h"
#include <cstring>
namespace ts3::audio {
namespace {
template<class T> T Read(void* p,size_t n) { T v; std::memcpy(&v,static_cast<char*>(p)+n,sizeof v); return v; }
template<class T> void Write(void* p,size_t n,T v) { std::memcpy(static_cast<char*>(p)+n,&v,sizeof v); }
double Evaluate(void* p) {
    return CallGameVirtual<double,48>(p,Read<float>(p,0x5c),Read<float>(p,0x64),Read<float>(p,0x68),Read<float>(p,0x6c),uint32_t(0),uint32_t(0));
}
}
void* __fastcall B49310(void* p,void*) { return Read<void*>(p,0xad4); }
float __fastcall B49320(void* p,void*) { return Read<float>(p,0x330); }
uint8_t __fastcall B49330(void* p,void*,uint32_t i,uint32_t guard) {
    if (uint8_t(guard) && (Read<uint32_t>(p,0xadc)&0x800)) return 0;
    return i<Read<uint32_t>(p,0x186c) ? Read<uint8_t>(p,0x1864+i)>0 : Read<uint32_t>(p,0x1860)!=0;
}
uint32_t __fastcall B49370(void* p,void*,uint32_t i) {
    const uint32_t difference=Read<uint32_t>(p,0x1860)-Read<uint8_t>(p,0x1864+i);
    return (difference&0xffffff00u) | uint32_t(int32_t(difference)>0);
}
uint32_t __fastcall B49390(void* p,void*) {
    const auto flags=Read<uint32_t>(p,0xadc);
    return (flags&0xffffff00u) | uint32_t((flags&0x80)==0x80);
}
double __fastcall B493B0(void* p,void*) { return Read<double>(p,0xac0); }
double __fastcall B493C0(void* p,void*) { return Evaluate(p)*Read<float>(p,0x70); }
double __fastcall B49400(void* p,void*) { return Read<uint8_t>(p,0xd0)==15 ? Evaluate(p) : 1.0; }
uint32_t __fastcall B49450(void* p,void*,uint32_t a,uint32_t b) {
    return CallGameVirtual<uint32_t,2>(static_cast<char*>(p)+0xf4,a,b);
}
uint32_t __fastcall B49470(void* p,void*,uint32_t value,uint32_t flag) {
    auto flags=Read<uint32_t>(p,0xadc);
    if (flags&0x80) return 128;
    Write(p,0xadc,flags|0x80u);
    Write(p,0xec,value);
    Write(p,0xac,uint8_t(flag));
    auto* system=CallGameFunction<void*,0x008B0A70>();
    return CallGameVirtual<uint32_t,85>(system,0x063e3de7u,Read<uint32_t>(p,0x40),value,uint32_t(uint8_t(flag)),p);
}
uint32_t __fastcall B494D0(void* p,void*) { return Read<uint8_t>(p,0xad)!=0 || Read<uint8_t>(p,0xacd)!=0; }
uint8_t __fastcall B494F0(void* p,void*) { return Read<uint8_t>(p,0xb0); }
uint8_t __fastcall B49500(void* p,void*) { return (Read<uint8_t>(p,0xd0)>>3)&1; }
float __fastcall B49510(void* p,void*) { return Read<float>(p,0x84); }
}
