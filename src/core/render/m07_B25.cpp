#include "ts3/render/m07_B25.h"
#if defined(_MSC_VER)
#include <intrin.h>
#endif
#include <cstring>
namespace ts3::render::m07::b25 {
Dependencies dependencies;
namespace {
void Copy16(void* to,const void* from) { uint8_t t[16]; std::memcpy(t,from,16); std::memcpy(to,t,16); }
void* ShaderManager() {
    if(dependencies.shaderManager) return dependencies.shaderManager;
#if defined(_M_IX86)
    return *reinterpret_cast<void**>(0x011D753C);
#else
    throw std::runtime_error("M07 shader manager unavailable");
#endif
}
uintptr_t Identity(void* output) {
    const uint32_t v[16]={0x3f800000,0,0,0,0,0x3f800000,0,0,0,0,0x3f800000,0,0,0,0,0x3f800000};
    std::memcpy(output,v,64);
    return (reinterpret_cast<uintptr_t>(output)&~uintptr_t(255))|1;
}
}
uintptr_t __fastcall Bind(Model* s,void*,void* owner,Binding* b,const void* mode) {
    if(s->flags&0x1000) return s->flags;
    if(s->flags&0x10) {
        bool pick=(static_cast<const uint8_t*>(mode)[4]&0x80)!=0;
        void* shader=CallThis(Original(0x0079B840,dependencies.shader),ShaderManager(),pick?"pickdefault":"shadererror",uint32_t(0),uint32_t(0));
        b->error=pick?0:1; b->shader=shader;
    }
    return CallThis(Original(0x006DAC50,dependencies.bind),owner,b);
}
uint8_t __fastcall SetHidden(Model* s,void*,uint8_t value) {
    if(value==((s->state1>>3)&1)) return value;
    s->state1^=(s->state1^(uint8_t(value*8)))&8;
    return uint8_t(CallThis(reinterpret_cast<Thiscall<uintptr_t,Model>>(s->vtable[0x90/4]),s));
}
uintptr_t __fastcall UpdateFlags(Model* s,void*) {
    s->state1&=~4;
    auto* sys=static_cast<uint8_t*>(Original(0x006DE470,dependencies.system)());
    uint32_t flags=CallThis(Original(0x006C93D0,dependencies.flags),s);
    uintptr_t result=flags;
    if((sys[60] || (flags&0x200000)) && !(s->state1&8)) {
        Lod* lod=CallThis(Original(0x006DB800,dependencies.lod),static_cast<void*>(s->handle));
        result=reinterpret_cast<uintptr_t>(lod);
        if(lod && ((flags&0x1000000)?lod->active!=0:(flags&0x20a000)!=0)
           && !s->pending && !(flags&0x20000030)) s->state1|=4;
    }
    return result;
}
uintptr_t __fastcall Route(Model* s,void*,int32_t index) {
    Lod* lod=CallThis(Original(0x006DB810,dependencies.lodResolved),static_cast<void*>(s->handle));
    if(lod) return CallThis(Original(0x0081FC30,dependencies.route),lod->routes[index]);
    return CallThis(Original(0x006FB430,dependencies.fallback),s,index);
}
uintptr_t __fastcall SetSelector(Model* s,void*,uint32_t value,int32_t kind) {
    if(s->selector==value) return value;
    s->selector=value;
    CallThis(Original(0x006FAD50,dependencies.refresh),s);
    // This is an interned, four-byte-prefixed key, not a C string name.
    const char* key="q&9\apickdefault";
    if(kind!=1 && kind!=2) {
        uintptr_t r=CallThis(reinterpret_cast<Thiscall<uintptr_t,Model,const char*>>(s->vtable[0x84/4]),s,key);
        if(!(r&255)) return r;
        return CallThis(Original(0x006DCED0,dependencies.dirty),static_cast<void*>(s->handle));
    }
    void* fixup=CallGameFunction<void*,0x004E3FB0>(12,4,0,"Renderer/ThumbnailFixupLOD",0,0,0,0);
    if(fixup) fixup=CallGameMethod<void*,0x007233D0>(fixup,kind==2?1:0);
    if(fixup) CallGameVirtual<uintptr_t,0>(fixup);
    CallThis(reinterpret_cast<Thiscall<void,Model,void*,const char*>>(s->vtable[0x80/4]),s,fixup,key);
    uintptr_t result=CallThis(Original(0x006DCED0,dependencies.dirty),static_cast<void*>(s->handle));
    if(fixup) return CallGameVirtual<uintptr_t,1>(fixup);
    return result;
}
uintptr_t __fastcall GetBounds(Model* s,void*,void* bounds,void* matrix) {
    auto* b=static_cast<uint8_t*>(bounds); auto* m=static_cast<uint8_t*>(matrix);
    Copy16(b,s->bounds); Copy16(b+16,s->bounds+16);
    for(int i=0;i<4;++i) Copy16(m+16*i,s->matrix+16*i);
    Lod* lod=CallThis(Original(0x006DB810,dependencies.lodResolved),static_cast<void*>(s->handle));
    if(!lod) return 0;
    uint32_t flags=CallThis(Original(0x006C93D0,dependencies.flags),s);
    if(flags&0x200000) return flags;
    const void* p=CallThis(Original(0x005042A0,dependencies.bounds),lod);
    Copy16(b,p); Copy16(b+16,static_cast<const uint8_t*>(p)+16);
    return reinterpret_cast<uintptr_t>(p);
}
uintptr_t __fastcall SetMatrix(Model* s,void*,const void* matrix) {
    if(s->begin!=s->end && std::memcmp(matrix,s->matrix,64)==0) return 0;
    return CallThis(Original(0x006FCF50,dependencies.matrixChanged),s,matrix);
}
uintptr_t __fastcall Inverse(Model* s,void*,int32_t index,void* output) {
    if(s->scratch) return CallThis(Original(0x005734E0,dependencies.inverse),s->scratch,index,output);
    return Identity(output);
}
uintptr_t __fastcall Transform(Model* s,void*,int32_t index,void* output) {
    if(s->scratch) return CallThis(Original(0x00573620,dependencies.transform),s->scratch,index,output);
    return Identity(output);
}
namespace {
void** ModelTable() {
#if defined(_M_IX86)
    return reinterpret_cast<void**>(0x00FF97F8);
#else
    throw std::runtime_error("M07 model vtable unavailable");
#endif
}
void Exchange(volatile long* p,long value) {
#if defined(_MSC_VER)
    _InterlockedExchange(p,value);
#else
    __atomic_exchange_n(p,value,__ATOMIC_SEQ_CST);
#endif
}
}
Model* __fastcall Construct(Model* s,void*,void* a,void* b) {
    CallGameMethod<void*,0x006FD710>(s,a,b);
    s->vtable=ModelTable();
    CallGameMethod<void*,0x006DD580>(s->handle);
    s->parts=nullptr; s->partsEnd=nullptr; s->partsCapacity=nullptr;
    s->partsTag="Renderer/SceneModelPartInfos";
    s->state2&=0xf8; s->flags44|=0x01000008;
    const uint32_t lo[4]={0xba83126f,0xba83126f,0xba83126f,0xba83126f};
    const uint32_t hi[4]={0x3a83126f,0x3a83126f,0x3a83126f,0x3a83126f};
    std::memcpy(s->bounds,lo,16); std::memcpy(s->bounds+16,hi,16);
    s->scratch=nullptr; s->a0=0;s->a4=0;s->a8=0;s->ac=0;s->b4=0;s->b8=0;
    s->state0=16;s->state1=2;s->b0=100;s->self=s;
    return s;
}
uintptr_t __fastcall Destroy(Model* s,void*) {
    void* scratch=s->scratch;
    s->vtable=ModelTable();
    if(scratch) {
        CallGameMethod<void,0x00573F50>(scratch);
        CallGameFunction<void,0x004E3F70>(scratch);
        s->scratch=nullptr;
    }
    s->scratch=nullptr;
    if(s->parts) CallGameFunction<void,0x004E3F70>(s->parts);
    CallGameMethod<void,0x006DD350>(s->handle);
    return CallGameMethod<uintptr_t,0x006FD930>(s);
}
uint8_t __fastcall Clone(Model* s,void*,Model** output) {
    auto* m=CallGameFunction<Model*,0x004E3FB0>(720,16,0,"Renderer/SceneModel",0,0,0,0);
    if(m) m=Construct(m,nullptr,s->owner0,s->owner1);
    *output=m;
    if(!m) return 0;
    CallGameVirtual<uintptr_t,0>(m);
    void* resource=CallGameFunction<void*,0x004E3F90>(72,"Renderer/SceneModelClone",0,0,0,0);
    if(resource) resource=CallGameMethod<void*,0x006D0570>(resource);
    if(resource) CallGameVirtual<uintptr_t,0>(resource);
    void* lod=CallGameMethod<void*,0x006DB800>(s->handle);
    if(lod) CallGameMethod<void,0x006D05D0>(resource,lod);
    CallGameMethod<void,0x006DCBE0>(m->handle,resource);
    void* extra=CallGameMethod<void*,0x00F1D820>(s->handle);
    if(extra) CallGameMethod<void,0x006DB880>(m->handle,extra);
    bool same=false;
    if(s->context) {
        // The context's +1C is a scalar owner ID, not a pointer.
        uint32_t owner; std::memcpy(&owner,static_cast<uint8_t*>(s->context)+28,4);
        same=owner==CallGameFunction<uint32_t,0x006E8330>();
    }
    void* info=CallGameMethod<void*,0x0079FBE0>(s->handle);
    CallGameMethod<uintptr_t,0x006DB8E0>(m->handle,info,uint32_t(same));
    if(resource) CallGameVirtual<uintptr_t,1>(resource);
    return 1;
}
void* __fastcall CreateTexture(Compositor* s,void*) {
    auto* t=CallGameFunction<uint8_t*,0x004E3F90>(104,"Renderer/Material/TexCompositor",0,0,0,0);
    if(t) t=CallGameMethod<uint8_t*,0x00723310>(t);
    void* adjusted=t?t+40:nullptr;
    void* old=s->texture;
    if(old!=adjusted) {
        if(adjusted) CallGameVirtual<uintptr_t,1>(adjusted);
        s->texture=adjusted;
        if(old) CallGameVirtual<uintptr_t,2>(old);
    }
    uint32_t size=s->height<s->width?s->height:s->width;
    uint32_t levels=1;
    while(size>4) {size>>=1;++levels;}
    // EA still dereferences t after allocation failure: no invented null recovery.
    uint8_t ok=CallGameMethod<uint8_t,0x00619C80>(t+40,s->width,s->height,levels,uint32_t(2|((s->flags&1)*8)),2,"TCService");
    Exchange(reinterpret_cast<volatile long*>(t+96),1);
    return ok?t:nullptr;
}
uintptr_t __fastcall Compose(Composition* s,void*) {
    uintptr_t result=uint32_t(s->state);
    if(result!=2) return result;
    Exchange(&s->state,4);
    if(!s->target) return result;
    if(s->flags&2) {
        auto* animated=static_cast<uint8_t*>(s->animated);
        uint32_t valid;std::memcpy(&valid,animated+68,4);
        uint8_t* texture=valid?animated+40:nullptr;
        CallGameMethod<void,0x006C93A0>(s->target,texture);
        CallGameMethod<void,0x00C853A0>(s->target,s->material);
        void* manager=CallGameFunction<void*,0x006879A0>();
        void* frame=CallGameMethod<void*,0x00687630>(texture,0);
        s->flags&=~8;
        if(s->target) CallGameMethod<uintptr_t,0x006C9760>(s->target,s->material,frame);
        auto* entry=CallGameMethod<uint8_t*,0x006C9B60>(s->target,"TextureSpeedScale");
        float scale=1;
        if(entry && entry[30]==1) {
            float* p;std::memcpy(&p,entry+20,sizeof(p));scale=*p;
        }
        float period;std::memcpy(&period,texture+24,4);
        float speed=0;
        if(double(period)>0.00001) { float reciprocal=1.0f/period; speed=reciprocal*scale; }
        struct Args {float speed;uint32_t a,b;uint8_t enabled;uint8_t pad[3];} args;
        args.speed=speed;args.a=0;args.b=0;args.enabled=0;
        return CallGameMethod<uintptr_t,0x00687CE0>(manager,s->target,0,&args);
    }
    if(!(s->flags&1)) {
        auto* texture=static_cast<uint8_t*>(s->texture);
        uint32_t valid;std::memcpy(&valid,texture+96,4);
        void* adjusted=valid?texture+40:nullptr;
        s->flags&=~8;
        if(s->target) return CallGameMethod<uintptr_t,0x006C9760>(s->target,s->material,adjusted);
        return reinterpret_cast<uintptr_t>(adjusted);
    }
    uint64_t id=CallGameMethod<uint64_t,0x00605CE0>(reinterpret_cast<void*>(s->resource));
    if((uint32_t(id)&uint32_t(id>>32))==0xffffffff) {
        // Raw instruction AND is FC, not IDA's inferred F8: retain bit 3.
        s->flags=(s->flags&0xfc)|4;
        Exchange(&s->state,0);
#if defined(_M_IX86)
        void* manager=reinterpret_cast<void*>(0x0115B890);
#else
        void* manager=nullptr; // Original bridge below fails explicitly on native builds.
#endif
        result=CallGameMethod<uintptr_t,0x005C9C40>(&s->texture,manager,&s->link,0,0);
        if(result&255) return CallGameMethod<uintptr_t,0x006CF0A0>(s);
        s->flags&=~8;
        if(s->target) return CallGameMethod<uintptr_t,0x006C9760>(s->target,s->material,static_cast<void*>(nullptr));
        return result;
    }
    void* manager=CallGameFunction<void*,0x006079C0>();
    void* resource=(s->resource&3)?nullptr:reinterpret_cast<void*>(s->resource);
    void* found=CallGameMethod<void*,0x006CCF60>(manager,resource);
    if(found) {
        result=CallGameMethod<uintptr_t,0x007C8D00>(&s->opaque28,found);
        s->flags&=~8;
        if(s->target) return CallGameMethod<uintptr_t,0x006C9760>(s->target,s->material,found);
        return result;
    }
    struct Request {uint32_t k0,k1,k2,k3;uint8_t a,b;} request;
    request.k0=s->key0;request.k1=s->key1;request.k2=s->key2;request.k3=s->key3;
    request.a=(s->flags>>4)&1;request.b=(s->flags>>5)&1;
    return CallGameMethod<uintptr_t,0x006CE910>(manager,resource,&request,s->p0C,uint32_t(s->index));
}
}
