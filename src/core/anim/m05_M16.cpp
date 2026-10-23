#include "ts3/anim/m05_M16.h"
#include <xmmintrin.h>
#include "ts3/math/m04_B11.h"
#include <cstring>
#include <cmath>
#include <initializer_list>
namespace ts3::anim::m16 {
namespace {
template<class T> T& Global(uint32_t address) {
#if defined(_M_IX86)
    return *reinterpret_cast<T*>(address);
#else
    (void)address; NotRewritten("m05_M16 game global");
#endif
}
float Mul(float a,float b) { return _mm_cvtss_f32(_mm_mul_ss(_mm_set_ss(a),_mm_set_ss(b))); }
float Sub(float a,float b) { return _mm_cvtss_f32(_mm_sub_ss(_mm_set_ss(a),_mm_set_ss(b))); }
float Div(float a,float b) { return _mm_cvtss_f32(_mm_div_ss(_mm_set_ss(a),_mm_set_ss(b))); }
uint32_t Bits(float f) { uint32_t u;std::memcpy(&u,&f,4);return u; }
float Float(uint32_t u) { float f;std::memcpy(&f,&u,4);return f; }
float Clamp(float f) {
    // EA selects by the sign of subtraction, including NaNs and signed zero.
    uint32_t mask=0u-(Bits(Sub(-0.0f,f))>>31);
    f=Float(mask&Bits(f));
    mask=0u-(Bits(Sub(1.f,f))>>31);
    return Float((mask&0x3f800000u)|(Bits(f)&~mask));
}
void Smooth(bool chain) {
    float dt=Sub(Global<float>(0x011cdd98),Global<float>(0x011cd460));
    Global<float>(0x011cd460)=Global<float>(0x011cdd98);
    float speed=Global<float>(chain?0x0114f300:0x0114f314);
    float head=Clamp(Mul(Mul(speed,Global<float>(chain?0x0114f2fc:0x0114f310)),dt));
    float eye=Clamp(Mul(Mul(Global<float>(chain?0x0114f2f8:0x0114f30c),speed),dt));
    auto target=Global<__m128>(0x011cd430);
    auto& h=Global<__m128>(0x011cd440);auto& e=Global<__m128>(0x011cd450);
    h=_mm_add_ps(_mm_mul_ps(_mm_sub_ps(target,h),_mm_set1_ps(head)),h);
    e=_mm_add_ps(_mm_mul_ps(_mm_sub_ps(target,e),_mm_set1_ps(eye)),e);
}
int Find(Context* context,const char* name) { return CallGameMethod<int,0x00575e30>(&context->rig->skeleton,name); }
void Child(b01::Node* self,Context* c,Pose* p) {
    // 004DA9D0 returns the embedded sentinel at +0c.
    if(self->children.previous!=&self->children) {
        auto link=self->children.next;
        auto child=link?reinterpret_cast<b01::Node*>(reinterpret_cast<unsigned char*>(link)-offsetof(b01::Node,link)):nullptr;
        CallGameVirtual<void,2>(child,c,p);
    }
}
uintptr_t Sync(Pose* p) { return CallGameMethod<uintptr_t,0x00574990>(p); }
SrtValue* World(Pose* p) { return CallGameMethod<SrtValue*,0x005749b0>(p); }
void Update(Pose* p,int first,int count) { CallGameMethod<void,0x00574500>(p,first,count); }
float Sin(float value) {
#if defined(_M_IX86)
    float out;
    __asm { fld value
            fsin
            fstp out }
    return out;
#else
    return std::sin(value);
#endif
}
float Cos(float value) {
#if defined(_M_IX86)
    float out;
    __asm { fld value
            fcos
            fstp out }
    return out;
#else
    return std::cos(value);
#endif
}
__m128 AxisAngle(__m128 axis,float angle) {
    auto s=_mm_set1_ps(Sin(angle));auto c=_mm_set1_ps(Cos(angle));
    auto x=_mm_mul_ps(_mm_shuffle_ps(axis,axis,0),s);
    auto y=_mm_mul_ps(_mm_shuffle_ps(axis,axis,85),s);
    auto z=_mm_mul_ps(_mm_shuffle_ps(axis,axis,170),s);
    return _mm_shuffle_ps(_mm_shuffle_ps(x,y,16),_mm_shuffle_ps(z,c,50),136);
}
__m128 Product(__m128 a,__m128 b) {
    auto aw=_mm_shuffle_ps(a,a,255),bw=_mm_shuffle_ps(b,b,255);
    auto v=_mm_add_ps(_mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(a,a,9),_mm_shuffle_ps(b,b,18)),
                                _mm_mul_ps(_mm_shuffle_ps(a,a,18),_mm_shuffle_ps(b,b,9))),_mm_mul_ps(aw,b));
    v=_mm_add_ps(v,_mm_mul_ps(bw,a));
    auto m=_mm_mul_ps(a,b);
    auto sum=_mm_add_ps(_mm_add_ps(_mm_shuffle_ps(m,m,2),_mm_shuffle_ps(m,m,1)),m);
    auto w=_mm_sub_ps(_mm_mul_ps(bw,aw),_mm_shuffle_ps(sum,sum,0));
    return _mm_shuffle_ps(v,_mm_shuffle_ps(v,w,10),132);
}
__m128 Rotation(const SrtValue* world) {
    alignas(16) unsigned char converted[48];
    ts3::math::b11::InvertPose(reinterpret_cast<__m128*>(const_cast<SrtValue*>(world)),reinterpret_cast<__m128*>(converted));
    return _mm_load_ps(reinterpret_cast<float*>(converted+16));
}
template<unsigned Address> uintptr_t Eyes(Context* c,Pose* p,int left,int right) {
    if(left<0||right<0) return static_cast<uintptr_t>(right);
    alignas(16) __m128 out;
    for(int index:{right,left}) {
        auto local=&b20::Local(p)[index];auto world=&World(p)[index];
        auto result=CallGameFunction<__m128*,Address>(&out,&Global<__m128>(0x011cd450),world,local);
        _mm_store_ps(reinterpret_cast<float*>(&local->rotation),*result);
    }
    p->flags&=~uint16_t(1);return Sync(p);
}
}
uintptr_t EvaluateChain(b01::Node* self,Context* c,Pose* p) {
    int joints[5];const char* names[]={"b__Neck0__","b__Neck1__","b__Neck2__","b__Neck3__","b__Head__"};
    for(int i=0;i<5;++i) joints[i]=Find(c,names[i]);
    Smooth(true);Child(self,c,p);
    auto& flags=Global<uint32_t>(0x011cd42c);
    if(!(flags&1)) { flags|=1;Global<float>(0x011cd428)=Mul(Global<float>(0x0114f2f4),0.005f); }
    if(!(flags&2)) { flags|=2;Global<float>(0x011cd424)=Mul(Global<float>(0x0114f2f4),0.005f); }
    float a=Mul(Global<float>(0x011cd440),Global<float>(0x011cd428));
    float b=Mul(Global<float>(0x011cd444),Global<float>(0x011cd424));
    Sync(p);
    auto axis=_mm_set_ps(0,0,1,0);
    auto ay=_mm_shuffle_ps(axis,axis,9),az=_mm_shuffle_ps(axis,axis,18);
    float sa=Sin(Div(a,2.f)),ca=Cos(Div(a,2.f));
    auto second=AxisAngle(_mm_set_ps(0,1,0,0),Div(b,2.f));
    for(int i=4;i>=0;--i) {
        auto rotation=Rotation(&World(p)[joints[i]]);
        auto local=&b20::Local(p)[joints[i]];
        auto ry=_mm_shuffle_ps(rotation,rotation,9),rz=_mm_shuffle_ps(rotation,rotation,18);
        auto cross=_mm_add_ps(_mm_sub_ps(_mm_mul_ps(ry,az),_mm_mul_ps(rz,ay)),_mm_mul_ps(_mm_shuffle_ps(rotation,rotation,255),axis));
        auto rotated=_mm_add_ps(_mm_mul_ps(_mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(cross,cross,18),ry),_mm_mul_ps(_mm_shuffle_ps(cross,cross,9),rz)),_mm_set1_ps(2.f)),axis);
        auto x=_mm_mul_ps(_mm_shuffle_ps(rotated,rotated,0),_mm_set1_ps(sa));
        auto y=_mm_mul_ps(_mm_shuffle_ps(rotated,rotated,85),_mm_set1_ps(sa));
        auto z=_mm_mul_ps(_mm_shuffle_ps(rotated,rotated,170),_mm_set1_ps(sa));
        auto first=_mm_shuffle_ps(_mm_shuffle_ps(x,y,16),_mm_shuffle_ps(z,_mm_set1_ps(ca),50),136);
        auto q=_mm_load_ps(reinterpret_cast<float*>(&local->rotation));
        _mm_store_ps(reinterpret_cast<float*>(&local->rotation),Product(first,Product(second,q)));
        p->flags&=~uint16_t(1);
        if(i>0) Update(p,joints[i-1],joints[i]-joints[i-1]+1);else Sync(p);
    }
    int left=Find(c,"b__LeftEye__"),right=Find(c,"b__RightEye__");
    return Eyes<0x00555820>(c,p,left,right);
}
uintptr_t Evaluate(b01::Node* self,Context* c,Pose* p) {
    Find(c,"b__Root__");World(p);
    int neck=Find(c,"b__Neck__"),head=Find(c,"b__Head__");
    if(neck<0||head<0) {
        int result=Find(c,"b__Neck0__");
        return result?EvaluateChain(self,c,p):0;
    }
    Smooth(false);Child(self,c,p);
    float a=Mul(Global<float>(0x011cd440),Global<float>(0x0114f308));
    float b=Mul(Sub(-0.0f,Global<float>(0x0114f304)),Global<float>(0x011cd444));
    Sync(p);
    alignas(16) unsigned char converted[48];
    ts3::math::b11::InvertPose(reinterpret_cast<__m128*>(&World(p)[head]),reinterpret_cast<__m128*>(converted));
    auto local=&b20::Local(p)[head];
    alignas(16) __m128 axis=_mm_set_ps(0,0,1,0),transformed;
    ts3::math::b11::RotateVector(&axis,reinterpret_cast<__m128*>(converted),&transformed);
    auto first=AxisAngle(transformed,Div(Mul(a,0.5f),2.f));
    auto second=AxisAngle(_mm_set_ps(0,0,0,1),Div(Mul(b,0.5f),2.f));
    auto q=_mm_load_ps(reinterpret_cast<float*>(&local->rotation));
    _mm_store_ps(reinterpret_cast<float*>(&local->rotation),Product(first,Product(second,q)));
    int left=Find(c,"b__LeftEye__"),right=Find(c,"b__RightEye__");
    if(left<0||right<0) return static_cast<uintptr_t>(right);
    Update(p,neck,(left>=right?left:right)-neck+1);
    return Eyes<0x00555360>(c,p,left,right);
}
void Copy(Buffer* self,Pose* pose) {
    if(pose) {
        auto dest=b20::Local(pose);
        std::memcpy(dest,self->srt,48*size_t(pose->count));
        CallGameMethod<void,0x00575120>(pose,self->vertexWeights);
    }
}
void EvaluateBuffer(Evaluator* self,Context*,Pose* pose) {
    CallGameMethod<void,0x00574c40>(pose);
    Copy(self->buffer,pose);
}
Evaluator* Construct(Evaluator* self,void* host) {
    b01::Construct(self,host);
    self->vtable=&Global<void*>(0x00fc14f4);self->buffer=nullptr;return self;
}
Evaluator* Destroy(Evaluator* self,uint32_t flags) {
    self->vtable=&Global<void*>(0x00fc14f4);
    auto buffer=self->buffer;
    if(buffer && --buffer->refs==0) { buffer->refs=1;CallGameVirtual<void,0>(buffer,uint32_t(1)); }
    b01::Reset(self);
    if(flags&1) CallGameFunction<void,0x004e3f70>(self);
    return self;
}
#if defined(_M_IX86)
namespace hooks {
uintptr_t __fastcall EvaluateChain(b01::Node* p,void*,Context* c,Pose* v){return m16::EvaluateChain(p,c,v);}
uintptr_t __fastcall Evaluate(b01::Node* p,void*,Context* c,Pose* v){return m16::Evaluate(p,c,v);}
void __fastcall Copy(Buffer* p,void*,Pose* v){m16::Copy(p,v);}
void __fastcall EvaluateBuffer(Evaluator* p,void*,Context* c,Pose* v){m16::EvaluateBuffer(p,c,v);}
Evaluator* __fastcall Construct(Evaluator* p,void*,void* h){return m16::Construct(p,h);}
Evaluator* __fastcall Destroy(Evaluator* p,void*,uint32_t f){return m16::Destroy(p,f);}
}
#endif
}
