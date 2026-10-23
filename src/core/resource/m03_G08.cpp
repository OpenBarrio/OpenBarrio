#include "ts3/resource/m03_G08.h"
#include <cstring>
#include <atomic>
namespace ts3::resource::g08 {
namespace {
template<class T> T* Data(uintptr_t a) {
#if defined(_M_IX86)
 return reinterpret_cast<T*>(a);
#else
 (void)a; AbortNotRewritten("G08 locale/string/allocator boundary"); return nullptr;
#endif
}
int Compare(const char16_t* a,uintptr_t b) {
 using F=int(__cdecl*)(const char16_t*,const char16_t*);
 return (*Data<F>(0xf95758))(a,Data<char16_t>(b));
}
struct Hit { d03::Node* owner; const char16_t* text; };
Hit Find(Locale* s,const uint32_t* key) {
 auto* end=reinterpret_cast<d03::Node*>(&s->next);
 for(auto* p=s->next;p!=end;p=p->next) if(p->value.resource) {
  auto* t=static_cast<Table*>(p->value.resource);
  d03::Iterator it;
  CallGameMethod<void,0x0059AF50>(&t->buckets,&it,key);
  if(it.node!=reinterpret_cast<d03::HashNode*>(t->sentinel)) {
   auto* n=reinterpret_cast<Entry*>(it.node);
   if(n->text && *n->text) return {p,n->text};
  }
 }
 return {nullptr,nullptr};
}
void Replace(String* s,uintptr_t pattern,uintptr_t first,uintptr_t last,int start,unsigned width) {
 int pos=CallGameMethod<int,0x009F26E0>(s,Data<char16_t>(pattern),start);
 while(pos!=-1) {
  auto remain=static_cast<uint32_t>(s->end-s->begin-pos);
  auto n=remain<width?remain:width;
  CallGameMethod<void,0x005416D0>(s,s->begin+pos,s->begin+pos+n,Data<char16_t>(first),Data<char16_t>(last));
  pos=CallGameMethod<int,0x009F26E0>(s,Data<char16_t>(pattern),pos);
 }
}
void FreeString(String& s) {
 auto bytes=reinterpret_cast<uintptr_t>(s.capacity)-reinterpret_cast<uintptr_t>(s.begin);
 if(static_cast<int32_t>(bytes&0xfffffffeu)>2 && s.begin) CallGameFunction<void,0x004E3F70>(s.begin);
}
struct Variant { void* vt; uint32_t reserved; uint16_t flags,kind; uint8_t rest[20]; };
uint32_t Managed(void* script,String& s,bool japanese) {
 Variant v{}; v.vt=Data<void>(0xfb96b4);
 CallGameMethod<void,0x004F5800>(&v,19,9,&s,16,1);
 if(japanese) Replace(&s,0xfd9e0c,0xfd9e04,0xfd9e0a,0,1);
 uint32_t result=0;
 CallGameVirtual<void,20>(script,&v,&result);
 v.vt=Data<void>(0xfb96b4);
 if(v.flags&4) CallGameMethod<void,0x004F55A0>(&v,0);
 FreeString(s);
 return result;
}
}
uintptr_t __fastcall ChangeLocale(Locale* self,void*,const char16_t* name) {
 const uint64_t id=d03::FindLocale(self,name);
 const uint32_t shifted=uint32_t(id)<<24;
 if(!self->state[0] && self->state[1]==shifted) return shifted;
 self->state[0]=0; self->state[1]=name?shifted:0;
 const uint64_t alias=name?d03::FindAlias(self,name)<<56:0;
 self->state[2]=uint32_t(alias); self->state[3]=uint32_t(alias>>32);
 // Original small-string has an inline UTF16 buffer beginning at word 8.
 alignas(void*) uint8_t temporary[52]{};
 CallGameMethod<void,0x00446BF0>(temporary,name?name:Data<char16_t>(0xf9a458));
 CallGameMethod<void,0x00455930>(temporary);
 CallGameFunction<void,0x0044B9B0>(temporary,0,0);
 auto* text=*reinterpret_cast<char16_t**>(temporary);
 CallGameFunction<void,0x004FF380>(self->locale,text,6);
 CallGameMethod<void,0x0059BDE0>(self);
 d03::SetFlag(!Compare(self->locale,0xfd9a60) || !Compare(self->locale,0xfd9bf8));
 void* bus=CallGameFunction<void*,0x007692C0>();
 if(bus) {
  struct Message { void* vt; int32_t refs; uint32_t lo,hi; };
  auto* m=CallGameFunction<Message*,0x004E3F90>(sizeof(Message),Data<char>(0xfd9cb8),0,0,0,0);
  if(m) {
   m->vt=Data<void>(0x100df80); std::atomic_ref<int32_t>(m->refs).exchange(0);
   m->lo=uint32_t(id); m->hi=uint32_t(id>>32);
   CallGameVirtual<void,1>(m);
  }
  CallGameVirtual<void,7>(bus,0xf0707d65u,m,0,0);
  if(m) CallGameVirtual<void,2>(m);
 }
 auto* cap=*reinterpret_cast<uint8_t**>(temporary+2*sizeof(void*));
 uintptr_t result=reinterpret_cast<uintptr_t>(text);
 auto bytes=reinterpret_cast<uintptr_t>(cap)-result;
 if(static_cast<int32_t>(bytes&0xfffffffeu)>2 && text && text!=*reinterpret_cast<char16_t**>(temporary+4*sizeof(void*)))
  result=CallGameFunction<uintptr_t,0x004E3F70>(text);
 return result;
}
uint32_t __fastcall ManagedLookup(Locale* self,void*,uint32_t lo,uint32_t hi) {
 void* script=CallGameFunction<void*,0x007692B0>();
 const uint32_t key[]={lo,hi}; auto hit=Find(self,key);
 String s{}; s.tag=Data<char>(0xfd9cb8);
 if(hit.text) CallGameMethod<void,0x0040B700>(&s,hit.text);
 else { s.begin=s.end=Data<char16_t>(0x11c7e54); s.capacity=s.begin+1; }
 return Managed(script,s,hit.owner && hit.owner!=self->next && !Compare(self->locale,0xfd99e8));
}
uint8_t __fastcall TextLookup(Locale* self,void*,uint32_t lo,uint32_t hi,String* out) {
 const uint32_t key[]={lo,hi}; auto hit=Find(self,key);
 if(!hit.text) { CallGameFunction<void,0x0040E920>(out,Data<char16_t>(0xfb9684)); return 0; }
 const char16_t* end=hit.text; while(*end) ++end;
 CallGameMethod<void,0x00408E20>(out,hit.text,end);
 Replace(out,0xfd9e1c,0xfbfba8,0xfbfbaa,0,2);
 // EA passes the exhausted search result (-1) into the second search.
 Replace(out,0xfd9e14,0xfd9e10,0xfd9e12,-1,2);
 if(hit.owner!=self->next && !Compare(self->locale,0xfd99e8)) Replace(out,0xfd9e0c,0xfd9e04,0xfd9e0a,0,1);
 return 1;
}
void* __fastcall ClearTable(Table* self,void*) {
 if(self->data) { CallGameMethod<void,0x004E5920>(*Data<void*>(0x11cb864),self->data); self->data=nullptr; }
 if(self->strings) { CallGameMethod<void,0x004E5920>(*Data<void*>(0x11cb864),self->strings); self->strings=nullptr; }
 std::memset(self->buckets,0,sizeof(self->buckets)); self->count=0;
 return self->buckets;
}
Table* __fastcall ConstructTable(Table* self,void*) {
 self->vtable=Data<void>(0xfb2c04); std::atomic_ref<int32_t>(self->refs).exchange(0);
 std::memset(self->base,0,sizeof(self->base)); self->vtable=Data<void>(0xfdded0);
 self->count=0; std::memset(self->buckets,0,sizeof(self->buckets));
 self->sentinel=reinterpret_cast<Entry*>(~uintptr_t(0));
 self->data=nullptr; self->strings=nullptr;
 return self;
}
}
