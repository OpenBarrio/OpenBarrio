#include "ts3/anim/m05_M32.h"
#include <cmath>
namespace ts3::anim::m32 {
namespace {
void* Table(uint32_t a) {
#if defined(_M_IX86)
 return reinterpret_cast<void*>(a);
#else
 static uintptr_t tables[4]; return &tables[a==0xfc1bb0?0:a==0xfc1ba8?1:a==0xfc1ba4?2:3];
#endif
}
void Status(Stream* s,int32_t r) { CallGameMethod<void,0x007d77c0>(s,r); }
}
void SetResource(Clip* c,Resource* r) {
#if defined(_M_IX86)
 auto& initialized=*reinterpret_cast<uint32_t*>(0x011cd7e0);
 auto& name=*reinterpret_cast<uint32_t*>(0x011cd7dc);
 if(!(initialized&1)) { initialized|=1; name=CallGameFunction<uint32_t,0x007cfe70>(reinterpret_cast<const char*>(0x00faeb48)); }
#else
 uint32_t name=0; NotRewritten("M32 resource namespace global");
#endif
 auto old=c->resource;
 if(r!=old) {
  if(r) CallGameVirtual<void,0>(r);
  c->resource=r;
  if(old) CallGameVirtual<void,1>(old);
 }
 if(!c->resource) return;
 c->period=CallGameMethod<float,0x0057a740>(c->resource);
 c->flags[3]=static_cast<uint8_t>((CallGameMethod<uint32_t,0x0057a690>(c->resource)>>1)&1);
 if(c->resource->ik) CallGameMethod<void,0x00569b90>(&c->ik,c->resource->ik);
 else CallGameMethod<void,0x005790a0>(&c->ik,c->resource->view,10);
 auto host=CallGameVirtual<void*,9>(c->base.hostSlot);
 CallGameMethod<void,0x005777a0>(&c->ik,name,host);
 CallGameMethod<void,0x00577a80>(&c->ik);
 CallGameMethod<void,0x00569160>(c);
}
uint8_t LoadResource(Clip* c,const void* key) {
 auto manager=CallGameFunction<void*,0x004afd20>();
 if(!manager) return 0;
 Resource* r=nullptr;
 auto found=CallGameVirtual<uint8_t,3>(manager,key,&r,0,0,0,0);
 if(!found) return 0;
 SetResource(c,r);
 CallGameVirtual<void,1>(r);
 return 1;
}
uint8_t Load(Clip* c,Stream* s) {
 uint32_t version=~0u,flags=0,context=0;
 uint32_t key[4]{},transform[4]{};
 Status(s,CallGameFunction<int32_t,0x004dd700>(s->handle,&version,s->extra));
 if(!s->good || version>3) return 0;
 Status(s,CallGameFunction<int32_t,0x004dd700>(s->handle,&flags,s->extra));
 if(!s->good) return 0;
 Status(s,CallGameFunction<int32_t,0x007d7530>(s->handle,key,s->extra));
 if(!s->good) return 0;
 CallGameMethod<void,0x005680d0>(s,transform);
 if(!s->good) return 0;
 LoadResource(c,key); // EA ignores failure of resource lookup.
 CallGameMethod<void,0x00568370>(c,transform);
 CallGameMethod<void,0x00579490>(&c->ik,s,&context);
 if(!s->good) return 0;
 CallGameMethod<void,0x00577a80>(&c->ik);
 if(flags&1) {
  auto mem=CallGameFunction<void*,0x004e3f90>(0x18,Table(0xfc1b8c),0,0,0,0);
  auto object=mem?CallGameMethod<void*,0x00582010>(mem):nullptr;
  void* retained=nullptr;
  CallGameMethod<void,0x00565970>(&retained,object);
  if(retained) CallGameMethod<void,0x00582120>(retained,s,&context);
  if(!retained || !s->good) { CallGameMethod<void,0x007460f0>(&retained); return 0; }
  CallGameMethod<void,0x00569360>(c,retained);
  CallGameMethod<void,0x007460f0>(&retained);
 }
 auto read32=[&](void* p) { CallGameMethod<void,0x00565700>(s,p); return s->good!=0; };
 auto read8=[&](void* p) { CallGameMethod<void,0x00565760>(s,p); return s->good!=0; };
 if(version==2 || version==3) {
  if(!read32(&c->stateDC)||!read32(&c->stateE0)||!read32(&c->stateE4)||!read32(&c->mode)||!read32(&c->valueF0)||!read32(&c->valueF4)||!read8(&c->flags[0])||!read8(&c->flags[1])||!read8(&c->flags[2])) return 0;
  if(version>=3) read8(&c->flags[5]);
  if(!s->good) return 0;
 } else if(version==1) {
  float end=0,start=0;
  if(!read32(&end)||!read32(&start)||!read32(&c->mode)||!read32(&c->valueF4)||!read8(&c->flags[1])||!read8(&c->flags[2])) return 0;
  c->stateE4=end-start;
  c->flags[0]=static_cast<uint8_t>(start>=0.0f); // unordered is false, including NaNs.
  CallGameMethod<void,0x00568210>(c,end);
 }
 if(c->flags[1]) CallGameMethod<void,0x00578d60>(&c->ik);
 CallGameMethod<void,0x00569160>(c);
 return 1;
}
uint8_t Save(PairController* c,Stream* s) {
 Status(s,CallGameFunction<int32_t,0x004ddbd0>(s->handle,1,s->extra));
 if(!s->good) return 0;
 CallGameFunction<void,0x005657a0>(c->first,s);
 if(!s->good) return 0;
 CallGameFunction<void,0x005657a0>(c->second,s);
 if(!s->good) return 0;
 Status(s,CallGameFunction<int32_t,0x004ddff0>(s->handle,c->value34,s->extra));
 if(!s->good) return 0;
 CallGameMethod<void,0x00565ea0>(s,c->value38);
 return s->good;
}
PairController* Construct(PairController* c,void* host) {
 b08::Construct(&c->base,host);
 c->base.vtable=Table(0xfc1bb0); c->base.secondary8=Table(0xfc1ba8); c->base.secondaryC=Table(0xfc1ba4);
 c->secondary28=Table(0xfc1e5c); c->first=c->second=nullptr; c->value34=c->value38=0;
 return c;
}
// Adapted from verified m01::FileType/MemoryType; only the return immediate differs.
uint32_t Type(void*) { return 0x34f49f94u; }
#if defined(_M_IX86)
namespace hooks {
void __fastcall SetResource(Clip* c,void*,Resource* r) { m32::SetResource(c,r); }
uint8_t __fastcall LoadResource(Clip* c,void*,const void* k) { return m32::LoadResource(c,k); }
uint8_t __fastcall Load(Clip* c,void*,Stream* s) { return m32::Load(c,s); }
uint8_t __fastcall Save(PairController* c,void*,Stream* s) { return m32::Save(c,s); }
PairController* __fastcall Construct(PairController* c,void*,void* h) { return m32::Construct(c,h); }
uint32_t __fastcall Type(void* c,void*) { return m32::Type(c); }
}
#endif
}
