#include "ts3/render/m07_B37.h"
#include <algorithm>
#include <cstring>
namespace ts3::render::m07::b37 {
Resolver resolve=nullptr;
Globals globals{};
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
template<class T,uint32_t A> T Global(T supplied) {
    if(supplied) return supplied;
#if defined(_M_IX86)
    return *reinterpret_cast<T*>(A);
#else
    AbortNotRewritten("m07_B37 missing native global"); return T{};
#endif
}
Library** LibrarySlot() {
    if(globals.library) return globals.library;
#if defined(_M_IX86)
    return reinterpret_cast<Library**>(0x011D0114);
#else
    AbortNotRewritten("m07_B37 library slot"); return nullptr;
#endif
}
Registry* Reg() {return Global<Registry*,0x011D7530>(globals.registry);}
void* Shaders() {return Global<void*,0x011D753C>(globals.shaders);}
void* Allocate(size_t n,const char* tag) {return F<void*,0x004E3F90>(n,tag,0u,0u,0u,0u);}
void** ListTable(bool derived=false) {
#if defined(_M_IX86)
    return reinterpret_cast<void**>(derived?0x00FF2F94:0x00FF2F00);
#else
    static void* base[1]{}; static void* child[1]{};return derived?child:base;
#endif
}
void ParameterValue(Registry* r,uint32_t id,uint32_t count,const void* data) {
    if(id>=r->count) return;
    auto& p=r->entries[id];++p.revision;p.count=count;p.type=2;p.stamp=r->stamp;p.data=data;
}
const void* DefaultValue() {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(0x00FF6D80);
#else
    static uint32_t value[4]{};return value;
#endif
}
uint32_t& DimensionFlags() {
    if(globals.dimensionsFlags) return *globals.dimensionsFlags;
#if defined(_M_IX86)
    return *reinterpret_cast<uint32_t*>(0x011D0168);
#else
    AbortNotRewritten("m07_B37 dimension flags");static uint32_t v;return v;
#endif
}
uint32_t& DimensionId() {
    if(globals.dimensionsId) return *globals.dimensionsId;
#if defined(_M_IX86)
    return *reinterpret_cast<uint32_t*>(0x011D0164);
#else
    AbortNotRewritten("m07_B37 dimension id");static uint32_t v;return v;
#endif
}
}
uintptr_t __fastcall SetSource(Batcher* s,void*,void* source) {
    uintptr_t result=reinterpret_cast<uintptr_t>(s->source);
    if(s->source==source) return result;
    if(s->source) M<void,0x00696D40>(s);
    s->source=source;
    void* retain=*static_cast<void**>(source);void* old=s->retained;
    if(retain!=old) {if(retain) V<void,0>(retain);s->retained=retain;if(old) V<void,1>(old);}
    void* ib=M<void*,0x00F20C20>(source);
    void* vb=M<void*,0x0068EAB0>(source,0u);
    struct Format {uint8_t pad[0x18];uint8_t* elements;uint32_t count;};
    auto* f=M<Format*,0x0093ECC0>(source);s->disabled=1;
    result=reinterpret_cast<uintptr_t>(f);
    if(f->count<2) return result;
    auto* elements=f->elements;result=reinterpret_cast<uintptr_t>(elements);
    uint32_t i=0;for(;i<f->count;++i) if(elements[8*i+6]==1) break;
    if(i==f->count || elements[8*i+4]!=5) return result;
    uint16_t offset;std::memcpy(&offset,elements+8*i+2,2);
    uint32_t byteOffset=uint32_t(offset)+3;
    uint32_t vbOffset=M<uint32_t,0x0068EAC0>(source,0u);
    uint32_t stride=M<uint32_t,0x0068EAD0>(source,0u);
    auto* mesh=M<uint32_t*,0x0081C8D0>(source);std::memcpy(s->mesh,mesh,24);
    uint32_t low,high;
    M<void,0x00696910>(s,ib,s->mesh[2],s->mesh[5],&low,&high);
    s->vertices=high-low+1;
    uint32_t bytes=stride*s->vertices+6*s->mesh[5];
    s->instances=bytes>=0x2000?8:0x4000/bytes;
    if(s->instances>40) s->instances=40;
    s->bytes=bytes*s->instances;s->disabled=0;
    void* p=Allocate(sizeof(void*)==4?32:64,"Renderer/InstancingEmulationVB");
    s->vb=p?M<void*,0x0061A380>(p):nullptr;
    M<void,0x0061A1C0>(s->vb,stride*s->vertices*s->instances,0u,"InstancingEmulation");
    p=Allocate(sizeof(void*)==4?36:72,"Renderer/InstancingEmulationIB");
    s->ib=p?M<void*,0x00616EE0>(p):nullptr;
    M<void,0x00616DF0>(s->ib,3*s->instances*s->mesh[5],0u,0u);
    s->triangles=s->mesh[5];
    uint32_t size=stride*s->vertices;
    auto* dst=M<uint8_t*,0x00619E80>(s->vb,0u,0u,0u);
    auto* src=M<uint8_t*,0x00619E80>(vb,vbOffset+stride*low,size,4u);
    for(uint32_t j=0;j<s->instances;++j) {
        std::memcpy(dst,src,size);
        for(uint32_t k=0;k<s->vertices;++k) dst[byteOffset+k*stride]=uint8_t(j);
        dst+=size;
    }
    M<void,0x00616C10>(s->vb);M<void,0x00616C10>(vb);
    s->indices=3*s->mesh[5];
    auto* in=M<uint16_t*,0x00616BB0>(ib,2*s->mesh[2],2*s->indices,4u);
    auto* out=M<uint16_t*,0x00616BB0>(s->ib,0u,0u,0u);
    for(uint32_t j=0;j<s->instances;++j) {
        for(uint32_t k=0;k<s->indices;++k) out[k]=uint16_t(in[k]+j*uint16_t(s->vertices));
        out+=s->indices;
    }
    M<void,0x00616C10>(ib);return M<uintptr_t,0x00616C10>(s->ib);
}
Library* __fastcall InitLibrary(Library* s,void*) {
    s->begin=s->end=s->capacity=nullptr;s->tag="Renderer/BatcherLibrary";
    s->begin=static_cast<Batcher**>(Allocate(32*sizeof(Batcher*),s->tag));s->end=s->begin;s->capacity=s->begin+32;
    Registry* r=Reg();
    uint32_t a=M<uint32_t,0x0079A130>(r,300348620u,0u,2u);
    uint32_t b=M<uint32_t,0x0079A130>(r,1872076651u,0u,2u);
    ParameterValue(r,a,0,DefaultValue());ParameterValue(r,b,0,DefaultValue());return s;
}
bool __fastcall FindBatcher(Library* s,void*,void* source,Batcher** output) {
    for(auto** p=s->begin;p!=s->end;++p) if((*p)->source==source) {++(*p)->refs;*output=*p;return true;}
    void* memory=Allocate(sizeof(Batcher),"Renderer/InstancingBatcher");if(!memory) return false;
    auto* b=M<Batcher*,0x00696CF0>(memory);if(!b) return false;
    ++b->refs;SetSource(b,nullptr,source);
    if(s->end>=s->capacity) M<void,0x008621C0>(&s->begin,s->end,&b);
    else {auto** p=s->end++;if(p) *p=b;}
    s->bytes+=b->bytes;*output=b;return true;
}
Instance* __fastcall InitInstance(Instance* s,void*) {
    s->key=nullptr;s->count=0;s->batches=nullptr;std::memset(s->tail,0,sizeof(s->tail));
    auto** slot=LibrarySlot();
    if(!*slot) {
        void* p=Allocate(sizeof(Library),"Renderer/InstanceLibrary");auto* lib=p?InitLibrary(static_cast<Library*>(p),nullptr):nullptr;
        if(*slot) {auto* old=*slot;M<void,0x00697100>(old);F<void,0x004E3F70>(old);}*slot=lib;
    }
    return s;
}
uint8_t __fastcall BindInstance(Instance* s,void*,void* key,void* asset) {
    if(s->key==key) return uint8_t(reinterpret_cast<uintptr_t>(key));
    s->key=key;s->count=M<uint32_t,0x006C93D0>(asset);
    size_t bytes=s->count>0x3fffffff?size_t(-1):size_t(s->count)*sizeof(Batcher*);
    s->batches=static_cast<Batcher**>(Allocate(bytes,"Renderer/Batcher"));
    uint8_t result=uint8_t(reinterpret_cast<uintptr_t>(s->batches));
    struct Asset {uint8_t pad[0x40];void** sources;};
    for(uint32_t i=0;i<s->count;++i) {s->batches[i]=nullptr;result=FindBatcher(*LibrarySlot(),nullptr,static_cast<Asset*>(asset)->sources[i],s->batches+i);}
    return result;
}
uint32_t __fastcall ShadowFlag(void*,void*,void* asset) {return (M<uint32_t,0x006C93D0>(asset)>>21)&1;}
bool __fastcall EnsureFormat(ShadowPass* s,void*) {
    if(s->format) return true;
    void* p=Allocate(sizeof(void*)==4?36:72,"Renderer/InteriorShadow/VertexFormat");
    s->format=p?M<void*,0x0061A300>(p):nullptr;if(!s->format) return false;
    const uint8_t elements[24]={0,0,0,0,3,0,0,0,0,0,16,0,1,0,2,0,0,0,24,0,5,0,6,0};
    M<void,0x0061A010>(s->format,elements,3u);return true;
}
bool __fastcall EnsureBuffer(ShadowPass* s,void*) {
    if(s->buffer) return true;
    void* p=Allocate(sizeof(void*)==4?32:64,"Renderer/InteriorShadow/VertexBuffer");
    s->buffer=p?M<void*,0x0061A380>(p):nullptr;if(!s->buffer) return false;
    if(!s->geometry) return false; // EA retains the new buffer on this failure.
    struct Geometry {uint8_t pad[0x38];uint32_t a,b;};
    struct Format {uint8_t pad[0x20];uint32_t* stride;};
    auto* g=static_cast<Geometry*>(s->geometry);
    s->vertices=108*(g->a+g->b);
    uint32_t size=s->vertices * *static_cast<Format*>(s->format)->stride;
    if(!uint8_t(M<uintptr_t,0x0061A1C0>(s->buffer,size,1u,"InteriorShadow"))) {
        if(s->buffer) V<void,0>(s->buffer,1u);s->buffer=nullptr;return false;
    }
    return true;
}
bool __fastcall InitShadow(ShadowPass* s,void*) {
    V<void,18>(s);
    if(void* system=F<void*,0x006E8330>()) {M<void,0x006E84B0>(system,s,0u);s->geometry=M<void*,0x006E83F0>(system,0u);}
    if(!EnsureFormat(s,nullptr)||!EnsureBuffer(s,nullptr)) return false;
    s->shader=M<void*,0x0079B840>(Shaders(),"SimDropShadow",0u,0u);if(!s->shader) return false;
    Registry* registry=Reg();
    uintptr_t key[4]{};
#if defined(_M_IX86)
    void* type=reinterpret_cast<void*>(0x00B2D882);
#else
    void* type=nullptr;
#endif
    if(!uint8_t(F<uintptr_t,0x004AECE0>(key,L"GenericBlobShadow",type,0u,0u,0u))) return false;
    s->textureId=M<uint32_t,0x0079A160>(registry,"simBlobShadowTexture",1u,5u);
    M<void,0x00722F50>(s->texture,key,0u,0u,0u);return true;
}
SceneList* __fastcall InitList(SceneList* s,void*) {
    s->table=ListTable();s->left=s->right=&s->left;s->root=nullptr;s->flags=0;s->count=0;s->tag="Renderer/InteriorShadowManager/SceneObjects";return s;
}
uintptr_t __fastcall DestroyList(SceneList* s,void*) {
    s->table=ListTable();return M<uintptr_t,0x006A0850>(&s->untouched,s->root);
}
SceneList* __fastcall DeleteList(SceneList* s,void*,uint32_t flags) {
    DestroyList(s,nullptr);if(flags&1) F<void,0x004E3F70>(s);return s;
}
SceneObjects* __fastcall InitObjects(SceneObjects* s,void*) {
    InitList(s,nullptr);s->table=ListTable(true);s->begin=s->end=s->capacity=nullptr;
    s->renderTag="Renderer/InteriorShadowManager/RenderData";s->shader[0]=s->shader[1]=nullptr;
    void* manager=nullptr;
#if defined(_M_IX86)
    manager=*reinterpret_cast<void**>(0x011D753C);
#endif
    if(globals.shaders) manager=globals.shaders;
    if(manager) {
        void* shader=nullptr;M<void,0x0079B190>(manager,394646129u,&shader,0u);
        if(shader) {
            for(uint32_t i=0;i<2;++i) {
                uintptr_t variant=0;
                if(i==1) {
#if defined(_M_IX86)
                    void* tag=reinterpret_cast<void*>(0x00FF2E58);
#else
                    void* tag=nullptr;
#endif
                    variant=M<uintptr_t,0x0079FB20>(shader,0u,tag,1u);
                }
                s->shader[i]=M<void*,0x0079B790>(manager,394646129u,variant,0u);
            }
            if(shader) V<void,1>(shader);
        }
    }
    s->deferred=8;
    auto* options=Global<uint8_t*,0x011D1530>(globals.options);
    s->immediate=options[0x178]?4:0;
    s->depthId=M<uint32_t,0x0079A160>(Reg(),"SimShadowDepthScale",1u,1u);return s;
}
uintptr_t __fastcall RenderShadows(SceneObjects* s,void*) {
    uint32_t count=uint32_t(s->end-s->begin);
    if(!s->immediate) {
        uint32_t n=std::min(count,s->deferred);
        for(uint32_t i=0;i<n;++i) s->begin[i].dirty=1;
        return n?uint32_t(n*64):count;
    }
    uintptr_t result=reinterpret_cast<uintptr_t>(s->begin);
    if(s->begin==s->end || !s->shader[0]) return result;
    void* device=F<void*,0x0060E2C0>();result=M<uintptr_t,0x00611620>(device);if(!uint8_t(result)) return result;
    void* resources=Global<void*,0x011CECDC>(globals.resources);
    void* large=M<void*,0x00617540>(resources,0x7440b7a6u);
    void* depth=M<void*,0x00617500>(resources,0xcf5c8f42u);result=reinterpret_cast<uintptr_t>(depth);
    if(!large || !depth) return result;
    void** small=M<void**,0x00617540>(resources,0x76b9af5eu);
    void** target=M<void**,0x00617500>(resources,0x47f8c49au);
    alignas(16) uint8_t saved[48];M<void,0x0060E2E0>(device,saved);M<void,0x0060EC50>(device);
    M<void,0x0060EF20>(device,0u,*target);
    struct Device {uint8_t pad[0x38];uint32_t depth;};
    uintptr_t d=static_cast<Device*>(device)->depth?0:M<uintptr_t,0x0060EE40>(device);
    M<void,0x0060EF70>(device,d);
    struct Texture {uint8_t pad[0x14];uint32_t width,height;};
    auto* t=static_cast<Texture*>(*small);uint32_t viewport[4]={0,0,t->width,t->height};
    M<void,0x0060FFE0>(device,viewport,0.0f,1.0f);M<void,0x0060F270>(device,7u,0u,1.0f,0u);
    uint32_t tile[4]={0,0,128,128};M<void,0x0060FFE0>(device,tile,0.0f,1.0f);
    M<void,0x0060F7E0>(device,0u);M<void,0x0060F7B0>(device,1u);M<void,0x0060F790>(device,1u);
    M<void,0x0060F7C0>(device,3u);M<void,0x0060F770>(device,1u);
    uint32_t n=std::min(count,s->immediate);
    for(uint32_t i=0;i<n;++i) {
        uintptr_t surface;uint32_t handle;F<void,0x006975A0>(i,&surface,&handle);
        M<void,0x0060F750>(device,surface);auto* r=s->begin+i;r->handle=handle;r->active=1;
        M<void,0x00698AE0>(s,r,r->object);M<void,0x0060F270>(device,2u,0u,1.0f,0u);
    }
    for(uint32_t i=n,stop=std::min(count,s->immediate+s->deferred);i<stop;++i) s->begin[i].dirty=1;
    auto* l=static_cast<Texture*>(*static_cast<void**>(large));
    alignas(16) float dimensions[4]={float(t->width),float(t->height),float(l->width),float(l->height)};
    auto* registry=Reg();auto& flags=DimensionFlags();auto& id=DimensionId();
    if(!(flags&1)) {flags|=1;id=M<uint32_t,0x0079A160>(registry,"ScratchTextureDimensions",1u,2u);}
    ParameterValue(registry,id,1,dimensions);
    M<void,0x0079A2D0>(s->shader[0],2u);M<void,0x0079A300>(s->shader[0],0u,0u);
    M<void,0x00C0FD60>(device);M<void,0x0060FCE0>(device);return M<uintptr_t,0x0060E410>(device,saved);
}
}
