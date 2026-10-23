#include "ts3/resource/m03_manager_registry_contracts.h"
#include "ts3/resource/m03_A04.h"
#include <atomic>
#include <cstring>
namespace ts3::resource::a04_detail {
namespace {
bool Less(const FilterEntry& a,const FilterEntry& b) {
    if(a.priority!=b.priority) return a.priority>b.priority;
    if(a.notify!=b.notify) return uintptr_t(a.notify)<uintptr_t(b.notify);
    if(a.context!=b.context) return uintptr_t(a.context)<uintptr_t(b.context);
    return uintptr_t(a.predicate)<uintptr_t(b.predicate);
}
TreeNode* Anchor(TreeView* t) { return reinterpret_cast<TreeNode*>(&t->right); }
void Assign(const InlineEntry* s,InlineEntry* d) {
    auto* fresh=s->callback; auto* old=d->callback;
    if(fresh!=old) {
        if(fresh) std::atomic_ref<int32_t>(*reinterpret_cast<int32_t*>(reinterpret_cast<char*>(fresh)+sizeof(void*))).fetch_add(1);
        d->callback=fresh;
        if(old) {
            std::atomic_ref<int32_t> refs(*reinterpret_cast<int32_t*>(reinterpret_cast<char*>(old)+sizeof(void*)));
            if(refs.fetch_sub(1)==1) { refs.exchange(1); CallGameVirtual<void,0>(old,1); }
        }
    }
    d->flags=s->flags;
}
template<class T> T& Field(void* p,size_t offset) {
#if defined(_M_IX86)
    return *reinterpret_cast<T*>(static_cast<char*>(p)+offset);
#else
    (void)p; (void)offset; NotRewritten("A04 raw manager layout requires native adapter");
#endif
}
void Lock(void* p,size_t offset) { CallGameMethod<int,0x004E16F0>(&Field<m03::Mutex>(p,offset),reinterpret_cast<void*>(uintptr_t(0x00FB2CD0))); }
void Unlock(void* p,size_t offset) { CallGameMethod<int,0x004E17B0>(&Field<m03::Mutex>(p,offset)); }
struct FactoryNode { uint32_t type; InlineEntry *begin,*end,*capacity; GameAllocator allocator; FactoryNode* next; };
FactoryNode* Lookup(void* p,uint32_t* type) {
    FactoryNode* out[2]; CallGameMethod<void*,0x00576C30>(&Field<uint32_t>(p,0x78),out,type); return out[0];
}
void Append(List* l,void* value) {
    auto* n=CallGameFunction<ListNode*,0x004E3F90>(sizeof(ListNode),l->name,0u);
    // EA dereferences failed allocation; do not turn this into a recoverable error.
    n->value=value; n->next=reinterpret_cast<ListNode*>(l); n->prev=l->prev;
    l->prev->next=n; l->prev=n;
}
struct PolicyResult { uint32_t grow,count; };
PolicyResult Policy(NameTable* t) {
    PolicyResult r; CallGameMethod<void*,0x004E09B0>(t->policy,&r,t->count,t->size,1u); return r;
}
}
VectorView* ConstructVector(VectorView* v,uint32_t count,const GameAllocator* a) {
    v->allocator=*a;
    v->begin=count?static_cast<InlineEntry*>(v->allocator.allocate(size_t(count)*sizeof(InlineEntry))):nullptr;
    v->end=v->begin;
    v->capacity=reinterpret_cast<InlineEntry*>(reinterpret_cast<uintptr_t>(v->begin)+size_t(count)*sizeof(InlineEntry));
    return v;
}
KeyNameNode* CreateNameNode(NameTable* t,const KeyNameValue* v) {
    auto* n=static_cast<KeyNameNode*>(t->allocator.allocate(sizeof(KeyNameNode)));
    if(n) { n->value.key=v->key; CallGameMethod<void*,0x00408EB0>(&n->value.name,&v->name); }
    n->next=nullptr; return n;
}
InlineEntry* AssignInline(const InlineEntry* first,const InlineEntry* last,InlineEntry* d) {
    while(first!=last) { Assign(first,d); ++first; ++d; } return d;
}
InlineEntry* AssignInlineBackward(const InlineEntry* first,const InlineEntry* last,InlineEntry* d) {
    while(first!=last) { --last; --d; Assign(last,d); } return d;
}
TreeNode** InsertNode(TreeView* t,TreeNode** out,TreeNode* parent,const FilterEntry* v,uint32_t flag) {
    bool right=false;
    if(!uint8_t(flag) && parent!=Anchor(t) && !Less(*v,parent->value)) right=true;
    auto* n=static_cast<TreeNode*>(t->allocator.allocate(sizeof(TreeNode)));
    n->value=*v;
    CallGameFunction<void,0x004E0C40>(n,parent,Anchor(t),uint32_t(right));
    ++t->size; *out=n; return out;
}
InsertResult* InsertFilter(TreeView* t,InsertResult* out,const FilterEntry* v,uint32_t) {
    TreeNode* x=t->parent; TreeNode* parent=Anchor(t); bool left=true;
    while(x) { parent=x; left=Less(*v,x->value); x=left?x->left:x->right; }
    TreeNode* previous=parent;
    if(left) {
        if(parent==t->left) { TreeNode* n; InsertNode(t,&n,parent,v,0); out->node=reinterpret_cast<FilterNode*>(n); out->inserted=1; return out; }
        previous=CallGameFunction<TreeNode*,0x004E0B60>(previous);
    }
    if(Less(previous->value,*v)) {
        TreeNode* n; InsertNode(t,&n,parent,v,0); out->node=reinterpret_cast<FilterNode*>(n); out->inserted=1;
    } else { out->node=reinterpret_cast<FilterNode*>(previous); out->inserted=0; }
    return out;
}
void* FindFactory(m03::ResourceManager* m,uint32_t type,uint32_t variant) {
    Lock(m,0x1a0); auto* node=Lookup(m,&type); void* result=nullptr;
    auto** buckets=Field<FactoryNode**>(m,0x7c); auto count=Field<uint32_t>(m,0x80);
    if(node!=buckets[count]) for(auto* e=node->begin;e!=node->end;++e) {
        if(variant==0xffffffff || uint8_t(CallGameVirtual<uint32_t,12>(e->callback,type,variant))) { result=e->callback; break; }
    }
    Unlock(m,0x1a0); return result;
}
int CollectFactories(m03::ResourceManager* m,List* list,uint32_t type) {
    Lock(m,0x1a0); int result=0;
    auto** buckets=Field<FactoryNode**>(m,0x7c); auto count=Field<uint32_t>(m,0x80);
    auto append=[&](FactoryNode* n) { for(auto* e=n->begin;e!=n->end;++e) { ++result; Append(list,e->callback); } };
    if(type==0xffffffff) {
        auto** cell=buckets; while(!*cell) ++cell; auto* n=*cell;
        while(n!=buckets[count]) { append(n); n=n->next; if(!n) { do { ++cell; } while(!*cell); n=*cell; } }
    } else { auto* n=Lookup(m,&type); if(n!=buckets[count]) append(n); }
    Unlock(m,0x1a0); return result;
}
int CollectCallbacks(m03::ResourceManager* m,List* list) {
    auto* e=Field<InlineEntry*>(m,0xa0); auto* end=Field<InlineEntry*>(m,0xa4); int count=0;
    for(;e!=end;++e) { ++count; if(list) Append(list,e->callback); } return count;
}
void* FindTypeName(m03::ResourceManager* m,uint32_t type) {
    Lock(m,0x140); void* out[2];
    CallGameMethod<void*,0x00D4E550>(&Field<uint32_t>(m,0xf0),out,&type);
    auto** buckets=Field<void**>(m,0xf4); auto count=Field<uint32_t>(m,0xf8);
    void* result=out[0]==buckets[count]?nullptr:Field<void*>(out[0],4);
    Unlock(m,0x140); return result;
}
TypeNameResult* InsertTypeName(NameTable* t,TypeNameResult* out,const TypeNameValue* v,uint32_t) {
    uint32_t index=v->type%t->count; auto** buckets=reinterpret_cast<TypeNameNode**>(t->buckets);
    for(auto* n=buckets[index];n;n=n->next) if(n->value.type==v->type) { out->node=n; out->bucketCell=buckets+index; out->inserted=0; return out; }
    auto policy=Policy(t);
    auto* n=static_cast<TypeNameNode*>(t->allocator.allocate(sizeof(TypeNameNode)));
    if(n) { n->value.type=v->type; CallGameMethod<void*,0x00408EB0>(&n->value.name,&v->name); }
    n->next=nullptr;
    if(uint8_t(policy.grow)) { index=v->type%policy.count; CallGameMethod<void,0x004B0330>(t,policy.count); }
    buckets=reinterpret_cast<TypeNameNode**>(t->buckets); n->next=buckets[index]; buckets[index]=n; ++t->size;
    out->node=n; out->bucketCell=buckets+index; out->inserted=1; return out;
}
KeyResult* InsertKeyName(NameTable* t,KeyResult* out,const KeyNameValue* v,uint32_t) {
    uint32_t words[4]; std::memcpy(words,&v->key,16); uint32_t hash=words[0]^words[3],index=hash%t->count;
    auto** buckets=reinterpret_cast<KeyNameNode**>(t->buckets);
    auto* n=CallGameMethod<KeyNameNode*,0x009A32B0>(t,buckets[index],&v->key,hash);
    if(n) { out->node=n; out->bucketCell=buckets+index; out->inserted=0; return out; }
    auto policy=Policy(t); n=CreateNameNode(t,v);
    if(uint8_t(policy.grow)) { index=hash%policy.count; CallGameMethod<void,0x004AA730>(t,policy.count); }
    buckets=reinterpret_cast<KeyNameNode**>(t->buckets); n->next=buckets[index]; buckets[index]=n; ++t->size;
    out->node=n; out->bucketCell=buckets+index; out->inserted=1; return out;
}
}

