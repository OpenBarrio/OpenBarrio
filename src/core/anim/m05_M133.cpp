#include "ts3/anim/m05_M133.h"
#include <cstring>
#include <intrin.h>
namespace ts3::anim::m133_impl {
namespace {
template<class T> T& At(void* p, size_t offset) { return *reinterpret_cast<T*>(static_cast<unsigned char*>(p)+offset); }
template<unsigned EA> void* Global() {
#if defined(_M_IX86)
    return *reinterpret_cast<void**>(EA);
#else
    AbortNotRewritten("m133 game global"); return nullptr;
#endif
}
template<unsigned EA> void* Address() {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(EA);
#else
    AbortNotRewritten("m133 game address"); return nullptr;
#endif
}
void Release(void* p) {
    if(p && --At<uint32_t>(p,sizeof(void*))==0) {
        At<uint32_t>(p,sizeof(void*))=1;
        CallGameVirtual<void,0>(p,1u);
    }
}
struct Key { uint32_t lane[4]; };
struct Entries { unsigned char* begin; unsigned char* end; unsigned char* capacity; uint32_t descriptor; unsigned char storage[80]; };
void Speed(void* actor,float speed,uint8_t enabled) {
    if(actor) if(auto* component=CallGameVirtual<void*,1>(actor,0xf4c95ed2u)) {
        CallGameVirtual<void,2>(component,speed);
        CallGameVirtual<void,5>(component,enabled);
    }
}
}
uintptr_t OnControllerStarted(void* notification,void** controller,uint32_t event) {
    auto* b=reinterpret_cast<Body*>(static_cast<unsigned char*>(notification)-offsetof(Body,notification));
    if(b) ++b->refs;
    float speed=1.0f;
    switch(event) {
    case 1: case 5:
        if(b->keys[2]==0x4100 || b->keys[2]==0x4200 || b->keys[2]==0x4300)
            if(b->actor) CallGameVirtual<void,10>(b->actor);
        break;
    case 2:
        if(b->routeEnabled && static_cast<uint8_t>(CallGameFunction<uint32_t,0x624150>(b->keys[2])))
            if(b->actor) CallGameVirtual<void,10>(b->actor);
        speed=b->speeds[0];
        if(!b->active && b->host && b->host!=b->master && At<int32_t>(b->host,12)>0 && CallGameVirtual<int,6>(b->master)==2) {
            alignas(16) unsigned char temporary[16];
            auto* text=Address<0x1027130>();
            CallGameMethod<void,0x40BAD0>(temporary,Address<0x1026F00>(),&text);
            CallGameMethod<void,0x849210>(b,temporary,1);
            CallGameMethod<void,0x4EF330>(temporary);
        }
        break;
    case 3:
        speed=b->speeds[1];
        if(b->stylesBegin!=b->stylesEnd && !b->active) CallGameMethod<void,0x849210>(b,&b->stylesBegin,1);
        break;
    case 4:
        speed=b->speeds[2];
        if(b->active) CallGameMethod<void,0x8485F0>(b);
        if(b->actor) {
            auto* state=CallGameVirtual<void*,1>(b->actor,0x749c6a98u);
            auto& flags=At<uint32_t>(state,44);
            // Assembly clears bit20 only, not both bits20/21 as the decompiler suggests.
            if(flags&0x80000) flags=(flags&0xffefffff)|0x200000;
        }
        break;
    case 7: speed=b->speeds[3]; break;
    case 9: case 10: {
        speed=b->speeds[event==9?4:5];
        if(!b->active) break;
        alignas(16) unsigned char adjacent[128],current[128];
        void* manager=Global<0x11D9140>();
        CallGameMethod<void,0x757690>(adjacent,reinterpret_cast<unsigned char*>(notification)+0x10);
        uint32_t candidate=b->index+(event==9?1u:0xffffffffu);
        auto count=CallGameMethod<uint32_t,0x74B990>(&b->progression);
        auto lookup=[&](uint32_t index,void* out) {
            return static_cast<uint8_t>(CallGameMethod<uint32_t,0x844980>(manager,At<void*>(b->progression,28u*index+24),b->keys[0],b->keys[1],b->keys[2],b->keys[3],out,1));
        };
        if(candidate<count && lookup(candidate,adjacent)) {
            if(At<void*>(adjacent,108)==At<void*>(adjacent,112)) CallGameMethod<void,0x8485F0>(b);
            else {
                CallGameMethod<void,0x757690>(current,reinterpret_cast<unsigned char*>(notification)+0x10);
                if(lookup(b->index,current) && !static_cast<uint8_t>(CallGameFunction<uint32_t,0x408CC0>(adjacent+108,current+108)))
                    CallGameMethod<void,0x8485F0>(b);
                CallGameMethod<void,0x754060>(current);
            }
        }
        CallGameMethod<void,0x754060>(adjacent);
        break;
    }
    default: break;
    }
    void* environment=Global<0x11ECBC4>();
    void* terrain=environment?At<void*>(environment,0x74):nullptr;
    if(static_cast<uint8_t>(CallGameMethod<uint32_t,0x523860>(Address<0x11CDDE0>(),0x8000))) {
        uint16_t mode=0;
        auto* locomotion=CallGameVirtual<void*,1>(b->host,0x14cfd2f8u);
        if(locomotion) mode=At<uint16_t>(locomotion,252);
        alignas(16) unsigned char position[16];
        CallGameVirtual<void,7>(b->host,position);
        if(!mode && terrain) {
            mode=CallGameMethod<uint16_t,0xD699A0>(terrain);
            if(mode==1 || mode==2) {
                uint8_t valid=0;
                float depth=static_cast<float>(CallGameMethod<double,0xD6E170>(terrain,position,&valid));
                if(!valid) mode=0;
                else if(depth<=0.07f) mode=1;
            }
        }
        if(mode==4) {
            alignas(16) unsigned char route[16];
            CallGameMethod<void,0x76C040>(route,position);
            if(!static_cast<uint8_t>(CallGameFunction<uint32_t,0xB046C0>(route,1))) mode=0;
        }
        if(locomotion && mode==2) CallGameVirtual<void,16>(locomotion);
        if(locomotion && mode==1) CallGameVirtual<void,17>(locomotion);
        if(mode) {
            void* source=*controller;
            if(source) { ++At<uint32_t>(source,sizeof(void*)); ++At<uint32_t>(source,sizeof(void*)); }
            void* found=nullptr;
            CallGameFunction<void,0x848A90>(&found,source,5);
            // The pre-cleanup stack operand at 0084D443 is the found smart pointer.
            if(found) {
                void* resource=At<void*>(found,0x5d0);
                Key key; std::memcpy(&key,static_cast<unsigned char*>(resource)+8,sizeof(key));
                Entries entries; entries.begin=entries.end=entries.storage; entries.capacity=entries.storage+80;
                if(static_cast<uint8_t>(CallGameMethod<uint32_t,0x842030>(Global<0x11D9140>(),key,static_cast<uint32_t>(mode),&entries)))
                    for(auto* p=entries.begin;p!=entries.end;p+=20)
                        CallGameMethod<void,0x84C3B0>(b,At<void*>(p,0),At<uint32_t>(p,16),0);
                CallGameMethod<void,0x912B60>(&entries,entries.begin,entries.end);
                Release(found);
            }
            Release(source);
        }
    }
    Speed(b->actor,speed,b->disabled==0);
    if(b->slaveCount>0 && b->slaves) {
        for(int32_t i=0;i<b->slaveCount;++i) {
            auto* actor=b->slaveHosts[i];
            if(actor) Speed(actor,speed,b->slaves[124*i+101]==0);
        }
    }
    auto* service=CallGameMethod<void*,0xC93970>(b->host);
    if(service) {
        auto* refs=&At<long>(service,sizeof(void*));
        _InterlockedExchangeAdd(refs,1);
        alignas(16) unsigned char position[16];
        CallGameVirtual<void,7>(b->host,position);
        float height=static_cast<float>(CallGameVirtual<double,70>(service,position));
        auto id=CallGameVirtual<uint64_t,4>(b->host);
        double previous=CallGameVirtual<double,41>(service,static_cast<uint32_t>(id),static_cast<uint32_t>(id>>32));
        if(height>previous) {
            id=CallGameVirtual<uint64_t,4>(b->host);
            CallGameVirtual<void,42>(service,height,static_cast<uint32_t>(id),static_cast<uint32_t>(id>>32));
        }
        if(!_InterlockedDecrement(refs)) { _InterlockedExchange(refs,1); CallGameVirtual<void,0>(service,1u); }
    }
    if(!b) return 0;
    uintptr_t result=--b->refs;
    if(!result) { b->refs=1; return CallGameVirtual<uintptr_t,0>(&b->releaseTable,1u); }
    return result;
}
}
