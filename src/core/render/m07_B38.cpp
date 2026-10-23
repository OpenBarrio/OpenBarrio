#include "ts3/render/m07_B38.h"
#include <cstring>
#include <xmmintrin.h>
namespace ts3::render::m07::b38 {
Resolver resolve=nullptr;
namespace {
template<class R,uint32_t A,class... T> R M(void* s,T... a) {
    if(resolve) if(void* p=resolve(A)) return reinterpret_cast<R(__fastcall*)(void*,void*,T...)>(p)(s,nullptr,a...);
    return CallGameMethod<R,A>(s,a...);
}
template<class R,uint32_t A,class... T> R F(T... a) {
    if(resolve) if(void* p=resolve(A)) return reinterpret_cast<R(__cdecl*)(T...)>(p)(a...);
    return CallGameFunction<R,A>(a...);
}
template<class R,int S,class... T> R V(void* s,T... a) {
    return reinterpret_cast<R(__fastcall*)(void*,void*,T...)>((*static_cast<void***>(s))[S])(s,nullptr,a...);
}
uintptr_t Low(uintptr_t v,uint8_t b) {return (v&~uintptr_t(255))|b;}
void* Alloc(uint32_t size,const char* name) {return F<void*,0x004E3F90>(size,name,0u,0u,0u,0u);}
void Push(Vector& v,int32_t& index) {
    auto* p=static_cast<int32_t*>(v.end);
    if(reinterpret_cast<uintptr_t>(p)<reinterpret_cast<uintptr_t>(v.capacity)) {
        v.end=reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(p)+4);
        if(p) *p=index;
    } else M<void,0x008621C0>(&v,p,&index);
}
template<class T> int32_t Count(Vector& v) {return int32_t((reinterpret_cast<intptr_t>(v.end)-reinterpret_cast<intptr_t>(v.begin))/sizeof(T));}
Entry* Find(Resources* s,uint32_t* key) {
    Entry* out[2]; M<void,0x00C2ABA0>(&s->table,out,key); return out[0];
}
void* FindModel(Swarm* s,uint32_t a,uint32_t b) {return M<void*,0x0071A640>(s,a,b);}
float Infinity() {
    if(resolve) if(void* p=resolve(0x0115B5C4)) return *static_cast<float*>(p);
#if defined(_M_IX86)
    return *reinterpret_cast<float*>(0x0115B5C4);
#else
    AbortNotRewritten("m07_B38 native instance bounds constant");return 0;
#endif
}
void** BaseTable() {
    if(resolve) if(void* p=resolve(0x00FF2F00)) return static_cast<void**>(p);
#if defined(_M_IX86)
    return reinterpret_cast<void**>(0x00FF2F00);
#else
    AbortNotRewritten("m07_B38 native shadow base vtable");return nullptr;
#endif
}
}
Shadow* __fastcall DeleteShadow(Shadow* s,void*,uint32_t flags) {
    if(s->allocation) F<void,0x004E3F70>(s->allocation);
    s->table=BaseTable();
    auto* sub=reinterpret_cast<uint8_t*>(s)+sizeof(void*);
    M<void,0x006A0850>(sub,s->backing);
    if(flags&1) F<void,0x004E3F70>(s);
    return s;
}
uintptr_t __fastcall UpdateShadow(void* s,void*) {
    M<void,0x00699CB0>(s);
    return M<uintptr_t,0x00699300>(s);
}
bool __fastcall AllocateSkin(Skin* s,void*,uint32_t count) {
    s->hashes=static_cast<uint32_t*>(Alloc(count>0x3fffffffu?0xffffffffu:count*4,"Renderer/Skin/JointNameHashes"));
    if(!s->hashes) return false;
    s->bindPose=F<void*,0x004E3FB0>(count>0x03ffffffu?0xffffffffu:count*64,16u,0u,"Renderer/Skin/BindPose",0u,0u,0u,0u);
    if(!s->bindPose) {F<void,0x004E3F70>(s->hashes);s->hashes=nullptr;return false;}
    s->count=count;return true;
}
uintptr_t __fastcall Acquire(Resources* s,void*,uint32_t a,uint32_t b,void** out) {
    uint32_t key[2]={a,b}; Entry* entry=Find(s,key);
    if(entry==s->table.buckets[s->table.sentinelIndex]) return Low(reinterpret_cast<uintptr_t>(s->table.buckets),0);
    if(!entry->material) {
        void* factory=F<void*,0x006CACD0>();
        M<void,0x006CAC30>(entry->definition,&entry->material,factory);
    }
    ++entry->users;
    uintptr_t result=V<uintptr_t,0>(entry->material);
    *out=entry->material;return Low(result,1);
}
uintptr_t __fastcall Release(Resources* s,void*,uint32_t a,uint32_t b) {
    uint32_t key[2]={a,b};Entry* entry=Find(s,key);
    uintptr_t result=reinterpret_cast<uintptr_t>(entry);
    if(entry!=s->table.buckets[s->table.sentinelIndex] && --entry->users==0 && entry->material) {
        void* old=entry->material;entry->material=nullptr;result=V<uintptr_t,1>(old);
    }
    return result;
}
uintptr_t __fastcall SetMaterial(MaterialOwner* s,void*,uint32_t a,uint32_t b,MaterialInput* input) {
    alignas(16) uint8_t params[32];M<void,0x00799300>(params);
    for(Param* p=input->begin;p!=input->end;++p) {
        switch(p->type) {
        case 0:M<void,0x00799600>(params,p->key,&p->data,1);break;
        case 1:M<void,0x00799620>(params,p->key,&p->data,1);break;
        case 3:M<void,0x00799600>(params,p->key,p->data.pointer,int32_t(p->count));break;
        case 4:M<void,0x00799620>(params,p->key,p->data.pointer,int32_t(p->count));break;
        case 6: {
            void* callback=nullptr;
#if defined(_M_IX86)
            callback=reinterpret_cast<void*>(0x00B2D882);
#endif
            M<void,0x00799660>(params,p->key,p->data.words[0],p->data.words[1],callback,0u,0u);break;
        }
        default:break;
        }
    }
    uint32_t key[2]={a,b};auto** slot=M<void**,0x0071ECD0>(s->table,key);
    void* fresh=Alloc(64,"Renderer/SwarmMaterialDef");
    if(fresh) fresh=M<void*,0x006CAF80>(fresh);
    void* old=slot[0];
    if(fresh!=old) {if(fresh) V<void,0>(fresh);slot[0]=fresh;if(old) V<void,1>(old);}
    M<void,0x00F20390>(slot[0],input->shader);
    M<void,0x006CAE90>(slot[0],params);
    if(slot[1]) {void* cached=slot[1];slot[1]=nullptr;V<void,1>(cached);}
    return Low(M<uintptr_t,0x00799330>(params),1);
}
uintptr_t __fastcall RemoveParticle(Swarm* s,void*,int32_t index) {
    // The original leaves EAX unspecified for a negative index.
    if(index<0) return 0;
    uintptr_t result=Count<Particle>(s->particles);
    if(index>=int32_t(result)) return result;
    auto* record=static_cast<Particle*>(s->particles.begin)+index;
    if(!record->owner) return result;
    Push(s->freeParticles,index);record=static_cast<Particle*>(s->particles.begin)+index;
    result=15u*uint32_t(index);record->owner=0;record->enabled=0;
    if(record->material) {
        V<void,4>(s->manager,record->key[0],record->key[1]);
        result=V<uintptr_t,1>(record->material);record->material=nullptr;
    }
    if(record->resource) {result=V<uintptr_t,2>(record->resource);record->resource=nullptr;}
    return result;
}
uintptr_t __fastcall AddInstance(Swarm* s,void*,uintptr_t owner,InstanceInput* in,int32_t* out) {
    void* model=FindModel(s,in->modelKey[0],in->modelKey[1]);
    if(!model) {if(!s->fallbackEnabled) return 0;model=s->fallback;}
    uint32_t extra[2]={0,0};
    if(in->metadata && in->metadata->count==2) std::memmove(extra,in->metadata->data+in->metadata->index,8);
    int32_t index;
    if(s->freeInstances.begin==s->freeInstances.end) {
        Instance temporary;float f=Infinity();
        // The boundary copy helper owns the other, intentionally indeterminate fields.
        std::memcpy(temporary.pad28,&f,4);std::memcpy(temporary.pad28+4,&f,4);std::memcpy(temporary.pad28+8,&f,4);
        float neg=_mm_cvtss_f32(_mm_sub_ss(_mm_set_ss(-0.0f),_mm_set_ss(f)));
        for(int i=0;i<3;++i) std::memcpy(temporary.pad28+12+4*i,&neg,4);
        auto* p=static_cast<Instance*>(s->instances.end);
        if(reinterpret_cast<uintptr_t>(p)<reinterpret_cast<uintptr_t>(s->instances.capacity)) {
            s->instances.end=p+1;if(p) M<void,0x0071C310>(p,&temporary);
        } else M<void,0x0071EF30>(&s->instances,p,&temporary);
        index=Count<Instance>(s->instances)-1;
    } else {auto* p=static_cast<int32_t*>(s->freeInstances.end)-1;index=*p;s->freeInstances.end=p;}
    *out=index;auto& r=static_cast<Instance*>(s->instances.begin)[index];
    r.model=model;r.owner=owner;r.binding=nullptr;r.material=nullptr;r.key[0]=r.key[1]=0;
    r.mode=16;r.enabled=1;r.flags=in->flags;r.extra[0]=extra[0];r.extra[1]=extra[1];
    uintptr_t result=in->materialKey[0];
    if(in->materialKey[0]||in->materialKey[1]) {
        void* material=nullptr;result=V<uintptr_t,3>(s->manager,in->materialKey[0],in->materialKey[1],&material);
        if(material) {
            r.binding=M<void*,0x008AB2B0>(material);result=reinterpret_cast<uintptr_t>(material);
            r.state=0;r.material=material;r.key[0]=in->materialKey[0];r.key[1]=in->materialKey[1];
        }
    }
    return Low(result,1);
}
uintptr_t __fastcall RemoveInstance(Swarm* s,void*,int32_t index) {
    if(index<0) return 0;
    uintptr_t result=Count<Instance>(s->instances);
    if(index>=int32_t(result)) return result;
    auto* r=static_cast<Instance*>(s->instances.begin)+index;
    if(!r->owner) return result;
    Push(s->freeInstances,index);r=static_cast<Instance*>(s->instances.begin)+index;
    if(r->material) {V<void,4>(s->manager,r->key[0],r->key[1]);r->material=nullptr;r->key[0]=r->key[1]=0;r->binding=nullptr;}
    if(r->model!=s->fallback) {
        auto manager=F<void*,0x006E8330>();if(manager) M<void,0x006E84C0>(manager,r->model);
        if(r->model) V<void,1>(r->model);
    }
    result=F<uintptr_t,0x006879A0>();if(result) result=M<uintptr_t,0x00687E30>(reinterpret_cast<void*>(result),r->model);
    r->model=nullptr;r->owner=0;r->enabled=0;return result;
}
uintptr_t __fastcall AddEmitter(Swarm* s,void*,uint32_t a,uint32_t b,int32_t* out,float value) {
    void* model=FindModel(s,a,b);if(!model) {if(!s->fallbackEnabled) return 0;model=s->fallback;}
    alignas(16) Emitter temporary;auto* p=reinterpret_cast<float*>(temporary.prefix+64);for(int i=0;i<4;++i) p[i]=1.0f;
    temporary.model=nullptr;temporary.value=1.0f;temporary.enabled=0;
    temporary.key[0]=temporary.key[1]=0;temporary.material=nullptr;
    // +5c is a real zero float, but +59..5b remain untouched.
    float zero=0;std::memcpy(temporary.pad59+3,&zero,4);
    int32_t index;
    if(s->freeEmitters.begin==s->freeEmitters.end) {
        auto* end=static_cast<Emitter*>(s->emitters.end);
        if(reinterpret_cast<uintptr_t>(end)<reinterpret_cast<uintptr_t>(s->emitters.capacity)) {s->emitters.end=end+1;if(end) M<void,0x0071C3A0>(end,&temporary);}
        else M<void,0x0071F060>(&s->emitters,end,&temporary);
        index=Count<Emitter>(s->emitters)-1;
    } else {auto* end=static_cast<int32_t*>(s->freeEmitters.end)-1;index=*end;s->freeEmitters.end=end;M<void,0x0071C3A0>(static_cast<Emitter*>(s->emitters.begin)+index,&temporary);}
    *out=index;auto& r=static_cast<Emitter*>(s->emitters.begin)[index];r.model=model;r.value=value;
    return Low(uint32_t(index)*112u,1);
}
uintptr_t __fastcall RemoveEmitter(Swarm* s,void*,int32_t index) {
    if(index<0) return 0;
    uintptr_t result=Count<Emitter>(s->emitters);if(index>=int32_t(result)) return result;
    auto* r=static_cast<Emitter*>(s->emitters.begin)+index;if(!r->model) return result;
    if(r->material) {V<void,4>(s->manager,r->key[0],r->key[1]);r->material=nullptr;r->key[0]=r->key[1]=0;}
    // The manager accessor receives the freelist receiver left in ECX, not the model.
    Push(s->freeEmitters,index);result=reinterpret_cast<uintptr_t>(r->model);
    if(r->model!=s->fallback) {
        result=M<uintptr_t,0x006E8330>(&s->freeEmitters);
        if(result) result=M<uintptr_t,0x006E84C0>(reinterpret_cast<void*>(result),r->model);
        if(r->model) result=V<uintptr_t,1>(r->model);
    }
    r->model=nullptr;r->enabled=0;return result;
}
uintptr_t __fastcall RemoveMaterial(Materials* s,void*,int32_t index) {
    if(index<0) return 0;uintptr_t count=uintptr_t(s->end-s->begin);if(index>=int32_t(count)) return count;
    Push(s->free,index);return V<uintptr_t,2>(s->begin[index]);
}
int32_t __fastcall Unsupported(void*,void*,uintptr_t,uintptr_t,uintptr_t,uintptr_t) {return -1;}
uint32_t __fastcall RequiredSize(void*,void*,uintptr_t,uintptr_t,uintptr_t) {return 64;}
}