namespace ts3::resource {
#if defined(_M_IX86)
namespace d=a04_detail;
uint32_t a04::InsertNode(void* out,void* parent,void* value,uint32_t flag) { return uint32_t(uintptr_t(d::InsertNode(reinterpret_cast<d::TreeView*>(this),static_cast<d::TreeNode**>(out),static_cast<d::TreeNode*>(parent),static_cast<const d::FilterEntry*>(value),flag))); }
uint32_t a04::ConstructVector(uint32_t n,void* allocator) { return uint32_t(uintptr_t(d::ConstructVector(reinterpret_cast<d::VectorView*>(this),n,static_cast<const GameAllocator*>(allocator)))); }
uint32_t a04::CreateNameNode(void* value) { return uint32_t(uintptr_t(d::CreateNameNode(reinterpret_cast<d::NameTable*>(this),static_cast<const d::KeyNameValue*>(value)))); }
uint32_t a04::AssignInline(void* first,void* last,void* dest) { return uint32_t(uintptr_t(d::AssignInline(static_cast<const d::InlineEntry*>(first),static_cast<const d::InlineEntry*>(last),static_cast<d::InlineEntry*>(dest)))); }
uint32_t a04::AssignInlineBackward(void* first,void* last,void* dest) { return uint32_t(uintptr_t(d::AssignInlineBackward(static_cast<const d::InlineEntry*>(first),static_cast<const d::InlineEntry*>(last),static_cast<d::InlineEntry*>(dest)))); }
uint32_t a04::FindFactory(uint32_t type,uint32_t variant) { return uint32_t(uintptr_t(d::FindFactory(reinterpret_cast<m03::ResourceManager*>(this),type,variant))); }
uint32_t a04::CollectFactories(void* list,uint32_t type) { return d::CollectFactories(reinterpret_cast<m03::ResourceManager*>(this),static_cast<d::List*>(list),type); }
uint32_t a04::CollectCallbacks(void* list) { return d::CollectCallbacks(reinterpret_cast<m03::ResourceManager*>(this),static_cast<d::List*>(list)); }
uint32_t a04::FindTypeName(uint32_t type) { return uint32_t(uintptr_t(d::FindTypeName(reinterpret_cast<m03::ResourceManager*>(this),type))); }
uint32_t a04::InsertFilter(void* out,void* value,uint32_t flag) { return uint32_t(uintptr_t(d::InsertFilter(reinterpret_cast<d::TreeView*>(this),static_cast<d::InsertResult*>(out),static_cast<const d::FilterEntry*>(value),flag))); }
uint32_t a04::InsertTypeName(void* out,void* value,uint32_t flag) { return uint32_t(uintptr_t(d::InsertTypeName(reinterpret_cast<d::NameTable*>(this),static_cast<d::TypeNameResult*>(out),static_cast<const d::TypeNameValue*>(value),flag))); }
uint32_t a04::InsertKeyName(void* out,void* value,uint32_t flag) { return uint32_t(uintptr_t(d::InsertKeyName(reinterpret_cast<d::NameTable*>(this),static_cast<d::KeyResult*>(out),static_cast<const d::KeyNameValue*>(value),flag))); }
#endif
}
