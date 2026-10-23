#include "ts3/render/m07_B18.h"
#include <atomic>
#include <cstring>
#include <utility>
namespace ts3::render::m07::b18 {
void* (*resolve)(uint32_t)=nullptr;
namespace {
template<class F> F Fn(uint32_t a) { return Original(a,resolve?reinterpret_cast<F>(resolve(a)):nullptr); }
void* Data(uint32_t a) { return Fn<void*>(a); }
void Empty(String& s,uint32_t tag) { s.begin=s.end=Data(0x011C7E54); s.capacity=Data(0x011C7E56); s.tag=static_cast<const char*>(Data(tag)); }
void Empty(Vector& v,uint32_t tag) { v.begin=v.end=v.capacity=nullptr; v.tag=static_cast<const char*>(Data(tag)); }
uintptr_t Free(void* p) { return Fn<uintptr_t(__cdecl*)(void*)>(0x004E3F70)(p); }
void* Alloc(size_t n,uint32_t tag) { return Fn<void*(__cdecl*)(size_t,const void*,uint32_t,uint32_t,uint32_t,uint32_t)>(0x004E3F90)(n,Data(tag),0,0,0,0); }
bool Owned(const String& s) { return static_cast<intptr_t>((reinterpret_cast<uintptr_t>(s.capacity)-reinterpret_cast<uintptr_t>(s.begin))&~uintptr_t(1))>2 && s.begin; }
uintptr_t DestroyRecords(void* self,void* b,void* e) { return CallThis(Fn<Thiscall<uintptr_t,void,void*,void*>>(0x0079BD70),self,b,e); }
void Push(Vector& v,Shader* s) {
    auto* end=static_cast<Shader**>(v.end);
    if(reinterpret_cast<uintptr_t>(end)>=reinterpret_cast<uintptr_t>(v.capacity))
        CallThis(Fn<Thiscall<void,Vector,void*,Shader**>>(0x008621C0),&v,v.end,&s);
    else { v.end=end+1; if(end) *end=s; }
}
uint32_t Read(void* stream,void* out,uint32_t endian) { return Fn<uint32_t(__cdecl*)(void*,void*,uint32_t)>(0x004DD700)(stream,out,endian); }
uintptr_t Validate(Reader* r,uint32_t ok) { return CallThis(Fn<Thiscall<uintptr_t,Reader,uint32_t>>(0x007D77C0),r,ok); }
}
State* __fastcall InitState(State* s,void*,void* owner) {
    for(auto& p:s->zeros) p=nullptr;
    s->zero154=s->zero160=nullptr;
    Empty(s->vertex,0x01015C84); Empty(s->pixel,0x01015C68); s->owner=owner;
    std::memset(s->arrays3,0,64); std::memset(s->arrays2,0,64);
    std::memset(s->arrays1,0,64); std::memset(s->arrays0,0,64);
    s->zeroC8=s->zeroCC=0; return s;
}
Set* __fastcall InitSet(Set* s,void*) {
    s->vtable=Data(0x01015D0C); s->unknown=0;
    reinterpret_cast<std::atomic<int32_t>*>(&s->refs)->exchange(0);
    Empty(s->modes,0x01015CF0); Empty(s->bytecode,0x01015CD4); Empty(s->fields,0x01015CB4);
    Empty(s->name,0x01015CA0); return s;
}
uintptr_t __fastcall SetName(Set* s,void*,const void* input) {
    String temp{}; temp.tag=static_cast<const char*>(Data(0x01015AF4));
    auto result=CallThis(Fn<Thiscall<uintptr_t,String,const void*>>(0x0040B700),&temp,input);
    if(&temp!=&s->name) result=CallThis(Fn<Thiscall<uintptr_t,String,void*,void*>>(0x00408E20),&s->name,temp.begin,temp.end);
    if(Owned(temp)) result=Free(temp.begin);
    return result;
}
uintptr_t __fastcall DestroySet(Set* s,void*) {
    s->vtable=Data(0x01015D0C);
    if(Owned(s->name)) Free(s->name.begin);
    DestroyRecords(&s->fields,s->fields.begin,s->fields.end);
    if(s->fields.begin) Free(s->fields.begin);
    if(s->bytecode.begin) Free(s->bytecode.begin);
    uintptr_t result=reinterpret_cast<uintptr_t>(s->modes.begin);
    if(s->modes.begin) result=Free(s->modes.begin);
    s->vtable=Data(0x00FA1B78); return result;
}
Set* __fastcall DeleteSet(Set* s,void*,uint32_t flags) {
    // Exact-004018E0 donor ResourceManDestroy: different destructor/free targets only.
    DestroySet(s,nullptr); if(flags&1) Free(s); return s;
}
Param* __fastcall InitParam(Param* s,void*) {
    s->a=0; s->b=0; s->type=1; s->min=0xbf800000; s->max=0x3f800000; s->epsilon=981668463;
    Empty(s->name,0x01015AF4); Empty(s->semantic,0x01015AF4); s->tail=0; return s;
}
Params* __fastcall InitParams(Params* s,void*) {
    Empty(s->array,0x01015D78); Empty(s->data,0x01015D50);
    for(auto& name:s->names) Empty(name,0x01015D3C); return s;
}
Watcher* __fastcall InitWatcher(Watcher* s,void*) {
    auto ctor=Fn<Thiscall<void,void,uint32_t,uint32_t>>(0x004F35B0);
    CallThis(ctor,static_cast<void*>(s->mutexes),4u,0u); CallThis(ctor,static_cast<void*>(s->mutexes+24),4u,0u);
    reinterpret_cast<std::atomic<int32_t>*>(&s->refs)->exchange(0);
    s->first=s->last=nullptr; Empty(s->name,0x01015E6C); Empty(s->notifiers,0x01015E48); return s;
}
uintptr_t __fastcall Resize(Vector* s,void*,uint32_t count) {
    auto* end=static_cast<uint8_t*>(s->end); auto* begin=static_cast<uint8_t*>(s->begin);
    constexpr size_t stride=sizeof(Field);
    auto size=uint32_t((reinterpret_cast<intptr_t>(end)-reinterpret_cast<intptr_t>(begin))/intptr_t(stride));
    if(count<=size) {
        auto* at=begin+size_t(count)*stride;
        void* moved=Fn<void*(__cdecl*)(void*,void*,void*)>(0x0079BF80)(end,end,at);
        auto result=DestroyRecords(s,moved,s->end); s->end=static_cast<uint8_t*>(s->end)-(size-count)*stride; return result;
    }
    Field temp;
    temp.name.begin=temp.name.end=Data(0x011C7E54); temp.name.capacity=Data(0x011C7E55); temp.name.tag=static_cast<const char*>(Data(0x01015AF4));
    CallThis(Fn<Thiscall<void,Vector,void*,uint32_t,void*>>(0x0079C580),s,s->end,count-size,static_cast<void*>(&temp));
    auto result=reinterpret_cast<uintptr_t>(temp.name.begin);
    if(reinterpret_cast<intptr_t>(temp.name.capacity)-reinterpret_cast<intptr_t>(temp.name.begin)>1 && temp.name.begin) result=Free(temp.name.begin);
    return result;
}
uintptr_t __fastcall ReadPackage(Package* s,void*,Reader* r,uint32_t filter) {
    uint32_t count; Validate(r,Read(r->stream,&count,r->endian));
    uint32_t low=filter&255, high=filter&0xff00; uintptr_t result=high;
    for(uint32_t i=0;i<count && r->good;++i) {
        uint32_t magic,key,flags,skip;
        result=Validate(r,Read(r->stream,&magic,r->endian));
        result=Validate(r,Read(r->stream,&key,r->endian));
        result=Validate(r,Read(r->stream,&flags,r->endian));
        result=Validate(r,Read(r->stream,&skip,r->endian));
        if(!r->good || magic!=0x4b4c4253) { result=Validate(r,0); r->good=0; return result; }
        if((!low || (flags&low)) && (!high || (flags&high))) {
            uint32_t n; result=Validate(r,Read(r->stream,&n,r->endian));
            for(uint32_t j=0;j<n && r->good;++j) {
                uint32_t variant; Validate(r,Read(r->stream,&variant,r->endian));
                auto* memory=static_cast<Shader*>(Alloc(sizeof(Shader),0x00FFC4E8));
                auto* shader=memory?CallThis(Fn<Thiscall<Shader*,Shader,uint32_t,uint32_t>>(0x0079A290),memory,key,variant):nullptr;
                shader->flags=flags; // EA dereferences even on allocation failure.
                CallThis(Fn<Thiscall<void,Shader,Reader*,void*>>(0x0079A690),shader,r,s->resource);
                if(!r->good) { CallThis(Fn<Thiscall<void,Shader>>(0x0079A670),shader); Free(shader); break; }
                Push(s->shaders,shader);
            }
        } else CallThis(Fn<Thiscall<void,Reader,uint32_t,uint32_t>>(0x007D8260),r,skip,1u);
        result=i+1;
    }
    return result;
}
uintptr_t __fastcall Reload(Package* s,void*,Package* other) {
    struct Node { uint32_t key[2]; Shader* shader; };
    struct Map { uint32_t prefix; Node** table; uint32_t buckets,count; float low,high; void* storage; const char* tag; } map;
    map.table=static_cast<Node**>(Data(0x0114C900)); map.buckets=1;map.count=0;map.low=1;map.high=2;map.storage=nullptr;map.tag=static_cast<const char*>(Data(0x01015B64));
    void* iter[3];
    for(auto p=static_cast<Shader**>(s->shaders.begin);p!=s->shaders.end;++p) {
        CallThis(Fn<Thiscall<void,Shader>>(0x0079A670),*p);
        Node node{{(*p)->key[0],(*p)->key[1]},*p};
        CallThis(Fn<Thiscall<void,Map,void*,Node*,uint32_t>>(0x0079B290),&map,static_cast<void*>(iter),&node,0u);
    }
    auto p=static_cast<Shader**>(other->shaders.begin),end=static_cast<Shader**>(other->shaders.end);
    while(p!=end) {
        Shader* shader=*p;
        CallThis(Fn<Thiscall<void,Map,void*,uint32_t*>>(0x0079B210),&map,static_cast<void*>(iter),shader->key);
        auto* node=static_cast<Node*>(iter[0]);
        if(node==map.table[map.buckets]) { Push(s->shaders,shader); *p=*--end; }
        else { CallThis(Fn<Thiscall<void,Shader,Shader*>>(0x0079A400),node->shader,shader); ++p; }
    }
    std::swap(s->resource,other->resource); std::swap(s->second,other->second); std::swap(s->third,other->third);
    auto result=CallThis(Fn<Thiscall<uintptr_t,Map,Node**,uint32_t>>(0x00AEBDB0),&map,map.table,map.buckets);
    if(map.buckets>1) result=Free(map.table); return result;
}
uint8_t __fastcall ReadParams(Factory* s,void*,void* stream,uint32_t endian,uint32_t version) {
    uint32_t count,min=0xbf800000,max=0x3f800000;
    auto ok=uint8_t(Read(stream,&count,endian)); if(!ok) return ok;
    if(count==0xffffffff) return 1;
    if(s->params) {
        CallThis(Fn<Thiscall<void,Params,void*,void*>>(0x0079FE80),s->params,s->params->array.begin,s->params->array.end);
        s->params->array.end=s->params->array.begin;s->params->data.end=s->params->data.begin;
    } else {
        auto* p=static_cast<Params*>(Alloc(sizeof(Params),0x01015DA4)); s->params=p?InitParams(p,nullptr):nullptr;
        if(!s->params) return 0;
    }
    uint8_t text[2048];
    for(auto& name:s->params->names) {
        if(Fn<int32_t(__cdecl*)(void*,void*,uint32_t,uint32_t)>(0x004DD850)(stream,text,1023,0)==-1) return 0;
        CallThis(Fn<Thiscall<void,String,const void*>>(0x00409010),&name,static_cast<const void*>(text));
    }
    CallThis(Fn<Thiscall<void,Params,uint32_t>>(0x007A02A0),s->params,count);
    if(CallThis(Fn<Thiscall<uint32_t,Params>>(0x0079FD40),s->params)<count) return 0;
    for(uint32_t i=0;i<count;++i) {
        uint32_t type,a,b;
        if(!uint8_t(Read(stream,&type,endian)) || !uint8_t(Read(stream,&a,endian)) || !uint8_t(Read(stream,&b,endian)) || (version>=2 && (!uint8_t(Read(stream,&min,endian)) || !uint8_t(Read(stream,&max,endian))))) return 0;
        Param p;InitParam(&p,nullptr);p.type=type;p.a=a;p.b=b;p.min=min;p.max=max;
        CallThis(Fn<Thiscall<void,Params,Param*>>(0x007A04B0),s->params,&p);
        CallThis(Fn<Thiscall<void,Param>>(0x0079FDF0),&p);
    }
    uint32_t bytes;if(!uint8_t(Read(stream,&bytes,endian))) return 0;
    if(bytes) {
        CallThis(Fn<Thiscall<void,Vector,uint32_t>>(0x00405410),&s->params->data,bytes);
        if(reinterpret_cast<uintptr_t>(s->params->data.capacity)-reinterpret_cast<uintptr_t>(s->params->data.begin)<bytes) return 0;
        auto** table=*static_cast<void***>(stream);
        if(CallThis(reinterpret_cast<Thiscall<uint32_t,void,void*,uint32_t>>(table[12]),stream,s->params->data.begin,bytes)!=bytes) return 0;
    }
    return 1;
}
}
