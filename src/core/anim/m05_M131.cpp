#include "ts3/anim/m05_M131.h"
#include <cfloat>
#if defined(_MSC_VER)
#include <intrin.h>
#endif
namespace ts3::anim::m131 {
namespace {
struct Ref { void** table; long refs; };
void Release(void* object, bool atomic) {
    if (!object) return;
    auto* ref = static_cast<Ref*>(object);
    long count;
#if defined(_MSC_VER)
    if (atomic) count = _InterlockedDecrement(&ref->refs);
    else count = --ref->refs;
#else
    count = --ref->refs;
#endif
    if (!count) {
#if defined(_MSC_VER)
        if (atomic) _InterlockedExchange(&ref->refs,1);
        else ref->refs=1;
#else
        ref->refs=1;
#endif
        CallGameVirtual<void,0>(object,1);
    }
}
}
double Distance(void* source, void* provider, void* argument, unsigned char force) {
    if (CallGameVirtual<int,1>(provider) == 1 && !force)
        return CallGameVirtual<double,2>(provider);
    // Padding is deliberately not initialized: the original writes only the two flag bytes.
    Query query;
    query.source=source;
    query.limit=2000.0f;
    query.zero[0]=query.zero[1]=query.zero[2]=0.0f;
    query.flags[0]=query.flags[1]=0;
    query.retained=nullptr;
    QueryResult result;
    result.controller=nullptr;
    result.flag=0;
    const unsigned char found=CallGameVirtual<unsigned char,3>(provider,&query,&result);
    float distance=FLT_MAX;
    if (found) {
        alignas(16) unsigned char transform[20];
        alignas(16) unsigned char vector[16];
        distance=static_cast<float>(CallGameFunction<double,0x008492B0>(result.controller,transform,vector,argument));
    }
    Release(result.controller,false);
    Release(query.retained,true);
    return distance;
}
uintptr_t Sample(Body* self, void* output) {
    // Count is truncating signed division, not a begin != end test.
    const auto bytes=reinterpret_cast<intptr_t>(self->end)-reinterpret_cast<intptr_t>(self->begin);
    if (bytes / static_cast<intptr_t>(sizeof(RouteEntry))) {
#if defined(_M_IX86)
        void* manager=*reinterpret_cast<void**>(0x011D9140);
#else
        void* manager=nullptr;
#endif
        return CallGameMethod<uintptr_t,0x00844980>(manager,self->begin->route,
            self->routeArgs[0],self->routeArgs[1],self->routeArgs[2],self->routeArgs[3],output,1);
    }
    return CallGameMethod<uintptr_t,0x007578B0>(output,self->scratch);
}
namespace {
uintptr_t DropController(void* object) {
    auto* ref=static_cast<Ref*>(object);
    const long count=--ref->refs;
    if (count) return static_cast<uintptr_t>(count);
    ref->refs=1;
    return CallGameVirtual<uintptr_t,0>(object,1);
}
void AssignController(void*& destination,void* source) {
    if (destination==source) return;
    if (source) ++static_cast<Ref*>(source)->refs;
    void* old=destination; destination=source;
    if (old) DropController(old);
}
struct ControllerPrefix { void** vt; long refs; void* interface8; void* interfaceC; };
}
uintptr_t Attach(BuildBody* self, void* controller, void** slaves, int count, int mode, int track, unsigned char conform, float time) {
    if (track==2 || !self->started) { track=2; self->started=1; }
    if (controller) ++static_cast<Ref*>(controller)->refs;
    const uint64_t wanted=uint64_t(uint32_t(count))*sizeof(void*)+sizeof(size_t);
    const size_t bytes=wanted>SIZE_MAX?SIZE_MAX:static_cast<size_t>(wanted);
    auto* allocation=CallGameFunction<size_t*,0x004E3F90>(bytes,"Animation/ControllerPtrArray",0,0,0,0);
    void** temporary=nullptr;
    if (allocation) {
        *allocation=static_cast<uint32_t>(count);
        temporary=reinterpret_cast<void**>(allocation+1);
        for (uint32_t i=0;i<static_cast<uint32_t>(count);++i) temporary[i]=nullptr;
    }
    void* route=nullptr;
    bool allow=true;
    auto slaveHost=[&](int i) { return CallGameMethod<void*,0x00839CA0>(self->host,i); };
    if (conform) {
        auto* node=CallGameVirtual<void*,1>(self->host,349164280);
        if (node) {
            route=CallGameMethod<void*,0x00955EF0>(node);
            if (self->count>0 && self->shared && slaveHost(0)) {
                if (CallGameVirtual<int,6>(slaveHost(0))==2) route=nullptr;
            }
            if ((CallGameMethod<uint32_t,0x00953660>(node,0)&0xcf00)==0x200 ||
                (CallGameMethod<uint32_t,0x00953690>(node)&0xcf00)==0xe00) allow=false;
        }
        auto* storage=CallGameFunction<void*,0x004E3F90>(72,"Animation/ConformToRouteController",0,0,0,0);
        void* wrapped=nullptr;
        if (storage) {
            const bool special=CallGameFunction<unsigned char,0x0055DA60>(self->routeArgs[2]) && mode==4;
            const bool shared=count>0 && self->shared && allow;
            auto* resource=CallGameMethod<void*,0x00C93970>(self->host);
            wrapped=CallGameMethod<void*,0x0056E4C0>(storage,self->graph,resource,route,controller,
                self->routeArgs[0],self->routeArgs[2],time,self->conformFlag,shared,special);
        }
        AssignController(controller,wrapped);
    }
    auto setOwner=[&](void* ctrl) {
        void* secondary=ctrl?static_cast<void*>(&static_cast<ControllerPrefix*>(ctrl)->interfaceC):nullptr;
        CallGameMethod<void,0x00566170>(secondary,&self->owner,mode);
    };
    setOwner(controller);
    auto attach=[&](void* graph,void* ctrl) {
        CallGameVirtual<void,5>(graph,10000,ctrl,track,0.0f,0.0f,0,0,0,1);
    };
    auto mark=[](void* graph) {
        auto* node=CallGameVirtual<unsigned char*,1>(graph,1956407960);
        if (node) CallGameVirtual<void,11>(node+8,1);
    };
    attach(self->graph,controller);
    if (slaves && count>0) {
        mark(self->graph);
        for (int i=0;i<count;++i) {
            if (!slaves[i]) continue;
            AssignController(temporary[i],slaves[i]);
            if (self->shared) {
                if (slaveHost(i) && CallGameVirtual<int,6>(slaveHost(i))==2) route=nullptr;
                if (conform) {
                    auto* storage=CallGameFunction<void*,0x004E3F90>(72,"Animation/ConformToRouteController",0,0,0,0);
                    void* wrapped=nullptr;
                    if (storage) {
                        auto& state=self->slaves[i];
                        const bool special=CallGameFunction<unsigned char,0x0055DA60>(state.b) && mode==4;
                        const bool shared=route && !allow;
                        auto* resource=CallGameMethod<void*,0x00C93970>(self->host);
                        wrapped=CallGameMethod<void*,0x0056E4C0>(storage,self->controllers[i],resource,route,slaves[i],
                            state.a,state.b,time,self->conformFlag,shared,special);
                    }
                    AssignController(temporary[i],wrapped);
                }
                setOwner(temporary[i]);
            }
            attach(self->controllers[i],temporary[i]);
            mark(self->controllers[i]);
        }
    }
    auto retain=[&](void* ctrl) -> uintptr_t {
        if (ctrl) ++static_cast<Ref*>(ctrl)->refs;
#if defined(_M_IX86)
        const char* file=reinterpret_cast<const char*>(0x00F98980);
#else
        const char* file="EA controller list";
#endif
        auto* link=CallGameFunction<ControllerLink*,0x004E3F90>(sizeof(ControllerLink),self->descriptor,0,0,file,231);
        // The original only checks link+8, not allocation failure, before dereferencing link.
        link->controller=ctrl;
        if (ctrl) ++static_cast<Ref*>(ctrl)->refs;
        link->mode=mode;
        auto* sentinel=reinterpret_cast<ControllerLink*>(&self->next);
        link->next=sentinel; link->previous=self->previous;
        self->previous->next=link; self->previous=link;
        return ctrl?DropController(ctrl):reinterpret_cast<uintptr_t>(link);
    };
    uintptr_t result=CallGameVirtual<uintptr_t,4>(self->graph,controller);
    if (static_cast<unsigned char>(result)) result=retain(controller);
    if (self->shared) {
        for (int i=0;i<count;++i) {
            result=reinterpret_cast<uintptr_t>(&self->controllers[i]);
            if (self->controllers[i]) {
                result=CallGameVirtual<uintptr_t,4>(self->controllers[i],temporary[i]);
                if (static_cast<unsigned char>(result)) result=retain(temporary[i]);
            }
        }
    }
    if (temporary) {
        for (size_t i=*allocation;i>0;--i) if (temporary[i-1]) DropController(temporary[i-1]);
        result=CallGameFunction<uintptr_t,0x004E3F70>(allocation);
    }
    if (controller) result=DropController(controller);
    return result;
}
#if defined(_M_IX86)
uintptr_t __fastcall AttachHook(BuildBody* self, void*, void* controller, void** slaves, int count, int mode, int track, unsigned char conform, float time) {
    return Attach(self,controller,slaves,count,mode,track,conform,time);
}
#endif

uintptr_t Reset(ResetBody* self, unsigned char force) {
    self->latch=1;
    auto* resource=CallGameMethod<Ref*,0x00C93970>(self->host);
#if defined(_MSC_VER)
    if (resource) _InterlockedIncrement(&resource->refs);
#else
    if (resource) ++resource->refs;
#endif
    const bool active=self->active && !force;
    if (self->token) {
        if (self->manager) CallGameVirtual<void,30>(self->manager,self->token,0);
        self->token=0;
    }
    auto enable=[](void* graph,bool enabled) {
        if (!graph) return;
        auto* node=CallGameVirtual<void*,1>(graph,int32_t(-188129582));
        if (node) {
            CallGameVirtual<void,2>(node,1.0f);
            CallGameVirtual<void,5>(node,enabled);
        }
    };
    if (active || self->forced || (resource && !CallGameVirtual<int,31>(resource))) {
        enable(self->graph,!self->enabled);
        if (self->count>0 && self->slaves) {
            for (int i=0;i<self->count;++i)
                if (self->controllers[i]) enable(self->controllers[i],!self->slaves[i].enabled);
        }
    }
    const int state=CallGameMethod<int,0x0084AF20>(self);
    bool mask=true;
    if (resource) {
        void* route=CallGameVirtual<void*,3>(resource);
        mask=CallGameMethod<unsigned char,0x0081C880>(route,0,0x2000,0)!=0;
    }
    if (!active && resource && !mask && !self->forced && (state==1 || state==4)) {
        alignas(16) unsigned char a[16],b[16];
        if (CallGameVirtual<unsigned char,79>(resource)) {
            auto* position=CallGameVirtual<void*,80>(resource,a);
            auto* transform=CallGameVirtual<void*,77>(resource,b,position);
            CallGameVirtual<void,12>(self->manager,transform);
            auto* node=CallGameVirtual<unsigned char*,1>(self->graph,1956407960);
            *reinterpret_cast<uint32_t*>(node+44) |= 0x4000;
        } else {
            auto* transform=CallGameVirtual<void*,77>(resource,b);
            CallGameVirtual<void,8>(self->manager,transform);
        }
    }
    int mode=active?2:((state==1 || state==4)?6:3);
    if (!active && self->forced) mode=1;
    CallGameVirtual<void,20>(self->host,mode,0,0);
    if (!active) CallGameVirtual<void,17>(self->host);
    for (int i=0;i<self->count;++i) {
        auto* slave=CallGameMethod<void*,0x00839CA0>(self->host,i);
        if (slave) {
            CallGameVirtual<void,20>(slave,mode,0,0);
            if (!active) CallGameVirtual<void,17>(slave);
        }
    }
    auto result=CallGameMethod<uintptr_t,0x0083A650>(self->host,0,0,0);
    if (resource) {
        result=reinterpret_cast<uintptr_t>(&resource->refs);
#if defined(_MSC_VER)
        const auto count=_InterlockedDecrement(&resource->refs);
#else
        const auto count=--resource->refs;
#endif
        if (!count) {
#if defined(_MSC_VER)
            _InterlockedExchange(&resource->refs,1);
#else
            resource->refs=1;
#endif
            result=CallGameVirtual<uintptr_t,0>(resource,1);
        }
    }
    return result;
}
#if defined(_M_IX86)
uintptr_t __fastcall ResetHook(ResetBody* self, void*, unsigned char force) { return Reset(self,force); }
uintptr_t __fastcall SampleHook(Body* self, void*, void* output) { return Sample(self,output); }
#endif
}
