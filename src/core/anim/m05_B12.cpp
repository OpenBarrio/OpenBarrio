#include "ts3/anim/m05_B12.h"
#include <cstring>
#include <limits>
namespace ts3::anim {
float __fastcall Duration82F0(B12Clip* s, void*) {
    if (s->mode && s->resource) return static_cast<float>(static_cast<double>(s->mode) * s->period);
#if defined(_M_IX86)
    return *reinterpret_cast<const float*>(0x0114f594);
#else
    return std::numeric_limits<float>::max();
#endif
}
void __fastcall Source8330(B12Clip* s, void*, void* r) {
    auto old=s->source;
    if (r != old) {
        if (r) CallGameVirtual<void,0>(r);
        s->source=r;
        if (old) CallGameVirtual<void,1>(old);
    }
}
uint8_t __fastcall Acquire8370(B12Clip* s, void*, const void* key) {
    auto manager=CallGameFunction<void*,0x004afd20>();
    if (!manager) return 0;
    void* result=nullptr;
    auto ok=CallGameVirtual<uint8_t,3>(manager,key,&result,0,0,0,0);
    if (!ok) return 0;
    auto old=s->source;
    s->source=result;
    if (old) CallGameVirtual<void,1>(old);
    return 1;
}
uint32_t __fastcall Type87E0(void*, void*) { return 0xb489fc2c; }
void* __fastcall Query87F0(void* s, void*, uint32_t id) {
    if (id == 0x7489d72b || id == 0xb489fc2c) return reinterpret_cast<unsigned char*>(s)-8;
    return nullptr;
}
static void InitString(B12String& s) {
    s.begin=s.end=s.storage;
    s.capacity=s.storage+96;
    *s.begin=0;
}
static void CopyString(B12String& s, const B12String& r) {
    CallGameMethod<void,0x0099c9c0>(&s,r.begin,r.end);
}
B12Record* __fastcall Copy8C50(B12Record* s, void*, const B12Record* r) {
    InitString(s->first); CopyString(s->first,r->first);
    InitString(s->second); CopyString(s->second,r->second);
    for (unsigned i=0;i<11;++i) s->words[i]=r->words[i];
    return s;
}
static void AssignString(B12String& s, const B12String& r) {
    if (&s != &r) {
        if (s.begin != s.end) { *s.begin=0; s.end=s.begin; }
        CopyString(s,r);
    }
}
B12Record* __fastcall Assign8D30(B12Record* s, void*, const B12Record* r) {
    AssignString(s->first,r->first); AssignString(s->second,r->second);
    for (unsigned i=0;i<11;++i) s->words[i]=r->words[i];
    return s;
}
uintptr_t __fastcall Append90B0(B12Vector* s, void*) {
    auto end=s->end;
    if (reinterpret_cast<uintptr_t>(end) < reinterpret_cast<uintptr_t>(s->capacity)) {
        s->end=reinterpret_cast<unsigned char*>(reinterpret_cast<uintptr_t>(end)+24);
        if (end) std::memset(end,0,24);
        return reinterpret_cast<uintptr_t>(end);
    }
    const uint32_t zero[6]={};
    return CallGameMethod<uintptr_t,0x00629280>(s,end,zero);
}
B12Record** CopyRange9110(B12Record** out, const B12Record* first, const B12Record* last, B12Record* dest) {
    *out=dest;
    while (first != last) {
        if (*out) Copy8C50(*out,nullptr,first);
        *out=reinterpret_cast<B12Record*>(reinterpret_cast<uintptr_t>(*out)+sizeof(B12Record));
        ++first;
    }
    return out;
}
void __fastcall SetTimeA0E0(B12Blend* s, void*, float value) {
    float duration=s->duration;
    double child=CallGameVirtual<float,8>(s->first);
    float time=static_cast<float>((child/duration)*value);
    CallGameVirtual<void,4>(s->first,time);
    duration=s->duration;
    child=CallGameVirtual<float,8>(s->second);
    time=static_cast<float>((child/duration)*value);
    CallGameVirtual<void,4>(s->second,time);
}
template<int Slot> static float Scaled(B12Blend* s) {
    float duration=s->duration;
    double child=CallGameVirtual<float,8>(s->first);
    float ratio=static_cast<float>(duration/child);
    double value=CallGameVirtual<float,Slot>(s->first);
    return static_cast<float>(value*ratio);
}
float __fastcall TimeA140(B12Blend* s, void*) { return Scaled<5>(s); }
float __fastcall TimeA170(B12Blend* s, void*) { return Scaled<6>(s); }
float __fastcall TimeA1A0(B12Blend* s, void*) { return Scaled<7>(s); }
uint8_t __fastcall ActiveA1D0(B12Blend* s, void*) {
    if (s->first && CallGameVirtual<uint8_t,11>(s->first)) return 1;
    if (s->second && CallGameVirtual<uint8_t,11>(s->second)) return 1;
    return 0;
}
void __fastcall ReleaseA200(B12Blend* s, void*) {
    if (s->first) CallGameVirtual<void,1>(s->first);
    if (s->second) CallGameVirtual<void,1>(s->second);
}
}
