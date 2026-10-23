#include "ts3/anim/m05_M157.h"
#include <cstring>
#if defined(_M_IX86)
#include <intrin.h>
#endif
namespace ts3::anim::m157 {
namespace {
void* Interface(Body* b) { return &b->interfaceTable; }
template<unsigned A> void* Address() {
#if defined(_M_IX86)
 return reinterpret_cast<void*>(uintptr_t(A));
#else
 AbortNotRewritten("m157 game data"); return nullptr;
#endif
}
template<unsigned A> uint32_t Global() { return *static_cast<uint32_t*>(Address<A>()); }
void* Resolve(Body* b,uint32_t key) { return b->host?CallGameMethod<void*,0x935770>(b->host,key):nullptr; }
void* Add(void* p,size_t n) { return static_cast<unsigned char*>(p)+n; }
void Retain(void* p) {
#if defined(_M_IX86)
 _InterlockedIncrement(reinterpret_cast<volatile long*>(Add(p,4)));
#else
 AbortNotRewritten("m157 retain original resource");
#endif
}
void ReleaseController(void* p) {
 if(!p) return;
 auto& refs=*static_cast<uint32_t*>(Add(p,8));
 if(--refs==0) { refs=1; CallGameVirtual<void,0>(Add(p,4),1); }
}
}
StyleOwner* Construct(StyleOwner* s,void* arg) {
 CallGameMethod<void,0x839A40>(s,arg);
 s->table=Address<0x1044440>(); s->secondary=Address<0x104443C>();
 CallGameMethod<void,0x846820>(s->style,arg);
 CallGameMethod<void,0x8399E0>(s,s->style);
 return s;
}
double LoopSound(Body* b) {
 // Only the first twenty words are cleared by EA; the following seven are scratch.
 struct Temporary { void* refs[2]; uint32_t words[18]; uint8_t scratch[28]; void* begin; void* end; void* descriptor; const char* label; } t;
 std::memset(&t,0,offsetof(Temporary,scratch));
 t.begin=t.end=Address<0x11C7E54>(); t.descriptor=Address<0x11C7E56>();
 t.label=static_cast<const char*>(Address<0x10043E0>());
 auto ok=CallGameMethod<uint8_t,0x844EC0>(*static_cast<void**>(Address<0x11D9140>()),b->keys[0],b->keys[1],b->keys[2],b->keys[3],&t);
 float result=5.0f;
 if(ok&&t.words[0]) {
  auto actor=Resolve(b,Global<0x11DC484>());
  CallGameFunction<double,0x84B430>(actor?Add(actor,8):nullptr,t.words[0],&result,1);
 }
 CallGameMethod<void,0x754060>(&t);
 return result;
}
int Stairs(Body* b,void* arg) {
 CallGameMethod<void,0x954CF0>(b);
 CallGameMethod<void,0x839F00>(Interface(b),0);
 auto owned=b->owned;
 if(owned) { b->owned=nullptr; CallGameVirtual<void,5>(owned); }
 alignas(16) unsigned char temporary[16];
 auto actor=Resolve(b,Global<0x11E9158>());
 bool selected=false;
 if(actor) {
  auto keys=CallGameMethod<void*,0x959000>(actor,temporary);
  selected=CallGameMethod<uint8_t,0x954240>(b,keys)!=0;
 }
 if(!selected) selected=CallGameMethod<uint8_t,0x954240>(b,Add(arg,48))||CallGameMethod<uint8_t,0x954240>(b,Add(arg,80));
 if(selected&&b->mode==1) {
  auto allocation=CallGameFunction<void*,0x4E3F90>(352,Address<0x1044554>(),0,0,0,0);
  void* stairs=nullptr;
  if(allocation) {
   auto sim=Resolve(b,Global<0x11DC484>());
   stairs=CallGameMethod<void*,0x854BA0>(allocation,Interface(b),sim?Add(sim,8):nullptr);
  }
  CallGameMethod<void,0x839D60>(Interface(b),stairs);
  if(CallGameMethod<uint8_t,0x8555E0>(stairs,arg)) return 0;
  CallGameMethod<void,0x954CF0>(b);
 }
 CallGameVirtual<void,12>(Interface(b),Add(arg,80),Add(arg,96));
 return 2;
}
bool TryController(Body* b,void* arg) {
 auto controller=CallGameMethod<void*,0x580420>(Interface(b));
 void* target=nullptr;
 if(controller) {
  ++*static_cast<uint32_t*>(Add(controller,8));
  target=CallGameMethod<void*,0x8477B0>(controller);
 }
 if(target) CallGameMethod<void,0x85A8C0>(target,arg,1);
 else { CallGameMethod<void,0x954CF0>(b); CallGameMethod<void,0x839F00>(Interface(b),0); }
 ReleaseController(controller);
 return target!=nullptr;
}
uintptr_t Apply(Body* b,const uint32_t* keys,uint8_t suppress,uint8_t requested) {
 for(unsigned i=0;i<4;++i) b->keys[i]=keys[i];
 auto settings=CallGameFunction<void*,0x5EB7C0>();
 bool alternate=(b->flags&4)!=0;
 if(settings&&!b->disabled1&&!b->disabled0) {
  auto key=b->keys[1]|b->keys[3];
  float first,second;
  if(alternate) { first=static_cast<float>(CallGameMethod<double,0x5ECBA0>(settings,key)); second=static_cast<float>(CallGameMethod<double,0x5ECC40>(settings,key)); }
  else { first=static_cast<float>(CallGameMethod<double,0x5ECC40>(settings,key)); second=static_cast<float>(CallGameMethod<double,0x5ECBA0>(settings,key)); }
  auto resource=CallGameMethod<void*,0xC93970>(Interface(b));
  if(resource) {
   Retain(resource);
   auto check=CallGameVirtual<void*,3>(resource,0,0x2000,0);
   if(!CallGameMethod<uint8_t,0x81C880>(check)) second=0;
  }
  CallGameMethod<void,0x9D9FA0>(b->media,first);
  *static_cast<float*>(Add(b->media,112))=second;
  float third=static_cast<float>(CallGameMethod<double,0x5ECBF0>(settings,key));
  CallGameMethod<void,0x85AAF0>(b->media,third);
  CallGameMethod<void,0x7460F0>(&resource);
 }
 if(CallGameVirtual<int,6>(Interface(b))==1) b->loopSound=static_cast<float>(LoopSound(b));
 auto controller=CallGameMethod<void*,0x580420>(Interface(b));
 if(!controller) return 0;
 controller=CallGameMethod<void*,0x580420>(Interface(b));
 auto target=CallGameVirtual<void*,1>(controller,uint32_t(0xf4cfd7da));
 if(!target) return 0;
 bool enabled=requested&&(b->flags&1);
 bool other=(b->flags&2)!=0;
 *static_cast<uint8_t*>(Add(target,388))=alternate;
 if(b->pending&&suppress) enabled=false;
 CallGameMethod<void,0x84D7E0>(target,b->keys[0],b->keys[1],b->keys[2],b->keys[3],int(enabled),int(other));
 if(!enabled&&b->pending) CallGameMethod<void,0x84EA00>(target,&b->range);
 auto result=CallGameMethod<uintptr_t,0x9562A0>(&b->range);
 b->pending=0;
 return result;
}
intptr_t SetState(void* self,int state,int a,int c) {
 auto b=reinterpret_cast<Body*>(static_cast<unsigned char*>(self)-offsetof(Body,interfaceTable));
 b->state=state;
 if(b->owned) {
  alignas(16) unsigned char keys[16],zero0[16]{},zero1[16]{};
  CallGameVirtual<void,7>(self,keys);
  auto values=static_cast<uint32_t*>(b->host);
  CallGameVirtual<void,2>(b->owned,keys,zero0,zero1,3,values[2],values[3],a,c,state,0);
 }
 bool keep=false;
 if(b->media&&state==3) {
  auto resource=CallGameMethod<void*,0xC93970>(self);
  if(resource) Retain(resource);
  keep=CallGameFunction<uint8_t,0x81F550>(resource)!=0;
 }
 CallGameMethod<void,0x839F00>(self,0);
 if(b->media&&!keep) CallGameMethod<void,0x85D6D0>(b->media);
 auto actor=Resolve(b,Global<0x11DC484>());
 if(actor) {
  auto& flags=*static_cast<uint32_t*>(Add(actor,44));
  if(flags&0x80000) flags&=0xffcfffff;
 }
 CallGameMethod<void,0x954CF0>(b);
 intptr_t result=state;
 if(state==6) {
  b->pending=0; CallGameMethod<void,0x81F320>(&b->range);
  b->range.next=b->range.previous=&b->range;
  b->range.weight=1.0f;
  for(auto& i:b->range.ids) i=-1;
  b->range.zero[0]=b->range.zero[1]=0;
  std::memset(b->range.flags,0,8); b->range.flags[5]=b->range.flags[6]=1;
  result=-1;
 } else if(state==3) return result;
 if(b->media) return CallGameMethod<intptr_t,0x85B430>(b->media);
 return result;
}
}
#if defined(_M_IX86)
namespace ts3::anim::m157::hooks {
StyleOwner* __fastcall Construct(StyleOwner* p,void*,void* a) { return m157::Construct(p,a); }
double __fastcall LoopSound(Body* p,void*) { return m157::LoopSound(p); }
int __fastcall Stairs(Body* p,void*,void* a) { return m157::Stairs(p,a); }
bool __fastcall TryController(Body* p,void*,void* a) { return m157::TryController(p,a); }
uintptr_t __fastcall Apply(Body* p,void*,const uint32_t* a,uint8_t b,uint8_t c) { return m157::Apply(p,a,b,c); }
intptr_t __fastcall SetState(void* p,void*,int a,int b,int c) { return m157::SetState(p,a,b,c); }
}
#endif
