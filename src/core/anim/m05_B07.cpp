#include "ts3/anim/m05_B07.h"
#include <intrin.h>
#include <xmmintrin.h>
#include <cstring>
#include <cmath>
namespace ts3::anim {
using State=b07::State;
using StreamState=b07::StreamState;
uint32_t b07::TypeDF40(void*) { return 121353492; }
uint32_t b07::Type1BB0(void*) { return 186501084; }
uint32_t b07::Type5100(void*) { return 196530341; }
uint32_t b07::Type53F0(void*) { return 201291342; }
void* b07::QueryDF50(void* self,uint32_t id) {
    if(id==121353492 || id==1943976136) return self;
    return nullptr;
}
void* b07::Query1BC0(void* self,uint32_t id) {
    if(id==186501084 || id==1943976136) return self;
    return nullptr;
}
void* b07::Query5110(void* self,uint32_t id) {
    if(id==196530341 || id==1943976136) return self;
    return nullptr;
}
struct Ref { void** vtable; long count; };
static void Retain(void* p) { if(p) _InterlockedIncrement(&static_cast<Ref*>(p)->count); }
static uintptr_t Release(void* p) {
    if(p && _InterlockedDecrement(&static_cast<Ref*>(p)->count)==0) {
        _InterlockedExchange(&static_cast<Ref*>(p)->count,1);
        return CallGameVirtual<uintptr_t,0>(static_cast<Ref*>(p),uint32_t(1));
    }
    return reinterpret_cast<uintptr_t>(p);
}
static uintptr_t Replace(void*& slot,void* p) {
    void* old=slot;
    if(old!=p) {
        Retain(p);
        slot=p;
        return Release(old);
    }
    return reinterpret_cast<uintptr_t>(old);
}
uintptr_t b07::SetDFF0(State* self,void* p) { return Replace(self->first,p); }
uintptr_t b07::SetE040(State* self,void* p) { return Replace(self->second,p); }
static bool Step(StreamState* s,unsigned n,bool write,uintptr_t& eax) {
    s->cursor=static_cast<int32_t>(static_cast<uint32_t>(s->cursor)+n);
    if(s->cursor>=0) {
        if(write) eax=CallGameMethod<uintptr_t,0x00594140>(s,n);
        else eax=CallGameMethod<uintptr_t,0x00593FF0>(s,n);
    }
    return s->cursor<0;
}
static void Write(void* self,unsigned offset,StreamState* s,unsigned n,uintptr_t& eax,int resultKind) {
    unsigned char value[8]; std::memcpy(value,static_cast<unsigned char*>(self)+offset,n);
    if(Step(s,n,true,eax)) {
        std::memcpy(s->writeBase+s->cursor-n+1,value,n);
        if(resultKind==1) eax=reinterpret_cast<uintptr_t>(s->writeBase);
        if(resultKind==2) eax=static_cast<uint32_t>(s->cursor);
    }
}
static bool Read(void* self,unsigned offset,StreamState* s,unsigned n,uintptr_t& eax,int resultKind) {
    if(Step(s,n,false,eax)) {
        std::memcpy(static_cast<unsigned char*>(self)+offset,s->readBase+s->cursor-n+1,n);
        if(resultKind==1) std::memcpy(&eax,static_cast<unsigned char*>(self)+offset+n-4,4);
        if(resultKind==2) eax=static_cast<uint32_t>(s->cursor);
        if(resultKind==3) eax=reinterpret_cast<uintptr_t>(s->readBase);
        return true;
    }
    return false;
}
uintptr_t b07::Save5140(void* self,StreamState* s) {
    uintptr_t eax=0;
    Write(self,0x30,s,4,eax,1);
    Write(self,0x34,s,4,eax,2);
    return eax;
}
uintptr_t b07::Load51A0(void* self,StreamState* s) {
    uintptr_t eax=0;
    bool transferred=Read(self,0x30,s,4,eax,3);
    // FLD/FSTP quiets a signaling NaN in this float field.
    if(transferred) {
    auto fp=reinterpret_cast<float*>(static_cast<unsigned char*>(self)+0x30);
#if defined(_M_IX86)
    __asm {
        mov edx,fp
        fld dword ptr [edx]
        fstp dword ptr [edx]
    }
#else
    // Use a conversion rather than addition so negative zero remains negative.
    volatile double widened=*fp;
    *fp=static_cast<float>(widened);
#endif
    }
    Read(self,0x34,s,4,eax,2);
    return eax;
}
uintptr_t b07::Save1BF0(void* self,StreamState* s) {
    uintptr_t eax=0;
    Write(self,0x30,s,4,eax,1);
    Write(self,0x34,s,4,eax,2);
    Write(self,0x28,s,8,eax,1);
    Write(self,0x38,s,8,eax,1);
    Write(self,0x40,s,1,eax,0);
    Write(self,0x41,s,1,eax,1);
    Write(self,0x42,s,1,eax,2);
    Write(self,0x43,s,1,eax,0);
    return eax;
}
uintptr_t b07::Load1D10(void* self,StreamState* s) {
    uintptr_t eax=0;
    Read(self,0x30,s,4,eax,3);
    Read(self,0x34,s,4,eax,3);
    Read(self,0x28,s,8,eax,1);
    Read(self,0x38,s,8,eax,1);
    for(unsigned i=0;i<4;++i) {
        unsigned char v=static_cast<unsigned char>(reinterpret_cast<uintptr_t>(s));
        if(Step(s,1,false,eax)) v=s->readBase[s->cursor];
        v=v!=0;
        static_cast<unsigned char*>(self)[0x40+i]=v;
        eax=(eax & ~uintptr_t(255)) | v;
    }
    return eax;
}
math::Float4Value* b07::AdvanceE090(math::Float4Value* out,void* controller,uint32_t a,uint32_t b,float remaining,uint8_t notify,math::Float4Value* auxiliary,uint32_t stop) {
    *out={};
    Retain(controller);
    for(;;) {
        float end=static_cast<float>(CallGameVirtual<double,75>(controller));
        float time=static_cast<float>(CallGameVirtual<double,41>(controller,a,b));
        float target=time+remaining;
        // COMISS/JB takes the short path for unordered too.
        bool shortPath=!(target>=end);
        float sample=shortPath?target:end;
        int32_t index=-1;
        alignas(16) math::Float4Value temporary;
        auto p=CallGameVirtual<math::Float4Value*,71>(controller,&temporary,sample,auxiliary,&index);
        *out=*p;
        if(auxiliary && index>=0) {
            auto track=CallGameVirtual<unsigned char*,52>(controller,index);
            float weight; std::memcpy(&weight,track+0x20,4);
            if(0.001f>=weight) *auxiliary={};
        }
        if(notify) CallGameVirtual<void,42>(controller,sample,a,b);
        if(shortPath) break;
        remaining=remaining-(end-time);
        uint32_t oldType=*CallGameVirtual<uint32_t*,12>(controller);
        void* next=CallGameVirtual<void*,31>(controller);
        if(next!=controller) { Retain(next); Release(controller); controller=next; }
        if(!controller) return out;
        uint32_t type=*CallGameVirtual<uint32_t*,12>(controller);
        if(type==0xf00e2192 || type==0x8d43950a ||
           (oldType==0x50660d52 && type!=0x443f45ce) ||
           (type!=0xe15cafc5 && oldType==0xe15cafc5 && stop) || !(remaining>0)) break;
    }
    Release(controller);
    return out;
}
int32_t b07::Rotate23F0(void* output,const void* bind,int32_t index,const math::Float4Value* axis,float angle) {
    if(index<0) return index;
    unsigned offset=48u*static_cast<unsigned>(index);
    auto dst=reinterpret_cast<float*>(static_cast<unsigned char*>(output)+offset+16);
    auto src=reinterpret_cast<const float*>(static_cast<const unsigned char*>(bind)+offset+16);
    __m128 q=_mm_xor_ps(_mm_set_ps(0.f,-0.f,-0.f,-0.f),_mm_load_ps(src));
    __m128 v=_mm_load_ps(axis->lane);
    __m128 y=_mm_shuffle_ps(q,q,18), z=_mm_shuffle_ps(q,q,9);
    __m128 t=_mm_add_ps(_mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v,v,18),z),_mm_mul_ps(_mm_shuffle_ps(v,v,9),y)),_mm_mul_ps(_mm_shuffle_ps(q,q,255),v));
    __m128 rotated=_mm_add_ps(_mm_mul_ps(_mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(t,t,18),z),_mm_mul_ps(_mm_shuffle_ps(t,t,9),y)),_mm_set1_ps(2.f)),v);
    float half=angle/2.f, sn,cosValue;
