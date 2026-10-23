#include "ts3/anim/m05_M137.h"
#include <xmmintrin.h>
#include <cstring>
#include <intrin.h>
namespace ts3::anim::m137_detail {
namespace {
struct Ref { void** vtable; long refs; };
void Add(void* p) { if(p) _InterlockedIncrement(&static_cast<Ref*>(p)->refs); }
void Release(void* p,bool atomic=false) {
 if(!p)return;
 auto* r=static_cast<Ref*>(p);
 long n=atomic?_InterlockedDecrement(&r->refs):--r->refs;
 if(!n) { if(atomic)_InterlockedExchange(&r->refs,1);else r->refs=1; CallGameVirtual<void,0>(r,1); }
}
void Destroy(Request& r) { CallGameMethod<void,0x008499b0>(&r); }
void Destroy(Style& s) { CallGameMethod<void,0x00754060>(&s); }
void Free(void* p) { if(p)CallGameFunction<void,0x004e3f70>(p); }
Request Make(Body* b,float time) {
 Request r{};r.name=b->name;
 CallGameMethod<void,0x00849bf0>(b,&r); r.time=time; return r;
}
void Mirror(Request& r) { CallGameMethod<void,0x00848620>(&r); }
Style GetStyle(Body* b) {
 Style s; CallGameMethod<void,0x007513f0>(&s); CallGameMethod<void,0x0084bc00>(b,&s); return s;
}
Controllers Allocate(Body* b) {
 Controllers c{nullptr,nullptr,nullptr,"Locomotion/AnimatedSimBody/SlaveControllersVector"};
 CallGameMethod<void,0x00aef830>(&c,b->slaveCount);return c;
}
bool Build(void* builder,Request& r,ControllerResult& out) {
 return builder && CallGameVirtual<uint8_t,3>(builder,&r,&out)!=0;
}
bool BuildSlaves(Body* b,Controllers& c,int kind,Request& r,bool final=false) {
 if(b->slaveCount<=0 || !b->slaveConfig)return true;
 Request copy;CallGameMethod<void,0x008499e0>(&copy,&r);
 // The callee owns/destructs the by-value copy, not the caller's request.
 return CallGameMethod<uint8_t,0x00849d40>(b,&c,kind,copy,final)!=0;
}
void Dispatch(Body* b,void* controller,Controllers& c,int kind,int mode,float scale) {
 CallGameMethod<void,0x0084b590>(b,controller,b->slaveCount>0?c.begin:nullptr,b->slaveCount,kind,mode,1,scale);
}
void ClearSlaves(Body* b,Controllers& c) {
 for(int i=0;i<b->slaveCount;++i)if(b->slaves[i]) {Release(c.begin[i]);c.begin[i]=nullptr;}
}
double Duration(void* p) { return CallGameFunction<double,0x00849540>(p); }
void SetScale(float& scale,float time,void* p) {
 // Duration returns ST0, with a float rounding point before division.
 double duration=Duration(p);scale=1.0f;if(duration>0.0)scale=time/static_cast<float>(duration);
}
void* Actor(Body* b) {return CallGameMethod<void*,0x00c93970>(b->canvas);}
float Target(void* actor,uint64_t key,float time,float speed) {
#if defined(_M_IX86)
 // Preserve the live x87 phase return through the multiply/add (no float or
 // double spill before EA's final FSTP). The request time below is SSE-rounded.
 void* entry=(*reinterpret_cast<void***>(actor))[41];
 uint32_t keyLow=uint32_t(key),keyHigh=uint32_t(key>>32);float out;
 __asm {
  push keyHigh
  push keyLow
  mov ecx,actor
  call entry
  fld time
  fmul speed
  faddp st(1),st(0)
  fstp out
 }
 return out;
#else
 (void)actor;(void)key;(void)time;(void)speed;NotRewritten("root-motion phase sampling");
#endif
}
void* Manager() {
#if defined(_M_IX86)
 return *reinterpret_cast<void**>(0x011d9140);
#else
 NotRewritten("locomotion manager singleton");
#endif
}
}
bool Continue(Body* b) {
 float time=b->speed*b->times[0]; Request request=Make(b,time);
 if(b->mirror)Mirror(request);
 ControllerResult first{};
 if(!Build(b->builder,request,first) || !first.controller) {
  Release(first.controller);Destroy(request);return false;
 }
 SetScale(b->scales[1],time,first.controller);Release(first.controller);
 if(b->reverse)request.reverse=1;
 ControllerResult result{};
 if(b->builder && Build(b->builder,request,result)) {
  if(!result.controller) {Destroy(request);return true;}
  Controllers c=Allocate(b);Request slave{};
  if(b->slaveTime)slave.time=time;
  if(b->mirror)Mirror(slave);
  if(b->reverse)slave.reverse=1;
  if(!BuildSlaves(b,c,3,slave)) {
   Destroy(slave);Free(c.begin);Release(result.controller);Destroy(request);return false;
  }
  ++b->counter;
  Dispatch(b,result.controller,c,3,1,b->scaleEnabled?b->scales[1]:1.0f);
  if(b->scaleEnabled)b->scales[1]=1.0f;
  ClearSlaves(b,c);Destroy(slave);Free(c.begin);
 }
 Release(result.controller);Destroy(request);return true;
}
bool Start(Body* b) {
 b->started=0;b->finished=0;
 CallGameMethod<void,0x0084f890>(b);
 void* actor=Actor(b);if(!actor)return false;Add(actor);
 alignas(16) float position[4],rotation[4],next[4];
 CallGameVirtual<void,7>(b->master,position);CallGameVirtual<void,10>(b->master,rotation);
 alignas(16) __m128 movement[2]={_mm_setzero_ps(),_mm_setzero_ps()};
 Add(actor);CallGameFunction<void,0x00849600>(actor,movement,b->master);
 Style style=GetStyle(b);
 uint64_t key=CallGameVirtual<uint64_t,4>(b->master);
 float target=Target(actor,key,b->times[1],b->speed);
 CallGameVirtual<void,71>(actor,next,target,0,static_cast<void*>(nullptr));
 float time=b->times[1]*b->speed;Request request=Make(b,time);ControllerResult result{};
 if(!Build(style.slots[1],request,result) || !result.controller) {
  Release(result.controller);Destroy(request);Destroy(style);Release(actor,true);return false;
 }
 b->mirror=result.mirror;SetScale(b->scales[0],time,result.controller);b->started=1;
 if(b->reverse)request.reverse=1;
 // EA ignores the second builder's boolean return and reuses its output slot.
 Build(style.slots[1],request,result);
 Controllers c=Allocate(b);Request slave{};
 if(b->slaveTime)slave.time=time;
 if(b->reverse)slave.reverse=1;
 if(!BuildSlaves(b,c,2,slave)) {
  Destroy(slave);Free(c.begin);Release(result.controller);Destroy(request);Destroy(style);Release(actor,true);return false;
 }
 __m128 square=_mm_mul_ps(movement[0],movement[0]);
 __m128 sum=_mm_add_ps(_mm_add_ps(_mm_shuffle_ps(square,square,2),_mm_shuffle_ps(square,square,1)),square);
 slave.name=0;
 if(_mm_cvtss_f32(sum)>0.0f && *CallGameVirtual<const uint32_t*,12>(actor)!=0xe15cafc5u) {
  if(b->reverse)movement[1]=_mm_sub_ps(movement[1],movement[0]);
  CallGameMethod<void,0x0084bc70>(b,b->reverse?&movement[1]:&movement[0],1,0);
 }
 struct Task {void* object; uint32_t data[3];uint32_t argument;};
 struct Tasks {Task* begin;Task* end;Task* capacity;};
 Task storage[4];Tasks tasks{storage,storage,storage+4};
 CallGameMethod<void,0x00844820>(Manager(),style.taskSource,&tasks);
 for(auto* t=tasks.begin;t!=tasks.end;++t)CallGameMethod<void,0x0084c3b0>(b,t->object,t->argument,1);
 Dispatch(b,result.controller,c,2,0,style.scaleEnabled?b->scales[0]:1.0f);
 if(style.scaleEnabled)b->scales[0]=1.0f;
 ClearSlaves(b,c);
 CallGameMethod<void,0x00912b60>(&tasks,tasks.begin,tasks.end);
 // The successful start path does not destroy the slave request (EA lifetime quirk).
 Free(c.begin);Release(result.controller);Destroy(request);Destroy(style);Release(actor,true);return true;
}
bool Finish(Body* b,int32_t mode) {
 void* actor=Actor(b);if(!actor)return false;Add(actor);Style style=GetStyle(b);
 if(b->finished || !style.slots[3]) {Destroy(style);Release(actor,true);return false;}
 b->otherCounter=b->counter=b->initialCounter;
 float time=b->times[2]*b->speed;Request request=Make(b,time);
 if(b->mirror)Mirror(request);
 if(b->reverse)request.reverse=1;
 b->finished=1;void* builder=style.slots[3];Add(builder);
 if(b->special && (mode==0 || mode==1)) {
  Style alternate;CallGameMethod<void,0x00757690>(&alternate,&style);
  unsigned count=CallGameMethod<unsigned,0x0074b990>(&b->stylesBegin);
  if(b->styleIndex<count) {
   void* definition=b->stylesBegin[b->styleIndex].definition;
   CallGameMethod<void,0x00844980>(Manager(),definition,b->styleArgs[0],b->styleArgs[1],b->styleArgs[2],b->styleArgs[3],&alternate,1);
  }
  CallGameFunction<void,0x00848760>(b->name,1.0f,uint8_t(alternate.scaleEnabled==0));
  void** candidates0[]={&alternate.slots[4],&alternate.slots[3],&style.slots[4]};
  void** candidates1[]={&alternate.slots[3],&style.slots[3]};
  void*** candidates=mode==0?candidates0:candidates1;unsigned countIndices=mode==0?3:2;
  for(unsigned i=0;i<countIndices;++i) {
   void** p=candidates[i];
   if(*p && !CallGameVirtual<uint8_t,4>(*p)) {CallGameMethod<void,0x00873360>(&builder,p);break;}
  }
  Destroy(alternate);
 }
 ControllerResult result{};
 if(!Build(builder,request,result) || !result.controller) {
  Release(result.controller);Release(builder,true);Destroy(request);Destroy(style);Release(actor,true);return false;
 }
 b->scales[2]=1.0f;
 if(!b->special && mode==2)SetScale(b->scales[2],time,result.controller);
 CallGameMethod<void,0x0084ce30>(b,0);
 Controllers c=Allocate(b);Request slave{};
 if(b->slaveTime)slave.time=b->times[2]*b->speed;
 else if(b->slaveCount==1) {
  auto* config=static_cast<unsigned char*>(b->slaveConfig);
  if(config[105]) {
   void* source;std::memcpy(&source,config+12,sizeof source);float scratch;
   slave.time=static_cast<float>(CallGameFunction<double,0x0084b430>(b->slaves[0],source,&scratch,1)*b->speed);
  }
 }
 if(b->mirror)Mirror(slave);
 if(b->reverse)slave.reverse=1;
 bool success=BuildSlaves(b,c,4,slave,mode==0);
 if(success) {
  if(mode==0 || mode==1)Dispatch(b,result.controller,c,4,2,1.0f);
  else if(mode==2) {
   Dispatch(b,result.controller,c,4,b->special?1:0,b->scaleEnabled?b->scales[2]:1.0f);
   if(b->scaleEnabled)b->scales[2]=1.0f;
  }
  ClearSlaves(b,c);
 }
 Destroy(slave);Free(c.begin);Release(result.controller);Release(builder,true);Destroy(request);Destroy(style);Release(actor,true);return success;
}
}
