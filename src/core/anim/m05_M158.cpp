#include "ts3/anim/m05_M158.h"
#include <atomic>
#include <cstring>
namespace ts3::anim::M158 {
namespace {
template<unsigned A> void* Data() {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(A);
#else
    AbortNotRewritten("M158 game data"); return nullptr;
#endif
}
template<unsigned A> void* Global() { return *static_cast<void**>(Data<A>()); }
void* Alloc(uint32_t size, const void* descriptor) {
    return CallGameFunction<void*,0x004E3F90>(size,descriptor,0u,0u,0u,0u);
}
void Add(void* p) { auto& n=*reinterpret_cast<int32_t*>(static_cast<char*>(p)+sizeof(void*)); std::atomic_ref<int32_t>(n).fetch_add(1); }
void Release(void* p) {
    auto& n=*reinterpret_cast<int32_t*>(static_cast<char*>(p)+sizeof(void*));
    if(std::atomic_ref<int32_t>(n).fetch_sub(1)==1) { std::atomic_ref<int32_t>(n).exchange(1); CallGameVirtual<void,0>(p,1u); }
}
void ReleaseTemporary(void* p) {
    if(!p) return;
    auto* sub=static_cast<char*>(p)+sizeof(void*);
    auto& n=*reinterpret_cast<int32_t*>(sub+sizeof(void*));
    if(--n==0) { n=1; CallGameVirtual<void,0>(sub,1u); }
}
// Original forwards floats through FLD/FSTP: in particular signaling NaNs
// are quieted before the dependency sees them. Keep the source field unchanged.
float ForwardFloat(float value) {
#if defined(_M_IX86)
    __asm { fld value }
    __asm { fstp value }
#else
    uint32_t bits; std::memcpy(&bits,&value,4);
    if((bits&0x7f800000u)==0x7f800000u && (bits&0x007fffffu)) {
        bits|=0x00400000u; std::memcpy(&value,&bits,4);
    }
#endif
    return value;
}
void* Body(Steering* s) { return CallGameMethod<void*,0x00580420>(s->entity); }
void* Brain(Steering* s) { return CallGameMethod<void*,0x006E8400>(s->entity); }
template<unsigned C> void* NewBody(Steering* s,void* actor,uint32_t size,const void* desc) {
    auto* p=Alloc(size,desc); return p?CallGameMethod<void*,C>(p,s->entity,static_cast<char*>(actor)+8):nullptr;
}
template<unsigned C> void* NewBrain(Steering* s,uint32_t size,const void* desc) {
    auto* p=Alloc(size,desc); return p?CallGameMethod<void*,C>(p,s?static_cast<void*>(s->entity):nullptr):nullptr;
}
}
bool Refresh(Steering* s,int32_t body,int32_t brain) {
    void* resource=CallGameMethod<void*,0x00C93970>(s->entity);
    if(!resource) { CallGameMethod<void,0x00954CF0>(s); return true; }
    Add(resource);
    if(!body) body=CallGameFunction<int32_t,0x00953810>(resource,s->mode,s->status[0]);
    if(!brain) brain=CallGameFunction<int32_t,0x00953860>(resource,s->mode,s->status[0]);
    uint8_t restore=0, special=0; uint32_t oldMode=1;
    if(Body(s) && CallGameVirtual<int32_t,0>(Body(s))!=body) {
        auto* controller=CallGameMethod<void*,0x008477B0>(Body(s));
        if(controller) {
            restore=CallGameMethod<uint8_t,0x0085A8F0>(controller);
            if(restore) oldMode=*reinterpret_cast<uint32_t*>(static_cast<char*>(controller)+28);
        }
        auto* animated=CallGameVirtual<void*,1>(Body(s),int32_t(-187705382));
        if(animated) {
            // entity+4 is the body's saved controller count, not a pointer.
            int32_t count=CallGameVirtual<int32_t,9>(animated);
            std::memcpy(s->entity+4,&count,4);
            if(count>0) CallGameMethod<void,0x00849570>(animated,&s->secondary);
            if(!restore) {
                s->status[3]=CallGameMethod<uint8_t,0x0084E900>(animated,&s->ownedNext);
                if(s->status[3]) s->flags[4]=1;
            }
        }
        if(CallGameVirtual<void*,1>(Body(s),int32_t(-187712788))) special=1;
        CallGameMethod<void,0x00839D60>(s->entity,static_cast<void*>(nullptr));
    }
    if(Brain(s) && CallGameVirtual<int32_t,0>(Brain(s))!=brain)
        CallGameMethod<void,0x00839DA0>(s->entity,static_cast<void*>(nullptr));
    if(!Body(s)) {
        void* actor=s->host?CallGameMethod<void*,0x00935770>(s->host,Global<0x011DC484>()):nullptr;
        void* made=nullptr; bool vehicle=false;
        if(actor && body==-187705382) made=NewBody<0x0084EFF0>(s,actor,608,Data<0x01044750>());
        else if(actor && body==-186933121) { made=NewBody<0x008565D0>(s,actor,256,Data<0x0104473C>()); vehicle=true; }
        else if(actor && body==111854906) { made=NewBody<0x008557A0>(s,actor,256,Data<0x01044724>()); vehicle=true; }
        else if(actor && body==2075959087) { made=NewBody<0x00856AC0>(s,actor,240,Data<0x01044708>()); vehicle=true; }
        else if(actor && body==258026897) { made=NewBody<0x00857220>(s,actor,304,Data<0x010446F4>()); vehicle=true; }
        else { auto* p=Alloc(160,Data<0x010446DC>()); if(p) made=CallGameMethod<void*,0x00858510>(p,s->entity,ForwardFloat(s->speed)); }
        void* temporary=nullptr;
        CallGameMethod<void,0x00839D20>(&temporary,made);
        if(vehicle) {
            CallGameMethod<void,0x00859260>(temporary,ForwardFloat(s->speed));
            CallGameMethod<void,0x00859280>(temporary,ForwardFloat(s->acceleration));
            if(body==258026897) {
                CallGameMethod<void,0x00857030>(temporary,int32_t(s->railFirst),int32_t(s->railLast));
                int8_t limit=s->railFirst>s->railLast?s->railFirst:s->railLast;
                if(s->railIndex>=0 && s->railIndex<=limit && s->railTime>0.0f)
                    CallGameMethod<void,0x00856FF0>(temporary,uint32_t(uint8_t(s->railIndex)),ForwardFloat(s->railTime));
            }
        }
        CallGameMethod<void,0x00839D60>(s->entity,temporary);
        CallGameMethod<void,0x009564A0>(s,s->path,uint32_t(special),1u);
        if(s->route && s->routeData && s->routeCount>0)
            CallGameMethod<void,0x00955550>(s,s->route,s->routeData,s->routeCount,uint32_t(s->routeFlag));
        if(restore) { auto* c=CallGameMethod<void*,0x008477B0>(Body(s)); if(c) CallGameMethod<void,0x0085A8C0>(c,oldMode,1u); }
        ReleaseTemporary(temporary);
    }
    if(!Brain(s)) {
        void* made;
        switch(brain) {
        case -723706642: made=NewBrain<0x00953410>(s,56,Data<0x010446C8>()); break;
        case 121805242: made=NewBrain<0x009534A0>(s,56,Data<0x010446A8>()); break;
        case 104361402: made=NewBrain<0x00953390>(s,40,Data<0x01044694>()); break;
        case -1082181015: made=NewBrain<0x00953530>(s,40,Data<0x01044678>()); break;
        default: made=NewBrain<0x00953310>(s,40,Data<0x01044660>()); break;
        }
        void* temporary=nullptr; CallGameMethod<void,0x00839D20>(&temporary,made);
        CallGameMethod<void,0x00839DA0>(s->entity,temporary); ReleaseTemporary(temporary);
    }
    CallGameMethod<void,0x00956210>(s,s->settings);
    Release(resource);
    return true;
}
bool Attach(Steering* s,void* entity,void* retained,uint32_t,uint32_t,uint32_t argument,uint8_t transform) {
    CallGameMethod<void,0x00954CF0>(s); s->state248=0;
    CallGameMethod<void,0x00839F00>(s->entity,entity); s->argument=argument;
    alignas(16) unsigned char scratch[16], second[16], output[16];
    auto hostLookup=[&]() { return s->host?CallGameMethod<void*,0x00935770>(s->host,Global<0x011E9158>()):nullptr; };
    if(auto* actor=hostLookup()) {
        auto* p=CallGameMethod<void*,0x00959000>(actor,scratch);
        s->status[0]=CallGameMethod<uint8_t,0x00954240>(s,p);
    } else s->status[0]=0;
    Refresh(s,0,0);
    void* old=s->retained;
    if(retained!=old) { if(retained) CallGameVirtual<void,4>(retained); s->retained=retained; if(old) CallGameVirtual<void,5>(old); }
    if(s->host) {
        auto* actor=CallGameMethod<char*,0x00935770>(s->host,Global<0x011DC484>());
        if(actor) { auto& flags=*reinterpret_cast<uint32_t*>(actor+44); if(flags&0x80000u) {
            flags|=0x100000;
            if(Body(s) && CallGameVirtual<int32_t,0>(Body(s))==-187712788) flags&=~0x200000u; else flags|=0x200000;
        } }
    }
    if(auto* actor=hostLookup()) {
        CallGameMethod<void,0x00959170>(actor,1u);
        if(entity && transform) {
            uint32_t v18; uint32_t v19,v20;
            auto* p=CallGameMethod<void*,0x00959000>(actor,second);
            CallGameVirtual<void,72>(entity,output,p,scratch,&v20,&v18,&v19);
            auto* host=static_cast<char*>(s->host);
            CallGameVirtual<void,42>(entity,v18,*reinterpret_cast<uint32_t*>(host+8),*reinterpret_cast<uint32_t*>(host+12));
        }
    }
    return true;
}
Steering* Construct(Steering* s,void* host) {
    s->vtable=Data<0x01043260>(); s->host=host;
    CallGameMethod<void,0x00839DF0>(s->entity);
    s->secondary=Data<0x00FC2104>();
    *reinterpret_cast<void**>(s->entity)=Data<0x01044590>();
    s->vtable=Data<0x01044618>(); s->secondary=Data<0x01044578>();
    s->route=nullptr; s->routeData=nullptr; s->routeCount=0; s->routeFlag=0;
    s->ownedDescriptor=static_cast<const char*>(Data<0x0104476C>());
    s->ownedNext=&s->ownedNext; s->ownedPrevious=&s->ownedNext;
    s->weight=1.0f; s->ids[0]=-1; s->ids[1]=-1; s->ids[2]=-1; s->state132=0; s->state136=0;
    for(unsigned i=1;i<8;++i) s->flags[i]=0;
    s->flags[5]=1; s->flags[6]=1;
    s->mode=0; s->retained=nullptr;
    CallGameMethod<void,0x00859860>(s->settings);
    s->speed=5.0f; s->acceleration=5.0f; s->argument=0;
    std::memset(s->status,0,4);
    s->nameDescriptor=static_cast<const char*>(Data<0x010447B8>()); s->nameCapacity=static_cast<const char*>(Data<0x011C7E55>());
    s->nameBegin=static_cast<const char*>(Data<0x011C7E54>()); s->nameEnd=s->nameBegin;
    s->debug=0; s->p224=0; s->p228=0; s->p232=nullptr; s->b236=0; s->b237=0;
    s->p240=nullptr; s->p244=nullptr; s->state248=0; s->state252=0; s->p256=nullptr;
    s->railFirst=-1; s->railLast=-1; s->railIndex=-1; s->railTime=0; s->avoidance=nullptr;
    s->state68=0; s->state64=0;
    CallGameMethod<void,0x0083AB40>(Global<0x011D9140>(),s);
    auto* p=Alloc(192,Data<0x0104478C>());
    auto* made=p?CallGameMethod<void*,0x0085D590>(p):nullptr;
    auto* old=s->avoidance;
    if(made!=old) { if(made) Add(made); s->avoidance=made; if(old) Release(old); }
    return s;
}
void* ConstructIfPresent(void* storage,void* host) {
    if(!storage) return nullptr;
    return Construct(static_cast<Steering*>(storage),host);
}
bool CreateManager() {
    auto* manager=Alloc(sizeof(void*),Data<0x00FC2C98>());
    if(manager) *static_cast<void**>(manager)=Data<0x01054B30>();
    *static_cast<void**>(Data<0x011EA34C>())=manager;
    return true;
}
}
