#include "ts3/resource/m03_D03.h"
#include "ts3/layout.h"
#include "ts3/misc/cls_4a3a40_x.h"
#include "ts3/misc/clu_4a6c30_x.h"
#include "ts3/resource/m03_A01.h"
#include <cstring>
namespace ts3::resource::d03 {
namespace {
template<class T> T* At(uintptr_t address) {
#if defined(_M_IX86)
 return reinterpret_cast<T*>(address);
#else
 (void)address; AbortNotRewritten("m03_D03 locale global/data boundary"); return nullptr;
#endif
}
int Compare(const char16_t* a,const char16_t* b) {
#if defined(_M_IX86)
 using F=int(__cdecl*)(const char16_t*,const char16_t*);
 return (*At<F>(0x00f95758))(a,b);
#else
 while(*a && *a==*b) { ++a; ++b; } return int(*a)-int(*b);
#endif
}
}
void SetFlag(uint32_t flag) { *At<uint8_t>(0x011ce2c4)=uint8_t(flag); }
uint64_t FindLocale(Locale*,const char16_t* name) {
 if(!name) return 0;
 for(unsigned i=0;i<23;++i) if(!Compare(name,*At<const char16_t*>(0x01152d30+i*12))) return i;
 return 0;
}
uint64_t FindAlias(Locale* self,const char16_t* name) {
 if(!name) return 0;
 for(unsigned i=0;i<5;++i) if(!Compare(name,*At<const char16_t*>(0x01152e44+i*8)))
   return FindLocale(self,*At<const char16_t*>(0x01152e48+i*8));
 return FindLocale(self,name);
}
const char16_t* TypeName(Locale*) { return At<char16_t>(0x00fd98b8); }
uintptr_t SelectFont(Locale* self) {
 const uintptr_t names[]={0xfd9a60,0xfd9a24,0xfd99e8,0xfd99ac,0xfd9974,0xfd9940,0xfd9934,0xfd9928,0xfd991c,0xfd9910};
 const uintptr_t fonts[]={0xfd9a30,0xfd99f4,0xfd99b8,0xfd9980,0xfd994c};
 uintptr_t font=0xfd98f0;
 for(unsigned i=0;i<10;++i) if(!Compare(self->locale,At<char16_t>(names[i]))) { font=i<5?fonts[i]:0xfd98c4; break; }
 return CallGameFunction<uintptr_t,0x006922f0>(At<char16_t>(font));
}
void Clear(Locale* self) {
 auto* end=reinterpret_cast<Node*>(&self->next);
 for(auto* n=self->next;n!=end;n=n->next) if(n->value.resource) {
   CallGameVirtual<void,1>(n->value.resource); n->value.resource=nullptr;
 }
 for(auto* n=self->next;n!=end;) { auto* old=n; n=n->next; CallGameFunction<void,0x004e3f70>(old); }
 self->next=self->prev=end;
}
Iterator* Find(Table* self,Iterator* out,const uint32_t* key) {
 auto** bucket=&self->buckets[key[0]%7919u];
 auto* node=*bucket;
 while(node && (node->key[0]!=key[0] || node->key[1]!=key[1])) node=node->next;
 if(!node) { bucket=&self->sentinel; node=*bucket; }
 *out={node,bucket}; return out;
}
Locale* Construct(Locale* self,void* allocator) {
 CallGameMethod<void,0x0059ecc0>(self,0x6224dd27u,0u);
 self->vtable=At<void>(0xfd9a6c);
 if(!allocator) allocator=CallGameFunction<void*,0x00931020>();
 self->allocator=allocator;
 for(auto& s:self->state) s=0;
 self->mapVtable=At<void>(0xfd8248); self->type=0x220557da; self->one=1;
 self->next=self->prev=reinterpret_cast<Node*>(&self->next); self->tag=At<void>(0xfd9aa0);
 *At<Locale*>(0x011ce2d0)=self;
 return self;
}
Locale* Delete(Locale* self,uint32_t flags) {
 reinterpret_cast<ts3::misc::ResourceMapList*>(self)->DestructRefList();
 if(flags&1) CallGameFunction<void,0x004e3f70>(self);
 return self;
}
uintptr_t Register(void* allocator) {
 void* service=*At<void*>(0x011ce3f4);
 uintptr_t result=CallGameMethod<uintptr_t,0x0059efe0>(service,0x6224dd27u);
 if(uint8_t(result)) return result;
 auto* p=CallGameFunction<Locale*,0x004e3f90>(sizeof(Locale),At<char>(0xfd9ad0),0u,0u,0u,0u);
 if(p) Construct(p,allocator);
 return CallGameMethod<uintptr_t,0x0059f500>(service,p,0u);
}
Node** Insert(void* list,Node** out,Node* before,const Payload* value) {
 void* allocator=*reinterpret_cast<void**>(static_cast<uint8_t*>(list)+2*sizeof(void*));
 auto* n=CallGameFunction<Node*,0x004e3f90>(sizeof(Node),allocator,0u,0u,At<char>(0xf98980),0xe7u);
 // EA tests allocation+8, not allocation: ordinary null allocation is NOT handled.
 if(reinterpret_cast<uintptr_t>(n)+2*sizeof(void*)) n->value=*value;
 n->next=before; n->prev=before->prev; before->prev->next=n; before->prev=n;
 *out=n; return out;
}
uintptr_t Stop(Locale* self) {
 Clear(self);
 void* manager=m03::a01::Singleton();
 if(!manager) return 0;
 return CallGameVirtual<uintptr_t,9>(manager,0u,At<void>(0x0059bfc0),self,&self->mapVtable,0u);
}
uintptr_t Lookup(Locale* self,const char16_t* text) {
 ts3::misc::ResourceSortKey key{};
 if(!ts3::misc::ParseResourceKey(text,&key)) return 0;
 uint32_t words[4]; std::memcpy(words,&key,sizeof(words));
 uint64_t id=uint64_t(words[0])|(uint64_t(words[1])<<32);
 return CallGameMethod<uintptr_t,0x0059c5b0>(self,id);
}
}
