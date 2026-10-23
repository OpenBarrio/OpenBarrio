#include "ts3/resource/m03_A03.h"
#include "ts3/misc/clu_4a6c30_x.h"
#if defined(_M_IX86)
#include <intrin.h>
namespace ts3::resource::m03 {
using namespace manager;
using Vector=a03::Vector;
namespace {
long* Count(void* p) { return reinterpret_cast<long*>(static_cast<uint8_t*>(p)+4); }
void* DbRef(void* p) { return static_cast<uint8_t*>(p)+4; }
void Retain(void* p,bool db) { if(db) CallGameVirtual<uint32_t,0>(DbRef(p)); else CallGameVirtual<uint32_t,1>(p); }
void Release(void* p,bool db) { if(db) CallGameVirtual<uint32_t,1>(DbRef(p)); else CallGameVirtual<uint32_t,2>(p); }
template<class E> void Assign(E* d,const E* s,bool db) {
    void* fresh=*reinterpret_cast<void* const*>(s); void* old=*reinterpret_cast<void**>(d);
    if(fresh!=old) {
        if(fresh) Retain(fresh,db);
        *reinterpret_cast<void**>(d)=fresh;
        if(old) Release(old,db);
    }
    d->flags=s->flags;
}
uint32_t Destroy(Vector* v,bool db) {
    auto first=static_cast<CallbackEntry*>(v->begin), last=static_cast<CallbackEntry*>(v->end);
    for(auto p=first;p<last;++p) if(p->callback) Release(p->callback,db);
    if(!v->begin) return 0;
    return CallGameVirtual<uint32_t,3>(v->allocator,v->begin,static_cast<uint32_t>(static_cast<uint8_t*>(v->capacity)-static_cast<uint8_t*>(v->begin)) & ~uint32_t(7));
}
}
uint32_t a03::DestroyDatabases(Vector* v) { return Destroy(v,true); }
uint32_t a03::DestroyCallbacks(Vector* v) { return Destroy(v,false); }
CallbackEntry** a03::CopyInline(CallbackEntry** out,const CallbackEntry* first,const CallbackEntry* last,CallbackEntry* dest) {
    *out=dest;
    for(;first!=last;++first) {
        if(*out) {
            (*out)->callback=first->callback;
            if(first->callback) _InterlockedIncrement(Count(first->callback));
            (*out)->flags=first->flags;
        }
        *out=reinterpret_cast<CallbackEntry*>(reinterpret_cast<uintptr_t>(*out)+sizeof(CallbackEntry));
    }
    return out;
}
DatabaseEntry* a03::AssignDatabases(const DatabaseEntry* first,const DatabaseEntry* last,DatabaseEntry* dest) {
    for(;first!=last;++first,++dest) Assign(dest,first,true);
    return dest;
}
CallbackEntry* a03::AssignCallbacks(const CallbackEntry* first,const CallbackEntry* last,CallbackEntry* dest) {
    for(;first!=last;++first,++dest) Assign(dest,first,false);
    return dest;
}
DatabaseEntry* a03::BackwardDatabases(const DatabaseEntry* first,const DatabaseEntry* last,DatabaseEntry* dest) {
    while(first!=last) Assign(--dest,--last,true);
    return dest;
}
CallbackEntry* a03::BackwardCallbacks(const CallbackEntry* first,const CallbackEntry* last,CallbackEntry* dest) {
    while(first!=last) Assign(--dest,--last,false);
    return dest;
}
uintptr_t a03::ReleaseInline(const CallbackEntry* first,const CallbackEntry* last) {
    uintptr_t result=0; // incoming EAX is unspecified for empty/all-null ranges
    for(;first<last;++first) if(auto p=first->callback) {
        result=reinterpret_cast<uintptr_t>(Count(p));
        if(_InterlockedExchangeAdd(Count(p),-1)==1) {
            _InterlockedExchange(Count(p),1);
            result=CallGameVirtual<uintptr_t,0>(p,uint32_t(1));
        }
    }
    return result;
}
CallbackEntry* a03::AssignInline(CallbackEntry* dest,const CallbackEntry* source) {
    auto fresh=source->callback; auto old=dest->callback;
    if(fresh!=old) {
        if(fresh) _InterlockedIncrement(Count(fresh));
        dest->callback=fresh;
        if(old && _InterlockedExchangeAdd(Count(old),-1)==1) {
            _InterlockedExchange(Count(old),1);
            CallGameVirtual<uintptr_t,0>(old,uint32_t(1));
        }
    }
    dest->flags=source->flags;
    return dest;
}
FilterNode** a03::Erase(FilterSet* tree,FilterNode** out,FilterNode* node) {
    --tree->mnSize;
    auto next=CallGameFunction<FilterNode*,0x004E0B20>(node);
    ts3::misc::EraseResourceTree(reinterpret_cast<ts3::misc::ResourceTreeNode*>(node),reinterpret_cast<ts3::misc::ResourceTreeNode*>(&tree->mAnchor));
    CallGameVirtual<void,3>(tree->mAllocator.core_allocator(),node,uint32_t(32));
    *out=next;
    return out;
}
uint32_t a03::FindType(ResourceManager* manager,const char16_t* name) {
    auto base=reinterpret_cast<uint8_t*>(manager);
    auto mutex=base+0x140;
    CallGameMethod<int,0x004E16F0>(mutex,reinterpret_cast<const void*>(0xFB2CD0));
    auto buckets=*reinterpret_cast<uint8_t***>(base+0xF4);
    auto count=*reinterpret_cast<uint32_t*>(base+0xF8);
    auto bucket=buckets; while(!*bucket) ++bucket;
    auto node=*bucket;
    while(node!=buckets[count]) {
        using Compare=int(__cdecl*)(const char16_t*,const char16_t*);
        auto compare=*reinterpret_cast<Compare*>(0xF95758);
        if(compare(*reinterpret_cast<char16_t**>(node+4),name)==0) {
            uint32_t value=*reinterpret_cast<uint32_t*>(node);
            CallGameMethod<int,0x004E17B0>(mutex); return value;
        }
        node=*reinterpret_cast<uint8_t**>(node+0x14);
        if(!node) { do { ++bucket; } while(!*bucket); node=*bucket; }
    }
    CallGameMethod<int,0x004E17B0>(mutex);
    return 0xFFFFFFFF;
}
int32_t a03::Notify(ResourceManager* manager,void* database,const KeyRange* range) {
    auto& tree=manager::FilterTree(manager);
    auto& vector=manager::Databases(manager);
    auto mutex=reinterpret_cast<uint8_t*>(manager)+0x48;
    CallGameMethod<int,0x004E16F0>(mutex,reinterpret_cast<const void*>(0xFB2CD0));
    bool present=false;
    if(range && tree.mnSize) for(auto& e:vector) if(e.database==database) { present=true; break; }
    if(present) for(auto it=tree.begin();it!=tree.end();++it) {
        auto last=range->last;
        for(auto key=range->first;key!=last;++key) {
            auto selected=CallGameVirtual<void*,17>(manager,key);
            if(selected && selected!=database) continue;
            CallGameMethod<int,0x004E17B0>(mutex);
            if(!it->predicate || static_cast<uint8_t>(CallGameVirtual<uint32_t,1>(it->predicate,key))) it->notify(database,key,it->context);
            CallGameMethod<int,0x004E16F0>(mutex,reinterpret_cast<const void*>(0xFB2CD0));
        }
    }
    return CallGameMethod<int32_t,0x004E17B0>(mutex);
}
int32_t __fastcall A03Notify(ResourceManager* p,void*,void* db,const KeyRange* range) { return a03::Notify(p,db,range); }
uint32_t __fastcall A03DestroyDatabases(a03::Vector* p,void*) { return a03::DestroyDatabases(p); }
uint32_t __fastcall A03DestroyCallbacks(a03::Vector* p,void*) { return a03::DestroyCallbacks(p); }
uint32_t __fastcall A03FindType(ResourceManager* p,void*,const char16_t* text) { return a03::FindType(p,text); }
FilterNode** __fastcall A03Erase(FilterSet* p,void*,FilterNode** out,FilterNode* node) { return a03::Erase(p,out,node); }
uintptr_t __fastcall A03Release(void*,void*,const CallbackEntry* first,const CallbackEntry* last) { return a03::ReleaseInline(first,last); }
CallbackEntry* __fastcall A03Assign(CallbackEntry* p,void*,const CallbackEntry* source) { return a03::AssignInline(p,source); }
}
#endif
