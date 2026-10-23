#include "ts3/anim/m05_M70.h"
#include <cstring>
#include <cmath>
#include <xmmintrin.h>
namespace ts3::anim::m70 {
Scratch* Construct(Scratch* p) {
#if defined(_M_IX86)
    p->vtable=reinterpret_cast<void*>(0x00fc2d2c);
#else
    p->vtable=nullptr;
#endif
    p->field10=nullptr; p->field18=nullptr; p->rig=nullptr; p->resource=nullptr;
    p->palette=nullptr; p->field2c=nullptr; p->field30=nullptr; p->flags=0;
    std::memset(&p->values,0,16);
    return p;
}
void SetRig(Scratch* p,RigResource* rig) {
    if(rig!=p->rig) {
        if(p->palette) { CallGameFunction<void,0x004e3f70>(p->palette); p->palette=nullptr; }
        auto* old=p->rig;
        if(rig!=old) {
            if(rig) CallGameVirtual<void,0>(rig);
            p->rig=rig;
            if(old) CallGameVirtual<void,1>(old);
        }
        if(p->resource) {
            p->flags|=1;
            uint32_t count=rig?uint32_t(rig->skeleton.end-rig->skeleton.begin):0;
            // The retained object is BoneDeltas-shaped here, not a general ResourcePrefix.
            uint32_t retained=reinterpret_cast<BoneDeltas*>(p->resource)->count;
            if(retained!=count) b20::Release(&p->resource);
        }
    }
    if(rig) CallGameVirtual<void,1>(rig);
}
void Matrices(b20::Pose* p,Matrix44Value* out) {
    for(uint32_t i=0;i<p->count;++i) {
        alignas(16) Matrix44Value temporary;
        auto* m=CallGameMethod<Matrix44Value*,0x00577240>(p->local+i,&temporary);
        int32_t parent=p->rig->skeleton.begin[i].index58;
        if(parent!=-1) CallGameFunction<void,0x00574230>(m,out+parent,out+i);
        else std::memcpy(out+i,m,64);
    }
}
int32_t WorldRange(b20::Pose* p,int32_t first,int32_t count) {
    if(!p->local) p->local=p->localStorage;
    p->flags &= 0xfffe;
    if(!p->world) { p->world=p->worldStorage; p->flags &= 0xfffe; }
    int32_t end=static_cast<int32_t>(uint32_t(first)+uint32_t(count));
    for(int32_t i=first;i<end;++i) {
        int32_t parent=p->rig->skeleton.begin[i].index58;
        if(parent!=-1) CallGameFunction<void,0x0054c8d0>(p->local+i,p->world+parent,p->world+i);
        else std::memcpy(p->world+i,p->local+i,48);
    }
    return first<end?end:first;
}
void* CopyBackward(const void* begin,const void* end,void* destinationEnd) {
    auto* s=static_cast<const unsigned char*>(end);
    auto* d=static_cast<unsigned char*>(destinationEnd);
    while(s!=begin) {
        s-=64; d-=64;
        // Ordered chunks preserve the original overlap behaviour, including within a record.
        _mm_store_ps(reinterpret_cast<float*>(d),_mm_load_ps(reinterpret_cast<const float*>(s)));
        _mm_store_ps(reinterpret_cast<float*>(d+16),_mm_load_ps(reinterpret_cast<const float*>(s+16)));
        for(int i=32;i<48;i+=4) std::memcpy(d+i,s+i,4);
        float f; std::memcpy(&f,s+48,4);
#if defined(_M_IX86)
        __asm { fld f }
        __asm { fstp f }
#endif
        std::memcpy(d+48,&f,4);
    }
    return d;
}
static __m128 S(__m128 a,int lane) {
    alignas(16) float f[4]; _mm_store_ps(f,a); return _mm_set1_ps(f[lane]);
}
static float Angle(float w) {
#if defined(_M_IX86)
    float result;
    __asm { fld w }
    __asm { mov eax, 0x00f7edde }
    __asm { call eax }
    __asm { fstp result }
    return result;
#else
    return static_cast<float>(std::acos(static_cast<double>(w)));
#endif
}
static float Wrap(float angle) {
#if defined(_M_IX86)
    float result;
    // Keep the operand stack order and extended precision of EA's _CIfmod calls.
    if(angle>=0) {
        __asm { fld angle }
        __asm { fadd dword ptr ds:[0x00fc2cac] }
        __asm { fld qword ptr ds:[0x00fc13b8] }
        __asm { mov eax, 0x00f7efb0 }
        __asm { call eax }
        __asm { fsub dword ptr ds:[0x00fc2cac] }
        __asm { fstp result }
    } else {
        __asm { fld angle }
        __asm { fsub dword ptr ds:[0x00fc2cac] }
        __asm { fld qword ptr ds:[0x00fc13b8] }
        __asm { mov eax, 0x00f7efb0 }
        __asm { call eax }
        __asm { fadd dword ptr ds:[0x00fc2cac] }
        __asm { fstp result }
    }
    return result;
#else
    constexpr double turn=6.2831854820251465; constexpr float pi=3.1415927410125732f;
    return angle>=0?float(std::fmod(double(angle)+pi,turn)-pi):float(std::fmod(double(angle)-pi,turn)+pi);
#endif
}
uint32_t Evaluate(Scratch* p,b20::Pose* pose) {
    if(!pose) return 0;
    auto* local=b20::Local(pose);
    auto* world=CallGameMethod<SrtValue*,0x005749b0>(pose);
    const uint32_t hashes[10]={0xa303ce83,0x0c9e57d0,0x15af037e,0x646ea315,0xa92b596e,0xf0143b40,0xc6035e5e,0x83b1355c,0x83b1355c,0x81b330de};
    const unsigned group[10]={0,0,0,1,1,1,2,2,3,3};
    float sums[]={0.0f,0.0f,0.0f,0.0f};
    const __m128 one=_mm_set1_ps(1),half=_mm_set1_ps(.5f);
    for(unsigned i=0;i<10;++i) {
        int32_t index=CallGameMethod<int32_t,0x00575b00>(&p->rig->skeleton,hashes[i]);
        __m128 a=_mm_load_ps(reinterpret_cast<float*>(&local[index])+4);
        __m128 b=_mm_xor_ps(_mm_load_ps(reinterpret_cast<float*>(&p->rig->localBegin[index])+4),_mm_set_ps(0,-0.0f,-0.0f,-0.0f));
        __m128 b12=_mm_shuffle_ps(b,b,0x12),b9=_mm_shuffle_ps(b,b,9);
        __m128 xyz=_mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(a,a,0x12),b9),_mm_mul_ps(_mm_shuffle_ps(a,a,9),b12));
        xyz=_mm_add_ps(xyz,_mm_mul_ps(a,S(b,3)));
        xyz=_mm_add_ps(xyz,_mm_mul_ps(S(a,3),b));
        __m128 ab=_mm_mul_ps(a,b);
        __m128 dot=_mm_add_ps(_mm_add_ps(_mm_shuffle_ps(ab,ab,2),_mm_shuffle_ps(ab,ab,1)),ab);
        __m128 w=_mm_sub_ps(_mm_mul_ps(S(a,3),S(b,3)),S(dot,0));
        __m128 q=_mm_shuffle_ps(xyz,_mm_shuffle_ps(xyz,w,0x0a),0x84);
        __m128 squares=_mm_mul_ps(q,q);
        __m128 pair=_mm_add_ps(_mm_shuffle_ps(squares,squares,0x0e),squares);
        __m128 norm=S(_mm_add_ps(_mm_shuffle_ps(pair,pair,1),pair),0);
        __m128 r=_mm_rsqrt_ps(norm);
        r=_mm_add_ps(_mm_mul_ps(_mm_sub_ps(one,_mm_mul_ps(_mm_mul_ps(r,r),norm)),_mm_mul_ps(half,r)),r);
        r=_mm_add_ps(_mm_mul_ps(_mm_sub_ps(one,_mm_mul_ps(_mm_mul_ps(r,r),norm)),_mm_mul_ps(half,r)),r);
        float qw; _mm_store_ss(&qw,S(_mm_mul_ps(r,q),3));
        float angle=Angle(qw)*2.0f;
        sums[group[i]]+=std::fabs(Wrap(angle));
    }
    int32_t left=CallGameMethod<int32_t,0x00575b00>(&p->rig->skeleton,0xafa30b6bu);
    int32_t right=CallGameMethod<int32_t,0x00575b00>(&p->rig->skeleton,0x6fa96266u);
    auto y=[](const SrtValue& s) { float f;std::memcpy(&f,reinterpret_cast<const char*>(&s)+4,4);return f; };
    float height=(y(world[left])-y(world[right]))-(y(p->rig->worldBegin[left])-y(p->rig->worldBegin[right]));
    float* values=reinterpret_cast<float*>(&p->values);
    for(unsigned i=0;i<4;++i) {
        values[i]=sums[i]*(i<2?0.3333333432674408f:0.5f);
        if(height<0 && i<2) values[i]+=(-0.0f-height);
        else if(height>0 && i>=2) values[i]+=height;
        __m128 v=_mm_min_ps(one,_mm_max_ps(_mm_setzero_ps(),_mm_set1_ps(values[i])));
        _mm_store_ss(values+i,v);
    }
    return 4;
}
#if defined(_M_IX86)
namespace hooks {
uint32_t __fastcall Evaluate(Scratch* p,void*,b20::Pose* a){return m70::Evaluate(p,a);}
void __fastcall SetRig(Scratch* p,void*,RigResource* a){m70::SetRig(p,a);}
Scratch* __fastcall Construct(Scratch* p,void*){return m70::Construct(p);}
void __fastcall Matrices(b20::Pose* p,void*,Matrix44Value* a){m70::Matrices(p,a);}
int32_t __fastcall WorldRange(b20::Pose* p,void*,int32_t a,int32_t b){return m70::WorldRange(p,a,b);}
}
#endif
}
