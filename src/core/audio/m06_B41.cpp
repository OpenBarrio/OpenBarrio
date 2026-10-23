#include "ts3/audio/m06_B41.h"
#include <cstring>
namespace ts3::audio::b41 {
namespace {
void* Identity(uintptr_t a) {
#if defined(_M_IX86)
 return reinterpret_cast<void*>(a);
#else
 // Native type-identity tokens, not truncated pointers into the game image.
 static int tokens[7];
 const uintptr_t ids[]={0x0103A394,0x0103A378,0x0103A374,0x00F99AC0,0x0107396C,0x00FA1B78,0x00F98980};
 for(unsigned i=0;i<7;++i) if(ids[i]==a) return tokens+i;
 return nullptr;
#endif
}
void Retain(Ref* p) { if(p) CallGameVirtual<uint32_t,0>(p); }
void Release(Ref* p) { if(p) CallGameVirtual<uint32_t,1>(p); }
void Assign(Ref*& dst,Ref* src) {
 if(src!=dst) { Retain(src); Ref* old=dst; dst=src; Release(old); }
}
RefVector* Entries(void* secondary) {
 return reinterpret_cast<RefVector*>(static_cast<char*>(secondary)+offsetof(SceneList,entries)-offsetof(SceneList,secondary));
}
Primitive* Primary(void* secondary) {
 return reinterpret_cast<Primitive*>(static_cast<char*>(secondary)-offsetof(Primitive,secondary));
}
void* Secondary(Primitive* p) { return p?&p->secondary:nullptr; }
}
// Clone WorkQueue::QueueNotification deleting thunk, adapting the -8 adjustment.
SceneList* __fastcall F8A69B0(void* self,void*,uint32_t flags) {
 return F8A6A50(reinterpret_cast<SceneList*>(static_cast<char*>(self)-offsetof(SceneList,tertiary)),nullptr,flags);
}
void* __fastcall F8A69C0(void* self,void*,uint32_t id) {
 auto* primary=reinterpret_cast<SceneList*>(static_cast<char*>(self)-offsetof(SceneList,secondary));
 return CallGameMethod<void*,0x008A6560>(primary,id);
}
SceneList* __fastcall F8A69D0(SceneList* self,void*,void* owner) {
 self->state=0;
 self->vtable=Identity(0x0103A394); self->secondary=Identity(0x0103A378); self->tertiary=Identity(0x0103A374);
 auto& v=self->entries;
 v.begin=v.end=v.storage=v.inlineStorage; v.capacity=v.inlineStorage+10;
 v.label=static_cast<const char*>(Identity(0x00F99AC0));
 self->active=1; self->key[0]=0x3025861A; self->key[1]=0x93A25C7D;
 self->a=0; self->b=0; self->gain=1.0f; self->flag=0; self->owner=owner;
 return self;
}
SceneList* __fastcall F8A6A50(SceneList* self,void*,uint32_t flags) {
 self->vtable=Identity(0x0103A394); self->secondary=Identity(0x0103A378); self->tertiary=Identity(0x0103A374);
 CallGameMethod<void,0x008A6960>(&self->entries);
 self->tertiary=Identity(0x0107396C); self->secondary=self->vtable=Identity(0x00FA1B78);
 if(flags&1) {
#if defined(_M_IX86)
  auto** pool=reinterpret_cast<void**>(0x011D9FE0);
  self->vtable=*pool; *pool=self;
#else
  AbortNotRewritten("m06_B41 scene-list pool");
#endif
 }
 return self;
}
void __fastcall F8A6AA0(RefVector* v,void*,Ref** pos,Ref** value) {
 if(v->end!=v->capacity) {
  Ref** source=value;
  if(value>=pos && value<v->end) source=value+1;
  if(v->end) { *v->end=v->end[-1]; Retain(*v->end); }
  for(Ref** p=v->end-1;p!=pos;) { --p; Assign(p[1],*p); }
  Assign(*pos,*source);
  ++v->end;
 } else {
  size_t count=v->end-v->begin;
  size_t cap=count?count*2:1;
  auto** buffer=CallGameFunction<Ref**,0x004E3F90>(cap*sizeof(Ref*),v->label,0,0,Identity(0x00F98980),0xE7);
  Ref** out=buffer;
  for(Ref** p=v->begin;p!=pos;++p,++out) { *out=*p; Retain(*out); }
  *out=*value; Retain(*out); ++out;
  for(Ref** p=pos;p!=v->end;++p,++out) { *out=*p; Retain(*out); }
  for(Ref** p=v->begin;p!=v->end;++p) Release(*p);
  if(v->begin && v->begin!=v->storage) CallGameFunction<void,0x004E3F70>(v->begin);
  v->begin=buffer; v->end=out; v->capacity=buffer+cap;
 }
}
uint8_t __fastcall F8A6BF0(void* self,void*,Ref* value) {
 auto* v=Entries(self);
 Ref** p=v->begin;
 while(p!=v->end && *p!=value) ++p;
 if(p==v->end) return 0;
 for(;p+1<v->end;++p) Assign(*p,p[1]);
 --v->end; Release(*v->end);
 return 1;
}
uint8_t __fastcall F8A6C50(void* self,void*,Ref* value) {
 auto* v=Entries(self);
 for(Ref** p=v->begin;p!=v->end;++p) if(*p==value) return 0;
 Retain(value);
 if(v->end<v->capacity) {
  Ref** out=v->end++;
  if(out) { *out=value; Retain(value); }
 } else F8A6AA0(v,nullptr,v->end,&value);
 Release(value);
 return 1;
}
uint8_t __fastcall F8A6CE0(Primitive* self,void*) {
 void* manager=CallGameFunction<void*,0x0044C470>();
 if(manager) CallGameVirtual<void,11>(manager,Secondary(self),0x02B0C259u,-9999);
 return 1;
}
uint8_t __fastcall F8A6D10(Primitive* self,void*) { return self->enabled; }
// Clone PackedFileRecordQueryInterfaceById, adapting the first ID only.
void* __fastcall F8A6D20(void* self,void*,uint32_t id) {
 if(id==0xEE3F516Eu || id==0x4BC808E5u) return self;
 return nullptr;
}
uint8_t __fastcall F8A6D50(void* self,void*,uint32_t id,const void* message) {
 if(id==0x02B0C259) {
  uint32_t key; std::memcpy(&key,static_cast<const char*>(message)+0x18,4);
  auto* primary=Primary(self);
  if(key==CallGameVirtual<uint32_t,6>(primary)) CallGameVirtual<void,15>(primary);
 }
 return 1;
}
uint8_t __fastcall F8A6D90(Primitive* self,void*) {
 void* manager=CallGameFunction<void*,0x0044C470>();
 if(manager) CallGameVirtual<void,9>(manager,Secondary(self),0x02B0C259u,0,0);
 void* system=CallGameFunction<void*,0x008B0A70>();
 if(self->resource) { Ref* old=self->resource; self->resource=nullptr; Release(old); }
 uint32_t key=CallGameVirtual<uint32_t,6>(self);
 CallGameVirtual<void,7>(system,key,0,&self->resource,0x1407EC);
 CallGameVirtual<void,15>(self);
 return 1;
}
void __fastcall F8A6E10(GainView* self,void*,float value) {
 // LAHF/test AH,44/jnp skips ordered equality only; unordered updates.
 if(value!=self->gain) {
  CallGameVirtual<void,10>(self,1);
  self->gain=value;
 }
}
uint8_t __fastcall F8A6E40(GainView* self,void*,uint32_t value) {
 self->flag=static_cast<uint8_t>(value); return self->flag;
}
uint32_t __fastcall F8A6E50(RefCountView* self,void*) {
 self->refs+=1u; return self->refs;
}
}
