#include "ts3/anim/m05_M98.h"
#include <cfloat>
namespace ts3::anim::m98_impl {
namespace {
uint32_t Mask(unsigned address) {
#if defined(_M_IX86)
 return *reinterpret_cast<const uint32_t*>(address);
#else
 (void)address; AbortNotRewritten("m98 global mask"); return 0;
#endif
}
void* Allocate(size_t n,const char* label,const char* file=nullptr,unsigned line=0) {
 return CallGameFunction<void*,0x004E3F90>(n,label,0,0,file,line);
}
struct Vector { void* begin; void* end; void* capacity; void* allocator; };
struct Sound { Vector first; uintptr_t opaque[2]; Vector second; };
void Clear(Vector& v,unsigned shift) {
 if(v.begin) {
  auto bytes=static_cast<int32_t>(reinterpret_cast<uintptr_t>(v.capacity)-reinterpret_cast<uintptr_t>(v.begin));
  auto size=(static_cast<uint32_t>(bytes>>shift)<<shift);
  CallGameVirtual<void,3>(v.allocator,v.begin,size);
 }
}
}
uintptr_t CreateEffect(Event* event) {
 void* service=CallGameFunction<void*,0x00938B50>();
 if(!service) return 0;
 Sound sound;
 void* config=CallGameFunction<void*,0x00931020>();
 CallGameMethod<void,0x0093B020>(&sound,config);
 void* name=CallGameMethod<void*,0x0057F280>(event);
 CallGameMethod<void,0x0093B680>(&sound,0x8cdb555bu,name);
 uint32_t mask=Mask(0x011E9158)|Mask(0x011E8E90);
 uintptr_t output[3];
 CallGameMethod<void,0x0093B5A0>(&sound,output);
 name=CallGameMethod<void*,0x0057F280>(event);
 uint32_t hash[4]={CallGameFunction<uint32_t,0x004FA590>(name,0x811c9dc5u,0),0,0x2da18f83u,3};
 auto result=CallGameMethod<uintptr_t,0x0093A1E0>(service,output,mask,hash,0,0);
 Clear(sound.second,2); Clear(sound.first,4);
 return result;
}
uintptr_t HandleEffect(Context* context,Event* event,float,float) {
 uintptr_t result=event->type;
 if(event->type==0xc1e4000au) {
  result=event->data[7];
  if(context->state->manager) {
   void* entity=CallGameMethod<void*,0x00581810>(context->state->manager,event->data[7]);
   result=reinterpret_cast<uintptr_t>(entity);
   if(entity) {
    void* object=CallGameMethod<void*,0x00581560>(entity,0);
    result=reinterpret_cast<uintptr_t>(object);
    if(object) {
     void* effect=CallGameMethod<void*,0x00935770>(object,Mask(0x011E8E90));
     result=reinterpret_cast<uintptr_t>(effect);
     if(effect) result=CallGameMethod<uintptr_t,0x00944C60>(effect,0);
    }
   }
  }
  return result;
 }
 if(event->type!=0xc1e40005u) return result;
 void* entity=nullptr;
 if(context->state->manager) entity=CallGameMethod<void*,0x00581810>(context->state->manager,event->data[13]);
 if(entity) CallGameVirtual<void,1>(entity);
 else {
  void* memory=Allocate(32,"Animation/EffectEventHandler/SceneEntity");
  if(memory) {
   void* made=CallGameMethod<void*,0x00581890>(memory,1);
   if(made) { CallGameVirtual<void,1>(made); entity=made; }
  }
  if(context->state->manager) CallGameMethod<void,0x00582090>(context->state->manager,event->data[13],entity);
 }
 void* object=nullptr;
 if(entity) {
  void* initial=CallGameMethod<void*,0x00581560>(entity,1);
  result=CallGameMethod<uintptr_t,0x00B96150>(&object,initial);
  if(!object) {
   auto created=CreateEffect(event);
   result=CallGameMethod<uintptr_t,0x00B96150>(&object,reinterpret_cast<void*>(created));
   result=(*reinterpret_cast<uint32_t*>(static_cast<unsigned char*>(entity)+sizeof(void*)+8))&1;
   if(result==1) {
    CallGameMethod<void,0x00581510>(entity,1);
    result=CallGameMethod<uintptr_t,0x00581F70>(entity,object);
   }
  }
 } else {
  auto created=CreateEffect(event);
  result=CallGameMethod<uintptr_t,0x00B96150>(&object,reinterpret_cast<void*>(created));
 }
 if(object) {
  void* effect=CallGameMethod<void*,0x00935770>(object,Mask(0x011E8E90));
  if(effect) {
   const char* name=nullptr;
   if(context->state->name) name=CallGameMethod<const char*,0x00F20360>(context->state->name);
   if(!name||!*name) name="unknown_namespace";
   void* target=nullptr;
   if(event->data[11]==CallGameFunction<uint32_t,0x007CFE70>(name)) target=CallGameMethod<void*,0x00582660>(context);
   else if(context->state->manager) target=CallGameMethod<void*,0x00581840>(context->state->manager,event->data[11]);
   uint32_t slot=event->data[12],temporary=0;
   if(context->state->remap) slot=CallGameFunction<uint32_t,0x005826D0>(target,slot,&temporary);
   if(target&&slot) {
    void* animation=CallGameFunction<void*,0x0074B9B0>(object);
    if(animation) CallGameMethod<void,0x0095B4E0>(animation,target,slot,0,0);
   }
   CallGameMethod<void,0x00944C60>(effect,0);
   CallGameMethod<void,0x00944BF0>(effect,0,0);
   CallGameMethod<void,0x00944CD0>(effect);
  }
  result=CallGameMethod<uintptr_t,0x007460F0>(&object);
 }
 if(entity) result=CallGameVirtual<uintptr_t,2>(entity);
 return result;
}
void** FindSequence(Groups* self,uint32_t key) {
 for(Link* link=self->list.next;link!=&self->list;link=link->next) {
  auto node=reinterpret_cast<Node*>(link);
  if(node->key==key) return &node->controller;
 }
 const char* file=nullptr;
#if defined(_M_IX86)
 file=reinterpret_cast<const char*>(0x00F98980);
#endif
 auto node=static_cast<Node*>(Allocate(sizeof(Node),self->descriptor,file,0xe7));
 node->controller=nullptr; // EA faults on allocation failure rather than returning a friendly null.
 node->link.next=&self->list; node->link.previous=self->list.previous;
 self->list.previous->next=&node->link; self->list.previous=&node->link;
 node->key=key;
 void* storage=Allocate(112,"Animation/SequenceController");
 void* controller=storage?CallGameMethod<void*,0x00567480>(storage,self->host,key):nullptr;
 void* old=node->controller;
 if(controller!=old) {
  if(controller) ++*reinterpret_cast<uint32_t*>(static_cast<unsigned char*>(controller)+sizeof(void*));
  node->controller=controller;
  if(old) {
   auto& refs=*reinterpret_cast<uint32_t*>(static_cast<unsigned char*>(old)+sizeof(void*));
   if(--refs==0) { refs=1; CallGameVirtual<void,0>(old,1); }
  }
 }
 return &node->controller;
}
uintptr_t DispatchTrack(void* receiver,Context* context,Event* event,float start,float end) {
 return CallGameMethod<uintptr_t,0x00582930>(receiver,context,event,start,end);
}
uintptr_t DispatchStop(void* receiver,Context* context,Event* event,float start,float end) {
 return CallGameMethod<uintptr_t,0x00583050>(receiver,context,event,start,end);
}
uintptr_t Dispatch(void* receiver,Context* context,Event* event) {
 float start=context->state->controller?CallGameVirtual<float,6>(context->state->controller):0.0f;
 float end=context->state->controller?CallGameVirtual<float,7>(context->state->controller):FLT_MAX;
 if(event&&event->resource) {
  if(CallGameMethod<void*,0x0057A7A0>(event->resource)) CallGameMethod<void*,0x0057A7A0>(event->resource);
 }
 switch(event->type) {
 case 0xc1e40001u: case 0xc1e40002u: return DispatchTrack(receiver,context,event,start,end);
 case 0xc1e40003u: return DispatchStop(receiver,context,event,start,end);
 case 0xc1e40005u: case 0xc1e4000au: return HandleEffect(context,event,start,end);
 case 0xc1e40006u: return CallGameMethod<uintptr_t,0x00583120>(receiver,context,event,start,end);
 default: return event->type+0x3e1bffffu;
 }
}
}
