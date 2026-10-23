#include "ts3/anim/m05_M144.h"
#include <initializer_list>
#include <cmath>
#include <algorithm>
#include <xmmintrin.h>
namespace ts3::anim {
M144Controller* M144Controller::Destroy(uint32_t flags)
{
#if defined(_M_IX86)
    mainTable = reinterpret_cast<void*>(0x010273D0);
    secondaryTable = reinterpret_cast<void*>(0x010273CC);
    interfaceTable = reinterpret_cast<void*>(0x010273BC);
#endif
    CallGameMethod<void, 0x00856F10>(this);
    if (child) {
        if (--child->refs == 0) {
            child->refs = 1;
            CallGameVirtual<void, 0>(child, 1u);
        }
    }
    CallGameMethod<void, 0x00859890>(this);
    if (flags & 1u) CallGameFunction<void, 0x004E3F70>(static_cast<void*>(this));
    return this;
}
M144Controller* M144Controller::Construct(uint32_t a, uint32_t b)
{
    CallGameMethod<void, 0x0085A100>(this, a, b);
#if defined(_M_IX86)
    mainTable = reinterpret_cast<void*>(0x010274DC);
    secondaryTable = reinterpret_cast<void*>(0x010274D8);
    interfaceTable = reinterpret_cast<void*>(0x010274C8);
#endif
    child = nullptr;
    return this;
}
void* M144DestroySecondary(void* secondary, uint32_t flags)
{
    auto* self = reinterpret_cast<unsigned char*>(secondary) - sizeof(void*);
    return CallGameMethod<void*, 0x008569F0>(self, flags);
}
static __m128 Normalize4(__m128 q)
{
    __m128 sq = _mm_mul_ps(q,q);
    __m128 sum = _mm_add_ps(_mm_shuffle_ps(sq,sq,14),sq);
    sum = _mm_add_ps(_mm_shuffle_ps(sum,sum,1),sum);
    __m128 n = _mm_shuffle_ps(sum,sum,0);
    __m128 r = _mm_rsqrt_ps(n);
    const __m128 one = _mm_set1_ps(1.0f), half = _mm_set1_ps(0.5f);
    r = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(one,_mm_mul_ps(_mm_mul_ps(r,r),n)),_mm_mul_ps(half,r)),r);
    r = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(one,_mm_mul_ps(_mm_mul_ps(r,r),n)),_mm_mul_ps(half,r)),r);
    return _mm_mul_ps(r,q);
}
void M144Controller::UpdateRotation()
{
    if (!child) return;
    alignas(16) __m128 hostRotation;
    CallGameVirtual<void, 9>(host, &hostRotation);
    __m128 sq = _mm_mul_ps(hostRotation, hostRotation);
    __m128 sum = _mm_add_ps(_mm_shuffle_ps(sq,sq,14),sq);
    sum = _mm_add_ps(_mm_shuffle_ps(sum,sum,1),sum);
    __m128 n = _mm_shuffle_ps(sum,sum,0);
    __m128 r = _mm_rcp_ps(n), two = _mm_set1_ps(2.0f);
    r = _mm_mul_ps(_mm_sub_ps(two,_mm_mul_ps(r,n)),r);
    r = _mm_mul_ps(_mm_sub_ps(two,_mm_mul_ps(r,n)),r);
    __m128 inverse = _mm_mul_ps(r,_mm_xor_ps(hostRotation,_mm_set_ps(0.0f,-0.0f,-0.0f,-0.0f)));
    __m128 q = _mm_load_ps(rotation);
    __m128 iw = _mm_shuffle_ps(inverse,inverse,255), qw = _mm_shuffle_ps(q,q,255);
    __m128 cross = _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(q,q,18),_mm_shuffle_ps(inverse,inverse,9)),
                             _mm_mul_ps(_mm_shuffle_ps(q,q,9),_mm_shuffle_ps(inverse,inverse,18)));
    __m128 xyz = _mm_add_ps(_mm_add_ps(cross,_mm_mul_ps(q,iw)),_mm_mul_ps(qw,inverse));
    __m128 prod = _mm_mul_ps(q,inverse);
    __m128 dot = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(prod,prod,2),_mm_shuffle_ps(prod,prod,1)),prod);
    __m128 w = _mm_sub_ps(_mm_mul_ps(qw,iw),_mm_shuffle_ps(dot,dot,0));
    __m128 result = _mm_shuffle_ps(xyz,_mm_shuffle_ps(xyz,w,10),132);
    result = Normalize4(Normalize4(result));
    CallGameVirtual<void, 19>(child,&result);
    CallGameMethod<void, 0x0056A8E0>(child,speed);
}
}
namespace ts3::anim {
uintptr_t M144Controller::InitializeBicycle()
{
    uintptr_t result = CallGameMethod<uintptr_t,0x008591A0>(this);
    initialized = 1;
    if (!rigHost) return result;
    void* rig = CallGameVirtual<void*,8>(rigHost);
    if (!rig) return 0;
    static const char* const names[] = {"transformBone","steeringWheel","lowerBracket_anim","L_pedal_upright","R_pedal_upright","front_wheel_position","front_wheelRotate","rear_wheel_position","rear_wheelRotate"};
    auto* skeleton = static_cast<unsigned char*>(rig)+0x28;
    for (unsigned i=0;i<9;++i) bones[i]=CallGameMethod<int32_t,0x00575E30>(skeleton,names[i]);
    wheelbase=1.0f;
    if (bones[5]!=-1 && bones[7]!=-1) {
        void* memory=CallGameFunction<void*,0x005748A0>(105264u,"Animation/RigPose",0u,0u,0u,0u);
        M144Child* pose=nullptr;
        if (memory) {
            void* retained;
            CallGameMethod<void*,0x0074B950>(&retained,rig);
            pose=CallGameMethod<M144Child*,0x00574F50>(memory,retained);
        }
        if (pose) ++pose->refs;
        CallGameMethod<void,0x005743A0>(pose);
        CallGameMethod<void,0x00574990>(pose);
        auto* srt=CallGameMethod<unsigned char*,0x005749B0>(pose);
        __m128 d=_mm_sub_ps(_mm_load_ps(reinterpret_cast<float*>(srt+48*bones[5])),_mm_load_ps(reinterpret_cast<float*>(srt+48*bones[7])));
        __m128 sq=_mm_mul_ps(d,d);
        __m128 dot=_mm_add_ps(_mm_add_ps(_mm_shuffle_ps(sq,sq,2),_mm_shuffle_ps(sq,sq,1)),sq);
        wheelbase=_mm_cvtss_f32(_mm_sqrt_ps(_mm_shuffle_ps(dot,dot,0)));
        CallGameMethod<void,0x0055CFA0>(&pose);
    }
    radius=wheelbase*0.5f;
    void* memory=CallGameFunction<void*,0x004E3F90>(92u,"Animation/RotateController",0u,0u,0u,0u);
    M144Child* controller=memory?CallGameMethod<M144Child*,0x0056ABB0>(memory,rigHost):nullptr;
    if (controller) ++controller->refs;
    for (unsigned i : {0u,1u,5u}) if (bones[i]!=-1) CallGameVirtual<void,20>(controller,bones[i]);
    if (bones[2]!=-1) CallGameMethod<void,0x0056B230>(controller,bones[2]);
    alignas(16) float axis[4]={-1.0f,0.0f,0.0f,-1.0f};
    for (unsigned i : {3u,4u}) if (bones[i]!=-1) CallGameMethod<void,0x0056B2E0>(controller,bones[i],0.0f,0.2857143f,axis);
    for (unsigned i : {6u,8u}) if (bones[i]!=-1) CallGameMethod<void,0x0056B230>(controller,bones[i]);
    CallGameMethod<void,0x0056A8E0>(controller,speed);
    CallGameMethod<void,0x0056A900>(controller,0.61594999f);
    alignas(16) float identity[4]={0,0,0,1};
    CallGameVirtual<void,19>(controller,identity);
    CallGameMethod<void,0x005665F0>(&child,&controller);
    CallGameVirtual<void,5>(rigHost,10001u,child,2u,-1.0f,-1.0f,0u,0u,0u,0u);
    result=--controller->refs;
    if (!controller->refs) {
        controller->refs=1;
        return CallGameVirtual<uintptr_t,0>(controller,1u);
    }
    return result;
}
}
namespace ts3::anim {
static __m128 CrossEA(__m128 a,__m128 b)
{
    return _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(a,a,18),_mm_shuffle_ps(b,b,9)),
                      _mm_mul_ps(_mm_shuffle_ps(a,a,9),_mm_shuffle_ps(b,b,18)));
}
static __m128 Dot3EA(__m128 a,__m128 b)
{
    __m128 p=_mm_mul_ps(a,b);
    p=_mm_add_ps(_mm_add_ps(_mm_shuffle_ps(p,p,2),_mm_shuffle_ps(p,p,1)),p);
    return _mm_shuffle_ps(p,p,0);
}
static __m128 Normalize3EA(__m128 q,unsigned steps)
{
    __m128 n=Dot3EA(q,q),r=_mm_rsqrt_ps(n),one=_mm_set1_ps(1),half=_mm_set1_ps(0.5f);
    for(unsigned i=0;i<steps;++i) r=_mm_add_ps(_mm_mul_ps(_mm_sub_ps(one,_mm_mul_ps(_mm_mul_ps(r,r),n)),_mm_mul_ps(half,r)),r);
    return _mm_mul_ps(r,q);
}
static float SinEA(float x)
{
#if defined(_M_IX86)
    float out;
    __asm {
        fld x
        fsin
        fstp out
    }
    return out;
#else
    return static_cast<float>(std::sin(static_cast<double>(x)));
#endif
}
static float CosEA(float x)
{
#if defined(_M_IX86)
    float out;
    __asm {
        fld x
        fcos
        fstp out
    }
    return out;
#else
    return static_cast<float>(std::cos(static_cast<double>(x)));
#endif
}
static __declspec(noinline) float AcosEA(float x)
{
#if defined(_M_IX86)
    float out;
    uintptr_t target=0x00F7EDDE; // _CIacos consumes ST0, not a cdecl argument.
    __asm {
        fld x
        call target
        fstp out
    }
    return out;
#else
    return static_cast<float>(std::acos(static_cast<double>(x)));
#endif
}
void M144Controller::UpdateBicycle()
{
    if (!child) return;
    alignas(16) __m128 h;
    CallGameVirtual<void,9>(host,&h);
    CallGameMethod<void,0x0056A8E0>(child,speed);
    for(unsigned i : {2u,3u,4u}) CallGameMethod<void,0x0056A920>(child,bones[i],speed*0.30000001f);
    __m128 q=_mm_load_ps(rotation), axis=_mm_set_ps(0,1,0,0),two=_mm_set1_ps(2);
    auto forward=[&](__m128 quat) {
        __m128 v=_mm_add_ps(CrossEA(axis,quat),_mm_mul_ps(_mm_shuffle_ps(quat,quat,255),axis));
        return _mm_add_ps(_mm_mul_ps(CrossEA(v,quat),two),axis);
    };
    alignas(16) float hv[4],qv[4];
    _mm_store_ps(hv,forward(h));_mm_store_ps(qv,forward(q));
    hv[1]=0;qv[1]=0;
    __m128 a=_mm_load_ps(hv),b=_mm_load_ps(qv);
    if (!(_mm_cvtss_f32(Dot3EA(a,a))>0)) return;
    if (!(_mm_cvtss_f32(Dot3EA(b,b))>0)) return;
    a=Normalize3EA(Normalize3EA(a,2),1);
    b=Normalize3EA(Normalize3EA(b,2),1);
    __m128 dot=_mm_min_ps(_mm_max_ps(Dot3EA(b,a),_mm_set1_ps(-1)),_mm_set1_ps(1));
    float angle=AcosEA(_mm_cvtss_f32(dot));
    __m128 yAxis=_mm_set_ps(0,0,1,0);
    float sign=_mm_cvtss_f32(Dot3EA(CrossEA(b,a),yAxis));
    if (!(sign>=0)) angle=6.2831855f-angle;
    if (angle>3.1415927f) angle=angle-6.2831855f;
    float half=angle/2.0f,s=SinEA(half),c=CosEA(half);
    alignas(16) float wheel[4]={0.0f*s,1.0f*s,0.0f*s,c};
    CallGameVirtual<void,18>(child,bones[6],wheel);
    half=(-0.0f-angle)/2.0f;s=SinEA(half);c=CosEA(half);
    alignas(16) float steering[4]={0.0f*s,0.0f*s,1.0f*s,c};
    CallGameVirtual<void,18>(child,bones[1],steering);
}
}
