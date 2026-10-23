#include "ts3/render/m07_M02.h"
#include <cstring>
#include <intrin.h>
namespace ts3::render::m07::m02 {
Dependencies dependencies;
namespace {
template<class F> F Fn(uint32_t a) { return Original(a,dependencies.function?reinterpret_cast<F>(dependencies.function(a)):nullptr); }
void* Global(uint32_t a) {
    if(dependencies.global) return dependencies.global(a);
#if defined(_M_IX86)
    return reinterpret_cast<void*>(uintptr_t(a));
#else
    throw std::runtime_error("m07_M02: inject native globals");
#endif
}
template<class T> T& G(uint32_t a) { return *static_cast<T*>(Global(a)); }
void Ref(void* p,int slot) { if(p) CallThis(reinterpret_cast<Thiscall<void,void>>((*static_cast<void***>(p))[slot]),p); }
void Assign(void*& d,void* s,int add,int release,bool equality) {
    if(equality && d==s) return;
    Ref(s,add); void* old=d; d=s; Ref(old,release);
}
void* Allocate(uint32_t n,const char* tag) {
    return Fn<void* (__cdecl*)(uint32_t,const char*,uint32_t,uint32_t,uint32_t,uint32_t)>(0x004E3F90)(n,tag,0,0,0,0);
}
void Attach(void* owner,void* p,bool detach) {
    CallThis(Fn<Thiscall<void,void,void*>>(detach?0x006DEDE0:0x006DED60),static_cast<void*>(static_cast<uint8_t*>(owner)+40),p);
}
void* Sub40(void* p) { return p?static_cast<uint8_t*>(p)+40:nullptr; }
void DropMediator(Mediator* p) {
    if(p && --p->refs==0) {
        p->refs=1;
        CallThis(reinterpret_cast<Thiscall<void,void,int>>(p->table[0]),static_cast<void*>(&p->table),1);
    }
}
void Clear(RefArray& a) {
    CallThis(Fn<Thiscall<void,RefArray,Mediator**,Mediator**>>(0x006CC960),&a,a.begin,a.end); a.end=a.begin;
}
void Push(RefArray& a,Mediator* p) {
    ++p->refs; Mediator* tmp=p;
    if(reinterpret_cast<uintptr_t>(a.end)>=reinterpret_cast<uintptr_t>(a.capacity))
        CallThis(Fn<Thiscall<void,RefArray,Mediator**,Mediator**>>(0x006CD6C0),&a,a.end,&tmp);
    else { auto* at=a.end++; if(at) { *at=p; ++p->refs; } }
    DropMediator(tmp);
}
struct Iterator { Entry* entry; Entry** bucket; };
void Erase(Compositor* s,Iterator it) {
    Iterator out{};
    CallThis(Fn<Thiscall<void,void,Iterator*,Iterator>>(0x006CC2E0),static_cast<void*>(s->buckets),&out,it);
}
void Unlink(Node* n) { n->prev->next=n->next; n->next->prev=n->prev; }
}
uint32_t __fastcall AssignDrawable(Drawable* d,void*,const Drawable* s) {
    for(int i=0;i<2;++i) {
        auto word=s->streamWords[i]; auto* p=s->streams[i];
        Ref(p,1); Ref(d->streams[i],2); d->streams[i]=p; d->streamWords[i]=word;
    }
    // These four assignments deliberately do NOT suppress self-assignment refs.
    auto* p=s->vertexFormat; Ref(p,1); Ref(d->vertexFormat,2); d->vertexFormat=p;
    p=s->indices; Ref(p,1); Ref(d->indices,2); d->indices=p;
    d->word1C=s->word1C; Assign(d->resource,s->resource,1,2,true);
    p=s->atomicResource; auto* old=d->atomicResource;
    if(p!=old) {
        if(p) _InterlockedExchangeAdd(reinterpret_cast<volatile long*>(static_cast<uint8_t*>(p)+sizeof(void*)),1);
        d->atomicResource=p;
        if(old) {
            auto* count=reinterpret_cast<volatile long*>(static_cast<uint8_t*>(old)+sizeof(void*));
            if(!_InterlockedDecrement(count)) {
                _InterlockedExchange(count,1);
                CallThis(reinterpret_cast<Thiscall<void,void,int>>((*static_cast<void***>(old))[0]),old,1);
            }
        }
    }
    if(d->ref2C!=s->ref2C) {
        if(s->ref2C) { Ref(s->ref2C,0); if(d->attached) Attach(d->attached,Sub40(s->ref2C),false); }
        if(d->ref2C) { if(d->attached) Attach(d->attached,Sub40(d->ref2C),true); Ref(d->ref2C,1); }
        d->ref2C=s->ref2C;
    }
    Assign(d->ref28,s->ref28,0,1,true);
    p=s->ref30;
    if(d->ref30!=p) {
        if(p) { Ref(p,0); if(d->attached) Attach(d->attached,Sub40(p),false); }
        if(d->ref30) { if(d->attached) Attach(d->attached,Sub40(p),true); Ref(d->ref30,1); }
        d->ref30=p; // EA detaches the NEW pointer above, not the old one.
    }
    p=s->ref34; Ref(p,0); Ref(d->ref34,1); d->ref34=p;
    for(int i=0;i<6;++i) d->words38[i]=s->words38[i];
    uint32_t count=s->jointCount;
    CallThis(Fn<Thiscall<void,Drawable,uint32_t>>(0x0068EBA0),d,count);
    for(uint32_t i=0;i<count;++i) if(s->joints && int32_t(i)<int32_t(s->jointCount) && d->joints && int32_t(i)<int32_t(d->jointCount))
        for(int j=0;j<5;++j) d->joints[i].words[j]=s->joints[i].words[j];
    CallThis(Fn<Thiscall<void,Drawable,void*,uint32_t>>(0x0068ED50),d,s->map,s->mapCount);
    if(s->rig) {
        d->rig=Allocate(d->mapCount>0x3fffffff?0xffffffffu:4*d->mapCount,"Renderer/Drawable/JointMapRig");
        if(d->rig) std::memcpy(d->rig,s->rig,uint32_t(4*d->mapCount));
    }
    p=s->ref64; Ref(p,0); Ref(d->ref64,1); d->ref64=p;
    d->words68[0]=s->words68[0]; d->words68[1]=s->words68[1];
    // Byte-wise read/modify/write is significant for aliased sources.
    for(int byte=0;byte<3;++byte) for(int bit=0;bit<(byte==2?4:8);++bit)
        d->flags[byte]^=(d->flags[byte]^s->flags[byte])&(1<<bit);
    d->word74=s->word74; d->word78=s->word78; d->word7C=s->word7C;
    for(int i=0;i<4;++i) std::memcpy(d->matrices+i*16,s->matrices+i*16,16);
    d->wordC4=s->wordC4; d->wordC8=s->wordC8; return d->wordC4;
}
uintptr_t InitializeProbes() {
    auto& refs=G<uint32_t>(0x011D0D10); refs+=1;
    if(int32_t(refs)>1) return refs;
    G<uint32_t>(0x011D0D14)=0x8000;
    void* allocator=G<void*>(0x011CB864);
    auto* table=CallThis(Fn<Thiscall<float*,void,uint32_t,uint32_t,uint32_t,uint32_t>>(0x004E6A20),allocator,0x280000u,16u,0u,0u);
    G<float*>(0x011D0D44)=table;
    uint32_t scratch; float scale;
    auto* weights=Fn<float* (__cdecl*)(uint32_t,void*,float*)>(0x0070CA60)(128,&scratch,&scale);
    for(uint32_t face=0;face<2;++face) for(uint32_t y=0;y<128;++y) for(uint32_t x=0;x<128;++x) {
        alignas(16) float direction[4];
        // Original fild/fmul/fstp rounds only after the extended-precision product.
        float u=float(double(x)*double(0.0078740157f)),v=float(double(y)*double(0.0078740157f));
        Fn<void (__cdecl*)(float,float,uint32_t,float*)>(0x0070CBA0)(u,v,face,direction);
        Fn<void (__cdecl*)(float*,uint32_t,float*)>(0x0070CE90)(direction,4,table);
        float w=weights[y*128+x]*scale;
        for(int k=16;k<20;++k) table[k]=w;
        table+=20;
    }
    CallThis(Fn<Thiscall<void,void,void*>>(0x004E5920),allocator,static_cast<void*>(weights));
    Fn<void (__cdecl*)()>(0x006B44E0)(); Fn<void (__cdecl*)()>(0x006B49F0)();
    const uint32_t slots[]={0x011D0D38,0x011D0D30,0x011D0D2C};
    const char* tags[]={"Lighting/ParabolicBufferSysMem","Lighting/CubeStripSysMem","Lighting/CubeFaceExtractorSysMem"};
    const uint32_t widths[]={128,32,64},heights[]={256,192,64};
    for(int i=0;i<3;++i) {
        void* p=Allocate(52,tags[i]);
        if(p) p=CallThis(Fn<Thiscall<void*,void>>(0x00619BC0),p);
        Assign(G<void*>(slots[i]),p,1,2,true);
        CallThis(Fn<Thiscall<void,void,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,const char*>>(0x00619C80),G<void*>(slots[i]),widths[i],heights[i],1u,i==2?61u:62u,256u,"Lprobe");
    }
    G<void*>(0x011D0D34)=CallThis(Fn<Thiscall<void*,void,uint32_t,uint32_t>>(0x004E69D0),allocator,0x40000u,0u);
    void* vf=Allocate(36,"Lighting/CubeVF");
    if(vf) vf=CallThis(Fn<Thiscall<void*,void>>(0x0061A300),vf);
    Assign(G<void*>(0x011D0D40),vf,1,2,true);
    struct Element { uint16_t stream,offset; uint8_t type,method,usage,index; } elements[6]{};
    for(int i=0;i<6;++i) { elements[i].offset=uint16_t(i<4?i*16:(i==4?64:68)); elements[i].type=uint8_t(i<4?3:(i==4?6:2)); elements[i].usage=uint8_t(i<4?2:(i==4?0:1)); elements[i].index=uint8_t(i<4?i:0); }
    CallThis(Fn<Thiscall<void,void,void*,uint32_t>>(0x0061A010),G<void*>(0x011D0D40),static_cast<void*>(elements),6u);
    G<void*>(0x011D0D3C)=CallThis(Fn<Thiscall<void*,void,const char*,uint32_t,uint32_t>>(0x0079B840),G<void*>(0x011D753C),"lightprobe",0u,0u);
    auto* manager=G<ParamManager*>(0x011D7530);
    auto param=[&](uint32_t a,const char* name,uint32_t n,uint32_t type) {
        uint32_t id=CallThis(Fn<Thiscall<uint32_t,ParamManager,const char*,uint32_t,uint32_t>>(0x0079A160),manager,name,n,type); G<uint32_t>(a)=id; return id;
    };
    param(0x011D0D48,"convolveWeights",4,2); param(0x011D0D4C,"diffuseProbeWeights",16,2);
    param(0x011D0D54,"paraboloidLookupTexture",1,5); param(0x011D0D58,"cubeMapTexture",1,5);
    param(0x011D0D50,"AmbientDomeExterior",1,1); param(0x011D0D5C,"DiffuseLightProbe",1,5);
    param(0x011D0D60,"SpecularLightProbe",1,5); param(0x011D0D64,"SharpSpecularLightProbe",1,5);
    param(0x011D0DA0,"ExteriorSpecularLightProbe",1,5); auto id=param(0x011D0DA4,"ExteriorDiffuseLightProbe",1,5);
    auto bind=[&](uint32_t n,void* value) { if(n<manager->count) { auto& e=manager->entries[n]; ++e.refs; e.revision=manager->revision; e.value=value; } };
    bind(id,nullptr); bind(G<uint32_t>(0x011D0DA0),nullptr);
    param(0x011D0D68,"SpecularProbeMultiplierBottom",1,2); id=param(0x011D0D80,"SpecularProbeMultiplierTop",1,2);
    bind(G<uint32_t>(0x011D0D68),Global(0x011D0D70)); bind(id,Global(0x011D0D90));
    uint32_t keys[2][4]{}; uintptr_t result=0;
    const uint32_t names[]={0x00FF3E50,0x00FF3E30},dest[]={0x011D0DA8,0x011D0DC0};
    for(int i=0;i<2;++i) {
        Fn<void (__cdecl*)(void*,const void*,void*,uint32_t,uint32_t,uint32_t)>(0x004AECE0)(keys[i],Global(names[i]),Global(0x00B2D882),0,0,0);
        result=CallThis(Fn<Thiscall<uintptr_t,void,void*,uint32_t,uint32_t,uint32_t>>(0x00722F50),Global(dest[i]),static_cast<void*>(keys[i]),0u,0u,0u);
    }
    return result;
}
Mediator** __fastcall UpdateCompositor(Compositor* s,void*) {
    CallThis(Fn<Thiscall<void,Compositor>>(0x006CD380),s);
    CallThis(Fn<Thiscall<void,Compositor>>(0x006CDD70),s); s->pending=0;
    auto& flags=G<uint32_t>(0x011D13D0);
    const uint32_t addresses[]={0x011D13C0,0x011D13B0,0x011D13A0};
    const uint32_t tags[]={0x00FF5E0C,0x00FF5DD0,0x00FF5D94},dtors[]={0x00F92300,0x00F922D0,0x00F922A0};
    for(int i=0;i<3;++i) {
        auto& a=G<RefArray>(addresses[i]);
        if(!(flags&(1u<<i))) { flags|=1u<<i; a={nullptr,nullptr,nullptr,static_cast<const char*>(Global(tags[i]))}; Fn<int (__cdecl*)(void*)>(0x00F7E9A1)(reinterpret_cast<void*>(Fn<void (__cdecl*)()>(dtors[i]))); }
        Clear(a);
    }
    auto& starts=G<RefArray>(addresses[0]); auto& refs=G<RefArray>(addresses[1]); auto& releases=G<RefArray>(addresses[2]);
    Node* n=s->active.next;
    while(n!=&s->active) {
        auto* e=n?reinterpret_cast<Entry*>(reinterpret_cast<uint8_t*>(n)-offsetof(Entry,node)):nullptr;
        auto* m=e->mediator;
        if(m && m->state==6) { Push(starts,m); n=n->next; continue; }
        if(m && m->pending>0) { s->pending+=CallThis(Fn<Thiscall<uint32_t,Mediator>>(0x006CC210),m); n=n->next; continue; }
        if(m && e->release) {
            Push(releases,m); auto* old=n; n=n->next; Unlink(old);
            e->node.prev=s->parked.prev; e->node.next=&s->parked; s->parked.prev=&e->node; e->node.prev->next=&e->node; e->enabled=0; continue;
        }
        if(m) Push(refs,m);
        auto* old=n; n=n->next; Unlink(old);
        Iterator it{};
        if(m) {
            CallThis(Fn<Thiscall<void,void,Iterator*,const uint32_t*>>(0x006CC840),static_cast<void*>(s->buckets),&it,static_cast<const uint32_t*>(e->key));
            Erase(s,it);
        } else {
            uint32_t hash=(e->key[3]+e->key[1]+e->key[0])%521;
            auto* found=s->buckets[hash];
            while(found && (found->key[0]!=e->key[0] || found->key[2]!=e->key[2] || found->key[3]!=e->key[3] || found->key[1]!=e->key[1])) found=found->next;
            it=found?Iterator{found,&s->buckets[hash]}:Iterator{s->sentinel,&s->sentinel};
            if(it.entry!=s->sentinel) Erase(s,it);
        }
        DropMediator(e->mediator); Fn<void (__cdecl*)(void*)>(0x004E3F70)(e);
    }
    for(auto p=starts.begin,end=starts.end;p!=end;++p) {
        auto* m=*p; auto* builder=m->builder; auto k0=m->key0,k1=m->key1; int selector=m->selector; m->state=3;
        auto* service=Fn<void* (__cdecl*)()>(0x006079C0)();
        CallThis(Fn<Thiscall<void,void,Mediator*,void*,uint32_t,uint32_t,uint32_t,int>>(0x006CD090),service,m,builder,0u,k0,k1,selector);
        Ref(m->builder,1); m->builder=nullptr;
    }
    for(auto p=releases.begin,end=releases.end;p!=end;++p) {
        auto* m=*p; m->state=1;
        auto* service=G<void*>(0x011D3C84);
        if(service) CallThis(reinterpret_cast<Thiscall<void,void,void**,void*>>((*static_cast<void***>(service))[15]),service,&m->tagged,reinterpret_cast<void*>(Fn<void (__cdecl*)()>(0x00D4B150)));
        if(m->tagged && !(reinterpret_cast<uintptr_t>(m->tagged)&3)) { Ref(m->tagged,1); m->tagged=nullptr; }
        m->tagged=nullptr;
        if(m->texture) { void* old=m->texture; m->texture=nullptr; Ref(old,2); }
    }
    Clear(refs); return refs.begin;
}
}
