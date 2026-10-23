#include "ts3/anim/m05_M34.h"
#include <intrin.h>
namespace ts3::anim {
namespace {
void Tables(Controller& c,uintptr_t a,uintptr_t b,uintptr_t d) {
#if defined(_M_IX86)
 c.vtable=reinterpret_cast<void*>(a); c.secondary8=reinterpret_cast<void*>(b); c.secondaryC=reinterpret_cast<void*>(d);
#else
 (void)c;(void)a;(void)b;(void)d; NotRewritten("M34 vtables");
#endif
}
void Release(Controller* p) {
 if(p && --p->refs==0) { p->refs=1; CallGameVirtual<void,0>(p,1u); }
}
void Empty(M34Array& a,uintptr_t descriptor) {
 a.begin=nullptr; a.end=nullptr; a.capacity=nullptr;
#if defined(_M_IX86)
 a.descriptor=reinterpret_cast<void*>(descriptor);
#else
 (void)descriptor; NotRewritten("M34 descriptor");
#endif
}
}
void M34Destruct(M34Clip* s) {
 Tables(s->base,0x00FC1BB0,0x00FC1BA8,0x00FC1BA4);
#if defined(_M_IX86)
 s->listener=reinterpret_cast<void*>(0x00FC1E5C);
#endif
 if(s->second) CallGameMethod<void,0x00566590>(&s->second->secondaryC,&s->listener);
 Release(s->second);
 Release(s->first);
 CallGameMethod<void,0x00565B40>(s);
}
void M34Forward(void* listener,const M34Event* input,uint32_t unused) {
 (void)unused;
 auto s=reinterpret_cast<M34Clip*>(static_cast<unsigned char*>(listener)-offsetof(M34Clip,listener));
 if(s) ++s->base.refs;
 M34Event event;
 event.source=input->source; event.resource=input->resource;
 if(event.resource) _InterlockedExchangeAdd(reinterpret_cast<volatile long*>(&event.resource->atomicRefs),1);
 event.host=s->base.hostSlot; event.a=input->a; event.b=input->b;
 event.x=input->x; event.y=input->y; event.z=input->z; event.source=&s->base;
 CallGameMethod<void,0x005686C0>(s,&event);
 if(event.resource && _InterlockedExchangeAdd(reinterpret_cast<volatile long*>(&event.resource->atomicRefs),-1)==1) {
  _InterlockedExchange(reinterpret_cast<volatile long*>(&event.resource->atomicRefs),1);
  CallGameVirtual<void,0>(event.resource,1u);
 }
 Release(&s->base);
}
M34Clip* M34Delete(M34Clip* s,uint32_t flags) {
 M34Destruct(s);
 if(flags&1) CallGameFunction<void,0x004E3F70>(s);
 return s;
}
M34Orient* M34Construct(M34Orient* s,void* host) {
 CallGameMethod<void,0x00565A80>(s,host);
 Tables(s->base,0x00FC1C00,0x00FC1BF8,0x00FC1BF4);
 s->time=0.0f;
 Empty(s->orient,0x00FC13F8);
 return s;
}
void* M34DeleteSecondary(void* p,uint32_t flags) {
 // Verified ResourceBaseDestroySecondary twin: only adjustment and target differ.
 auto s=static_cast<unsigned char*>(p)-offsetof(Controller,secondaryC);
 return CallGameMethod<void*,0x0056ACF0>(s,flags);
}
M34Orient* M34ConstructRotor(M34Orient* s,void* host) {
 CallGameMethod<void,0x00565A80>(s,host);
 s->time=0.0f;
 Empty(s->orient,0x00FC13F8);
 Tables(s->base,0x00FC1CB0,0x00FC1CA8,0x00FC1CA4);
 Empty(s->rotation,0x00FC1C80);
 Empty(s->rotor,0x00FC1C58);
 return s;
}
}
