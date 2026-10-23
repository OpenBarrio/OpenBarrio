#include "ts3/anim/m05_M145.h"
#include <xmmintrin.h>
#include <initializer_list>
namespace ts3::anim {
using namespace ts3;
namespace {
uintptr_t Release(M145Child* p) {
    auto r=--p->refs;
    if (!r) { p->refs=1; return CallGameVirtual<uintptr_t,0>(p,1u); }
    return r;
}
}
uintptr_t M145::Construct(uint32_t a,uint32_t b) {
    CallGameMethod<void,0x0085A100>(this,a,b);
    vtable=reinterpret_cast<void*>(0x010275D4); secondary=reinterpret_cast<void*>(0x010275D0);
    interface18=reinterpret_cast<void*>(0x010275C0); child=nullptr; return reinterpret_cast<uintptr_t>(this);
}
uintptr_t M145::Destroy(uint32_t flags) {
    // Same body as verified RouteManagerObject::Destruct; only the installed tables differ.
    vtable=reinterpret_cast<void*>(0x010274DC); secondary=reinterpret_cast<void*>(0x010274D8);
    interface18=reinterpret_cast<void*>(0x010274C8);
    CallGameMethod<void,0x00856F10>(this);
    if(child) Release(child);
    CallGameMethod<void,0x00859890>(this);
    if(flags&1) CallGameFunction<void,0x004E3F70>(static_cast<void*>(this));
    return reinterpret_cast<uintptr_t>(this);
}
uintptr_t M145::Update() {
    if(child) return CallGameMethod<uintptr_t,0x0056A8E0>(child,speed*0.3f);
    return reinterpret_cast<uintptr_t>(this);
}
uintptr_t M145::Stop() {
    if(child) CallGameMethod<void,0x0056A8E0>(child,0.0f);
    if(host) CallGameVirtual<void,6>(host);
    return CallGameMethod<uintptr_t,0x008596B0>(this);
}
void* M145DestroySecondary(void* self,uint32_t flags) {
    return CallGameMethod<void*,0x00856F70>(static_cast<unsigned char*>(self)-sizeof(void*),flags);
}
uintptr_t M145::Initialize() {
    uintptr_t result=CallGameMethod<uintptr_t,0x008591A0>(this);
    if(!host) return result;
    void* rig=CallGameVirtual<void*,8>(host);
    if(!rig) return 0;
    const char* names[]={"front_L_wheelStructure","front_R_wheelStructure","front_L_wheelRotate","front_R_wheelRotate","rear_L_wheelStructure","rear_R_wheelStructure","rear_L_wheelRotate","rear_R_wheelRotate"};
    for(int i=0;i<8;++i) joints[i]=CallGameMethod<int32_t,0x00575E30>(static_cast<unsigned char*>(rig)+0x28,names[i]);
    void* mem=CallGameFunction<void*,0x005748A0>(105264u,"Animation/RigPose",0u,0u,0u,0u);
    M145Child* pose=nullptr;
    if(mem) {
        CallGameVirtual<void,0>(rig);
        pose=CallGameMethod<M145Child*,0x00574F50>(mem,rig);
        if(pose) ++pose->refs;
    }
    CallGameMethod<void,0x005743A0>(pose);
    CallGameMethod<void,0x00574990>(pose);
    auto* data=CallGameMethod<unsigned char*,0x005749B0>(pose);
    __m128 front=_mm_load_ps(reinterpret_cast<float*>(data+48*joints[0]));
    auto distance=[&](int j) {
        __m128 d=_mm_sub_ps(front,_mm_load_ps(reinterpret_cast<float*>(data+48*j)));
        __m128 q=_mm_mul_ps(d,d);
        __m128 sum=_mm_add_ps(_mm_add_ps(_mm_shuffle_ps(q,q,2),_mm_shuffle_ps(q,q,1)),q);
        return _mm_cvtss_f32(_mm_sqrt_ps(_mm_shuffle_ps(sum,sum,0)));
    };
    length=distance(joints[4]); halfLength=length*0.5f;
    width=distance(joints[1]); halfWidth=width*0.5f;
    mem=CallGameFunction<void*,0x004E3F90>(92u,"Animation/RotateController",0u,0u,0u,0u);
    M145Child* rotation=mem?CallGameMethod<M145Child*,0x0056ABB0>(mem,host):nullptr;
    if(rotation) ++rotation->refs;
    CallGameVirtual<void,20>(rotation,joints[0]); CallGameVirtual<void,20>(rotation,joints[1]);
    for(int i: {2,3,6,7}) CallGameMethod<void,0x0056B230>(rotation,joints[i]);
    CallGameMethod<void,0x0056A8E0>(rotation,speed);
    CallGameMethod<void,0x0056A900>(rotation,0.2857143f);
    alignas(16) float identity[4]={0,0,0,1};
    CallGameVirtual<void,19>(rotation,identity);
    CallGameMethod<void,0x005665F0>(&child,&rotation);
    CallGameVirtual<void,5>(host,10000u,child,2u,-1.0f,-1.0f,0u,0u,0u,0u);
    result=Release(rotation);
    if(pose) result=Release(pose);
    return result;
}
}
