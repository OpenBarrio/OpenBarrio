#include "ts3/render/m07_B30.h"
#include <cstring>
#include <intrin.h>
namespace ts3::render::m07::b30 {
Dependencies dependencies;
namespace {
void* Data(uint32_t a) { return dependencies.data?dependencies.data(a):Original<void*>(a); }
void* Allocate(uint32_t bytes,uint32_t alignment,const char* tag) {
    return Original(0x004E3FB0,dependencies.allocate)(bytes,alignment,0,tag,0,0,0,0);
}
uintptr_t Ref(Object* p,unsigned slot) { return CallThis(reinterpret_cast<Thiscall<uintptr_t,Object>>(p->table[slot]),p); }
uintptr_t Release(Object* p) { return Ref(p,1); }
void Add(Object* p) { Ref(p,0); }
void Install(Model* m,Object* p,uint32_t key) { CallThis(reinterpret_cast<Thiscall<void,Model,Object*,void*>>(m->table[32]),m,p,Data(key)); }
void Remove(Model* m,uint32_t key) { CallThis(reinterpret_cast<Thiscall<void,Model,void*>>(m->table[33]),m,Data(key)); }
uintptr_t Dirty(Model* m,bool immediate) { return CallThis(Original(immediate?0x006DCED0:0x006DCF10,immediate?dependencies.dirty:dependencies.deferred),static_cast<void*>(m->dirty)); }
Fixup* NewFixup(uint32_t bytes,uint32_t alignment,const char* tag,uint32_t table) {
    auto* f=static_cast<Fixup*>(Allocate(bytes,alignment,tag));
    if(f) {
        f->table=static_cast<void**>(Data(0x00FEFF6C));
        // Construction's interlocked exchange is preserved, even before publication.
        _InterlockedExchange(reinterpret_cast<volatile long*>(&f->refs),0);
        f->table=static_cast<void**>(Data(table));
    }
    return f;
}
Object* Texture(const uint32_t* words) {
    auto* f=NewFixup(sizeof(Fixup),8,"Renderer/TexCompFixupLOD",0x00FF9F3C);
    if(!f) return nullptr;
    for(unsigned j=0;j<8;++j) f->words[j]=words[j];
    f->flags&=0xfc; Add(reinterpret_cast<Object*>(f)); return reinterpret_cast<Object*>(f);
}
Object* TextureCtor(const void* a,const void* b) {
    auto* mem=Allocate(sizeof(Fixup),8,"Renderer/TexCompFixupLOD"); if(!mem) return nullptr;
    auto* p=CallThis(Original(0x006FE300,dependencies.texture),mem,a,b); if(p) Add(p); return p;
}
uint8_t Create(unsigned kind,void* self,void* a,void* b,Object** output) {
    const uint32_t sizes[]={720,736,560,832},ctors[]={0x006F5B40,0x006FA020,0x006FA490,0x006F9A10};
    const char* tags[]={"Renderer/SceneModel","Renderer/SceneModelFur","Renderer/SceneModelInstance","Renderer/SceneModelArray"};
    auto* mem=Allocate(sizes[kind],16,tags[kind]); if(!mem) return 0;
    auto* p=CallThis(Original(ctors[kind],dependencies.constructors[kind]),mem,a,b); if(!p) return 0;
    Add(p);
    if(!uint8_t(CallThis(Original(0x00700CB0,dependencies.attach),self,p))) { Release(p); return 0; }
    if(kind<2) {
        CallThis(Original(0x006F4900,dependencies.lod),p,int32_t(-2));
        if(output) { Add(p); *output=p; }
        Release(p);
    } else if(output) *output=p; else Release(p);
    return 1;
}
DebugInfo* Debug(DebugInfo* d,const char* text) {
    d->tag=static_cast<const char*>(Data(0x00FF9DE8));
    d->begin=static_cast<char*>(Data(0x011C7E54)); d->end=d->begin;
    d->capacity=static_cast<char*>(Data(0x011C7E55));
    Original(0x00590900,dependencies.assign)(d,text); return d;
}
void Highlight(Model* m,uint8_t enabled,bool roof) {
    if(!m) return;
    auto* f=NewFixup(sizeof(void*)+8,4,roof?"Renderer/RoofHighlightFixupLOD":"Renderer/FloorEdgeHighlightFixupLOD",roof?0x010711A0:0x01063558);
    auto* p=reinterpret_cast<Object*>(f);
    if(f) { reinterpret_cast<uint8_t*>(&f->words)[0]=enabled; Add(p); }
    uint32_t key=roof?0x01061840:0x0106351C;
    Install(m,p,key); Dirty(m,true); if(!enabled) Remove(m,key); if(p) Release(p);
}
}
uint8_t __fastcall CreateModel(void* s,void*,void* a,void* b,Object** out) { return Create(0,s,a,b,out); }
uint8_t __fastcall CreateFur(void* s,void*,void* a,void* b,Object** out) { return Create(1,s,a,b,out); }
uint8_t __fastcall CreateInstance(void* s,void*,void* a,void* b,Object** out) { return Create(2,s,a,b,out); }
uint8_t __fastcall CreateArray(void* s,void*,void* a,void* b,Object** out) { return Create(3,s,a,b,out); }
DebugInfo* __fastcall DebugClouds(void*,void*,DebugInfo* d) { return Debug(d,"SceneObject Clouds"); }
DebugInfo* __fastcall DebugSea(void*,void*,DebugInfo* d) { return Debug(d,"SceneObject Sea"); }
DebugInfo* __fastcall DebugSky(void*,void*,DebugInfo* d) { return Debug(d,"SceneObject Sky"); }
void __fastcall FloorHighlight(Floor* s,void*,uint8_t enabled) { Highlight(s->models[0],enabled,false); }
void __fastcall RoofHighlight(Roof* s,void*,uint8_t enabled) { Highlight(s->model,enabled,true); }
void __fastcall FloorTexture(Floor* s,void*) {
    for(unsigned i=0;i<2;++i) if(s->models[i]) {
        bool same=uint8_t(CallThis(Original(0x00A67AE0,dependencies.equal),static_cast<void*>(s->params),static_cast<const void*>(Data(0x011EB960))))!=0;
        if(same && !uint8_t(CallThis(reinterpret_cast<Thiscall<uint32_t,Model,void*>>(s->models[i]->table[34]),s->models[i],Data(0x00FF9ECC)))) continue;
        auto* p=same?TextureCtor(s->alternate,Data(0x011EB8D0)):Texture(s->params);
        // EA dereferences this even when allocation/construction failed.
        reinterpret_cast<Fixup*>(p)->flags|=1;
        Install(s->models[i],p,0x00FF9ECC); Dirty(s->models[i],true);
        if(same) Remove(s->models[i],0x00FF9ECC);
        Release(p);
    }
}
uint8_t __fastcall DistantTerrain(Terrain* s,void*,TerrainItem* item) {
    if(!item->model) return 0;
    volatile float delta=s->state->vector[1]-0.1425f;
    float height=delta+item->height;
    Object* p=nullptr;
    if(auto* mem=Allocate(sizeof(void*)+8,4,"Renderer/DistantTerrainFixupLOD")) {
        p=CallThis(Original(0x00718620,dependencies.distant),mem,height); if(p) Add(p);
    }
    Install(item->model,p,0x00FFBF98); Dirty(item->model,true); if(p) Release(p); return 1;
}
void __stdcall TerrainTexture(TerrainItem* item,uint8_t immediate) {
    if(!item->model) return;
    if(item->params[2] || item->params[6]) {
        auto* p=TextureCtor(item->params,item->params+4);
        Install(item->model,p,0x00FF9ECC); Dirty(item->model,immediate!=0); if(p) Release(p);
    } else if(!immediate) Remove(item->model,0x00FF9ECC);
}
uintptr_t __fastcall UpdateBindings(Bindings* s,void*,void* key,const uint32_t* params) {
    Cursor value{};
    CallThis(Original(0x00A68030,dependencies.findValue),&s->values,&value,&key);
    if(value.node!=s->values.buckets[s->values.index]) {
        uint32_t same=CallThis(Original(0x00A67AE0,dependencies.equal),static_cast<void*>(value.node->value),static_cast<const void*>(params));
        if(uint8_t(same)) return same;
        // REP MOVSD copies forwards, including overlapping inputs.
        auto* target=reinterpret_cast<uint32_t*>(value.node->value);
        for(unsigned j=0;j<12;++j) target[j]=params[j];
    }
    Cursor links{};
    uintptr_t result=CallThis(Original(0x00B26000,dependencies.findLinks),&s->links,&links,&key);
    while(links.node!=links.sentinel) {
        Cursor object{};
        CallThis(Original(0x00B26830,dependencies.findObject),&s->objects,&object,static_cast<const void*>(links.node->value));
        Model* model; std::memcpy(&model,object.node->value+24,sizeof(model));
        auto* p=Texture(params); Install(model,p,0x00FF9ECC); result=Dirty(model,false); if(p) result=Release(p);
        std::memcpy(&links.node,links.node->value+32,sizeof(links.node));
        while(!links.node) { ++links.bucket; links.node=*links.bucket; result=reinterpret_cast<uintptr_t>(links.bucket); }
    }
    return result;
}
}
