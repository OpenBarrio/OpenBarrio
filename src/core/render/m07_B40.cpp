#include "ts3/render/m07_B40.h"
#include <cstring>
#include <cmath>
#include <xmmintrin.h>
#include <intrin.h>
namespace ts3::render::m07::b40 {
Resolver resolve=nullptr;
namespace {
template<class R,uint32_t A,class... T> R M(void* s,T... a) {
    if(resolve) if(auto p=resolve(A)) return reinterpret_cast<R(__fastcall*)(void*,void*,T...)>(p)(s,nullptr,a...);
    return CallGameMethod<R,A>(s,a...);
}
template<class R,uint32_t A,class... T> R F(T... a) {
    if(resolve) if(auto p=resolve(A)) return reinterpret_cast<R(__cdecl*)(T...)>(p)(a...);
    return CallGameFunction<R,A>(a...);
}
template<class R,int S,class... T> R V(void* s,T... a) {
    return reinterpret_cast<R(__fastcall*)(void*,void*,T...)>((*static_cast<void***>(s))[S])(s,nullptr,a...);
}
void* Data(uint32_t a) {
    if(resolve) if(auto p=resolve(a)) return p;
#if defined(_M_IX86)
    return reinterpret_cast<void*>(a);
#else
    (void)a; AbortNotRewritten("m07_B40 native image datum");return nullptr;
#endif
}
const char* Tag(uint32_t a,const char* native) {
#if defined(_M_IX86)
    (void)native;return static_cast<const char*>(Data(a));
#else
    (void)a;return native;
#endif
}
Swarm* Parent(void* p,size_t offset) {return reinterpret_cast<Swarm*>(static_cast<uint8_t*>(p)-offset);}
uintptr_t Low(uintptr_t r,uint8_t b) {return (r&~uintptr_t(255))|b;}
uint32_t MeshCount(void* p) {
    if(resolve) if(auto f=resolve(0x00724F80)) return reinterpret_cast<uint32_t(__cdecl*)(void*)>(f)(p);
#if defined(_M_IX86)
    uint32_t r;const uintptr_t f=0x00724F80;
    __asm {
        push edi
        mov edi,p
        call f
        mov r,eax
        pop edi
    }
    return r;
#else
    AbortNotRewritten("m07_B40 EDI mesh-count bridge");return 0;
#endif
}
void* Color(Visualizer* p,float v) {
    if(resolve) if(auto f=resolve(0x00724FC0)) return reinterpret_cast<void*(__cdecl*)(Visualizer*,float)>(f)(p,v);
#if defined(_M_IX86)
    void* r;const uintptr_t f=0x00724FC0;
    __asm {
        mov eax,p
        movss xmm0,v
        call f
        mov r,eax
    }
    return r;
#else
    AbortNotRewritten("m07_B40 EAX/XMM0 color bridge");return nullptr;
#endif
}
float Density(uint32_t n,float a,float b,float scale,bool projected) {
#if defined(_M_IX86)
    // Count is unsigned. Keep the division/multiplication in x87 until the final store.
    float r;double count=double(n);
    if(projected) {
        __asm {
            fld qword ptr count
            fld dword ptr a
            fmul dword ptr b
            fdivp st(1),st(0)
            fmul dword ptr scale
            fstp dword ptr r
        }
    } else {
        __asm {
            fld qword ptr count
            fdiv dword ptr a
            fmul dword ptr scale
            fstp dword ptr r
        }
    }
    return r;
#else
    return float(projected?(double(n)/(double(a)*b))*scale:(double(n)/a)*scale);
#endif
}
}
uint32_t __fastcall Release(Swarm* s,void*) {
    uint32_t r=s->refs-1;
    s->refs=r;
    if(r==0) {
        s->refs=1;
        V<void,0>(&s->lifetime,1u);
        return 0;
    }
    return r;
}
uint32_t __fastcall ReleaseSecondary(void* s,void*) {return Release(Parent(s,offsetof(Swarm,secondary)),nullptr);}
uint32_t __fastcall AddLifetime(void* s,void*) {return M<uint32_t,0x00720B50>(Parent(s,offsetof(Swarm,lifetime)));}
uint32_t __fastcall AddSecondary(void* s,void*) {return M<uint32_t,0x00720B50>(Parent(s,offsetof(Swarm,secondary)));}
uint32_t __fastcall ReleaseLifetime(void* s,void*) {return Release(Parent(s,offsetof(Swarm,lifetime)),nullptr);}
uintptr_t __fastcall Convert(void* s,void*,uint32_t type,void* src,void* out) {
    return M<uintptr_t,0x0071EB30>(Parent(s,offsetof(Swarm,secondary)),type,src,out);
}
uintptr_t __fastcall Destroy(Swarm* s,void*) {
    if(s->bufferD0) F<uintptr_t,0x004E3F70>(s->bufferD0);
    if(s->bufferC0) F<uintptr_t,0x004E3F70>(s->bufferC0);
    M<uintptr_t,0x0071E7C0>(&s->vectorUnknown,s->items,s->capacity);
    s->size=0;
    if(s->capacity>1) F<uintptr_t,0x004E3F70>(s->items);
    s->lifetime=static_cast<void**>(Data(0x00FF5F1C));
    return M<uintptr_t,0x00720530>(&s->secondary);
}
void* __fastcall Query(Fixup* s,void*,uint32_t id) {
    // Adapted from the verified PackedFileRecord query twin; only IDs differ.
    if(id==0xee3f516eu) return s;
    if(id==0x0452ac14u) return s;
    return nullptr;
}
uintptr_t __fastcall Apply(Fixup* s,void*,Material* material,Material** out,void* context,void* arg) {
    Binding records[3];uint32_t count=0;
    auto texture=[&](const Key& k,const char* name) {
        Binding& r=records[count++];r.type=4;r.key=F<uint32_t,0x007CFE70>(name);r.texture=k;
    };
    if(s->diffuse.words[2]) texture(s->diffuse,"diffusemap");
    if(s->specular.words[2]) texture(s->specular,"specularmap");
    if(s->flags&2) {
        Binding& r=records[count++];r.type=1;r.key=F<uint32_t,0x007CFE70>("AlphaMaskThreshold");
        std::memset(&r.texture,0,sizeof(r.texture));r.value=Data(0x0115B85C);r.count=1;
    }
    if(count==0) {*out=material;return Low(V<uintptr_t,0>(material),1);}
    Substitution params;M<void,0x006DB3B0>(&params);
    params.bindings=records;params.count=count;params.context=context;params.id=0xc09c7582;
    params.alpha=(s->flags&2)!=0;params.option=s->flags&1;
    Material* selected=nullptr;
    bool ok;
    if(material->editable) {
        V<uintptr_t,0>(material);selected=material;
        M<void,0x006DB480>(material,material,&params,arg);ok=true;
    } else ok=uint8_t(M<uintptr_t,0x006DB6E0>(material,&selected,&params,arg))!=0;
    uintptr_t r;
    if(ok) {
        *out=selected;V<uintptr_t,0>(selected);
        if(selected) V<uintptr_t,1>(selected);
        r=M<uintptr_t,0x007D3AF0>(&params.unknown,params.buckets,params.capacity);
        params.size=0;
        if(params.capacity>1) r=F<uintptr_t,0x004E3F70>(params.buckets);
    } else {
        if(selected) V<uintptr_t,1>(selected);
        r=M<uintptr_t,0x00687440>(&params);
    }
    return Low(r,ok?1:0);
}
uintptr_t __fastcall Describe(Fixup* s,void*,void* out) {
    String spec,diff;
    const char* b=F<String*,0x007D9380>(&spec,&s->specular)->begin;
    const char* a=F<String*,0x007D9380>(&diff,&s->diffuse)->begin;
    F<uintptr_t,0x00567460>(out,"TexCompFixupLOD - Diffuse:%s Specular:%s",a,b);
    if(intptr_t(diff.capacity)-intptr_t(diff.begin)>1 && diff.begin) F<uintptr_t,0x004E3F70>(diff.begin);
    uintptr_t r=reinterpret_cast<uintptr_t>(spec.begin);
    if(intptr_t(spec.capacity)-intptr_t(spec.begin)>1 && spec.begin) r=F<uintptr_t,0x004E3F70>(spec.begin);
    return r;
}
void* __cdecl Visualize(void* mesh,uint32_t mode,Visualizer* out) {
    void* manager=*static_cast<void**>(Data(0x011D753C));
    out->shader=M<void*,0x0079B840>(manager,"colorVisualizer",0u,0u);out->zero=0;
    alignas(16) float extent[4];F<void*,0x007252C0>(extent,mesh);
    uint32_t n=MeshCount(mesh);float density=0;
    if(mode==1) density=Density(n,float(std::ceil(double(extent[0]))),float(std::ceil(double(extent[2]))),*static_cast<float*>(Data(0x0115B92C)),true);
    else if(mode==2) {
        __m128 x=_mm_set1_ps(extent[0]),y=_mm_set1_ps(extent[1]),z=_mm_set1_ps(extent[2]);
        float area=_mm_cvtss_f32(_mm_add_ps(_mm_add_ps(_mm_mul_ps(z,y),_mm_mul_ps(y,x)),_mm_mul_ps(z,x)));
        density=Density(n,area,0,*static_cast<float*>(Data(0x0115B928)),false);
    }
    out->enabled=1;return Color(out,density);
}
ReflectionMap* __fastcall InitReflection(ReflectionMap* s,void*) {
    s->load1=1;s->load2=2;s->tag=Tag(0x00FFD154,"Renderer/WaterReflectionManager/mReflectionMarkerMap");
    s->capacity=1;s->buckets=Data(0x0114C900);s->size=0;s->storage=nullptr;s->next=nullptr;
    s->flag=0;s->state=0;for(auto& p:s->entries) p=nullptr;return s;
}
uintptr_t __cdecl RegisterTree() {
    void* registry=*static_cast<void**>(Data(0x011CE3F4));
    uintptr_t r=M<uintptr_t,0x0059EFE0>(registry,0x87434d8du);
    if(uint8_t(r)) return r;
    void** p=F<void**,0x004E3F90>(16u,"Renderer/SpeedTree/Service",0u,0u,0u,0u);
    if(p) {M<void,0x0059ECC0>(p,0x87434d8du,0u);*p=Data(0x00FFB6C4);}
    return M<uintptr_t,0x0059F500>(registry,p,0u);
}
Tree* __fastcall InitTree(Tree* s,void*,void* owner,void* arg) {
    for(auto& v:s->lists) {v.begin=nullptr;v.end=nullptr;v.capacity=nullptr;v.tag=Tag(0x00FFBE94,"Renderer/SpeedTreeRenderer");}
    M<void,0x007B81C0>(s->helper);s->owner=owner;s->argument=arg;s->zero=0;
    for(auto& p:s->empty) p=nullptr;
#if defined(_M_IX86) || defined(_M_X64)
    _InterlockedExchange(reinterpret_cast<volatile long*>(&s->atomic),0);
#else
    s->atomic=0;
#endif
    s->flags[0]=0;s->flags[1]=0;return s;
}
}
