#include "ts3/resource/m03_D02.h"
#include <cstring>
namespace ts3::resource::m03::d02 {
namespace {
void* Offset(void* p,size_t n) { return static_cast<char*>(p)+n; }
uint32_t HashText(const char16_t* p) { uint32_t h=0x811c9dc5; while(*p) h=(h*0x1000193)^uint16_t(*p++); return h; }
void* Allocate(size_t n,const char* name) { return CallGameFunction<void*,0x004E3F90>(n,name,0u,0u,
#if defined(_M_IX86)
    reinterpret_cast<const char*>(0x00F98980),
#else
    static_cast<const char*>(nullptr),
#endif
    0xe7u); }
}
uintptr_t __fastcall Query(void* self,void*,const void* key,const void* iid,void** out) {
    void* record=nullptr;
    uintptr_t r=CallGameVirtual<uintptr_t,3>(Offset(self,2*sizeof(void*)),key,&record,0u,0u,0u,0u);
    if(uint8_t(r)) {
        r=CallGameVirtual<uintptr_t,3>(record,iid); *out=reinterpret_cast<void*>(r);
        if(r) return (r&~uintptr_t(255))|1;
    }
    if(record) r=CallGameVirtual<uintptr_t,1>(record);
    return r&~uintptr_t(255);
}
void* __fastcall Open(void* self,void*,const char16_t* source,uint32_t flags) {
    char16_t path[262];
    CallGameFunction<void,0x004FF380>(path,source,260u);
    CallGameFunction<void,0x004EC690>(path,-1);
    if(!uint8_t(CallGameFunction<uintptr_t,0x004DBBA0>(path)) && !uint8_t(CallGameFunction<uintptr_t,0x004DBF60>(path))) return nullptr;
    if(void* existing=CallGameMethod<void*,0x00734220>(self,source)) return existing;
    void* database=nullptr; void* watcher=nullptr;
    bool directory=(CallGameFunction<uintptr_t,0x004DBCA0>(path)&8)!=0;
    const char* name;
#if defined(_M_IX86)
    name=reinterpret_cast<const char*>(directory?0x00FFE134:0x00FFE114);
#else
    name=directory?"ResourceSystem/DBDirFiles":"ResourceSystem/DBPackedStream";
#endif
    void* raw=CallGameFunction<void*,0x004E41F0>(size_t(0xb0),name,0u,0u,0u,0u);
    if(raw) {
        if(directory) raw=CallGameMethod<void*,0x004A5500>(raw,nullptr,0u);
        else raw=CallGameMethod<void*,0x0072CC60>(raw,0u);
    }
    CallGameMethod<void*,0x004A0A00>(&database,raw);
    if(directory) watcher=CallGameVirtual<void*,8>(self,path);
    if(uint8_t(CallGameVirtual<uintptr_t,1>(database))) {
        CallGameVirtual<void,11>(database,path);
        if(uint8_t(CallGameVirtual<uintptr_t,6>(database,1u,6u,0u))) {
            if(!uint8_t(CallGameVirtual<uintptr_t,13>(Offset(self,2*sizeof(void*)),1u,database,flags))) {
                CallGameVirtual<void,7>(database); CallGameVirtual<void,2>(database);
                CallGameVirtual<void,1>(Offset(database,sizeof(void*))); return nullptr;
            }
            if(watcher) CallGameMethod<void,0x004A4930>(watcher,database,1u);
            CallGameVirtual<void,1>(Offset(database,sizeof(void*))); return database;
        }
        CallGameVirtual<void,2>(database);
    }
    CallGameVirtual<void,1>(Offset(database,sizeof(void*))); return nullptr;
}
Result<MapNode>* __fastcall InsertMap(Hash<MapNode>* h,void*,Result<MapNode>* out,const MapNode* value,uintptr_t) {
    uint32_t hash=HashText(value->key),index=hash%h->count;
    for(auto* n=h->buckets[index];n;n=n->next) if(n->key==value->key) { out->node=n;out->cell=h->buckets+index;out->inserted=0;return out; }
    uint32_t policy[2]; CallGameMethod<void*,0x004E09B0>(h->policy,policy,h->count,h->size,1u);
    auto* n=static_cast<MapNode*>(Allocate(sizeof(MapNode),h->allocName));
    if(n) { n->key=value->key;n->value=value->value; } n->next=nullptr;
    if(uint8_t(policy[0])) { index=hash%policy[1];CallGameMethod<void,0x00734860>(h,policy[1]); }
    n->next=h->buckets[index]; h->buckets[index]=n; ++h->size;
    out->node=n;out->cell=h->buckets+index;out->inserted=1;return out;
}
Result<SetNode>* __fastcall InsertSet(Hash<SetNode>* h,void*,Result<SetNode>* out,const SetNode* value,uintptr_t) {
    uint32_t hash=HashText(value->key),index=hash%h->count;
    auto* n=CallGameMethod<SetNode*,0x004A11E0>(h,h->buckets[index],value,hash);
    if(n) { out->node=n;out->cell=h->buckets+index;out->inserted=0;return out; }
    uint32_t policy[2];CallGameMethod<void*,0x004E09B0>(h->policy,policy,h->count,h->size,1u);
    n=static_cast<SetNode*>(Allocate(sizeof(SetNode),h->allocName));
    if(n) n->key=value->key; n->next=nullptr;
    if(uint8_t(policy[0])) { index=hash%policy[1];CallGameMethod<void,0x00734940>(h,policy[1]); }
    n->next=h->buckets[index];h->buckets[index]=n;++h->size;
    out->node=n;out->cell=h->buckets+index;out->inserted=1;return out;
}
String** __cdecl CopyStrings(String** out,const String* first,const String* last,String* dest) {
    *out=dest;
    while(first!=last) {
        if(auto* s=*out) {
            s->begin=s->end=s->capacity=nullptr;s->allocName=first->allocName;
            const auto length=first->end-first->begin;
            if(uint32_t(length+1)>1) { s->begin=static_cast<char16_t*>(Allocate(size_t(length+1)*2,s->allocName));s->end=s->begin;s->capacity=s->begin+length+1; }
            else {
#if defined(_M_IX86)
                s->begin=reinterpret_cast<char16_t*>(0x011C7E54);
#else
                static char16_t empty=0;s->begin=&empty;
#endif
                s->end=s->begin;s->capacity=s->begin+1;
            }
            std::memmove(s->begin,first->begin,size_t(length)*2);s->end=s->begin+length;*s->end=0;s->value=first->value;
        }
        *out=reinterpret_cast<String*>(uintptr_t(*out)+sizeof(String));++first;
    }
    return out;
}
}
