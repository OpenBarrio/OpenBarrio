#include "ts3/anim/m05_M29.h"
#include "ts3/anim/m05_B08.h"
#include "ts3/anim/m05_B22.h"
#include "ts3/scratch/clu_4e3f70_Scratch.h"
#include <atomic>
#include <cstring>
namespace ts3::anim::m29 {
static void* Address(uintptr_t a) {
#if defined(_M_IX86)
 return reinterpret_cast<void*>(a);
#else
 (void)a; NotRewritten("M29 original data");
#endif
}
static void Release(ResourcePrefix* p) {
 if(p && std::atomic_ref<int32_t>(p->atomicRefs).fetch_sub(1)==1) {
  std::atomic_ref<int32_t>(p->atomicRefs).exchange(1);
  CallGameVirtual<void,0>(p,1u);
 }
}
static bool Last(Clip* s) {
 return !s->resource || CallGameMethod<int32_t,0x0057FEB0>(&s->resource->eventBegin)<=s->last;
}
static float End(Clip* s) {return s->period+*static_cast<float*>(Address(0x0107A538));}
Clip* Construct(Clip* s,void* host,float t) {
 b08::Construct(&s->base,host);
 s->base.vtable=Address(0x00FC1B48);s->base.secondary8=Address(0x00FC1B40);s->base.secondaryC=Address(0x00FC1B3C);
 s->begin=s->end=s->storage;s->capacity=s->storage+0x4b0;
 b22::Construct(&s->ik);
 s->resource=nullptr;s->stateD4=nullptr;s->retained=nullptr;
 s->elapsed=0;s->period=0;s->mode=1;s->cycles=0;s->last=-1;
 std::memset(s->flags,0,sizeof s->flags);
 CallGameMethod<void,0x00568210>(s,t);
 return s;
}
Clip* Delete(Clip* s,uint32_t flags) {
 CallGameMethod<void,0x00568760>(s);
 if(flags&1) ts3::scratch::Free4E3F70(s);
 return s;
}
uint8_t FillEvent(Clip* s,Event* e) {
 e->host=s->base.hostSlot;e->owner=s;e->ik=&s->ik;
 auto p=s->retained;
 if(p!=e->retained) {
  if(p) std::atomic_ref<int32_t>(p->atomicRefs).fetch_add(1);
  auto old=e->retained;e->retained=p;Release(old);
 }
 e->flag1=s->flags[1];e->flag2=s->flags[2];e->last=uint8_t(Last(s));return e->last;
}
void Dispatch(Clip* s,float t,uint8_t filter) {
 if(!s->resource) return;
 (void)b23::ClipName(s->resource,nullptr);
 Event e{};FillEvent(s,&e);
 auto list=&s->resource->eventBegin;
 uint32_t count=CallGameMethod<uint32_t,0x0057FEA0>(list);
 auto send=[&](void* record,int32_t index) {
  e.record=record;
  uint32_t flags=*reinterpret_cast<uint32_t*>(static_cast<unsigned char*>(record)+0x1c);
  bool allowed=!filter || (flags&4);
  if(allowed && (flags&0x10)) {
   auto global=*static_cast<unsigned char**>(Address(0x011CD800));
   allowed=(*reinterpret_cast<uint32_t*>(global+0x28)&4)!=0;
  }
  if(allowed) CallGameMethod<void,0x00568710>(s,&e);
  s->last=index;e.last=uint8_t(Last(s));
 };
 if(!s->flags[5]) {
  for(uint32_t i=0;i<count;++i) {
   auto r=CallGameMethod<void*,0x0057FF00>(list,i);
   if(static_cast<int32_t>(i)<=s->last) continue;
   float time=*reinterpret_cast<float*>(static_cast<unsigned char*>(r)+0x10);
   if(!(t>=time)) break; // unordered COMISS takes the early exit too
   send(r,static_cast<int32_t>(i));
  }
 } else {
  t=s->period-t;
  for(int32_t i=static_cast<int32_t>(count-1);i>=0;--i) {
   auto r=CallGameMethod<void*,0x0057FF00>(list,static_cast<uint32_t>(i));
   if(s->last!=-1 && i>=s->last) continue;
   float time=*reinterpret_cast<float*>(static_cast<unsigned char*>(r)+0x10);
   if(!(time>=t)) break;
   send(r,i);
  }
 }
 Release(e.retained);
}
void Update(Clip* s,float dt) {
 if(s) ++s->base.refs;
 Event e{};FillEvent(s,&e);
 uint32_t previous=s->cycles;
 if(!s->flags[0]) {CallGameMethod<void,0x00568620>(s,&e);s->flags[0]=1;}
 if(dt>0) {
  s->total+=dt;s->time+=dt;
  if(s->time>s->period && s->mode!=1 && (!s->mode || s->cycles<s->mode)) {
   ++s->cycles;s->time-=s->period; // deliberately just one subtraction
  }
 }
 bool wrapped=s->mode!=1 && previous!=s->cycles;
 double remaining=CallGameVirtual<double,7>(s);
 bool ended=remaining<0;
 if(ended) {if(s->resource) Dispatch(s,End(s),0);}
 else {
  if(wrapped) {if(s->resource) Dispatch(s,End(s),0);s->last=-1;}
  Dispatch(s,s->time,0);
 }
 e.last=uint8_t(Last(s));
 bool notify=true;
 if(s->mode && s->resource) {
  auto clock=*static_cast<unsigned char**>(Address(0x011D8D40));
  float limit=static_cast<float>(static_cast<double>(*reinterpret_cast<int32_t*>(clock+0x44))* *static_cast<float*>(Address(0x010434C4)));
  double duration=CallGameVirtual<double,9>(s);
  notify=static_cast<double>(limit)>=duration; // unordered also suppresses notification
 }
 if(notify) CallGameMethod<void,0x00568670>(s,&e);
 if(ended) b08::Notify(&s->base,&e);
 else if(wrapped) CallGameMethod<void,0x005686C0>(s,&e);
 s->elapsed+=dt;
 Release(e.retained);
 if(--s->base.refs==0) {s->base.refs=1;CallGameVirtual<void,0>(s,1u);}
}
void Finish(Clip* s) {
 if(s->flags[0] && s->resource) Dispatch(s,End(s),1);
 b08::ReleaseNotify(&s->base);
}
}
