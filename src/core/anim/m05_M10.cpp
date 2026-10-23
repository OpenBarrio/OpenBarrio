#include "ts3/anim/m05_M10.h"
#include "ts3/anim/m05_B04.h"
#include "ts3/anim/m05_B20.h"
#include "ts3/anim/m05_B21.h"
#include "ts3/anim/m05_B24.h"
#include "ts3/math/m04_B11.h"
#include <cstring>
namespace ts3::anim::m10 {
static Entry* Sentinel(Blend* b) { return reinterpret_cast<Entry*>(&b->first); }
static void Table(Blend* b) {
#if defined(_M_IX86)
 b->base.vtable=reinterpret_cast<void**>(0xfc132c);
#else
 static void* table[11]{}; b->base.vtable=table;
#endif
}
static bool Advance(b01::Cursor* s,bool write) {
 s->position=static_cast<int32_t>(static_cast<uint32_t>(s->position)+4);
 if(s->position>=0) { if(write) CallGameMethod<void,0x594140>(s,4); else CallGameMethod<void,0x593ff0>(s,4); }
 return s->position<0;
}
static void Write(b01::Cursor* s,uint32_t v) { if(Advance(s,true)) std::memcpy(s->output+s->position-3,&v,4); }
static uint32_t Read(b01::Cursor* s) { uint32_t v=0; if(Advance(s,false)) std::memcpy(&v,s->input+s->position-3,4); return v; }
uint32_t Type(void*) { return 0x87eb0331u; }
Blend* Construct(Blend* b,void* host) {
 b01::Construct(&b->base,host); Table(b); CallGameMethod<void,0x83e190>(&b->first); return b;
}
Blend* Destroy(Blend* b,uint32_t flags) {
 Table(b); for(auto* p=b->first;p!=Sentinel(b);) { auto* next=p->next; p->next=b->free; b->free=p; p=next; }
 b01::Reset(&b->base); if(flags&1) CallGameFunction<void,0x4e3f70>(b); return b;
}
void Save(Blend* b,b01::Cursor* s) {
 uint32_t count=0; for(auto* p=b->first;p!=Sentinel(b);p=p->next) ++count;
 Write(s,count); for(auto* p=b->first;p!=Sentinel(b);p=p->next) { Write(s,p->key); b01::WriteNode(p->evaluator,s); }
}
void Load(Blend* b,b01::Cursor* s) {
 uint32_t count=Read(s); for(uint32_t i=0;i<count;++i) {
  uint32_t key=Read(s); b01::Node* node=nullptr;
  void* host=b->base.host; // Verified 0093ECC0 reads the evaluator host at +18.
  b01::ReadNode(host,&node,s);
#if defined(_M_IX86)
  B04Insert(reinterpret_cast<B04List*>(b),key,static_cast<uint32_t>(reinterpret_cast<uintptr_t>(node)));
#else
  // Native pointer-sized equivalent of the verified insertion helper.
  auto* at=b->first; while(at!=Sentinel(b) && key<=at->key) at=at->next;
  auto* n=b->free; b->free=n->next; n->key=key; n->evaluator=node;
  n->next=at; n->previous=at->previous; at->previous->next=n; at->previous=n;
#endif
 }
}
void Evaluate(Blend* b,Context* c,Pose* out) {
 if(b->first==Sentinel(b)) return;
 float* remaining=reinterpret_cast<float*>(b20::Fill(reinterpret_cast<b20::Pose*>(out),1.0f));
 auto* dest=reinterpret_cast<unsigned char*>(b20::Local(reinterpret_cast<b20::Pose*>(out)));
 float* extra=b20::Weights(reinterpret_cast<b20::Pose*>(out));
 Pose* temp=nullptr;
#if defined(_M_IX86)
 const char* descriptor=reinterpret_cast<const char*>(0xfc10d8);
#else
 const char* descriptor="Animation/RigPose";
#endif
 if(auto* allocation=CallGameFunction<Pose*,0x5748a0>(0x19b30,descriptor,0,0,0,0)) {
  void* rig=c->rig; if(rig) CallGameVirtual<void,0>(rig);
  temp=CallGameMethod<Pose*,0x574f50>(allocation,rig);
 }
 if(temp) temp->refs=static_cast<int32_t>(static_cast<uint32_t>(temp->refs)+1u);
 if(c->flags&2) CallGameFunction<void,0x57ccd0>(out);
 CallGameFunction<void,0x57e500>(out);
 bool stop=false;
 for(auto* p=b->first;p!=Sentinel(b) && !stop;p=p->next) {
  auto* n=p->evaluator; if(!n) continue;
  CallGameMethod<void,0x574c40>(temp);
  CallGameVirtual<void,2>(n,c,temp);
  auto* mask=temp->weights; if(!mask) stop=true;
  if(c->flags&2) {
   auto* src=reinterpret_cast<unsigned char*>(b20::Local(reinterpret_cast<b20::Pose*>(temp)));
   if(b24::ChannelCount(static_cast<RigResource*>(c->rig),nullptr)) {
    float w=b01::Weight(n), old=1.0f;
    if(remaining) { old=remaining[300]; w=old*w; }
    if(mask) w=mask[300]*w;
    if(w>0) { CallGameFunction<void,0x57cd20>(temp,out,w); remaining[300]=old-w; }
   }
   CallGameFunction<void,0x57e560>(remaining,mask,b01::Weight(n),temp,out);
   auto* skeleton=&static_cast<RigResource*>(c->rig)->skeleton;
   auto* joints=CallGameMethod<unsigned char*,0x9f0de0>(skeleton);
   auto count=static_cast<uint32_t>(b21::Count(reinterpret_cast<b21::SkeletonView*>(skeleton)));
   for(uint32_t i=0;i<count;++i) if(joints[112*i+96]&0x20) {
    float old=remaining[i]; float w=b01::Weight(n)*old; if(mask) w=mask[i]*w;
    if(w>0) { remaining[i]=old-w; ts3::math::b11::AccumulatePose(reinterpret_cast<__m128*>(dest+48*i),extra+i,reinterpret_cast<__m128*>(src+48*i),w); }
   }
  } else {
   CallGameFunction<void,0x57e560>(remaining,mask,b01::Weight(n),temp,out);
   float old=*remaining; float w=b01::Weight(n)*old; if(mask) w=*mask*w; *remaining=old-w;
  }
 }
 CallGameFunction<void,0x57cf60>(out,0);
 if(c->flags&2) CallGameFunction<void,0x57ce30>(out);
 b20::ClearFill(reinterpret_cast<b20::Pose*>(out));
 if(temp) {
  temp->refs=static_cast<int32_t>(static_cast<uint32_t>(temp->refs)-1u);
  if(temp->refs==0) { temp->refs=1; CallGameVirtual<void,0>(temp,1); }
 }
}
#if defined(_M_IX86)
void __fastcall HookEvaluate(Blend* b,void*,Context* c,Pose* p) { Evaluate(b,c,p); }
void __fastcall HookSave(Blend* b,void*,b01::Cursor* s) { Save(b,s); }
Blend* __fastcall HookConstruct(Blend* b,void*,void* h) { return Construct(b,h); }
Blend* __fastcall HookDestroy(Blend* b,void*,uint32_t f) { return Destroy(b,f); }
void __fastcall HookLoad(Blend* b,void*,b01::Cursor* s) { Load(b,s); }
uint32_t __fastcall HookType(void* b,void*) { return Type(b); }
#endif
}
