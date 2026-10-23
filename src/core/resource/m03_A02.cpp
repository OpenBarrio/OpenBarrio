#include "ts3/resource/m03_A02.h"
#include "ts3/layout.h"
#include <cstring>
namespace ts3::resource::m03::a02 {
namespace {
template<class R,unsigned S,class... A> R V(void* p,A... a) { return CallGameVirtual<R,S>(p,a...); }
void* Lookup(Manager* m,const void* k,const Leaves& l) { return l.lookup?l.lookup(m,k):CallGameMethod<void*,0x004B0070>(m,k); }
void Lock(Manager* m,const Leaves& l) { auto* p=reinterpret_cast<uint8_t*>(m)+0x170; if(l.lock) l.lock(p); else CallGameMethod<void,0x004E16F0>(p,reinterpret_cast<void*>(uintptr_t(0xFB2CD0))); }
void Unlock(Manager* m,const Leaves& l) { auto* p=reinterpret_cast<uint8_t*>(m)+0x170; if(l.unlock) l.unlock(p); else CallGameMethod<void,0x004E17B0>(p); }
bool Less(const uint32_t* a,const uint32_t* b) { if(a[3]!=b[3]) return int32_t(a[3])>int32_t(b[3]); for(int i=0;i<3;++i) if(a[i]!=b[i]) return a[i]<b[i]; return false; }
struct Node28 { uint32_t hash; uint8_t data[24]; void* next; };
struct Node20 { uint32_t hash; uint8_t data[16]; void* next; };
TS3_X86_OFFSET(Node28,next,0x1c);
TS3_X86_OFFSET(Node20,next,0x14);
template<class N> void Rehash(Hash* h,uint32_t count) {
 auto** buckets=static_cast<void**>(h->allocator->Alloc((size_t(count)+1)*sizeof(void*),h->name,h->flags));
 std::memset(buckets,0,size_t(count)*sizeof(void*)); buckets[count]=reinterpret_cast<void*>(~uintptr_t(0));
 for(uint32_t i=0;i<h->count;++i) while(h->buckets[i]) { auto* n=static_cast<N*>(h->buckets[i]); auto b=n->hash%count; h->buckets[i]=n->next; n->next=buckets[b]; buckets[b]=n; }
 if(h->count>1) h->allocator->Free(h->buckets,(size_t(h->count)+1)*sizeof(void*));
 h->buckets=buckets; h->count=count;
}
}
uint8_t Notify(Manager* m,const void* key,unsigned flags,const Leaves& l) {
 if(void* p=Lookup(m,&static_cast<const ResourcePrefix*>(key)->key,l)) return uint8_t(V<unsigned,7>(p,key,flags));
 if(uint8_t(flags)) for(auto* p=m->begin;p!=m->end;++p) if(uint8_t(V<unsigned,7>(p->object,key,flags))) return true;
 return false;
}
uintptr_t Broadcast(Manager* m,void* target,void* arg) { if(target) return V<uintptr_t,10>(target,arg); uintptr_t r=reinterpret_cast<uintptr_t>(m); for(auto* p=m->begin;p!=m->end;++p) r=V<uintptr_t,10>(p->object,arg); return r; }
TreeNode** Find(Tree* t,TreeNode** out,const uint32_t* key) {
 auto* sentinel=reinterpret_cast<TreeNode*>(&t->sentinelLeft); auto* best=sentinel; auto* n=t->root;
 while(n) { if(!Less(n->key,key)) { best=n; n=n->right; } else n=n->left; }
 *out=(best!=sentinel && !Less(key,best->key))?best:sentinel; return out;
}
void ReleaseAdjusted(Pair* b,Pair* e) { for(;reinterpret_cast<uintptr_t>(b)<reinterpret_cast<uintptr_t>(e);++b) if(b->object) V<void,1>(static_cast<uint8_t*>(b->object)+sizeof(void*)); }
void ReleasePairs(Pair* b,Pair* e) { for(;reinterpret_cast<uintptr_t>(b)<reinterpret_cast<uintptr_t>(e);++b) if(b->object) V<void,2>(b->object); }
void Rehash28(Hash* h,uint32_t n) { Rehash<Node28>(h,n); }
void Rehash20(Hash* h,uint32_t n) { Rehash<Node20>(h,n); }
Pair** CopyAdjusted(Pair** out,Pair* b,Pair* e,Pair* d) { *out=d; for(;b!=e;++b) { if(*out) { (*out)->object=b->object; if(b->object) V<void,0>(static_cast<uint8_t*>(b->object)+sizeof(void*)); (*out)->tag=b->tag; } *out=reinterpret_cast<Pair*>(reinterpret_cast<uintptr_t>(*out)+sizeof(Pair)); } return out; }
Pair** CopyPairs(Pair** out,Pair* b,Pair* e,Pair* d) { *out=d; for(;b!=e;++b) { if(*out) { (*out)->object=b->object; if(b->object) V<void,1>(b->object); (*out)->tag=b->tag; } *out=reinterpret_cast<Pair*>(reinterpret_cast<uintptr_t>(*out)+sizeof(Pair)); } return out; }
uint8_t Load(Manager* m,unsigned notify,const uint32_t* key,void** out,unsigned flags,void* db,void* factory,const void* alias,const Leaves& l) {
 Lock(m,l); uint8_t ok=0;
 if(uint8_t(notify)) { const void* k=alias?alias:key; if(void* p=V<void*,22>(m,k)) { ok=uint8_t(V<unsigned,6>(p,k,out)); if(ok) { Unlock(m,l); return true; } } }
 if(!db) db=V<void*,17>(m,key);
 if(db) { if(!factory) factory=V<void*,11>(m,key[2],unsigned(-1));
 if(factory) { if(!out) ok=true; else { void* stream=nullptr;
 if(uint8_t(V<unsigned,13>(db,key,&stream,1u,3u,1u,0u))) ok=uint8_t(V<unsigned,7>(factory,stream,out,flags,key[2]));
 if(ok) { if(alias) { if(l.rename) l.rename(*out,alias); else CallGameMethod<void,0x004AF0E0>(*out,alias); } if(uint8_t(notify)) V<unsigned,23>(m,*out,1u); }
 if(stream) V<void,2>(stream);
 } } }
 Unlock(m,l); return ok;
}
uint8_t LoadQuiet(Manager* m,const uint32_t* k,void** o,unsigned f,void* d,void* fac,const void* a,const Leaves& l) { return Load(m,0,k,o,f,d,fac,a,l); }
uint8_t Save(Manager* m,void* resource,unsigned flags,void* db,void* factory,const uint32_t* key) {
 if(!resource) return false; uint32_t k[4]; std::memcpy(k,key?key:reinterpret_cast<const uint32_t*>(&static_cast<const ResourcePrefix*>(resource)->key),16);
 if(!db) db=V<void*,17>(m,k); if(!db) return false;
 if(!factory) factory=V<void*,11>(m,k[2],unsigned(-1)); if(!factory) return false;
 void* stream=nullptr; uint8_t ok=0;
 if(uint8_t(V<unsigned,13>(db,k,&stream,3u,2u,1u,0u))) ok=uint8_t(V<unsigned,10>(factory,resource,stream,flags,k[2]));
 if(stream) V<void,2>(stream); return ok;
}

// ABI-compatible replace entries (fastcall dummy EDX models thiscall).
uint8_t __fastcall NotifyEntry(Manager* m,void*,const void* k,unsigned f) { return Notify(m,k,f); }
uintptr_t __fastcall BroadcastEntry(Manager* m,void*,void* t,void* a) { return Broadcast(m,t,a); }
TreeNode** __fastcall FindEntry(Tree* t,void*,TreeNode** o,const uint32_t* k) { return Find(t,o,k); }
void __stdcall ReleaseAdjustedEntry(Pair* b,Pair* e) { return ReleaseAdjusted(b,e); }
void __stdcall ReleasePairsEntry(Pair* b,Pair* e) { return ReleasePairs(b,e); }
void __fastcall Rehash28Entry(Hash* h,void*,uint32_t n) { return Rehash28(h,n); }
void __fastcall Rehash20Entry(Hash* h,void*,uint32_t n) { return Rehash20(h,n); }
Pair** __cdecl CopyAdjustedEntry(Pair** o,Pair* b,Pair* e,Pair* d) { return CopyAdjusted(o,b,e,d); }
Pair** __cdecl CopyPairsEntry(Pair** o,Pair* b,Pair* e,Pair* d) { return CopyPairs(o,b,e,d); }
uint8_t __fastcall LoadEntry(Manager* m,void*,unsigned n,const uint32_t* k,void** o,unsigned f,void* d,void* fac,const void* a) { return Load(m,n,k,o,f,d,fac,a); }
uint8_t __fastcall LoadQuietEntry(Manager* m,void*,const uint32_t* k,void** o,unsigned f,void* d,void* fac,const void* a) { return LoadQuiet(m,k,o,f,d,fac,a); }
uint8_t __fastcall SaveEntry(Manager* m,void*,void* r,unsigned f,void* d,void* fac,const uint32_t* k) { return Save(m,r,f,d,fac,k); }
}
