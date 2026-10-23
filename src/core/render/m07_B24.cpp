#include "ts3/render/m07_B24.h"
#include <cstring>
#include <intrin.h>
namespace ts3::render::m07::b24 {
void* (*resolve)(uint32_t)=nullptr;
namespace {
void* Address(uint32_t a) { if(resolve) return resolve(a); return Original<void*>(a); }
template<class R,class T,class... A> R M(uint32_t a,T* self,A... args) {
    return reinterpret_cast<Thiscall<R,T,A...>>(Address(a))(self,nullptr,args...);
}
template<class R,class... A> R F(uint32_t a,A... args) { return reinterpret_cast<R(__cdecl*)(A...)>(Address(a))(args...); }
template<class R,class T,class... A> R V(T* self,unsigned slot,A... args) {
    return reinterpret_cast<Thiscall<R,T,A...>>((*reinterpret_cast<void***>(self))[slot])(self,nullptr,args...);
}
void Free(void* p) { F<uintptr_t>(0x004E3F70,p); }
void* Alloc(size_t n,uint32_t tag) { return F<void*>(0x004E3F90,n,Address(tag),0u,0u,0u,0u); }
void BaseInit(Resource& b,uint32_t table) {
    b.table=static_cast<void**>(Address(0x00FB2C04)); _InterlockedExchange(&b.refs,0);
    for(auto& p:b.fields) p=nullptr;
    b.table=static_cast<void**>(Address(table));
}
void* NewTexture() {
    void* p=F<void*>(0x004E3FB0,size_t(72),4u,0u,Address(0x00FF5A14),0u,0u,0u,0u);
    if(p) { p=M<void*>(0x006CEF60,p); if(p) V<uintptr_t>(p,0); }
    return p;
}
uintptr_t ClearState(Definition* s,uintptr_t result) {
    if(s->cached) { result=M<uintptr_t>(0x0079FA50,s->cached); s->cached=nullptr; }
    if(s->parameters) {
        M<void>(0x006CAD50,s);
        result=M<uintptr_t>(0x004E5920,*static_cast<void**>(Address(0x011CB864)),s->parameters);
        s->parameters=nullptr;
    }
    s->word28=nullptr; s->word34=nullptr; s->word2c=nullptr;
    s->flags[0]=0; s->flags[1]=0;
    return (result&~uintptr_t(255))|1;
}
}
Items* __fastcall InitItems(Items* s,void*) {
    M<void>(0x006DEA60,s->helper); BaseInit(s->base,0x00FF5A54);
    s->state34=nullptr;s->state38=nullptr;s->state3c=nullptr;s->state40=nullptr;s->state44=nullptr;s->depth=0;
    s->next=&s->next;s->prev=&s->next;
    for(auto& p:s->words54) p=nullptr;
    s->begin=nullptr;s->end=nullptr;s->capacity=nullptr;
    s->tag=static_cast<const char*>(Address(0x00FF5A34));s->selector=~0u;return s;
}
uintptr_t __fastcall SetTexture(Items* s,void*,Entry* e,void* value) {
    uint32_t key=e->key; e->value=value;e->type=6;
    const auto removed=M<uint8_t>(0x006C9840,s,key);
    void* texture=NewTexture();
    auto r=M<uintptr_t>(0x006C9620,s,e,texture,value);
    if(removed && --s->depth==0) r=M<uintptr_t>(0x006DEA70,s->helper,3);
    if(texture) r=V<uintptr_t>(texture,1);
    return r;
}
uintptr_t __fastcall ResolveTextures(Items* s,void*,void* arg) {
    M<uintptr_t>(0x006DEA70,s->helper,2);s->depth=1;
    Entry* end=s->end;
    for(Entry* e=s->begin;e!=end;) {
        if(e->type==6 && e->value) {
            void* p=NewTexture();M<void>(0x006C9620,s,e,p,e->value);if(p) V<void>(p,1);
        }
        if(e->type==4) {
            auto* resource=static_cast<uint32_t*>(e->value);
            // EA bug: this branch deliberately skips the iterator increment.
            if(resource[2]==~0u) continue;
            void* p=NewTexture();M<void>(0x006C9BA0,s,e,p,resource,arg);
            if(resource[2]==0x8ffb80f6) {
                void* service=F<void*>(0x009E5310,resource);
                s->selector=M<uint32_t>(0x009E63A0,service);
            }
            if(p) V<void>(p,1);
        }
        ++e;
    }
    auto r=M<uintptr_t>(0x006C9520,s);
    if(--s->depth==0) r=M<uintptr_t>(0x006DEA70,s->helper,3);
    return r;
}
uintptr_t __fastcall UnloadItems(Items* s,void*) { return (M<uintptr_t>(0x006C9FF0,s)&~uintptr_t(255))|1; }
Items* __fastcall DeleteItems(Items* s,void*,uint32_t flags) {
    s->base.table=static_cast<void**>(Address(0x00FF5A54));M<void>(0x006C9FF0,s);
    if(s->begin) Free(s->begin);
    M<void>(0x00731CF0,s);M<void>(0x00C0FD60,s->helper);
    if(flags&1) Free(s);return s;
}
uint8_t __fastcall GetMaterial(Definition* s,void*,Items** out,void* arg) {
    if(!s->cached) {
        auto* p=static_cast<Items*>(Alloc(sizeof(Items),0x00FF5A98));
        s->cached=p?InitItems(p,nullptr):nullptr;if(!s->cached) return 0;
        M<void>(0x005044C0,s->cached);
        if(!M<uint8_t>(0x006CAB20,s->cached,s,arg)) { M<void>(0x0079FA50,s->cached);return 0; }
    }
    *out=s->cached;V<void>(s->cached,0);return 1;
}
#if defined(_M_IX86)
extern "C" uintptr_t __cdecl M07B24Clear(Definition* s,uintptr_t incoming) { return ClearState(s,incoming); }
__declspec(naked) uintptr_t __fastcall ClearDefinition(Definition*,void*) {
    __asm { push eax }
    __asm { push ecx }
    __asm { call M07B24Clear }
    __asm { add esp,8 }
    __asm { ret }
}
#else
uintptr_t __fastcall ClearDefinition(Definition* s,void*) { return ClearState(s,0); }
#endif
uintptr_t __fastcall DestroyDefinition(Definition* s,void*) {
    s->base.table=static_cast<void**>(Address(0x00FF5AAC));ClearState(s,0);
    return M<uintptr_t>(0x00731CF0,s);
}
Definition* __fastcall DeleteDefinition(Definition* s,void*,uint32_t flags) {
    DestroyDefinition(s,nullptr);if(flags&1) Free(s);return s;
}
Env* __fastcall InitEnv(Env* s,void*) {
    s->begin=nullptr;s->end=nullptr;s->capacity=nullptr;s->tag=static_cast<const char*>(Address(0x00FF5AF0));
    s->list.Init();return s;
}
Binding* __fastcall InitBinding(Binding* s,void*) {
    s->table=static_cast<void**>(Address(0x0107396C));_InterlockedExchange(&s->refs,0);
    s->table=static_cast<void**>(Address(0x00FF5B08));InitEnv(&s->env,nullptr);return s;
}
BindingTable* __fastcall InitBindings(BindingTable* s,void*) {
    s->loadFactor=1.0f;s->maxLoadFactor=2.0f;s->tag=static_cast<const char*>(Address(0x00FF5B10));
    s->one=1;s->table=Address(0x0114C900);s->count=0;s->storage=nullptr;
    M<void>(0x004E17E0,s->mutex,0,1);return s;
}
// The two ItemLists and fixed-vector form one contiguous selection context in
// the original. Container helpers take this context, not just its first list.
struct Selection {
    ts3::renderer::ItemList lists[2];
    uint32_t* begin;uint32_t* end;uint32_t* capacity;const char* tag;uint32_t* internal;
    uint32_t storage[48];uint32_t keys[12];
};
void* __fastcall SelectBinding(Items* s,void*,void* a,void* b,Binding** out) {
    Selection c;c.lists[0].Init();c.lists[1].Init();
    c.begin=c.end=c.internal=c.storage;c.capacity=c.keys;c.tag=static_cast<const char*>(Address(0x00F99AC0));
    M<void>(0x006CA4C0,s,&c,a,b);
    void* cache=M<void*>(0x006CB840,s);
    Env env;InitEnv(&env,nullptr);*out=nullptr;
    bool hit=false;
    if(cache && s->state44) {
        void* table=static_cast<void**>(s->state44)[2];
        M<void>(0x006CB7B0,&env,table,c.begin,int32_t(c.end-c.begin),&c);
        hit=M<uint8_t>(0x006CBB80,cache,&env,out)!=0;
    }
    if(!hit) {
        for(int i=0;i<12;++i) c.keys[i]=M<uint32_t>(0x006CA5B0,s,&c,i,0,1);
        if(cache) {
            auto* p=static_cast<Binding*>(Alloc(sizeof(Binding),0x00FF5A78));
            if(p) {
                InitBinding(p,nullptr);_InterlockedExchangeAdd(&p->refs,1);
                M<void>(0x006CB730,p,&env,c.keys);M<void>(0x006CBD50,cache,p);
                *out=p;_InterlockedExchangeAdd(&p->refs,1);
                if(!_InterlockedDecrement(&p->refs)) { _InterlockedExchange(&p->refs,1);V<void>(p,0,1); }
            }
        }
    }
    if(env.begin) Free(env.begin);
    void* r=c.begin;if(c.begin && c.begin!=c.internal) r=F<void*>(0x004E3F70,c.begin);
    return r;
}
struct Override { uint32_t type,key; void* scalar; uint32_t pad; uint32_t vector[4];void* payload;uint32_t length; };
struct Param { void* value;uint32_t count,extra,type; };
uint8_t __fastcall Compose(Definition* s,void*,const void* data,uint32_t count,uint8_t force,uint8_t append,Definition** out) {
    uint8_t internal[32];
    struct Flags { uint8_t* begin;uint8_t* end;uint8_t* capacity;const char* tag;uint8_t* internal; } flags{internal,internal,internal+32,static_cast<const char*>(Address(0x00F99AC0)),internal};
    auto* d=static_cast<Definition*>(Alloc(sizeof(Definition),0x00FF5AC4));
    if(!d) return 0;
    BaseInit(d->base,0x00FF5AAC);d->parameters=nullptr;d->cached=nullptr;d->word28=nullptr;d->word2c=nullptr;d->word34=nullptr;d->flags[0]=d->flags[1]=0;
    M<void>(0x006CB190,&flags,count);for(uint32_t i=0;i<count;++i) flags.begin[i]=0;
    V<void>(d,0);*out=d;d->word28=s->word28;d->word2c=s->word2c;
    alignas(void*) uint8_t container[32];M<void>(0x00799300,container);
    ts3::renderer::Holder798E30 holder;M<void>(0x006DE120,&holder,s->parameters);
    const int n=M<int>(0x00798DD0,&holder);
    const auto* entries=static_cast<const Override*>(data);
    auto addVector=[&](uint32_t key,const uint32_t* p,uint32_t extra) { M<void>(0x00799660,container,key,p[0],p[1],p[2],p[3],extra); };
    for(int i=0;i<n;++i) {
        Param p;M<void>(0x00798E60,&holder,i,&p);
        uint32_t key=uint32_t(holder.GetEntry(i));
        uint32_t j=0;while(j<count && entries[j].key!=key) ++j;
        const Override* e=j<count?entries+j:nullptr;
        if(p.type==1) {
            if(e) flags.begin[j]=1;
            M<void>(0x00799600,container,key,e?e->payload:p.value,p.count);
        } else if(p.type==2) {
            reinterpret_cast<ts3::renderer::Container799620*>(container)->Add(key,p.value,p.count);
        } else if(p.type==3) M<void>(0x00799640,container,key,p.value,p.count);
        else if(p.type>=4 && p.type<=6) {
            bool use=false;
            if(e) {
                flags.begin[j]=1;
                if(force) use=true;
                else { uint32_t compare[4]={};M<void>(0x00798F70,&holder,i,compare);use=compare[2]==e->vector[2]; }
            }
            if(use) {
                if(e->type==4) addVector(key,e->vector,p.extra);
                else if(e->type==6) M<void>(0x007996E0,container,key,e->scalar,p.extra);
            } else if(p.type==4) addVector(key,static_cast<uint32_t*>(p.value),p.extra);
            else {
                uintptr_t value=p.type==5?*static_cast<uint32_t*>(p.value):reinterpret_cast<uintptr_t>(p.value);
                M<void>(0x007996E0,container,key,value,p.extra);
            }
        }
    }
    if(append) for(uint32_t i=0;i<count;++i) if(!flags.begin[i]) {
        const auto& e=entries[i];
        if(e.type==1) M<void>(0x00799600,container,e.key,e.payload,e.length);
        else if(e.type==4) addVector(e.key,e.vector,0);
    }
    d->flags[0]=s->flags[0];d->flags[1]=s->flags[1];M<void>(0x006CAE90,d,container);M<void>(0x00799330,container);
    if(flags.begin && flags.begin!=flags.internal) Free(flags.begin);
    return 1;
}
}
