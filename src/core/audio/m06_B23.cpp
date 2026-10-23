#include "ts3/audio/m06_B23.h"
#include "ts3/resource/index_modifiable.h"
#include <cmath>
#include <xmmintrin.h>
namespace ts3::audio {
uintptr_t WaterPoints::Allocate(uint32_t count, void* first, void* last) {
    std::byte* storage = nullptr;
    if (count) storage = CallGameFunction<std::byte*, 0x004E3FB0>(count << 4, 16, 0, allocator, 0, 0,
        reinterpret_cast<const char*>(uintptr_t(0x00F98980)), 251);
    CallGameFunction<void, 0x00A725E0>(&first, first, last, storage, count);
    return reinterpret_cast<uintptr_t>(storage);
}
uintptr_t WaterPoints::Reserve(uint32_t count) {
    const auto span = static_cast<intptr_t>(reinterpret_cast<uintptr_t>(capacityEnd) - reinterpret_cast<uintptr_t>(begin));
    if (count <= static_cast<uintptr_t>(span >> 4)) return reinterpret_cast<uintptr_t>(begin);
    auto* replacement = reinterpret_cast<std::byte*>(Allocate(count, begin, end));
    if (begin) CallGameFunction<void, 0x004E3F70>(begin);
    const auto used = static_cast<intptr_t>(reinterpret_cast<uintptr_t>(end) - reinterpret_cast<uintptr_t>(begin));
    auto* newEnd = reinterpret_cast<std::byte*>(reinterpret_cast<uintptr_t>(replacement) + static_cast<uintptr_t>((used >> 4) * 16));
    begin = replacement;
    end = newEnd;
    capacityEnd = reinterpret_cast<std::byte*>(reinterpret_cast<uintptr_t>(replacement) + (count << 4));
    return reinterpret_cast<uintptr_t>(newEnd);
}
uintptr_t WaterPrimitive::Construct() {
    state=0; active=1; value=0; resource=nullptr; opaque=nullptr;
    vtable=reinterpret_cast<void*>(uintptr_t(0x0102A060));
    secondary=reinterpret_cast<void*>(uintptr_t(0x0102A0D0));
    interfaceTable=reinterpret_cast<void*>(uintptr_t(0x0102A05C));
    hash=0; tick=0; period=1; count=10;
#if defined(_M_IX86)
    radius=*reinterpret_cast<const float*>(uintptr_t(0x00F98A58));
#else
    radius=2.0f;
#endif
    points={nullptr,nullptr,nullptr,reinterpret_cast<void*>(uintptr_t(0x0102A0AC))};
    points.Reserve(count);
    return reinterpret_cast<uintptr_t>(this);
}
uintptr_t WaterPrimitive::ConstructPond() {
    Construct();
    vtable=reinterpret_cast<void*>(uintptr_t(0x0102A0E0));
    secondary=reinterpret_cast<void*>(uintptr_t(0x0102A0D0));
    interfaceTable=reinterpret_cast<void*>(uintptr_t(0x0102A05C));
    pondHash=0;
    return reinterpret_cast<uintptr_t>(this);
}
uint32_t WaterPrimitive::PondName() {
    if (!pondHash) pondHash=CallGameFunction<uint32_t, 0x004FA590>(reinterpret_cast<const void*>(uintptr_t(0x01016A2C)), 0x811C9DC5u, 1);
    return pondHash;
}
uintptr_t WaterPrimitive::Destroy(uint32_t flags) {
    vtable=reinterpret_cast<void*>(uintptr_t(0x0102A060));
    secondary=reinterpret_cast<void*>(uintptr_t(0x0102A0D0));
    interfaceTable=reinterpret_cast<void*>(uintptr_t(0x0102A05C));
    if(points.begin) CallGameFunction<void, 0x004E3F70>(points.begin);
    if(resource) CallGameVirtual<void,1>(resource);
    interfaceTable=reinterpret_cast<void*>(uintptr_t(0x0107396C));
    secondary=reinterpret_cast<void*>(uintptr_t(0x00FB2CBC));
    vtable=reinterpret_cast<void*>(uintptr_t(0x00FA1B78));
    if(flags&1) CallGameFunction<void, 0x004E3F70>(this);
    return reinterpret_cast<uintptr_t>(this);
}
uintptr_t DestroyWaterInterface(void* self, uint32_t flags) {
    auto* primary=reinterpret_cast<WaterPrimitive*>(static_cast<std::byte*>(self)-offsetof(WaterPrimitive,interfaceTable));
    return primary->Destroy(flags);
}
uintptr_t ScenePrimitive::Construct() {
    state=0; active=1; value=0; resource=nullptr; opaque=nullptr;
    vtable=reinterpret_cast<void*>(uintptr_t(0x0102A160));
    secondary=reinterpret_cast<void*>(uintptr_t(0x01029EE0));
    interfaceTable=reinterpret_cast<void*>(uintptr_t(0x01029888));
    hash=0;
    return reinterpret_cast<uintptr_t>(this);
}
bool WaterPrimitive::LoadPond() {
    CallGameVirtual<void,14>(this, 0.0f);
    auto* system=CallGameFunction<void*,0x008B0A70>();
    void* data=nullptr;
    const bool found=CallGameVirtual<bool,7>(system,0xDF06A88Au,0x3A61FF3Fu,&data,0x1407ECu);
    if(!found) { if(data) CallGameVirtual<void,1>(data); return false; }
    CallGameVirtual<void,10>(data,0xCB3F671Du,&period);
    CallGameVirtual<void,12>(data,0x2E65B70Fu,&radius);
    CallGameVirtual<void,11>(data,0xECBB5B04u,&count);
    bool result=CallGameMethod<bool,0x008A6D90>(this);
    if(data) CallGameVirtual<void,1>(data);
    return result;
}
uintptr_t WaterPrimitive::UpdatePond(uint32_t a, uint32_t b) {
    (void)a; (void)b;
    ++tick;
    const int32_t quotient=static_cast<int32_t>(tick)/period;
    if(static_cast<int32_t>(tick)%period) return static_cast<uint32_t>(quotient);
    void* world=nullptr;
#if defined(_M_IX86)
    world=*reinterpret_cast<void**>(uintptr_t(0x011ECBC4));
#endif
    if(!world) return static_cast<uint32_t>(quotient);
    auto* system=CallGameFunction<void*,0x008B0A70>();
    auto* pos=CallGameVirtual<const float*,6>(system,2);
    alignas(16) float center[4]={pos[0],pos[1],pos[2],pos[0]};
    CallGameVirtual<void,16>(this,center);
    uint32_t hits=0;
    for(auto* p=points.begin;p!=points.end;p+=16) {
        auto* v=reinterpret_cast<const float*>(p);
        auto* lot=CallGameMethod<void*,0x00C68990>(world,v[0],v[2]);
        bool hit;
        if(lot) {
            float x,z;
            CallGameMethod<void,0x00AB8540>(lot,v[0],v[2],&x,&z);
            uint8_t cell[3]={static_cast<uint8_t>(_mm_cvtt_ss2si(_mm_set_ss(x))),static_cast<uint8_t>(_mm_cvtt_ss2si(_mm_set_ss(z))),0};
            hit=CallGameMethod<bool,0x00AB1900>(lot,cell);
        } else hit=CallGameMethod<uintptr_t,0x00C60E70>(world,v[0],v[2],1)!=0;
        if(hit) ++hits;
    }
    float fraction=static_cast<float>(static_cast<double>(hits)/static_cast<double>(count));
    return CallGameVirtual<uintptr_t,14>(this,fraction);
}
namespace {
float Sin87(float angle) {
#if defined(_M_IX86)
    float result;
    __asm {
        fld angle
        fsin
        fstp result
    }
    return result;
#else
    return std::sin(angle);
#endif
}
float Cos87(float angle) {
#if defined(_M_IX86)
    float result;
    __asm {
        fld angle
        fcos
        fstp result
    }
    return result;
#else
    return std::cos(angle);
#endif
}
}
uintptr_t WaterPrimitive::Generate(void* centerArg) {
    const auto* center=static_cast<const float*>(centerArg);
    points.end=points.begin;
    const uint32_t half=count>>1;
    float step=static_cast<float>(6.283185482025146484375/static_cast<double>(half));
    std::byte* result=points.begin;
    for(uint32_t i=0;i<half;++i) {
        for(unsigned ring=0;ring<2;++ring) {
            if(reinterpret_cast<uintptr_t>(points.end)<reinterpret_cast<uintptr_t>(points.capacityEnd)) points.end+=16;
            else { alignas(16) std::byte unused[16]; CallGameMethod<void,0x00692C00>(&points,points.end,unused); }
            result=points.end-16;
            float angle=ring ? static_cast<float>(i)*step : static_cast<float>((static_cast<double>(i)+0.5)*step);
            float s=Sin87(angle), c=Cos87(angle);
            __m128 v=_mm_set_ps(s,c,0.0f,s);
            v=_mm_mul_ps(_mm_set1_ps(radius),v);
            v=_mm_add_ps(v,_mm_load_ps(center));
            _mm_store_ps(reinterpret_cast<float*>(result),v);
        }
    }
    return reinterpret_cast<uintptr_t>(result);
}
uintptr_t ScenePrimitive::Update(uint32_t a, uint32_t b) {
    (void)a; (void)b;
    auto* camera=CallGameFunction<ts3::resource::IndexModifiable*,0x0096B390>();
    // The verified shared getter is simply the object's +4 word.
    uintptr_t handle=camera->Count();
    if(!handle) return 0;
    auto* subject=CallGameMethod<void*,0x0096B6D0>(reinterpret_cast<void*>(handle));
    if(!subject) return CallGameVirtual<uintptr_t,14>(this,0.5f);
    auto* first=CallGameVirtual<void*,3>(subject,0x110FDD89u);
    auto* second=CallGameVirtual<void*,3>(subject,0xC49DCD29u);
    auto* third=CallGameVirtual<void*,3>(subject,0x0B287194u);
    float low,high;
    if(first) CallGameMethod<void,0x009716F0>(first,&low,&high);
    else if(third) CallGameMethod<void,0x0096EA00>(third,&low,&high);
    else if(second) CallGameMethod<void,0x0097C260>(second,&low,&high);
    else return CallGameVirtual<uintptr_t,14>(this,0.5f);
    alignas(16) float from[4],to[4];
    auto* start=CallGameVirtual<const float*,13>(subject,from);
    auto* finish=CallGameVirtual<const float*,12>(subject,to);
    __m128 v=_mm_sub_ps(_mm_load_ps(finish),_mm_load_ps(start));
    v=_mm_mul_ps(v,v);
    __m128 sum=_mm_add_ps(_mm_shuffle_ps(v,v,2),_mm_shuffle_ps(v,v,1));
    sum=_mm_add_ps(sum,v);
    float distance=_mm_cvtss_f32(_mm_sqrt_ps(_mm_shuffle_ps(sum,sum,0)));
    float output=1.0f-distance/(high-low);
    return CallGameVirtual<uintptr_t,14>(this,output);
}
uint32_t ScenePrimitive::Name() {
    if (!hash) hash=CallGameFunction<uint32_t, 0x004FA590>(reinterpret_cast<const void*>(uintptr_t(0x0102A1A0)), 0x811C9DC5u, 1);
    return hash;
}
}
