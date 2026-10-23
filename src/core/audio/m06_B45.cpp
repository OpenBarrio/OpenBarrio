#include "ts3/audio/m06_B45.h"
#include <cstring>
#include <atomic>
namespace ts3::audio::b45 {
namespace {
template<class T=void*> T Data(uint32_t address) {
#if defined(_M_IX86)
    return reinterpret_cast<T>(uintptr_t(address));
#else
    (void)address; return nullptr;
#endif
}
void Free(void* p) { CallGameFunction<void,0x004E3F70>(p); }
uint32_t Get(void* self,uint32_t key,uint16_t type) {
    Variant* v=nullptr;
    uint32_t result=CallGameVirtual<uint32_t,8>(self,key,&v);
    if (!(result & 0xFF) || v->type!=type) return 0;
    if (v->flags & 0x30) return *static_cast<uint32_t*>(v->indirect);
    return v->word;
}
}
Property* __cdecl LowerBound(Property* first,Property* last,const uint32_t* key) {
    auto count=last-first;
    while(count>0) {
        auto half=count/2; auto mid=first+half;
        if(mid->key<*key) { first=mid+1; count-=half+1; }
        else count=half;
    }
    return first;
}
Property** __cdecl CopyProperties(Property** out,Property* first,Property* last,Property* destination) {
    *out=destination;
    while(first!=last) {
        if(*out) {
            (*out)->key=first->key;
            auto& value=(*out)->value;
            value.table=Data(0x00FB96B4); value.flags=0; value.type=0;
            CallGameMethod<void,0x004F62C0>(&value,&first->value);
        }
        *out=reinterpret_cast<Property*>(reinterpret_cast<uintptr_t>(*out)+sizeof(Property));
        ++first;
    }
    return out;
}
uint32_t __fastcall NameHash(void*,void*,void* key,void* service) {
    char* empty=Data<char*>(0x011C7E54);
    String name{empty,empty,empty+1,Data<const char*>(0x0103A620)};
    CallGameMethod<void,0x004068A0>(service,key,0x28u);
    CallGameMethod<void,0x00406A30>(service,key,&name);
    CallGameMethod<void,0x004068A0>(service,key,0x29u);
    uint32_t result=CallGameFunction<uint32_t,0x004FA590>(name.begin,0x811C9DC5u,1u);
    if(name.capacity-name.begin>1 && name.begin) Free(name.begin);
    return result;
}
Group* __fastcall ConstructGroup(Group* g,void*) {
    g->table=Data(0x0103A6AC); std::atomic_ref<int32_t>(g->refs).exchange(0);
    for(auto& field:g->base) field=nullptr;
    g->secondary=Data(0x0103A658); g->hash=0x19AD940B;
    g->active=0; g->begin=g->end=g->capacity=nullptr; g->label=Data<const char*>(0x0103A6E8);
    g->owner=nullptr; g->sentinel.next=g->sentinel.previous=&g->sentinel;
    g->first.table=g->second.table=Data(0x00FB96B4);
    g->first.flags=g->first.type=g->second.flags=g->second.type=0;
    g->groups=g->groupsEnd=g->groupsCapacity=nullptr; g->groupsLabel=Data<const char*>(0x0103A6C4);
    g->next=g->previous=nullptr;
    return g;
}
void __fastcall DestroyGroup(Group* g,void*) {
    g->table=Data(0x0103A6AC); g->secondary=Data(0x0103A658);
    if(g->owner) { g->next->next=g->previous; g->previous->previous=g->next; }
    if(g->groups) Free(g->groups);
    g->second.table=Data(0x00FB96B4);
    if(g->second.flags&4) CallGameMethod<void,0x004F55A0>(&g->second,0u);
    g->first.table=Data(0x00FB96B4);
    if(g->first.flags&4) CallGameMethod<void,0x004F55A0>(&g->first,0u);
    if(g->owner) CallGameVirtual<void,1>(g->owner);
    CallGameMethod<void,0x008A77B0>(&g->begin,g->begin,g->end);
    if(g->begin) Free(g->begin);
    CallGameMethod<void,0x00731CF0>(g);
}
Group* __fastcall DeleteGroup(Group* g,void*,uint32_t flags) {
    DestroyGroup(g,nullptr);
    if(flags&1) Free(g);
    return g;
}
uint32_t __fastcall Get19(void* self,void*,uint32_t key) { return Get(self,key,0x13); }
uint32_t __fastcall Get18(void* self,void*,uint32_t key) { return Get(self,key,0x12); }
uint32_t __cdecl ReadString(void* stream,String* string) {
    if(string->begin!=string->end) { *string->begin=0; string->end=string->begin; }
    uint32_t length=0;
    CallGameFunction<void,0x004DD700>(stream,&length,0u);
    if(!length) return 1;
    CallGameMethod<void,0x00409E00>(string,length,0u);
    uint32_t result=CallGameVirtual<uint32_t,12>(stream,string->begin,length);
    return (result&0xFFFFFF00u) | uint32_t(result==length);
}
uint32_t __cdecl ReadWideString(void* stream,WideString* string) {
    if(string->begin!=string->end) { *string->begin=0; string->end=string->begin; }
    uint32_t length=0;
    CallGameFunction<void,0x004DD700>(stream,&length,0u);
    if(!length) return 1;
    CallGameMethod<void,0x0040BC30>(string,length,0u);
    uint32_t bytes=length*2;
    uint32_t result=CallGameVirtual<uint32_t,12>(stream,string->begin,bytes);
    return (result&0xFFFFFF00u) | uint32_t(result==bytes);
}
Configuration* __fastcall ConstructConfiguration(Configuration* c,void*) {
    c->table=Data(0x0103A72C); std::atomic_ref<int32_t>(c->refs).exchange(0);
    CallGameMethod<void,0x004E17E0>(c->mutex,0u,1u);
    c->services=c->allocator=nullptr;
    c->root=&c->root; c->left=&c->root; c->right=nullptr; c->flag=0; std::memset(c->pad,0,sizeof(c->pad)); c->count=0; c->label=Data<const char*>(0x0103A620);
    void* alloc=CallGameFunction<void*,0x004E3F90>(16u,c->label,0u,0u,0u,0u);
    if(alloc) { CallGameMethod<void,0x00406260>(alloc); *static_cast<void**>(alloc)=Data(0x0103A63C); }
    if(alloc!=c->allocator) {
        if(alloc) CallGameVirtual<void,4>(alloc);
        void* old=c->allocator; c->allocator=alloc;
        if(old) CallGameVirtual<void,5>(old);
    }
    void* services=CallGameFunction<void*,0x00409B20>();
    if(services!=c->services) {
        if(services) CallGameVirtual<void,1>(services);
        void* old=c->services; c->services=services;
        if(old) CallGameVirtual<void,2>(old);
    }
    CallGameVirtual<void,3>(c->services);
    void* registry=CallGameFunction<void*,0x00409AF0>();
    CallGameVirtual<void,5>(c->services,registry);
    void* hash=CallGameFunction<void*,0x004E3F90>(8u,Data<const char*>(0x0103A718),0u,0u,0u,0u);
    if(hash) { *static_cast<void**>(hash)=Data(0x0103A60C); *reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(hash)+sizeof(void*))=0; }
    void* entry=CallGameVirtual<void*,44>(registry,Data<void*>(0x0103A710),hash);
    CallGameMethod<void,0x004077C0>(entry);
    return c;
}
Configuration* __fastcall DeleteConfiguration(Configuration* c,void*,uint32_t flags) {
    CallGameMethod<void,0x006A0850>(&c->mapReserved,c->right);
    if(c->allocator) CallGameVirtual<void,5>(c->allocator);
    if(c->services) CallGameVirtual<void,2>(c->services);
    CallGameMethod<void,0x004E1670>(c->mutex);
    c->table=Data(0x00FB84D0);
    if(flags&1) CallGameFunction<void,0x004E4230>(c);
    return c;
}
}
