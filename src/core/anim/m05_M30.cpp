#include "ts3/anim/m05_M30.h"
#include "ts3/ui/clu_571870_UI.h"
#include <atomic>
#include <cstring>
namespace ts3::anim::m30 {
namespace {
struct Identity { void* table; uint32_t unused,low,high; };
int32_t Add(int32_t* p,int32_t v) { return std::atomic_ref<int32_t>(*p).fetch_add(v); }
}
void Rebuild(Clip* n) {
 n->end=n->begin;
 n->flags[4]=0;
 auto host=CallGameVirtual<Identity*,9>(n->base.hostSlot);
 auto list=reinterpret_cast<const ui::UiPageList*>(&n->ik);
 auto pages=static_cast<uint32_t>(list->CountPages());
 for(uint32_t p=0;p<pages;++p) {
  auto count=static_cast<uint32_t>(list->CountItems(p));
  for(uint32_t i=0;i<count;++i) {
   uint32_t out[5]={0,0,0,0xffffffff,0};
   if(!list->GetItemFields(p,i,out)) continue;
   auto identity=CallGameMethod<Identity*,0x005775d0>(&n->ik);
   uint32_t low=identity?identity->low:0,high=identity?identity->high:0;
   Identity* target;
   if(out[0]==low && out[1]==high) target=host;
   else {
    target=CallGameMethod<Identity*,0x00935830>(out);
    if(!target || target->low!=host->low || target->high!=host->high) n->flags[4]=1;
   }
   if(!target) continue;
   void* service=nullptr;
#if defined(_M_IX86)
   service=*reinterpret_cast<void**>(0x011dc484);
#endif
   // Snapshot the identifier before entering external services/callbacks.
   const auto targetLow=target->low,targetHigh=target->high;
   auto object=CallGameMethod<unsigned char*,0x00935770>(target,service);
   if(!object) continue;
   auto rig=CallGameVirtual<unsigned char*,8>(object+8);
   if(!rig) continue;
   CallGameVirtual<void,0>(rig);
   auto bone=CallGameMethod<int32_t,0x00575b00>(rig+0x28,out[3]);
   if(bone!=-1) {
    CallGameMethod<void,0x005690b0>(&n->begin);
    n->end[-1]={targetLow,targetHigh,p,i,static_cast<uint32_t>(bone),out[2]};
   }
   CallGameVirtual<void,1>(rig);
  }
 }
}
void SetMirror(Clip* n,uint8_t value) {
 if(n->flags[1]!=value) {
  CallGameMethod<void,0x00578d60>(&n->ik);
  Rebuild(n);
 }
 n->flags[1]=value;
}
void SetMap(Clip* n,Map* value) {
 auto old=n->map;
 if(value!=old) {
  if(value) Add(&value->refs,1);
  n->map=value;
  if(old && Add(&old->refs,-1)==1) {
   std::atomic_ref<int32_t>(old->refs).exchange(1);
   CallGameVirtual<void,0>(old,1u);
  }
 }
 if(n->map) {
  auto end=n->map->end;
  for(auto e=n->map->begin;e!=end;++e) {
   if(!e->value) continue;
   auto v=CallGameMethod<void*,0x00581560>(e->value,0u);
   if(!v) {
    auto words=reinterpret_cast<const uint32_t*>(e->value);
    if(words[4] || words[5]) n->flags[6]=1;
   }
   CallGameMethod<void,0x005777a0>(&n->ik,e->key,v);
  }
  CallGameMethod<void,0x00577a80>(&n->ik);
 }
 Rebuild(n);
}
void Merge(Clip* n,Map* source) {
 n->flags[6]=0;
 if(!n->map) return;
 if(n->map!=source) {
  auto end=source->end;
  for(auto e=source->begin;e!=end;++e) {
   auto value=e->value;
   auto prior=CallGameMethod<unsigned char*,0x00581810>(n->map,e->key);
   if(prior) {
    auto v=CallGameMethod<void*,0x00581910>(prior,1u);
    CallGameMethod<void,0x005815a0>(value,v);
    uint32_t flags; std::memcpy(&flags,prior+12,4);
    CallGameMethod<void,0x005814d0>(value,static_cast<uint8_t>((flags&1)==1));
    std::memcpy(&flags,prior+12,4); // First callback may have modified the second flag.
    CallGameMethod<void,0x00581510>(value,static_cast<uint8_t>((flags&2)==2));
    CallGameMethod<void,0x00581750>(n->map,e->key,value);
   } else CallGameMethod<void,0x00582090>(n->map,e->key,value);
  }
 }
 SetMap(n,n->map);
}
void Apply(Clip* n,const uint32_t* data) {
 for(uint32_t i=0;i<data[0];++i) {
  auto e=data+4+4*i;
  if(n->map) {
   auto v=CallGameMethod<void*,0x00581840>(n->map,e[2]);
   CallGameMethod<void,0x00578650>(&n->ik,e[0],e[1],v,e[2],e[3]);
  } else CallGameMethod<void,0x00578910>(&n->ik,e[0],e[1],e[2],e[3]);
 }
 if(data[0]) CallGameMethod<void,0x00577a80>(&n->ik);
 Rebuild(n);
}
void* Evaluate(Clip* n,void* context) {
 if(!n->resource) return nullptr;
 if(n->flags[6]) Merge(n,n->map);
 auto factory=CallGameMethod<void*,0x0054c770>(context,0x53deec77u);
 auto evaluator=CallGameVirtual<unsigned char*,1>(factory,0x53deec77u);
 CallGameMethod<void,0x0054de90>(evaluator,n->resource);
 float time;
 if(n->flags[5]) {
  float period=CallGameVirtual<float,10>(n);
  time=period-CallGameVirtual<float,6>(n);
 } else time=CallGameVirtual<float,6>(n);
 CallGameMethod<void,0x0054dcd0>(evaluator,time);
 if(n->stateE4>=0.0f) CallGameMethod<void,0x0054dcf0>(evaluator,n->stateE4);
 n->stateE4=0.0f;
 CallGameMethod<void,0x0054dd10>(evaluator,n->flags[1]);
 evaluator[0x3c9]=n->flags[5];
 if(n->secondary) CallGameMethod<void,0x0054def0>(evaluator,n->secondary);
 auto end=n->end;
 for(auto b=n->begin;b!=end;++b)
  CallGameMethod<void,0x0054dd20>(evaluator,b->page,b->item,b->low,b->high,b->bone,b->slot);
 return evaluator;
}
}
#if defined(_M_IX86)
namespace ts3::anim::m30::hooks {
void __fastcall Rebuild(Clip* n,void*) { m30::Rebuild(n); }
void __fastcall SetMirror(Clip* n,void*,uint8_t v) { m30::SetMirror(n,v); }
void __fastcall SetMap(Clip* n,void*,Map* v) { m30::SetMap(n,v); }
void __fastcall Merge(Clip* n,void*,Map* v) { m30::Merge(n,v); }
void __fastcall Apply(Clip* n,void*,const uint32_t* v) { m30::Apply(n,v); }
void* __fastcall Evaluate(Clip* n,void*,void* c) { return m30::Evaluate(n,c); }
}
#endif
