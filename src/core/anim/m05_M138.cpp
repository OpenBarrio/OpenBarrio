#include "ts3/anim/m05_M138.h"
#include <atomic>
namespace ts3::anim::m138 {
namespace {
struct Request { void* rig; float time,x,y,z; uint8_t mirror,forced; uint16_t padding; void* names; };
struct Binding { void* controller; uint8_t flag; };
struct Vector { void** begin; void** end; void** capacity; const char* descriptor; };
struct Effect { void* object; uint32_t opaque[3]; uint32_t value; };
struct Effects { Effect* begin; Effect* end; Effect* capacity; };
TS3_X86_SIZE(Request,28);
void Release(void* p) { if(p) CallGameMethod<void,0x00b31c50>(p); }
void ReleaseNames(void* p) {
 if(!p) return;
 struct Prefix { void* vtable; int32_t refs; };
 auto* resource=static_cast<Prefix*>(p);
 std::atomic_ref<int32_t> refs(resource->refs);
 if(refs.fetch_sub(1)==1) { refs.exchange(1); CallGameVirtual<void,0>(p,1); }
}
void DestroyRequest(Request& r) { CallGameMethod<void,0x008499b0>(&r); }
void DestroyBinding(Binding& b) { CallGameMethod<void,0x0055cfa0>(&b); }
void Free(void* p) { if(p) CallGameFunction<void,0x004e3f70>(p); }
void Prepare(Body* s,Request& r) { CallGameMethod<void,0x00849bf0>(s,&r); }
void Mirror(Request& r) { CallGameMethod<void,0x00848620>(&r); }
bool Create(void* source,Request& r,Binding& b) {
 return source && CallGameVirtual<uint8_t,3>(source,&r,&b)!=0 && b.controller;
}
Vector Slaves(Body* s) {
 Vector v{nullptr,nullptr,nullptr,
#if defined(_M_IX86)
 reinterpret_cast<const char*>(0x010272f4)
#else
 "Locomotion/AnimatedSimBody/SlaveControllersVector"
#endif
 };
 CallGameMethod<void,0x00aef830>(&v,s->slaveCount); return v;
}
bool CreateSlaves(Body* s,Vector& v,int slot,Request r) {
 // The 28-byte request is passed by value, with its retained namespace copy.
 Request copy; CallGameMethod<void,0x008499e0>(&copy,&r);
 return CallGameMethod<uint8_t,0x00849d40>(s,&v,slot,copy,0)!=0;
}
void Attach(Body* s,Binding& b,Vector& v,int slot,int mode,uint32_t flag,float rate) {
 // Raw decoding proves seven stack arguments and RET 1C. IDA's apparent
 // EDI argument is a stale saved-register alias, not an input to this entry.
 CallGameMethod<void,0x0084b590>(s,b.controller,s->slaveCount>0?v.begin:nullptr,s->slaveCount,slot,mode,flag,rate);
}
void ReleaseSlaves(Body* s,Vector& v) {
 for(int32_t i=0;i<s->slaveCount;++i) if(s->slaves[i]) { Release(v.begin[i]); v.begin[i]=nullptr; }
}
void* Manager() {
#if defined(_M_IX86)
 return *reinterpret_cast<void**>(0x011d9140);
#else
 NotRewritten("animation style manager");
#endif
}
void Resolve(Body* s,Style& style,uint32_t key) {
 CallGameMethod<void,0x00757690>(&style,&s->style);
 // Although IDA labels this stdcall, ECX supplies the manager as well.
 CallGameMethod<void,0x00844980>(Manager(),key,s->style.values[1],s->style.values[2],s->style.values[3],s->style.values[4],&style,1);
}
bool Transition(Body* s,bool forward) {
 const uint32_t current=s->current,target=s->target;
 s->flags[forward?1:0]=0;
 if(forward ? target<=current : target>=current) return false;
 // EA performs signed byte-distance / 28, then an unsigned comparison.
 const uint32_t count=static_cast<uint32_t>(static_cast<int32_t>(reinterpret_cast<uintptr_t>(s->stepsEnd)-reinterpret_cast<uintptr_t>(s->steps))/28);
 if(count<=target) return false;
 float* times=forward?s->forwardTimes:s->backwardTimes;
 if(!times || (!forward && current>=count)) return false;
 const uint32_t index=forward?current+1:current;
 const float time=times[index]*s->timeScale;
 Request r{s->rig,0,0,0,0,0,0,0,nullptr}; Prepare(s,r); r.time=time;
 Style style; Resolve(s,style,s->steps[index].key);
 void* source=style.refs[forward?10:11];
 bool result=false;
 if(source) {
  if(s->mirror) Mirror(r);
  Binding first{nullptr,0};
  if(Create(source,r,first)) {
   double duration=CallGameFunction<double,0x00849540>(first.controller);
   float rounded=static_cast<float>(duration);
   float& rate=forward?s->forwardRate:s->backwardRate;
   rate=1.0f; if(duration>0.0) rate=time/rounded;
   DestroyBinding(first);
   if(s->forced) r.forced=1;
   Binding second{nullptr,0};
   if(Create(source,r,second)) {
    Vector v=Slaves(s);
    Request slave{nullptr,0,0,0,0,0,0,0,nullptr};
    if(s->sync) slave.time=time;
    if(s->mirror) Mirror(slave);
    if(s->forced) slave.forced=1;
    bool ready=s->slaveCount<=0 || !s->slaveSource || CreateSlaves(s,v,forward?9:10,slave);
    s->flags[forward?1:0]=1;
    if(!ready) {
     DestroyRequest(slave); Free(v.begin); DestroyBinding(second);
     CallGameMethod<void,0x00754060>(&style); DestroyRequest(r); return false;
    }
    CallGameMethod<void,0x0084ce30>(s,0);
    Effect local[4]; Effects effects{local,local,local+4};
    CallGameMethod<void,0x00844820>(Manager(),style.values[0],&effects);
    for(auto* e=effects.begin;e!=effects.end;++e) CallGameMethod<void,0x0084c3b0>(s,e->object,e->value,1);
    Attach(s,second,v,forward?9:10,1,1,s->style.flags[1]?rate:1.0f);
    if(s->style.flags[1]) rate=1.0f;
    ReleaseSlaves(s,v);
    CallGameMethod<void,0x00912b60>(&effects,effects.begin,effects.end);
    DestroyRequest(slave); Free(v.begin);
   }
   DestroyBinding(second); result=true;
  } else DestroyBinding(first);
 }
 CallGameMethod<void,0x00754060>(&style); DestroyRequest(r); return result;
}
}
bool CreateBody(Body* s) {
 Request r{s->rig,0,0,0,0,0,0,0,nullptr}; Prepare(s,r);
 if(s->forced) r.forced=1;
 Binding b{nullptr,0};
 if(Create(s->style.refs[5],r,b)) {
  Vector v=Slaves(s);
  Request slave{nullptr,0,0,0,0,0,0,0,nullptr}; if(s->forced) slave.forced=1;
  if(s->slaveCount>0 && s->slaveSource && !CreateSlaves(s,v,8,slave)) {
   DestroyRequest(slave); Free(v.begin); DestroyBinding(b); DestroyRequest(r); return false;
  }
  Attach(s,b,v,8,2,0,1.0f); ReleaseSlaves(s,v); Free(v.begin);
  Release(b.controller); ReleaseNames(r.names); return true;
 }
 // On an absent source EA never constructs/releases the binding wrapper.
 if(s->style.refs[5]) Release(b.controller);
 ReleaseNames(r.names); return false;
}
bool Forward(Body* s) { return Transition(s,true); }
bool Backward(Body* s) { return Transition(s,false); }
bool __fastcall HookCreateBody(Body* s,void*) { return CreateBody(s); }
bool __fastcall HookForward(Body* s,void*) { return Forward(s); }
bool __fastcall HookBackward(Body* s,void*) { return Backward(s); }

}
