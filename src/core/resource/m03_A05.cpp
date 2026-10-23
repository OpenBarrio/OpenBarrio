#include "ts3/resource/m03_A05.h"
#include "ts3/resource/m03_manager_registry_contracts.h"
#include "ts3/layout.h"
#include <cstring>
namespace ts3::resource::m03::a05 {
namespace {
constexpr uint32_t timeout=0xffffffffu;
void Lock(Mutex* m) { ts3::CallGameMethod<int32_t,0x004e16f0>(m,&timeout); }
uint32_t Unlock(Mutex* m) { return ts3::CallGameMethod<uint32_t,0x004e17b0>(m); }
void ReleaseString(String& s) {
    const auto delta=static_cast<intptr_t>((reinterpret_cast<uintptr_t>(s.cap)-reinterpret_cast<uintptr_t>(s.begin)) & ~uintptr_t(1));
    if(delta>2 && s.begin) ts3::CallGameFunction<void,0x004e3f70>(s.begin);
}
template<class N> void Retire(Table<N>* t,N** buckets,uint32_t count) {
    for(uint32_t i=0;i<count;++i) {
        auto* p=buckets[i];
        while(p) { auto* old=p; p=p->next; ReleaseString(old->value.name); t->allocator->Free(old,sizeof(N)); }
        buckets[i]=nullptr;
    }
}
template<class N> Iterator<N>* Erase(Table<N>* t,Iterator<N>* out,Iterator<N> in) {
    out->node=in.node->next; out->bucket=in.bucket;
    while(!out->node) out->node=*++out->bucket;
    if(*in.bucket==in.node) *in.bucket=in.node->next;
    else { auto* p=*in.bucket; while(p->next!=in.node) p=p->next; p->next=in.node->next; }
    ReleaseString(in.node->value.name);
    t->allocator->Free(in.node,sizeof(N));
    --t->size;
    return out;
}
void CopyVector(RawVector* out,const RawVector* in) {
    const auto count=static_cast<uint32_t>((reinterpret_cast<uintptr_t>(in->end)-reinterpret_cast<uintptr_t>(in->begin))/sizeof(manager::CallbackEntry));
    ts3::CallGameMethod<void*,0x004b0f50>(out,count,&in->allocator);
    manager::CallbackEntry* end;
    ts3::CallGameFunction<void*,0x004b0430>(&end,in->begin,in->end,out->begin,static_cast<const void*>(nullptr));
    out->end=end;
}
}
void RetireTypes(Table<TypeNode>* t,TypeNode** b,uint32_t n) { Retire(t,b,n); }
void RetireKeys(Table<KeyNode>* t,KeyNode** b,uint32_t n) { Retire(t,b,n); }
Value* CopyValue(Value* out,const Value* in) { out->hash=in->hash; CopyVector(&out->entries,&in->entries); return out; }
Value* MakeValue(Value* out,const uint32_t* hash,const RawVector* in) { out->hash=*hash; CopyVector(&out->entries,in); return out; }
Iterator<TypeNode>* EraseType(Table<TypeNode>* t,Iterator<TypeNode>* o,Iterator<TypeNode> i) { return Erase(t,o,i); }
Iterator<KeyNode>* EraseKey(Table<KeyNode>* t,Iterator<KeyNode>* o,Iterator<KeyNode> i) { return Erase(t,o,i); }
uint32_t CollectDatabases(Manager* self,DatabaseList* list,const Key* key) {
    Lock(&self->prefix.mutex);
    uint32_t count=0;
    auto* end=self->prefix.databases.end();
    for(auto* p=self->prefix.databases.begin();p!=end;++p) {
        void* db=p->database;
        if(!key || (ts3::CallGameVirtual<uint32_t,13>(db,key,static_cast<void*>(nullptr),1,6,1,0)&0xff)) {
            ++count;
            if(list) {
                auto* n=static_cast<ListNode*>(list->allocator->Alloc(sizeof(ListNode),list->name,list->flags));
                if(reinterpret_cast<uintptr_t>(n)!=uintptr_t(0)-offsetof(ListNode,value)) n->value=db;
                n->next=reinterpret_cast<ListNode*>(list); n->prev=list->prev;
                list->prev->next=n; list->prev=n;
            }
        }
    }
    Unlock(&self->prefix.mutex); return count;
}
uint32_t GetName(Manager* self,const Key* key,String* out) {
    const auto type=ts3::CallGameVirtual<uint32_t,17>(self,key);
    if(type) {
        auto r=ts3::CallGameFunction<uint32_t,0x004aef70>(key,out,type);
        if(r&0xff) return r;
    }
    Lock(&self->nameMutex);
    Iterator<KeyNode> it;
    ts3::CallGameMethod<void*,0x005e5e20>(&self->keyNames,&it,key);
    if(it.node==self->keyNames.buckets[self->keyNames.bucketCount])
        ts3::CallGameFunction<void,0x004af000>(key,out,self,type,static_cast<void*>(nullptr));
    else if(&it.node->value.name!=out)
        ts3::CallGameMethod<void*,0x00408e20>(out,it.node->value.name.begin,it.node->value.name.end);
    return Unlock(&self->nameMutex);
}
uint32_t CollectKeys(Manager* self,KeyVector* out,void* filter,uint32_t mode) {
    uint32_t count=0;
    if(!(mode&0xff)) {
        auto* end=self->prefix.databases.end();
        for(auto* p=self->prefix.databases.begin();p!=end;++p) count+=ts3::CallGameVirtual<uint32_t,12>(p->database,out,filter);
        return count;
    }
    ResourceList list{}; list.next=list.prev=reinterpret_cast<ListNode*>(&list);
#if defined(_M_IX86)
    list.name=reinterpret_cast<const char*>(0x00fb2cd4);
#else
    list.name="ResourceSystem/ResourceMan/Manager";
#endif
    auto* end=self->prefix.callbacks.end();
    for(auto* p=self->prefix.callbacks.begin();p!=end;++p) ts3::CallGameVirtual<void,9>(p->callback,&list,filter);
    auto* sentinel=reinterpret_cast<ListNode*>(&list);
    for(auto* p=list.next;p!=sentinel;p=p->next) {
        auto* key=reinterpret_cast<Key*>(static_cast<uint8_t*>(p->value)+8);
        if(reinterpret_cast<uintptr_t>(out->end)>=reinterpret_cast<uintptr_t>(out->capacity))
            ts3::CallGameMethod<void,0x006d3810>(out,out->end,key);
        else { auto* dest=out->end; out->end=reinterpret_cast<Key*>(reinterpret_cast<uintptr_t>(dest)+sizeof(Key)); if(dest) *dest=*key; }
        ++count;
    }
    for(auto* p=list.next;p!=sentinel;) {
        auto* old=p; p=p->next;
        if(old->value) ts3::CallGameVirtual<void,1>(old->value);
        ts3::CallGameFunction<void,0x004e3f70>(old);
    }
    return count;
}
uint8_t ControlFilter(Manager* self,uint32_t add,manager::FilterNotify notify,void* context,void* predicate,int32_t priority) {
    if(!notify) return 0;
    manager::FilterEntry entry{notify,context,predicate,priority};
    auto* tree=reinterpret_cast<uint8_t*>(self)+offsetof(a01::Manager,reserved0c);
    manager::FilterNode* found;
    ts3::CallGameMethod<void*,0x004b0180>(tree,&found,&entry);
    auto* sentinel=reinterpret_cast<manager::FilterNode*>(tree+sizeof(uint32_t));
    if(add&0xff) {
        if(found!=sentinel) return 0;
        manager::InsertResult result;
        ts3::CallGameMethod<void*,0x004b13f0>(tree,&result,&entry,add&0xffffff00u);
    } else {
        if(found==sentinel) return 0;
        ts3::CallGameMethod<void*,0x004b0dd0>(tree,&found,found);
    }
    return 1;
}
uint32_t RemoveKey(Manager* self,const Key* key) {
    Lock(&self->nameMutex); Iterator<KeyNode> it;
    ts3::CallGameMethod<void*,0x005e5e20>(&self->keyNames,&it,key);
    bool hit=it.node!=self->keyNames.buckets[self->keyNames.bucketCount];
    if(hit) EraseKey(&self->keyNames,&it,it);
    return (Unlock(&self->nameMutex)&0xffffff00u)|uint32_t(hit);
}
uint32_t RemoveType(Manager* self,uint32_t type) {
    Lock(&self->nameMutex); Iterator<TypeNode> it;
    ts3::CallGameMethod<void*,0x00d4e550>(&self->typeNames,&it,&type);
    bool hit=it.node!=self->typeNames.buckets[self->typeNames.bucketCount];
    if(hit) EraseType(&self->typeNames,&it,it);
    return (Unlock(&self->nameMutex)&0xffffff00u)|uint32_t(hit);
}
}
