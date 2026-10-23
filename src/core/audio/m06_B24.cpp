#include "ts3/audio/m06_B24.h"
#include <cstring>
#include <algorithm>
#include <emmintrin.h>
namespace ts3::audio::b24 {
namespace {
uint32_t Bits(float f) { uint32_t u; std::memcpy(&u,&f,4); return u; }
float Float(uint32_t u) { float f; std::memcpy(&f,&u,4); return f; }
uintptr_t Get(Sound* s,uint32_t key,float* out) { return CallGameVirtual<uintptr_t,4>(&s->secondary,key,out); }
uintptr_t Set(Sound* s,uint32_t key,float value) { return CallGameVirtual<uintptr_t,13>(s->player,key,value,1u); }
double Curve(void* p,float x) { return CallGameMethod<double,0x008aa6b0>(p,x); }
void Retain(void* p) { if(p) CallGameVirtual<uintptr_t,0>(p); }
}
uintptr_t Load(Sound* s) {
    uintptr_t r=CallGameMethod<uintptr_t,0x008af9f0>(s);
    if(!(r&255)) return r;
    const uint32_t keys[]={0xc238095f,0x401b5e99,0xc820c227};
    for(unsigned i=0;i<3;++i) if(CallGameVirtual<uintptr_t,6>(&s->properties,keys[i])&255) {
#if defined(_M_IX86)
        const char* label=reinterpret_cast<const char*>(0x0102a1fc);
#else
        const char* label="Audio/ResponseCurve";
#endif
        void* p=CallGameFunction<void*,0x004e3f90>(0x360u,label,0u,0u,0u,0u);
        if(p) p=CallGameMethod<void*,0x008aaf70>(p);
        s->curves[i]=p;
        CallGameMethod<uintptr_t,0x008aaef0>(p,&s->properties,keys[i]);
    }
    CallGameVirtual<uintptr_t,9>(&s->properties,0x0a880ddbu,&s->enabled);
    CallGameVirtual<uintptr_t,12>(&s->properties,0xa27efff6u,&s->attenuation);
    CallGameVirtual<uintptr_t,12>(&s->properties,0x0b66da56u,&s->fade);
    CallGameVirtual<uintptr_t,12>(&s->properties,0x95d452e6u,&s->release);
    CallGameVirtual<uintptr_t,12>(&s->properties,0x49cb23b0u,&s->maximum);
    float randomRange=0;
    if(CallGameVirtual<uintptr_t,12>(&s->properties,0x77c00609u,&randomRange)&255) {
#if defined(_M_IX86)
        using Rand=int(__cdecl*)();
        const int n=(*reinterpret_cast<Rand*>(0x00f95498))();
#else
        const int n=CallGameFunction<int,0x00f95498>();
#endif
        s->variation=s->variation+(float(n)*0.000030518509f)*randomRange;
    }
    uint8_t paused=0;
    r=CallGameVirtual<uintptr_t,9>(&s->properties,0xf14973d7u,&paused);
    if((r&255) && paused) {
        r=CallGameFunction<uintptr_t,0x00869340>();
        if(r&255) {
#if defined(_M_IX86)
            const char* name=reinterpret_cast<const char*>(0x0102a1f0);
#else
            const char* name="gamepause";
#endif
            r=CallGameVirtual<uintptr_t,47>(s,name);
        }
    }
    return (r&~uintptr_t(255))|1;
}
void* Copy(Sound* s,Sound* to) {
    CallGameMethod<uintptr_t,0x008ae2e0>(s,to);
    to->identity=s->identity; return s->identity;
}
void** Assign(void** s,void* p) {
    // Adapted from the verified 006E9540 twin; its two concrete refcount targets differ.
    void* old=*s;
    if(p!=old) {
        if(p) static_cast<misc::ResourceBaseSec*>(p)->AddRef();
        *s=p;
        if(old) static_cast<misc::RefCounted00*>(old)->Release();
    }
    return s;
}
void** Clear(void** s) { *s=CallGameFunction<void*,0x004e0b20>(*s); return s; }
Pair* CopyPair(Pair* s,const Pair* p) { s->key=p->key; s->resource=p->resource; Retain(s->resource); return s; }
uintptr_t Publish(Sound* s) {
    CallGameMethod<uintptr_t,0x008ad6f0>(s);
    float v;
    if((Get(s,0x4af82a78,&v)&255) && v!=0) CallGameVirtual<uintptr_t,3>(&s->secondary,0x647a7836u,s->maximum,1u);
    if(s->curves[1] && (Get(s,0x779a5ff4,&v)&255)) {
        float x;
        if(s->curves[0] && (Get(s,0x489c3762,&x)&255)) v=float(Curve(s->curves[0],x)+double(v));
        // EA's sign-bit clamps, deliberately not std::clamp (NaNs and -0 differ).
        uint32_t b=Bits(v)&uint32_t(int32_t(Bits(-0.0f-v))>>31);
        float lower=Float(b);
        uint32_t mask=uint32_t(int32_t(Bits(1.0f-lower))>>31);
        v=Float((mask&0x3f800000u)|(~mask&b));
        Set(s,0x82beafe0,float(Curve(s->curves[1],v)));
    }
    if(s->begin!=s->end) return CallGameVirtual<uintptr_t,15>(s->player,0x9fa9372eu,s->begin,uint32_t((s->end-s->begin)*2));
    return reinterpret_cast<uintptr_t>(s->begin);
}
uintptr_t Tick(Sound* s,float a,float b,uint32_t c) {
    uintptr_t r=CallGameMethod<uintptr_t,0x008acce0>(s,a,b,c);
    if(s->state>=2) {
        r=CallGameVirtual<uintptr_t,8>(s,-1,1u);
        if(s->player && !(r&255) && s->active) {
            float v=0; r=Get(s,0x647a7836,&v);
            if(r&255) r=Set(s,0x647a7836,s->fade*v);
            if(s->release>0) r=Set(s,0x49eb68db,s->release);
        }
    }
    return r;
}
float Gain(Sound* s,float a,float b,float c,float d,Sound* source,uint8_t* output) {
    float gain=CallGameMethod<float,0x008abda0>(s,a,b,c,d,source,static_cast<uint8_t*>(nullptr));
    void* manager=nullptr;
#if defined(_M_IX86)
    auto adjusted=*reinterpret_cast<uint8_t**>(0x011d8c40);
    if(adjusted) manager=adjusted-0x27bd0;
#else
    AbortNotRewritten("audio manager singleton for Gain");
#endif
    if(!source) source=s;
    void* ids[3]; uint32_t rooms[3]; int32_t levels[3];
    if(s->curves[2] || s->enabled) for(unsigned i=0;i<3;++i)
        CallGameMethod<uintptr_t,0x00865660>(manager,i==0?2u:i==1?3u:1u,&ids[i],&rooms[i],&levels[i]);
    if(s->curves[2] && gain>0) gain=float(Curve(s->curves[2],s->identity && s->identity==&ids[0]?1.0f:0.0f)*double(gain));
    uint8_t previous=s->active; s->active=0;
    if(s->enabled && gain>0) {
        float v=0; int32_t level=0; uint32_t room=0;
        if(Get(source,0x39ff78d1,&v)&255) level=_mm_cvtt_ss2si(_mm_set_ss(v));
        v=0;
        if(Get(source,0x5dbed967,&v)&255) {
            // x87 FISTP qword under truncation; keep only the low dword.
            if(v>=-9223372036854775808.0 && v<9223372036854775808.0) room=uint32_t(int64_t(v));
            else room=0;
        }
        const bool strict=static_cast<uint8_t*>(manager)[0x27d09]!=0;
        s->active=1;
        for(unsigned i=0;i<3;++i) {
            bool equal;
            if(strict) equal=(s->identity==ids[i] && room==rooms[i]) || (!room && !rooms[i]);
            else equal=(!room && !rooms[i]) || (s->identity==ids[i] && room && rooms[i] && level==levels[i]);
            if(equal) { s->active=0; break; }
        }
        if(s->active) {
            auto absdiff=[](int32_t x,int32_t y) { uint32_t u=uint32_t(x)-uint32_t(y); uint32_t mask=uint32_t(int32_t(u)>>31); return int32_t((u^mask)-mask); };
            uint32_t distance=uint32_t(std::min(absdiff(level,levels[0]),absdiff(level,levels[1])));
            if(distance) {
                for(uint32_t i=0;i<distance;++i) gain=s->attenuation*gain;
            } else {
                float scale=strict?s->attenuation:((1.0f-s->attenuation)*0.5f+s->attenuation);
                gain=scale*gain;
            }
        }
    }
    if(!s->active && previous && s->player && !output) {
        float v=0;
        if(Get(s,0x647a7836,&v)&255) {
            double curve=Curve(reinterpret_cast<uint8_t*>(s)+0xae0,s->position);
            float f=float(curve); if(double(v)>curve) v=f;
            Set(s,0x647a7836,v);
        }
        v=0; if(Get(s,0x49eb68db,&v)&255) Set(s,0x49eb68db,v);
    } else if(output) *output=s->active;
    return gain;
}
Temporary* InitTemporary(Temporary* s,void* p,uint8_t flag) {
    s->resource=p; Retain(p); s->flag=flag;
    if(p) CallGameVirtual<uintptr_t,1>(p);
    return s;
}
Pair* MakePair(Pair* s,const uint32_t* key,void* const* p) { s->key=*key; s->resource=*p; Retain(s->resource); return s; }
TaggedPair* CopyTagged(TaggedPair* s,const TaggedPair* p) { s->key=p->key; s->resource=p->resource; Retain(s->resource); s->flag=p->flag; return s; }
void* Rehash(Map* s,uint32_t n) {
    auto buckets=static_cast<Node**>(CallGameFunction<void*,0x004e3f90>(uint32_t((n+1)*sizeof(Node*)),s->label,0x400000u,0u,
#if defined(_M_IX86)
        reinterpret_cast<const char*>(0x00f98980),
#else
        "", 
#endif
        0xe7u));
    std::memset(buckets,0,n*sizeof(Node*)); buckets[n]=reinterpret_cast<Node*>(~uintptr_t(0));
    void* r=buckets;
    for(uint32_t i=0;i<s->count;++i) while(Node* p=s->buckets[i]) {
        uint32_t k=p->key%n; s->buckets[i]=p->next; r=buckets[k]; p->next=buckets[k]; buckets[k]=p;
    }
    if(s->count>1) r=CallGameFunction<void*,0x004e3f70>(s->buckets);
    s->buckets=buckets; s->count=n; return r;
}
Sound* Construct(Sound* s) {
    CallGameMethod<uintptr_t,0x008b05d0>(s);
    s->attenuation=0.1f; s->fade=0.05f;
    s->curves[0]=s->curves[1]=s->curves[2]=s->identity=nullptr;
    s->release=0; s->maximum=150; s->enabled=s->active=0;
#if defined(_M_IX86)
    s->primary=reinterpret_cast<void*>(0x0102a230); s->secondary=reinterpret_cast<void*>(0x0103a910); s->tertiary=reinterpret_cast<void*>(0x0102a22c);
    s->label=reinterpret_cast<const char*>(0x0102a214); s->begin=s->end=reinterpret_cast<uint16_t*>(0x011c7e54); s->capacity=reinterpret_cast<uint16_t*>(0x011c7e56);
#else
    AbortNotRewritten("sound interface tables and empty UTF16 storage");
#endif
    return s;
}
uint32_t Type() { return 0x00f97f7a; }
}
