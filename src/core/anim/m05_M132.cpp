#include "ts3/anim/m05_M132.h"
#include <cmath>
#include <cstring>
#include <intrin.h>
namespace ts3::anim::m132 {
namespace {
struct Ref { void** vtable; int32_t refs; };
struct Request { void* host; float zero1,zero2,angle,zero4; uint16_t flags; Ref* map; };
TS3_X86_OFFSET(Request,angle,12); TS3_X86_OFFSET(Request,map,24);
struct Result { Ref* controller; uint8_t flag; };
// Scratch style record consumed only by the original style helpers. Opaque slots
// widen on native builds; no x86 pointer blob is copied into a native object.
struct Style { void* slots[20]; uint32_t extra[7]; void* strings[4]; };
TS3_X86_OFFSET(Style,extra,80); TS3_X86_SIZE(Style,124);
float Bits(uint32_t bits) { float r; std::memcpy(&r,&bits,4); return r; }
float Abs(float x) { uint32_t b; std::memcpy(&b,&x,4); return Bits(b&0x7fffffff); }
__m128 Sum3(__m128 x) { auto a=_mm_add_ps(_mm_add_ps(_mm_shuffle_ps(x,x,2),_mm_shuffle_ps(x,x,1)),x); return _mm_shuffle_ps(a,a,0); }
__m128 Cross(__m128 a,__m128 b) {
 return _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(a,a,18),_mm_shuffle_ps(b,b,9)),_mm_mul_ps(_mm_shuffle_ps(a,a,9),_mm_shuffle_ps(b,b,18)));
}
__m128 Forward(__m128 q) {
 auto z=_mm_set_ps(0,1,0,0);
 auto t=_mm_add_ps(Cross(z,q),_mm_mul_ps(_mm_shuffle_ps(q,q,255),z));
 return _mm_add_ps(_mm_mul_ps(Cross(t,q),_mm_set1_ps(2)),z);
}
__m128 Normalize(__m128 v,int refinements) {
 auto len=Sum3(_mm_mul_ps(v,v)); auto r=_mm_rsqrt_ps(len);
 for(int i=0;i<refinements;++i)
  r=_mm_add_ps(_mm_mul_ps(_mm_sub_ps(_mm_set1_ps(1),_mm_mul_ps(_mm_mul_ps(r,r),len)),_mm_mul_ps(r,_mm_set1_ps(.5f))),r);
 return _mm_mul_ps(r,v);
}
__declspec(noinline) float Acos(float x) {
#if defined(_M_IX86)
 float r; uintptr_t entry=0x00f7edde;
 __asm { fld x
 call entry
 fstp r }
 return r;
#else
 return std::acos(x);
#endif
}
__declspec(noinline) float Wrap(float angle) {
 float rounded, fraction; double turns;
#if defined(_M_IX86)
 float inv=Bits(0x3e22f983); // original 1/(2*pi), rounded before the x87 multiply
 __asm { fld angle
 fmul inv
 fst rounded
 fstp turns }
 double integral=CallGameFunction<double,0x00f7edf0>(turns);
 double difference=double(rounded)-integral;
 fraction=float(difference);
 bool over=difference>0.5;
#else
 turns=double(angle)*double(Bits(0x3e22f983)); rounded=float(turns);
 double difference=double(rounded)-std::floor(turns); fraction=float(difference);
 bool over=difference>0.5;
#endif
 if(over) fraction-=1.0f;
 float r=fraction*Bits(0x40c90fdb);
 if(r>Bits(0x40490fdb)) r-=Bits(0x40c90fdb);
 return r;
}
float Angle(__m128 a,__m128 b,bool direction) {
 alignas(16) float aa[4],bb[4]; _mm_store_ps(aa,a); _mm_store_ps(bb,b); aa[1]=bb[1]=0;
 a=_mm_load_ps(aa); b=_mm_load_ps(bb);
 float la=_mm_cvtss_f32(Sum3(_mm_mul_ps(a,a))),lb=_mm_cvtss_f32(Sum3(_mm_mul_ps(b,b)));
 if(!(la>0) || (direction?!(lb>0):lb==0)) return 0;
 a=Normalize(Normalize(a,2),1); b=Normalize(b,direction?2:1);
 if(direction) b=Normalize(b,1);
 auto dot=_mm_min_ps(_mm_max_ps(Sum3(_mm_mul_ps(a,b)),_mm_set1_ps(-1)),_mm_set1_ps(1));
 float angle=Acos(_mm_cvtss_f32(dot));
 auto cross=Sum3(_mm_mul_ps(Cross(b,a),_mm_set_ps(0,0,1,0)));
 if(_mm_cvtss_f32(cross)<0) angle=Bits(0x40c90fdb)-angle;
 return Wrap(angle);
}
struct ActorPrefix { void** vtable; unsigned char opaque[8]; int32_t count; };
TS3_X86_OFFSET(ActorPrefix,count,12);
int ActorCount(void* actor) { return static_cast<ActorPrefix*>(actor)->count; }
void Release(Ref* r,bool atomic) {
 if(!r) return;
 int32_t count;
 if(atomic) count=_InterlockedDecrement(reinterpret_cast<volatile long*>(&r->refs));
 else count=--r->refs;
 if(count==0) {
  if(atomic) _InterlockedExchange(reinterpret_cast<volatile long*>(&r->refs),1); else r->refs=1;
  CallGameVirtual<void,0>(r,1);
 }
}
bool BroadStyle(uint32_t s) { return s==0x100 || s==0x10000 || s==0x900 || s==0xa00 || s==0xb00; }
float Threshold(bool broad,bool reconcile) {
#if defined(_M_IX86)
 auto* flags=reinterpret_cast<uint32_t*>(reconcile?0x011d9288:0x011d9280);
 if(!(*flags&1)) { *flags|=1; *reinterpret_cast<uint32_t*>(reconcile?0x011d9284:0x011d927c)=reconcile?1024391733u:1048971922u; }
 if(!reconcile && !(*flags&2)) { *flags|=2; *reinterpret_cast<uint32_t*>(0x011d9278)=1024391733u; }
 return *reinterpret_cast<float*>(reconcile?0x011d9284:(broad?0x011d927c:0x011d9278));
#else
 return Bits(broad&&!reconcile?1048971922u:1024391733u);
#endif
}
// 84B590's exact body saves EDI at 84B5C7 then overwrites it at 84B5C8;
// it consumes ECX plus seven stack arguments (RET 1Ch at 84BBF0).
// Thus these call sites need a normal method bridge, not an incoming-EDI adapter.
void Start(Body* self,Ref* controller,float angle,int mode,bool reconcile=false) {
 float duration=float(CallGameVirtual<double,8>(controller));
 double desiredWide=CallGameFunction<double,0x008487a0>(Abs(angle),self->style);
 float desired=float(desiredWide);
 float ratio=0;
 // EA compares the unrounded x87 return, then divides by its rounded float.
 if(desiredWide>0) ratio=duration/desired;
 if(reconcile) self->turning=1;
 if(!(Abs(ratio-1.0f)<Bits(0x37800000))) {
  void* allocation=CallGameFunction<void*,0x004e3f90>(56,"Animation/TimeScaleController",0,0,0,0);
  Ref* scaled=allocation?CallGameMethod<Ref*,0x0056f830>(allocation,self->host,controller,ratio):nullptr;
  if(scaled) ++scaled->refs;
  CallGameMethod<void,0x0084b590>(self,scaled,0,0,mode,0,0.0f,1.0f);
  CallGameMethod<void,0x0055cfa0>(&scaled);
 } else CallGameMethod<void,0x0084b590>(self,controller,0,0,mode,0,0.0f,1.0f);
}
void* Route(Body* self) { return CallGameMethod<void*,0x00c93970>(self->actor); }
bool RouteFlag(void* route,uint32_t flag) { auto* state=CallGameVirtual<void*,3>(route); return CallGameMethod<unsigned char,0x0081c880>(state,0,flag,0)!=0; }
}
char __fastcall TurnToward(Body* self,void*,const __m128* direction,int mode,char requested) {
 void* actor=self->actor; void* source=self->owner;
 if(ActorCount(actor)>0) {
  if(self->blocked) return 0;
  if(actor!=source && CallGameVirtual<int,6>(source)==2) source=actor;
 }
 __m128 orientation; CallGameVirtual<void,10>(source,&orientation);
 float angle=Angle(Forward(orientation),*direction,true);
 Style style; CallGameMethod<void,0x007513f0>(&style); CallGameMethod<void,0x0084bc00>(self,&style);
 float threshold=mode==1?Threshold(BroadStyle(style.extra[3]),false):0;
 // COMISS/JBE rejects unordered as well as <=.
 if(!(Abs(angle)>threshold)) { CallGameMethod<void,0x00754060>(&style); return 0; }
 Request req{self->host,0,0,0,0,0,nullptr}; CallGameMethod<void,0x00849bf0>(self,&req);
 req.angle=angle;
 if(self->mirrored) CallGameMethod<void,0x00848620>(&req);
 Result result{nullptr,0}; void* builder=style.slots[7];
 if(builder) CallGameVirtual<void,3>(builder,&req,&result);
 if(!result.controller) {
  // EA's failure path releases only map; it does NOT call the request destructor.
  Release(req.map,true); CallGameMethod<void,0x00754060>(&style); return 0;
 }
 Start(self,result.controller,req.angle,mode);
 self->requested=requested; self->turning=requested;
 Release(result.controller,false); CallGameMethod<void,0x008499b0>(&req); CallGameMethod<void,0x00754060>(&style);
 return 1;
}
char __fastcall ReconcileTurn(Body* self,void*) {
 if(self->host) { auto* property=CallGameVirtual<void*,1>(self->host,uint32_t(0xf4c95ed2)); if(property) CallGameVirtual<void,2>(property,1.0f); }
 __m128 orientation,target; CallGameVirtual<void,9>(self->owner,&orientation); target=orientation;
 if(Route(self)) {
  auto* route=Route(self);
  if(CallGameVirtual<unsigned char,79>(route)) { alignas(16) __m128 tmp; target=*CallGameVirtual<__m128*,80>(Route(self),&tmp); }
  if(RouteFlag(Route(self),0x20000)) return 0;
 }
 if(ActorCount(self->actor)>0 && self->owner && CallGameVirtual<int,6>(self->owner)==2) {
  auto* route=Route(self);
  if(route && CallGameVirtual<unsigned char,79>(route)) {
   alignas(16) __m128 tmp; __m128 delta=_mm_sub_ps(*CallGameVirtual<__m128*,80>(route,&tmp),orientation);
   auto mag=_mm_max_ps(_mm_mul_ps(_mm_set1_ps(-1),delta),delta);
   if(_mm_movemask_ps(_mm_cmple_ps(mag,_mm_set1_ps(Bits(0x37800000))))!=15) { CallGameVirtual<void,81>(route,&orientation); target=orientation; }
  }
 }
 float angle=Angle(Forward(orientation),Forward(target),false);
 float threshold=Threshold(false,true);
 if(self->disabled || !(Abs(angle)>=threshold)) return 0;
 auto* route=Route(self);
 if(route) {
  alignas(16) unsigned char position[16]; CallGameVirtual<void,77>(route,position);
  int count=CallGameVirtual<int,19>(route);
  if(count && CallGameVirtual<int,22>(route,count-1)==8 &&
   (self->style==512 || (self->style==1024 && (1u<<(self->gait&31))!=4) || self->style==1536) && !RouteFlag(route,0x400000)) {
   void* state=CallGameVirtual<void*,3>(route); int x=CallGameMethod<int,0x0081c8e0>(state);
   state=CallGameVirtual<void*,3>(route); int y=CallGameMethod<int,0x0081c8d0>(state);
   uint64_t routeKey=CallGameVirtual<uint64_t,84>(route); uint64_t ownerKey=CallGameVirtual<uint64_t,4>(self->owner);
   if(!CallGameFunction<unsigned char,0x007967d0>(ownerKey,routeKey,position,&target,y,x,0)) { self->routeFailed=1; return 0; }
  }
 }
 Request req{self->host,0,0,0,0,0,nullptr}; CallGameMethod<void,0x00849bf0>(self,&req); req.angle=angle;
 if(self->mirrored) CallGameMethod<void,0x00848620>(&req);
 Result result{nullptr,0}; if(self->builder) CallGameVirtual<void,3>(self->builder,&req,&result);
 if(!result.controller) { CallGameMethod<void,0x008499b0>(&req); return 0; }
 Start(self,result.controller,req.angle,5,true);
 CallGameMethod<void,0x0055cfa0>(&result.controller); CallGameMethod<void,0x008499b0>(&req);
 return 1;
}
}
