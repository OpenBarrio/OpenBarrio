#include "ts3/audio/m06_B20.h"
namespace ts3::audio {
namespace {
template<class T> T* Data(uintptr_t address) {
#if defined(_M_IX86)
    return reinterpret_cast<T*>(address);
#else
    (void)address; NotRewritten("m06_B20 game data dependency");
#endif
}
void Tables(b20* p,uintptr_t a,uintptr_t b,uintptr_t c) {
    p->vtable=Data<void*>(a);p->secondary=Data<void>(b);p->reference=Data<void>(c);
}
uintptr_t Free(void* p) { return CallGameFunction<uintptr_t,0x004E3F70>(p); }
float UnsignedFloat(uint32_t n) {
#if defined(_M_IX86)
    float out; const float correction=4294967296.0f;
    __asm {
        fild dword ptr [n]
        cmp dword ptr [n],0
        jge positive
        fadd dword ptr [correction]
    positive:
        fstp dword ptr [out]
    }
    return out;
#else
    return static_cast<float>(n);
#endif
}
void* System() { return CallGameFunction<void*,0x008B0A70>(); }
struct alignas(16) Position { float x,y,z,w; };
Position Listener() {
    auto p=CallGameVirtual<float*,6>(System(),2u);
    return {p[0],p[1],p[2],p[0]};
}
uintptr_t Publish(b20* p,float f) {return CallGameVirtual<uintptr_t,14>(p,f);}
}
b20* b20::InitWind() {
    Tables(this,0x01029BD8,0x01029EE0,0x01029888);
    state=0;enabled=1;value=0;link=nullptr;owner=nullptr;counter=0;
    return this;
}
uint32_t b20::NameWind() {
    if(counter==0) counter=CallGameFunction<uint32_t,0x004FA590>(Data<const char>(0x01029C18),0x811c9dc5u,1u);
    return counter;
}
uintptr_t b20::Wind(uint32_t,uint32_t) {
    auto adjusted=*Data<uint8_t*>(0x011D8C40);
    auto manager=adjusted?adjusted-0x27BD0:nullptr;
    return Publish(this,UnsignedFloat(*reinterpret_cast<uint32_t*>(manager+0x27D58)));
}
b20* b20::DeleteBase(uint32_t flags) {
    if(link) CallGameVirtual<uintptr_t,1>(link);
    Tables(this,0x00FA1B78,0x00FB2CBC,0x0107396C);
    if(flags&1) Free(this);
    return this;
}
b20* b20::DeleteReference(uint32_t flags) {
    auto p=reinterpret_cast<b20*>(reinterpret_cast<uint8_t*>(this)-offsetof(b20,reference));
    return p->DeleteBase(flags);
}
b20* b20::InitModels(uint32_t arg,void* variant) {
    Tables(this,0x01029C60,0x01029C50,0x01029C4C);
    state=0;enabled=1;value=0;link=nullptr;owner=nullptr;
    counter=0;interval=5;parameter=arg;
    radius=*Data<float>(0x0106809C);
    map.load=*Data<float>(0x0107A538);map.growth=*Data<float>(0x00F98A58);
    map.label=Data<const char>(0x01029C38);map.bucketCount=1;
    map.buckets=Data<void*>(0x0114C900);map.size=0;map.nextResize=0;
#if defined(_M_IX86)
    auto v=static_cast<uint8_t*>(variant);
    bool array=(*reinterpret_cast<uint16_t*>(v+8)&0x30)!=0;
    uint32_t count=array?*reinterpret_cast<uint32_t*>(v+0x18):uint32_t(*reinterpret_cast<uint16_t*>(v+10)!=0);
    auto args=array?*reinterpret_cast<uint8_t**>(v+16):count?v+16:nullptr;
    using Parse=double(__cdecl*)(const wchar_t*,wchar_t**);
    radius=static_cast<float>((*Data<Parse>(0x00F954DC))(*reinterpret_cast<const wchar_t**>(args),nullptr));
    interval=CallGameFunction<uint32_t,0x004FED70>(*reinterpret_cast<const wchar_t**>(args+16),static_cast<wchar_t**>(nullptr),10);
    for(uint32_t i=2;i<count;++i) {
        auto hash=CallGameFunction<uint32_t,0x004FA640>(*reinterpret_cast<const wchar_t**>(args+16*i),0x811c9dc5u,1u);
        auto key=CallGameFunction<uint32_t,0x007CFCA0>(hash);
        uint32_t result[2];
        CallGameMethod<void,0x00B7AAD0>(&map,result,&key,0u);
    }
#else
    (void)variant;NotRewritten("m06_B20 variant parsing");
#endif
    return this;
}
void b20::DestroyModels() {
    Tables(this,0x01029C60,0x01029C50,0x01029C4C);
    CallGameMethod<void,0x007F3790>(&map,map.buckets,map.bucketCount);
    map.size=0;
    if(map.bucketCount>1) Free(map.buckets);
    if(link) CallGameVirtual<uintptr_t,1>(link);
    Tables(this,0x00FA1B78,0x00FB2CBC,0x0107396C);
}
b20* b20::DeleteModels(uint32_t flags) {
    DestroyModels();if(flags&1) Free(this);return this;
}
b20* b20::DeleteSecondary(uint32_t flags) {
    return reinterpret_cast<b20*>(reinterpret_cast<uint8_t*>(this)-offsetof(b20,secondary))->DeleteModels(flags);
}
b20* b20::DeleteModelReference(uint32_t flags) {
    return reinterpret_cast<b20*>(reinterpret_cast<uint8_t*>(this)-offsetof(b20,reference))->DeleteModels(flags);
}
uintptr_t b20::Models(uint32_t,uint32_t) {
    ++counter;
    const int32_t quotient=static_cast<int32_t>(counter)/static_cast<int32_t>(interval);
    if(static_cast<int32_t>(counter)%static_cast<int32_t>(interval)) return static_cast<uint32_t>(quotient);
    auto pos=Listener();
    auto world=CallGameFunction<void*,0x00686A90>();
    if(!world) return 0;
    struct List {uint64_t* begin;uint64_t* end;uint64_t* capacity;const void* label;} list{};
    list.label=Data<void>(0x01003D7C);
    CallGameMethod<void,0x00686C20>(world,&pos,radius,12u,&list);
    auto registry=CallGameFunction<void*,0x00938B50>();
    uint32_t count=0;
    auto sentinel=map.buckets[map.bucketCount];
    for(auto p=list.begin;p!=list.end;++p) {
        auto object=CallGameMethod<void*,0x00939560>(registry,uint32_t(*p),uint32_t(*p>>32));
        if(!object) continue;
        auto model=CallGameMethod<void*,0x00935770>(object,*Data<void*>(0x011E8FC4));
        if(!model) continue;
        auto key=*CallGameMethod<uint32_t*,0x00949D30>(model);
        void* found;
        CallGameMethod<void,0x00592EC0>(&map,&found,&key);
        if(found!=sentinel) ++count;
    }
    Publish(this,UnsignedFloat(count));
    if(list.begin) return Free(list.begin);
    return reinterpret_cast<uintptr_t>(list.begin); // last mov eax,[list.begin], even when null
}
uint32_t b20::Configure() {
    Publish(this,0.0f);
    void* resource=nullptr;
    auto result=CallGameVirtual<uint32_t,7>(System(),0x1696782fu,0xae530577u,&resource,0x001407ecu);
    if(!(result&255)) {
        if(resource) result=CallGameVirtual<uint32_t,1>(resource);
        return result&0xffffff00u;
    }
    CallGameVirtual<void,10>(resource,0xcb3f671du,&parameter);
    CallGameVirtual<void,12>(resource,0x2e65b70fu,&radius);
    result=CallGameMethod<uint32_t,0x008A6D90>(this);
    auto low=result&255;
    if(resource) result=CallGameVirtual<uint32_t,1>(resource);
    return (result&0xffffff00u)|low;
}
uintptr_t b20::Density(uint32_t,uint32_t) {
    ++interval; // this family uses +24 as the tick and +28 as its divisor
    const int32_t quotient=static_cast<int32_t>(interval)/static_cast<int32_t>(parameter);
    if(static_cast<int32_t>(interval)%static_cast<int32_t>(parameter)) return static_cast<uint32_t>(quotient);
    if(!*Data<void*>(0x011ECBC4)) return static_cast<uint32_t>(quotient);
    auto pos=Listener();
    auto world=CallGameFunction<void*,0x008292D0>();
    auto region=world?CallGameMethod<uint8_t*,0x00829C00>(world):nullptr;
    auto query=region?*reinterpret_cast<void**>(region+0x14):nullptr;
    if(!query) return reinterpret_cast<uintptr_t>(region);
    auto f=CallGameMethod<float,0x0066DA70>(query,pos.x,pos.z,radius);
    return Publish(this,f);
}
}
