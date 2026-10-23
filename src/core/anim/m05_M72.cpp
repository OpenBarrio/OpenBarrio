#include "ts3/anim/m05_M72.h"
#include <cstring>
#include <cmath>
#include <xmmintrin.h>
namespace ts3::anim::m72 {
namespace {
void Copy(Record* d,const Record* s) {
    std::memcpy(d,s,48);
    uint32_t b; std::memcpy(&b,s->bytes+48,4);
    // FLD/FSTP quiets signaling NaNs; the remaining twelve bytes are untouched.
    if((b&0x7f800000u)==0x7f800000u && (b&0x7fffffu)) b|=0x400000u;
    std::memcpy(d->bytes+48,&b,4);
}
void* Table(uintptr_t value) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(value);
#else
    static void* tables[2]{}; return &tables[value==0x0107396c];
#endif
}
}
void Insert(Records* s,Record* where,const Record* value) {
    if(s->end!=s->capacity) {
        if(reinterpret_cast<uintptr_t>(value)>=reinterpret_cast<uintptr_t>(where) && reinterpret_cast<uintptr_t>(value)<reinterpret_cast<uintptr_t>(s->end)) ++value;
        if(s->end) Copy(s->end,s->end-1);
        for(auto p=s->end-1;p!=where;) { --p; Copy(p+1,p); }
        Copy(where,value);
        ++s->end;
    } else {
        auto count=(reinterpret_cast<uintptr_t>(s->end)-reinterpret_cast<uintptr_t>(s->begin))/64;
        auto capacity=count?count*2:1;
        // EA passes a literal zero destination to its uninitialized-copy helper.
        Record* out=nullptr;
        for(auto p=s->begin;p!=where;++p) {
            if(out) Copy(out,p);
            out=reinterpret_cast<Record*>(reinterpret_cast<uintptr_t>(out)+64);
        }
        if(out) Copy(out,value);
        out=reinterpret_cast<Record*>(reinterpret_cast<uintptr_t>(out)+64);
        for(auto p=where;p!=s->end;++p) { if(out) Copy(out,p); ++out; }
        s->begin=nullptr; s->end=out;
        s->capacity=reinterpret_cast<Record*>(capacity*64);
    }
}
void Destroy(Pose* s) {
    s->flags&=0xfffe;
    s->vtable=static_cast<void**>(Table(0x00fc2d4c));
    s->srt=nullptr; s->auxiliary38=nullptr; s->auxiliary3c=nullptr;
    s->workspace=nullptr; s->auxiliary40=nullptr;
    if(s->weights) {
        auto w=static_cast<Weights*>(s->weights);
        if(w->begin) CallGameFunction<void,0x004e3f70>(w->begin);
        CallGameFunction<void,0x004e3f70>(w);
    }
    if(s->rig) CallGameVirtual<void,1>(s->rig);
    s->vtable=static_cast<void**>(Table(0x0107396c));
}
void Reset(Pose* s) {
    s->translation={}; s->rotation={0,0,0,1};
    if(s->workspace) {
        for(unsigned i=0;i<6;++i) {
            auto p=static_cast<unsigned char*>(s->workspace)+i*0x3020;
            std::memcpy(p+sizeof(void*),p,sizeof(void*));
            std::memset(p+0x3010,0,4);
        }
    }
    s->flags&=0xfffe;
    s->srt=nullptr; s->auxiliary38=nullptr; s->auxiliary3c=nullptr;
}
void EnsureWeights(Pose* s) {
    if(!s->rig || s->weights) return;
    void* descriptor=Table(0x00fc2d54);
    auto w=CallGameFunction<Weights*,0x004e3f90>(uint32_t(16),descriptor,0,0,0,0);
    if(w) {
        auto count=CallGameMethod<uint32_t,0x0057b4a0>(s->rig);
        uint32_t padded=static_cast<uint32_t>(std::ceil(static_cast<double>(count)*0.25))*4u;
        s->weights=CallGameMethod<Weights*,0x00795f50>(w,padded,&descriptor);
    } else s->weights=nullptr;
}
void Blend(Pose* s,const Weights* a,const Weights* b,float t) {
    if(!a || !b) return;
    EnsureWeights(s);
    auto w=static_cast<Weights*>(s->weights);
    auto count=w->end-w->begin;
    for(ptrdiff_t i=0;i<count;++i) {
        auto x=_mm_sub_ss(_mm_set_ss(b->begin[i]),_mm_set_ss(a->begin[i]));
        x=_mm_mul_ss(x,_mm_set_ss(t));
        w->begin[i]=_mm_cvtss_f32(_mm_add_ss(x,_mm_set_ss(a->begin[i])));
    }
}
void SetWeight(Pose* s,uint32_t hash,float value) {
    if(!s->rig) return;
    auto index=CallGameMethod<int32_t,0x0057b4f0>(s->rig,hash);
    if(index<0) return;
    EnsureWeights(s);
    auto w=static_cast<Weights*>(s->weights);
    if(index<w->end-w->begin) w->begin[index]=value;
}
}