#if defined(_M_IX86)
    __asm {
        fld half
        fsin
        fstp sn
        fld half
        fcos
        fstp cosValue
    }
#else
    sn=std::sin(half); cosValue=std::cos(half);
#endif
    __m128 s=_mm_set1_ps(sn);
    __m128 x=_mm_mul_ps(_mm_shuffle_ps(rotated,rotated,0),s);
    __m128 yy=_mm_mul_ps(_mm_shuffle_ps(rotated,rotated,85),s);
    __m128 zz=_mm_mul_ps(_mm_shuffle_ps(rotated,rotated,170),s);
    __m128 r=_mm_shuffle_ps(_mm_shuffle_ps(x,yy,16),_mm_shuffle_ps(zz,_mm_set1_ps(cosValue),50),136);
    q=_mm_load_ps(dst);
    __m128 rw=_mm_shuffle_ps(r,r,255), qw=_mm_shuffle_ps(q,q,255);
    t=_mm_add_ps(_mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(q,q,18),_mm_shuffle_ps(r,r,9)),_mm_mul_ps(_mm_shuffle_ps(q,q,9),_mm_shuffle_ps(r,r,18))),_mm_mul_ps(q,rw));
    t=_mm_add_ps(t,_mm_mul_ps(qw,r));
    __m128 prod=_mm_mul_ps(q,r);
    __m128 sum=_mm_add_ps(_mm_add_ps(_mm_shuffle_ps(prod,prod,2),_mm_shuffle_ps(prod,prod,1)),prod);
    __m128 w=_mm_sub_ps(_mm_mul_ps(qw,rw),_mm_shuffle_ps(sum,sum,0));
    _mm_store_ps(dst,_mm_shuffle_ps(t,_mm_shuffle_ps(t,w,10),132));
    return static_cast<int32_t>(offset);
}
}
