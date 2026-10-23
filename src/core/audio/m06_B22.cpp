#include "ts3/audio/m06_B22.h"
#include <cstring>
namespace ts3::audio::b22 {
namespace {
void* World() {
#if defined(_M_IX86)
    return *reinterpret_cast<void**>(0x011ECBC4);
#else
    return nullptr;
#endif
}
Primitive* Init(Primitive* p,uintptr_t table) {
#if defined(_M_IX86)
    p->vtable=reinterpret_cast<void**>(table);
    p->secondary=reinterpret_cast<void**>(0x01029EE0);
    p->interfaceView=reinterpret_cast<void**>(0x01029888);
#else
    (void)table; p->vtable=nullptr;p->secondary=nullptr;p->interfaceView=nullptr;
#endif
    p->state=0;p->enabled=1;p->gain=0;p->owner=nullptr;p->resource=nullptr;p->hash=0;return p;
}
uint32_t Hash(Primitive* p,uintptr_t name) {
    if(!p->hash) p->hash=CallGameFunction<uint32_t,0x004FA590>(reinterpret_cast<const void*>(name),uint32_t{0x811C9DC5},uint32_t{1});
    return p->hash;
}
uintptr_t Gain(Primitive* p,float f) { return CallGameVirtual<uintptr_t,14>(p,f); }
uint8_t Load(Primitive* p,bool samples) {
    Gain(p,0);void* system=CallGameFunction<void*,0x008B0A70>();void* r=nullptr;
    uint8_t ok=CallGameVirtual<uint8_t,7>(system,samples?0x7ED45EB4u:0xF2699AD8u,samples?0x14439AB1u:0x70A02FEAu,&r,uint32_t{0x1407EC});
    if(!ok) { if(r)CallGameVirtual<void,1>(r);return 0; }
    CallGameVirtual<void,10>(r,uint32_t{0xCB3F671D},&p->period);
    if(samples) {
        CallGameVirtual<void,12>(r,uint32_t{0x2E65B70F},&p->sampleValue);
        CallGameVirtual<void,11>(r,uint32_t{0xECBB5B04},&p->sampleCount);
    } else CallGameVirtual<void,12>(r,uint32_t{0x03593710},&p->depth);
    uint8_t result=CallGameMethod<uint8_t,0x008A6D90>(p);
    if(r)CallGameVirtual<void,1>(r);
    return result;
}
}
Primitive* Construct(Primitive* p) { return Init(p,0x01029EF0); }
Primitive* ConstructWorld(Primitive* p) { return Init(p,0x01029F68); }
Primitive* ConstructDepth(Primitive* p) { Init(p,0x01029FE0);p->tick=0;p->period=1;p->depth=1;return p; }
uint32_t Name(Primitive* p) { return Hash(p,0x01029F30); }
uint32_t WorldName(Primitive* p) { return Hash(p,0x01029FA8); }
uint32_t DepthName(Primitive* p) { return Hash(p,0x0102A020); }
uint32_t SampleName(Primitive* p) { return Hash(p,0x0102A0A4); }
uintptr_t WorldGain(Primitive* p,uintptr_t,uintptr_t) {
    void* w=World();if(!w)return 0;
    float value;std::memcpy(&value,static_cast<uint8_t*>(w)+0x124,4);
#if defined(_M_IX86)
    const float scale=*reinterpret_cast<const float*>(0x01029F64);
#else
    const float scale=0.01f;
#endif
    return Gain(p,value*scale);
}
uint8_t LoadDepth(Primitive* p) { return Load(p,false); }
uint8_t LoadSamples(Primitive* p) { return Load(p,true); }
Primitive* DeleteSecondary(void* p,uint32_t flags) {
    // Same adjustment pattern as verified ResourceBaseDestroySecondary.
    return CallGameMethod<Primitive*,0x00870220>(reinterpret_cast<Primitive*>(static_cast<uint8_t*>(p)-offsetof(Primitive,secondary)),flags);
}
uintptr_t UpdateDepth(Primitive* p,uintptr_t,uintptr_t) {
    ++p->tick;int32_t t=static_cast<int32_t>(p->tick);int32_t q=t/p->period;
    if(t%p->period)return static_cast<uint32_t>(q);
    void* sys=CallGameFunction<void*,0x008B0A70>();
    auto* position=CallGameVirtual<float*,6>(sys,uint32_t{1});
    float x=position[0],y=position[1],z=position[2];
    void* w=World();if(!w)return reinterpret_cast<uintptr_t>(position);
    void* cell=CallGameMethod<void*,0x00C68990>(w,x,z);uint8_t wet;
    if(cell) {
        float a,b;CallGameMethod<void,0x00AB8540>(cell,x,z,&a,&b);
        uint8_t coord[8]{}; // The callee consumes low-byte grid coordinates only.
        coord[0]=static_cast<uint8_t>(static_cast<int32_t>(a));coord[1]=static_cast<uint8_t>(static_cast<int32_t>(b));
        wet=CallGameMethod<uint8_t,0x00AB1950>(cell,coord);
    } else wet=CallGameMethod<uint8_t,0x00C6CBC0>(w,x,z);
    float f=0;
    if(wet) {
        float height=static_cast<float>(CallGameMethod<long double,0x00C61670>(w)-static_cast<long double>(y));
        if(height>p->depth)f=1;
        else if(p->depth>0 && height>0)f=height/p->depth;
    }
    return Gain(p,f);
}
uintptr_t UpdateSamples(Primitive* p,uintptr_t,uintptr_t) {
    ++p->tick;int32_t t=static_cast<int32_t>(p->tick);int32_t q=t/p->period;
    if(t%p->period)return static_cast<uint32_t>(q);
    void* w=World();if(!w)return static_cast<uint32_t>(q);
    void* sys=CallGameFunction<void*,0x008B0A70>();auto* pos=CallGameVirtual<float*,6>(sys,uint32_t{2});
    alignas(16) Point point{pos[0],pos[1],pos[2],pos[0]};
    CallGameVirtual<void,16>(p,&point);
    float height=static_cast<float>(CallGameMethod<long double,0x00C61670>(w));uint32_t count=0;
    for(Point* it=p->begin;it!=p->end;++it) {
        long double ground=CallGameMethod<long double,0x00C6A250>(w,it->x,it->z,uint32_t{0},uint32_t{1},uint32_t{0},uint32_t{0},uint32_t{0},uint32_t{0},uint32_t{0});
        if(static_cast<long double>(height)>ground)++count;
    }
    return Gain(p,static_cast<float>(static_cast<long double>(count)/static_cast<long double>(p->sampleCount)));
}
}
