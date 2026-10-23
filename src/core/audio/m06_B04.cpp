#include "ts3/audio/m06_B04.h"
#include "ts3/cas/clu_4a6710_CAS.h"
#include <cstring>
#include <xmmintrin.h>
namespace ts3::audio::b04 {
namespace {
void* Address(uintptr_t a) {
#if defined(_M_IX86)
 return reinterpret_cast<void*>(a);
#else
 (void)a; AbortNotRewritten("m06_B04 game data"); return nullptr;
#endif
}
uintptr_t Ref(void* p,int slot) { return slot?CallGameVirtual<uintptr_t,1>(p):CallGameVirtual<uintptr_t,0>(p); }
void Interfaces(Owner* s,uintptr_t first) { s->primary=Address(first);s->secondary=Address(0x1027a84);s->third=Address(0x1027b44);s->fourth=Address(0x1027b3c); }
uintptr_t Assign(Pair* d,const Pair* s,uintptr_t r) {d->id=s->id;auto* p=s->object;auto* old=d->object;if(p!=old){if(p)r=Ref(p,0);d->object=p;if(old)r=Ref(old,1);}return r;}
}
Owner* Construct(Owner* s) {CallGameMethod<void,0x861e50>(s);Interfaces(s,0x1027b48);return s;}
uint32_t Type(void*) {return 0x57572dacu;}
Owner* Delete(Owner* s,uint32_t flags) {
 Interfaces(s,0x1027a90);if(s->flags[3])CallGameMethod<void,0x861290>(s);
 CallGameMethod<void,0x8b31e0>(&s->pairs);s->fourth=Address(0xfb2cbc);s->third=Address(0x107396c);
 if(flags&1)CallGameFunction<void,0x4e3f70>(s);return s;
}
Owner* DeleteAdjusted(void* s,uint32_t f) {return Delete(reinterpret_cast<Owner*>(static_cast<uint8_t*>(s)-offsetof(Owner,fourth)),f);}
uintptr_t Insert(PairVector* s,Pair* pos,const Pair* source) {
 if(s->end!=s->capacity) {
  if(source>=pos && source<s->end)++source;
  auto* tail=s->end;*tail=tail[-1];if(tail->object)Ref(tail->object,0);
  auto r=CallGameFunction<uintptr_t,0xc8c850>(pos,s->end-1,s->end);
  r=Assign(pos,source,r);++s->end;return r;
 }
 auto n=s->end-s->begin;auto count=n?2*n:1;
 auto* allocation=CallGameFunction<Pair*,0x4e3f90>(count*sizeof(Pair),s->label,0,0,Address(0xf98980),0xe7);
 Pair* out=nullptr;CallGameFunction<void,0xc8c790>(&out,s->begin,pos,allocation);
 auto* inserted=out;if(inserted){*inserted=*source;if(inserted->object)Ref(inserted->object,0);}
 CallGameFunction<void,0xc8c790>(&out,pos,s->end,inserted+1);
 CallGameMethod<void,0xc8c9b0>(s,s->begin,s->end);
 if(s->begin && s->begin!=s->storage)CallGameFunction<void,0x4e3f70>(s->begin);
 s->begin=allocation;s->end=out;s->capacity=allocation+count;return count;
}
uintptr_t Add(Owner* s,uint32_t id,void* object) {
 Pair* found=nullptr;for(auto* p=s->pairs.begin;p!=s->pairs.end;++p)if(p->id==id){found=p;break;}
 uintptr_t r=0;
 if(!found){
  if(object)Ref(object,0);Pair temp{id,object};if(object)Ref(object,0);
  auto* dest=s->pairs.end;r=reinterpret_cast<uintptr_t>(dest);
  if(dest>=s->pairs.capacity)r=Insert(&s->pairs,dest,&temp);
  else {++s->pairs.end;if(dest){*dest=temp;if(object)r=Ref(object,0);else return (r&~uintptr_t(255))|1;}}
  if(temp.object)r=Ref(temp.object,1);if(object)r=Ref(object,1);
 }else{
  auto* system=CallGameFunction<void*,0x8b0a70>();auto key=CallGameVirtual<uint64_t,71>(system,id);
  if(!key)return 0;
  auto* old=found->object;void* delegate=nullptr;
  if(old)delegate=CallGameVirtual<void*,3>(old,0x4e73a43au);
  if(delegate)return CallGameVirtual<uintptr_t,4>(delegate,object);
  auto* mem=CallGameFunction<void*,0x8a6590>(112);void* node=nullptr;
  if(mem){node=CallGameMethod<void*,0x8a69d0>(mem,s);if(node)Ref(node,0);}
  // EA deliberately dereferences the failed allocation on this path.
  CallGameVirtual<void,4>(node);CallGameVirtual<void,12>(node,1);
  CallGameMethod<void,0x8a6530>(node,uint32_t(key),uint32_t(key>>32));
  auto* view=static_cast<uint8_t*>(node)+sizeof(void*);
  CallGameVirtual<void,4>(view,old);CallGameVirtual<void,4>(view,object);
  if(node!=found->object){Ref(node,0);auto* previous=found->object;found->object=node;if(previous)Ref(previous,1);}
  r=Ref(node,1);
 }
 return (r&~uintptr_t(255))|1;
}
void Select(Owner* s,Table* t,const float* position) {
 if(!s->flags[0])return;s->selected.end=s->selected.begin;
 float radius=s->radius*s->radius;auto** b=t->buckets;auto* n=*b;while(!n)n=*++b;
 for(;n!=t->sentinel;){
  if(!(n->flags&1) && n->kind==s->kind){
   __m128 d=_mm_sub_ps(_mm_load_ps(n->position),_mm_load_ps(position));auto sq=_mm_mul_ps(d,d);
   auto sum=_mm_add_ps(_mm_add_ps(_mm_shuffle_ps(sq,sq,2),_mm_shuffle_ps(sq,sq,1)),sq);
   if(radius>_mm_cvtss_f32(sum)){
    auto* end=s->selected.end;
    if(end<s->selected.capacity){++s->selected.end;if(end)*end=n;}
    else CallGameMethod<void,0x8621c0>(&s->selected,end,&n);
   }
  }
  n=n->next;while(!n)n=*++b;
 }
}
uintptr_t Grow(void* pool) {
 auto& capacity=*reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(pool)+12);uint32_t n=capacity&0x7fffffffu;
 if(capacity&0x80000000u){n*=2;if(n>340)n=340;capacity=(capacity&0x80000000u)|n;}
 void* allocator=nullptr;
#if defined(_M_IX86)
 allocator=*reinterpret_cast<void**>(0x11cb864);
#endif
 auto* p=CallGameMethod<void*,0x4e6a20>(allocator,48*n+16,16,0,0);
 return CallGameMethod<uintptr_t,0x7ccea0>(pool,p,16,48,16,n);
}
uintptr_t Resolve(Lookup* s,const TextVector* v,const char16_t* base) {
 int count=int((v->end-v->begin)/250);uint32_t limit=(1u<<(count&31))-1;
 char16_t text[250],saved[250];
 for(int k=count;k>0;--k){uintptr_t last=0;uint32_t mask=(1u<<(k&31))-1;
  while(mask<=limit){
   ts3::cas::CopyUtf16(reinterpret_cast<uint16_t*>(text),reinterpret_cast<const uint16_t*>(base),250);
   for(int j=0;j<count;++j)if((mask>>(j&31))&1)CallGameFunction<void,0x4ff470>(text,v->begin+j*250,250);
   auto hash=CallGameFunction<uint32_t,0x4fa640>(text,0x811c9dc5u,1);uintptr_t value=0;
   if(CallGameVirtual<uint8_t,7>(s->service,hash,&value)){last=value;ts3::cas::CopyUtf16(reinterpret_cast<uint16_t*>(saved),reinterpret_cast<const uint16_t*>(text),250);}
   uint32_t low=uint32_t(-mask)&mask,next=mask+low;mask=(((next^mask)>>2)/low)|next;
  }
  if(last)return last;
 }
 auto hash=CallGameFunction<uint32_t,0x4fa640>(base,0x811c9dc5u,1);uintptr_t value=0;
 return CallGameVirtual<uint8_t,7>(s->service,hash,&value)?value:0;
}
const void* CopyVector(void* d,const void* s) {auto v=_mm_load_ps(static_cast<const float*>(s));_mm_store_ps(static_cast<float*>(d),v);return s;}
Iterator* Unlink(Table* t,Iterator* out,Node* node,Node** bucket) {
 *out={node->next,bucket};while(!out->node)out->node=*++out->bucket;
 auto* prev=*bucket;if(prev==node)*bucket=prev->next;else{while(prev->next!=node)prev=prev->next;prev->next=node->next;}
 --t->count;return out;
}
void** CopyRecords(void** out,const void* first,const void* last,void* dest) {
 auto* s=static_cast<const uint8_t*>(first);auto* d=static_cast<uint8_t*>(dest);*out=d;
 while(s!=last){if(d)std::memcpy(d,s,500);s+=500;d+=500;}*out=d;return out;
}
Iterator* Find(Table* t,Iterator* out,const uint32_t* key) {
 auto** bucket=&t->buckets[key[0]%53u];auto* n=*bucket;
 while(n && (n->key[0]!=key[0] || n->key[1]!=key[1]))n=n->next;
 if(!n){bucket=&t->sentinel;n=*bucket;}*out={n,bucket};return out;
}
}
