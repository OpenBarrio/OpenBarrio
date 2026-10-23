#include "ts3/anim/m05_M129.h"
#include <intrin.h>
#include <cstring>
namespace ts3::anim::m129_impl {
namespace {
void Add(Ref* p) { _InterlockedIncrement(reinterpret_cast<volatile long*>(&p->refs)); }
uintptr_t Release(Ref* p,bool atomic) {
    auto result=reinterpret_cast<uintptr_t>(&p->refs);
    int n=atomic?_InterlockedDecrement(reinterpret_cast<volatile long*>(&p->refs)):--p->refs;
    if(!n) {
        if(atomic) _InterlockedExchange(reinterpret_cast<volatile long*>(&p->refs),1); else p->refs=1;
        result=CallGameVirtual<uintptr_t,0>(p,1);
    }
    return result;
}
struct Scene { void** vtable; int32_t refs; void** secondary; uint32_t flags; void* fields[3]; };
}
double RootDistance(void* controller) {
    alignas(16) float translation[4],rotation[4]; float auxiliary;
    return CallGameFunction<double,0x008492B0>(controller,translation,rotation,&auxiliary);
}
void NotifyControllers(Body* self,uint32_t event) {
    auto end=&self->list;
    for(auto node=end->next;node!=end;node=node->next) {
        auto c=node->controller;
        if(c && CallGameVirtual<uint8_t,4>(c->owner,c)) {
            c=node->controller;
            if(!c->latch) CallGameMethod<void,0x00566170>(&c->interfaces[1],event,node->payload);
        }
    }
}
double QueryMotion(void* source,uint32_t style,void* provider,float amount,int mode,float* distance) {
    if(mode==1) {
        uint32_t bits; std::memcpy(&bits,&amount,4); bits &= 0x7fffffff; std::memcpy(&amount,&bits,4);
        *distance=0.f;
        return CallGameFunction<double,0x008487A0>(amount,style);
    }
    Request request; request.source=source;
    for(auto& v:request.values) v=0.f;
    request.flags[0]=request.flags[1]=0; request.resource=nullptr;
    if(mode==0) request.values[0]=amount;
    else if(mode==2) request.values[1]=amount;
    else if(mode==3) request.values[2]=amount;
    Result result; result.controller=nullptr; result.flag=0;
    bool ok=CallGameVirtual<uint8_t,3>(provider,&request,&result)!=0;
    float value=0.f;
    if(ok) {
        alignas(16) float translation[4],rotation[4]; float auxiliary;
        *distance=static_cast<float>(CallGameFunction<double,0x008492B0>(result.controller,translation,rotation,&auxiliary));
        value=static_cast<float>(CallGameVirtual<double,8>(result.controller));
    } else *distance=0.f;
    if(result.controller) Release(result.controller,false);
    if(request.resource) Release(request.resource,true);
    return value;
}
uintptr_t InstallNamespace(Body* self,Destination* destination) {
    auto memory=CallGameFunction<void*,0x004E3F90>(size_t(sizeof(void*)==4?24:48),"Locomotion/AnimatedSimBody/NamespaceMap",0,0,0,0);
    Ref* map=memory?CallGameMethod<Ref*,0x00582010>(memory):nullptr;
    if(map) Add(map);
    auto scene=CallGameFunction<Scene*,0x004E3F90>(size_t(sizeof(void*)==4?32:sizeof(Scene)),"Locomotion/AnimatedSimBody/SceneEntity",0,0,0,0);
    if(scene) {
#if defined(_M_IX86)
        scene->vtable=reinterpret_cast<void**>(0x00FCB7EC);
        scene->secondary=reinterpret_cast<void**>(0x00FCB7DC);
#else
        AbortNotRewritten("SceneEntity vtables");
#endif
        _InterlockedExchange(reinterpret_cast<volatile long*>(&scene->refs),0);
        scene->flags=0x40000001; scene->fields[0]=scene->fields[1]=scene->fields[2]=nullptr;
        CallGameVirtual<void,1>(scene);
    }
    uint64_t key=CallGameVirtual<uint64_t,4>(self->prefix[5]);
    CallGameMethod<void,0x00581F40>(scene,uint32_t(key),uint32_t(key>>32));
    CallGameMethod<void,0x00582090>(map,uint32_t(0x050c5d67),scene);
    uintptr_t result=reinterpret_cast<uintptr_t>(destination);
    auto old=destination->map;
    if(map!=old) {
        if(map) Add(map);
        destination->map=map;
        if(old) result=Release(old,true);
    }
    if(scene) result=CallGameVirtual<uintptr_t,2>(scene);
    if(map) result=Release(map,true);
    return result;
}
}
