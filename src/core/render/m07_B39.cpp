#include "ts3/render/m07_B39.h"
namespace ts3::render::m07::b39 {
Dependencies dependencies;
namespace {
void* Address(uint32_t address) { return dependencies.resolve ? dependencies.resolve(address) : Original<void*>(address); }
void** Table(uint32_t address) { return static_cast<void**>(Address(address)); }
const char* Tag(uint32_t address) { return static_cast<const char*>(Address(address)); }
void Empty(Vector& v,uint32_t tag) { v.first=v.last=v.limit=nullptr; v.tag=Tag(tag); }
template<class T,class U> T* Parent(U* sub,size_t offset) { return reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(sub)-offset); }
}
Swarm* __fastcall ConstructSwarm(Swarm* s,void*) {
    s->table=Table(0x00ffc1b0);
    Original(0x00720440,dependencies.base)(&s->resources,nullptr);
    s->shader=Table(0x00ff7728); s->bindings=Table(0x00fe441c); s->lifetime=Table(0x00ff5f1c); s->refs=0;
    s->table=Table(0x00ffc820); s->resources.table=Table(0x00ffc7c8);
    s->shader=Table(0x00ffc7b8); s->bindings=Table(0x00ffc7b0); s->lifetime=Table(0x00ffc7a0);
    for(auto& h:s->handles) h=nullptr;
    for(auto& id:s->ids) id=0xffffffffu;
    Empty(s->params,0x00ffc774); Empty(s->textures,0x00ffc748);
    s->count=10000; s->capacity=128; s->state=0; s->index=0xffffffffu; s->cursor=0;
    Empty(s->textureRecords,0x00ffc71c); Empty(s->otherRecords,0x00ffc6ec);
    s->handle3130=nullptr; Empty(s->matrices,0x00ffc6c8); Empty(s->records,0x00ffc69c); s->state3154=0;
    if(dependencies.camera) dependencies.camera(&s->camera,nullptr); else s->camera.Init();
    s->handle3890=nullptr; s->enabled=1;
    // 00723000 clears precisely five cells, leaving +38ac untouched.
    for(auto& p:s->small) p=nullptr;
    s->manager=nullptr; s->handle38b4=nullptr; s->flag=0; s->word38bc=0;
    s->handle38c0=nullptr; s->handle38c4=nullptr;
    Empty(s->bindingsVector,0x00ffc674); s->name=nullptr; s->nameEnd=nullptr; s->nameTag=Tag(0x00ffc650); s->nameSize=0x553;
    // +38e0 is cleared separately; it is the high-level string's second pointer.
    Empty(s->tail,0x00ffc628);
    return s;
}
uint32_t __fastcall SelectKind(void*,void*,const uint32_t* kinds) {
    while(*kinds && *kinds!=1 && *kinds!=64) ++kinds;
    return *kinds;
}
void* __fastcall DestroySecondary(Resources* s,void*,uint32_t flags) {
    return Original(0x00721130,dependencies.destroySwarm)(Parent<Swarm>(s,offsetof(Swarm,resources)),nullptr,flags);
}
void* __fastcall DestroyLifetime(void*** s,void*,uint32_t flags) {
    return Original(0x00721130,dependencies.destroySwarm)(Parent<Swarm>(s,offsetof(Swarm,lifetime)),nullptr,flags);
}
uint32_t __fastcall AddRef(Swarm* s,void*) { return ++s->refs; }
uint32_t __fastcall Release(Swarm* s,void*) {
    uint32_t refs=--s->refs;
    if(!refs) {
        s->refs=1;
        auto fn=reinterpret_cast<Thiscall<void*,void,uint32_t>>(s->lifetime[0]);
        fn(&s->lifetime,nullptr,1);
        return 0;
    }
    return refs;
}
uint32_t __fastcall ReleaseSecondary(Resources* s,void*) { return Release(Parent<Swarm>(s,offsetof(Swarm,resources)),nullptr); }
uint32_t __fastcall ReleaseLifetime(void*** s,void*) { return Release(Parent<Swarm>(s,offsetof(Swarm,lifetime)),nullptr); }
uint32_t __fastcall AddRefSecondary(Resources* s,void*) { return AddRef(Parent<Swarm>(s,offsetof(Swarm,resources)),nullptr); }
uint32_t __fastcall AddRefLifetime(void*** s,void*) { return AddRef(Parent<Swarm>(s,offsetof(Swarm,lifetime)),nullptr); }
Manager* __fastcall ConstructManager(Manager* s,void*) {
    s->table=Table(0x00ffc2c8);
    Original(0x00720440,dependencies.base)(&s->resources,nullptr);
    s->lifetime=Table(0x00ff5f1c); s->refs=0;
    s->table=Table(0x00ffc990); s->resources.table=Table(0x00ffc938); s->lifetime=Table(0x00ffc928);
    s->materials.low=*static_cast<const float*>(Address(0x0107a538));
    s->materials.high=*static_cast<const float*>(Address(0x00f98a58));
    s->materials.tag=Tag(0x00ffc8fc); s->materials.count=0; s->materials.data=nullptr;
    s->materials.capacity=1; s->materials.buckets=Address(0x0114c900);
    Empty(s->first,0x00ffc8d4); Empty(s->second,0x00ffc8a8);
    return s;
}
void* __fastcall DestroyManagerLifetime(void*** s,void*,uint32_t flags) {
    return Original(0x00721150,dependencies.destroyManager)(Parent<Manager>(s,offsetof(Manager,lifetime)),nullptr,flags);
}
void* __fastcall DestroyManagerSecondary(Resources* s,void*,uint32_t flags) {
    return Original(0x00721150,dependencies.destroyManager)(Parent<Manager>(s,offsetof(Manager,resources)),nullptr,flags);
}
uint32_t __fastcall ManagerAddRef(Manager* s,void*) { return ++s->refs; }
}
