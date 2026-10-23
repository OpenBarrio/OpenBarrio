#include "ts3/render/m07_M04.h"
namespace ts3::render::m07::m04 {
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
void ConvertElements(uint8_t* format,Elements* elements) {
    using Fn=void(__stdcall*)(uint8_t*,Elements*);
    auto injected=resolve?reinterpret_cast<Fn>(resolve(0x006D47F0)):nullptr;
    Original<Fn>(0x006D47F0,injected)(format,elements);
}
template<int S> void V(void* s) { reinterpret_cast<void(__fastcall*)(void*,void*)>((*static_cast<void***>(s))[S])(s,nullptr); }
void Release(void* s) { if(s) V<2>(s); }
void* Allocate(uint32_t n,uint32_t alignment,const char* tag) {
    return F<void*,0x004E3FB0>(n,alignment,0u,tag,0u,0u,0u,0u);
}
uint32_t Bytes(const uint32_t* a,const uint32_t* b) {return uint32_t(reinterpret_cast<uintptr_t>(b)-reinterpret_cast<uintptr_t>(a))&~3u;}
}
uint8_t __fastcall Build(Builder* s,void*,void** output) {
    if(s->disabled || s->vertices==s->verticesEnd) return 0;
    bool unindexed=true;
    for(auto* g=s->groups;g!=s->groupsEnd;++g)
        for(auto* p=g->begin;p!=g->end;++p) if(p->indexed) unindexed=false;
    if(s->indices==s->indicesEnd && !unindexed) return 0;
    void* vb=Allocate(32,4,"Renderer/ModelBuilder/VertexBuffer");
    if(vb) {vb=M<void*,0x0061A380>(vb);if(vb) V<1>(vb);}
    if(!M<uint8_t,0x0061A1C0>(vb,Bytes(s->vertices,s->verticesEnd),0u,"ModelBuilder") ||
       !M<uint8_t,0x0061A240>(vb,s->vertices,Bytes(s->vertices,s->verticesEnd))) {Release(vb);return 0;}
    void* ib; M<void,0x006D14E0>(&ib,"Renderer/ModelBuilder/IndexBuffer");
    if(!unindexed) {
        uint32_t maximum=0;for(auto* i=s->indices;i!=s->indicesEnd;++i) if(*i>maximum) maximum=*i;
        uint32_t count=Bytes(s->indices,s->indicesEnd)/4;
        uint32_t wide=maximum>0xFFDCu;
        if(!M<uint8_t,0x00616DF0>(ib,count,wide,0u)) {Release(ib);Release(vb);return 0;}
        if(wide) {
            if(!M<uint8_t,0x00616E50>(ib,s->indices,count*4)) {Release(ib);Release(vb);return 0;}
        } else {
            auto* dest=M<uint16_t*,0x00616BB0>(ib,0u,count*2,0u);
            if(!dest) {Release(ib);Release(vb);return 0;}
            for(uint32_t i=0;i<count;++i) dest[i]=uint16_t(s->indices[i]);
            M<void,0x00616C10>(ib);
        }
    }
    void* model=Allocate(72,8,"Renderer/ModelBuilder/Model");
    if(model) model=M<void*,0x006D0570>(model);
    *output=model;
    uint32_t count=uint32_t((reinterpret_cast<uintptr_t>(s->groupsEnd)-reinterpret_cast<uintptr_t>(s->groups))/sizeof(Group));
    M<void,0x006D04F0>(model,count);
    void* manager=F<void*,0x0061A510>();
    for(uint32_t i=0;i<count;++i) {
        auto* g=s->groups+i;
        uint32_t partCount=uint32_t((reinterpret_cast<uintptr_t>(g->end)-reinterpret_cast<uintptr_t>(g->begin))/sizeof(Part));
        void* lod=Allocate(128,16,"Renderer/ModelBuilder/ModelLOD");
        if(lod) {lod=M<void*,0x006DB0F0>(lod);if(lod) V<0>(lod);}
        M<void,0x006DACA0>(lod,partCount);
        for(uint32_t j=0;j<partCount;++j) {
            auto* p=g->begin+j;
            void* vf=nullptr;
            Elements elements{nullptr,nullptr,nullptr,"Renderer/ModelBuilder/vfElements"};
            ConvertElements(s->formats+72*p->words[3],&elements);
            if(!M<uint8_t,0x0061A9B0>(manager,elements.begin,uint32_t((reinterpret_cast<uintptr_t>(elements.end)-reinterpret_cast<uintptr_t>(elements.begin))/8),&vf)) {
                if(elements.begin) F<void,0x004E3F70>(elements.begin);
                Release(vf);if(lod) V<1>(lod);Release(ib);Release(vb);return 0;
            }
            void* mesh=static_cast<DependencyLod*>(lod)->parts[j];
            M<void,0x0068EA80>(mesh,vf);
            M<void,0x0068EAE0>(mesh,0u,vb,4*p->words[4]);
            if(p->indexed) M<void,0x0068EB30>(mesh,ib);
            uint32_t primitiveCount=M<uint32_t,0x006D0870>(s,p->words[16],p->words[7]-p->words[6]);
            uint32_t draw[6]{p->words[16],0,p->words[6],p->words[17],p->words[18]-p->words[17]+1,primitiveCount};
            M<void,0x0068EB70>(mesh,draw);
            M<void,0x0068F8F0>(mesh,&p->words[8]);
            M<void,0x0068EE30>(mesh,uint32_t(s->mode));
            M<void,0x0068EF90>(mesh,uint32_t(s->option));
            auto* input=s->materials+p->words[2];
            void* def=Allocate(64,8,"Renderer/ModelBuilder/MaterialDef");
            if(def) {def=M<void*,0x006CAF80>(def);if(def) V<0>(def);}
            M<void,0x00F20390>(def,input->key);
            M<void,0x006CAE10>(def,input->begin,input->count);
            void* material=nullptr;
            void* context=F<void*,0x006CACD0>();
            M<void,0x006CAC30>(def,&material,context);
            M<void,0x00690F20>(mesh,material,0u,0u);
            if(material) V<1>(material);
            if(def) V<1>(def);
            if(elements.begin) F<void,0x004E3F70>(elements.begin);
            Release(vf);
        }
        M<void,0x006DAFE0>(lod);
        uint32_t desc[4]{0,i,g->nearBits,g->farBits};
        M<void,0x006D0330>(model,i,desc);
        M<void,0x006D0410>(model,i,lod);
        if(lod) V<1>(lod);
    }
    V<0>(model);Release(ib);Release(vb);return 1;
}
}
