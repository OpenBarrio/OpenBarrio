#include "ts3/render/m07_M13.h"
namespace ts3::render::m07::m13 {
#if defined(_M_IX86)
namespace {
template<class T> T& At(void* p, size_t n) { return *reinterpret_cast<T*>(static_cast<uint8_t*>(p)+n); }
void* Allocate(uint32_t size, uint32_t tag) {
    return CallGameFunction<void*,0x004E3F90>(size,reinterpret_cast<const char*>(tag),0u,0u,0u,0u);
}
void StoreGlobal(uint32_t address,void* value) { *reinterpret_cast<void**>(address)=value; }
struct Keys { uint32_t type,group; };
struct Vector { void** table; uint8_t* begin; uint8_t* end; uint8_t* capacity; const char* tag; };
}
#endif
uint8_t __fastcall Initialize(Service* self,void*) {
#if defined(_M_IX86)
    void* config=*reinterpret_cast<void**>(0x011D753C);
    if(!CallGameMethod<void*,0x0079B840>(config,reinterpret_cast<const char*>(0x00FFC530),0u,0u)) return 1;
    CallGameFunction<void,0x00915710>();
    void* manager=Allocate(0xe0,0x00FFCA30);
    if(manager) manager=CallGameMethod<void*,0x00720A60>(manager);
    if(manager) CallGameVirtual<void,5>(manager);
    void* resources=Allocate(0x3990,0x00FFCA18);
    if(resources) resources=CallGameMethod<void*,0x00720620>(resources);
    void* previous=self->resources;
    if(resources!=previous) {
        if(resources) CallGameVirtual<void,0>(resources);
        self->resources=resources;
        if(previous) CallGameVirtual<void,1>(previous);
    }
    auto fail=[&]() -> uint8_t {
        CallGameVirtual<void,6>(self);
        if(manager) CallGameVirtual<void,6>(manager);
        return 0;
    };
    if(!self->resources || !CallGameVirtual<uint8_t,32>(self->resources)) return fail();
    CallGameMethod<void,0x0071BE00>(self->resources,manager);
    void* decals=Allocate(0xb0,0x00FFC9FC);
    if(decals) decals=CallGameMethod<void*,0x00927110>(decals,manager);
    CallGameMethod<void,0x00AB2670>(&self->decals,decals);
    StoreGlobal(0x011D2E48,self->decals);
    void* swarm=CallGameFunction<void*,0x0091C420>();
    CallGameMethod<void,0x00AB2670>(&self->swarm,swarm);
    if(!self->swarm) return fail();
    StoreGlobal(0x011D2E50,self->swarm);
    CallGameVirtual<void,4>(self->swarm,manager ? static_cast<uint8_t*>(manager)+4 : nullptr);
    CallGameVirtual<void,6>(self->swarm,self->resources);
    CallGameVirtual<void,8>(self->swarm,self->decals);
    if(!CallGameVirtual<uint8_t,0>(self->swarm)) return fail();
    CallGameVirtual<void,16>(self->swarm,1.0f,5u);
    void* events=CallGameFunction<void*,0x0044C470>();
    if(events) {
        const uint32_t ids[]={0xb390246d,0xa6d1f26c,0x735dac73,0xb35daf22,0xcbc7ff16,
            0x0d9046e6,0xe73a6138,0x9ec35ddd,0x5488d5d3,0x038e3c6c,0x038e3c0e,
            0x9b8e0967,0x9422103a,0x4d1010b9,0x0f9f4229};
        for(auto id:ids) CallGameVirtual<void,9>(events,&self->listener,id,1u,0u);
    }
    void* catalog=CallGameFunction<void*,0x004AFD20>();
    void* filter=CallGameFunction<void*,0x004E41F0>(8u,reinterpret_cast<const char*>(0x00FFC9F4),0u,0u,0u,0u);
    if(filter) filter=CallGameMethod<void*,0x009244B0>(filter);
    void* retained=nullptr;
    CallGameMethod<void,0x005C9100>(&retained,filter);
    CallGameVirtual<void,10>(catalog,1u,retained,0u);
    Vector keys{reinterpret_cast<void**>(0x00FD8248),nullptr,nullptr,nullptr,reinterpret_cast<const char*>(0x00FFC9D4)};
    Keys query{0x00fd8248,0xea5118b0};
    if(CallGameVirtual<uint32_t,8>(catalog,&keys.begin,&query,0u)>0) {
        CallGameFunction<void,0x004AFCD0>(&keys.begin);
        for(auto* key=keys.begin;key!=keys.end;key+=16) {
            void* resource=nullptr;
            void* out=CallGameMethod<void*,0x006FDF20>(&resource);
            if(CallGameVirtual<uint8_t,6>(catalog,key,out,0u,0u,0u,0u))
                CallGameVirtual<void,26>(self->swarm,resource ? static_cast<uint8_t*>(resource)+0x18 : nullptr);
            if(resource) CallGameVirtual<void,1>(resource);
        }
    }
    if(keys.begin) CallGameFunction<void,0x004E3F70>(keys.begin);
    if(retained) CallGameVirtual<void,2>(retained);
    void* renderer=Allocate(0x260,0x00FFC9AC);
    if(renderer) renderer=CallGameMethod<void*,0x0071C0A0>(renderer);
    CallGameMethod<void,0x009DC510>(&self->renderer,renderer);
    if(!self->renderer) return fail();
    At<void*>(self->renderer,0x210)=self->resources;
    void* destination=self->renderer;
    At<void*>(destination,0x214)=CallGameVirtual<void*,40>(self->swarm);
    CallGameMethod<void,0x004F3220>(&self->lock);
    self->initialized=1;
    StoreGlobal(0x011D2E4C,&self->callback);
    if(manager) CallGameVirtual<void,6>(manager);
    return 1;
#else
    (void)self;
    NotRewritten("m07_M13 Initialize requires original Swarm dependencies");
#endif
}
}
