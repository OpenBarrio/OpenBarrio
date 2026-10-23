#include "ts3/audio/m06_B18.h"
#include "ts3/layout.h"
#include <cstring>
#include <cmath>
namespace ts3::audio::b18impl {
static_assert(sizeof(void*)!=4 || offsetof(Primitive,hash)==0x20);
static_assert(sizeof(void*)!=4 || sizeof(Curve)==0x360);
static_assert(sizeof(void*)!=4 || offsetof(Listener,active)==0xa60);
static void* Table(uint32_t address) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(address);
#else
    // Native identity tokens, never truncated game pointers.
    static uint32_t identities[3];
    return &identities[address==0x10297f8?0:address==0x1029890?1:2];
#endif
}
static Primitive* Construct(Primitive* p,uint32_t table) {
    p->primary=Table(table); p->secondary=Table(0x1029ee0); p->interfaceTable=Table(0x1029888);
    p->state=0; p->enabled=1; p->value=0; p->resource=nullptr; p->auxiliary=nullptr; p->hash=0;
    return p;
}
Primitive* ConstructWeather(Primitive* p) { return Construct(p,0x10297f8); }
Primitive* ConstructLot(Primitive* p) { return Construct(p,0x1029890); }
bool LoadListener(Listener* p,void* source) {
    void* view=nullptr;
    bool found=CallGameVirtual<bool,7>(source,uint32_t(0x5ddf6d75),uint32_t(0x7fb30f8a),&view,uint32_t(0x1407ec));
    if (!found) {
        if (view) CallGameVirtual<void,1>(view);
        return false;
    }
    CallGameMethod<void,0x008AAEF0>(&p->curves[0],view,uint32_t(0xf8deb1c3));
    CallGameMethod<void,0x008AAEF0>(&p->curves[1],view,uint32_t(0x0a1495cb));
    p->active=CallGameVirtual<uint8_t,6>(view,uint32_t(0x16074fa6));
    if (p->active) CallGameMethod<void,0x008AAEF0>(&p->curves[2],view,uint32_t(0x16074fa6));
    if (view) CallGameVirtual<void,1>(view);
    return true;
}
void DestroyListener(Listener* p) {
    for(int i=2;i>=0;--i) {
        auto& c=p->curves[i];
        if(c.begin && c.begin!=c.inlineBegin) CallGameFunction<void,0x004E3F70>(c.begin);
    }
}
Listener* ConstructListener(Listener* p) {
    std::memset(p->vectors,0,sizeof(p->vectors));
    for(auto& c:p->curves) CallGameMethod<void,0x008AAF70>(&c);
    p->active=0; p->flag=0; p->word0=0; p->word1=0;
    p->orientation[0]=0; p->orientation[1]=0; p->orientation[2]=0; p->orientation[3]=1;
    std::memset(p->velocity,0,sizeof(p->velocity));
    return p;
}
void SetValue(Primitive* p,float value) {
    if (value!=p->value) {
        CallGameVirtual<void,10>(p,uint32_t(1));
        p->value=value;
    }
}
int32_t ReleaseSecondary(void* p) {
    return CallGameMethod<int32_t,0x008BC100>(static_cast<uint8_t*>(p)-sizeof(void*));
}
int32_t AddRefSecondary(void* p) {
    return CallGameMethod<int32_t,0x008A6850>(static_cast<uint8_t*>(p)-sizeof(void*));
}
void* DeleteSecondary(void* p,uint32_t flags) {
    return CallGameMethod<void*,0x0086E520>(static_cast<uint8_t*>(p)-sizeof(void*),flags);
}
uint32_t WeatherName(Primitive* p) {
    if(p->hash==0) {
#if defined(_M_IX86)
        p->hash=CallGameFunction<uint32_t,0x004FA590>(reinterpret_cast<const void*>(0x1029838),uint32_t(0x811c9dc5),uint32_t(1));
#else
        NotRewritten("weather name hash");
#endif
    }
    return p->hash;
}
uint32_t LotName(Primitive* p) {
    if(p->hash==0) {
#if defined(_M_IX86)
        p->hash=CallGameFunction<uint32_t,0x004FA590>(reinterpret_cast<const void*>(0x10298d0),uint32_t(0x811c9dc5),uint32_t(1));
#else
        NotRewritten("lot name hash");
#endif
    }
    return p->hash;
}
uintptr_t UpdateWeather(Primitive* p,uint32_t,uint32_t) {
#if defined(_M_IX86)
    void* world=*reinterpret_cast<void**>(0x11ecbc4);
    uint32_t mode=world?CallGameMethod<uint32_t,0x00C5FED0>(world):0;
    float value=0;
    if(mode==1) value=*reinterpret_cast<const float*>(0xf98a58);
    if(mode==2) value=*reinterpret_cast<const float*>(0xfb2f6c);
    return CallGameVirtual<uintptr_t,14>(p,value);
#else
    NotRewritten("weather singleton");
#endif
}
uintptr_t UpdateLot(Primitive* p,uint32_t,uint32_t) {
#if defined(_M_IX86)
    auto world=*reinterpret_cast<uint8_t**>(0x11ecbc4);
    if(!world) return 0;
    void* lot=*reinterpret_cast<void**>(world+0x64);
    float value=lot && !CallGameMethod<uint32_t,0x00C10CF0>(lot)?1.0f:0.0f;
    return CallGameVirtual<uintptr_t,14>(p,value);
#else
    NotRewritten("lot singleton");
#endif
}
uintptr_t UpdateManager(Primitive* p,uint32_t,uint32_t) {
#if defined(_M_IX86)
    auto adjusted=*reinterpret_cast<uint8_t**>(0x11d8c40);
    // EA dereferences 0x27d5c when the singleton is null (not a zero default).
    const float* scalar=adjusted?reinterpret_cast<float*>(adjusted+0x18c):reinterpret_cast<float*>(0x27d5c);
    return CallGameVirtual<uintptr_t,14>(p,*scalar);
#else
    NotRewritten("manager singleton");
#endif
}
}
