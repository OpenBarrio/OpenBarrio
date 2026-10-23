#include "ts3/anim/m05_B28.h"
#include <atomic>
namespace ts3::anim::b28 {
TS3_X86_SIZE(Entity,28); TS3_X86_OFFSET(Parent,enabled,80);
static void* Table(uintptr_t address) {
#if defined(_M_IX86)
 return reinterpret_cast<void*>(address);
#else
 (void)address; AbortNotRewritten("B28 original table"); return nullptr;
#endif
}
static void Free(void* p) { CallGameFunction<void,0x004E3F70>(p); }
static void* Lookup(Key k,bool guarded) {
 void* manager=CallGameFunction<void*,0x00938B50>();
 if(guarded && !manager) return nullptr;
 return CallGameMethod<void*,0x00939560>(manager,k.low,k.high);
}
void* Resolve(Entity* s,uint8_t clear) {
 if(!(s->key.low|s->key.high)) return nullptr;
 void* p=Lookup(s->key,true);
 if(clear && !p) s->key={0,0};
 return p;
}
Parent* SetParent(Entity* s,Parent* p) {
 Parent* result=nullptr;
 if(s->parent) { Free(s->parent); result=p; }
 s->parent=p; return result;
}
void* ResolveKey(Key* k) { return Lookup(*k,false); }
void* ResolveParent(Parent* p) {
 void* obj=ResolveKey(&p->key); if(!obj) return nullptr;
 void* token=nullptr;
#if defined(_M_IX86)
 token=*reinterpret_cast<void**>(0x011E9114);
#endif
 obj=CallGameMethod<void*,0x00935770>(obj,token); if(!obj) return nullptr;
 obj=CallGameMethod<void*,0x00951F00>(obj,p->slot); if(!obj) return nullptr;
 return CallGameVirtual<void*,2>(obj,1);
}
Entity* Find(Map* s,uint32_t key) {
 for(auto p=s->begin;p!=s->end;++p) if(p->key==key) return p->value;
 return nullptr;
}
uint8_t Replace(Map* s,uint32_t key,Entity* value) {
 if(!key || !value) return 0;
 for(auto p=s->begin;p!=s->end;++p) if(p->key==key) {
  CallGameVirtual<void,1>(value); Entity* old=p->value; p->value=value;
  if(old) {
   void* a=Resolve(old,0); void* b=Resolve(value,0);
   if(a==b) {old->flags=0x40000001;old->key={0,0};}
   CallGameVirtual<void,2>(old);
  }
  return 1;
 }
 return 0;
}
void* FindResolved(Map* s,uint32_t key) { auto p=Find(s,key); return p?Resolve(p,0):nullptr; }
Entity* ConstructEntity(Entity* s,uint32_t flags) {
 s->vtable=Table(0x00FF39F8); std::atomic_ref<int32_t>(s->refs).exchange(0);
 s->interfaceTable=Table(0x00FA1B84); s->flags=flags;
 s->vtable=Table(0x00FCB7EC); s->interfaceTable=Table(0x00FCB7DC);
 s->key={0,0};s->parent=nullptr;return s;
}
Parent* TakeParent(Entity* s,uint8_t take) {
 if(!(s->flags&4)) {
  auto p=s->parent;
  if(!p || !(p->key.low|p->key.high) || ResolveKey(&p->key)) s->flags|=4;
 }
 auto p=s->parent;if(take) s->parent=nullptr;return p;
}
static void Check(Stream* s,uint8_t ok) {CallGameMethod<void,0x007D77C0>(s,ok);}
static void Write(Stream* s,uintptr_t v) {Check(s,CallGameFunction<uint8_t,0x004DDBD0>(s->data,v,s->version));}
static void WriteKey(Stream* s,Key k) {Check(s,CallGameFunction<uint8_t,0x004DDD20>(s->data,k.low,k.high,s->version));}
static void Read(Stream* s,void* out) {Check(s,CallGameFunction<uint8_t,0x004DD700>(s->data,out,s->version));}
static void ReadKey(Stream* s,Key* out) {Check(s,CallGameFunction<uint8_t,0x004DD660>(s->data,out,s->version));}
uint8_t SaveEntity(Entity* e,Stream* s,uint32_t) {
 Write(s,e->flags);Write(s,reinterpret_cast<uintptr_t>(Resolve(e,0)));WriteKey(s,e->key);
 Write(s,reinterpret_cast<uintptr_t>(e->parent));
 if(e->parent) {
  WriteKey(s,e->parent->key);void* p=ResolveParent(e->parent);Write(s,reinterpret_cast<uintptr_t>(p));
  if(p) Write(s,e->parent->slot);
  CallGameMethod<void,0x007D79C0>(s,e->parent->opaque+4);
  Check(s,CallGameFunction<uint8_t,0x004DDA60>(s->data,e->parent->enabled));
  Check(s,CallGameFunction<uint8_t,0x004DDA60>(s->data,e->parent->mode));
 }
 return 1;
}
uint8_t LoadEntity(Entity* e,Stream* s,uint32_t) {
 uint32_t flags=0;Read(s,&flags);e->flags=flags;
 uintptr_t ignored=0;Read(s,&ignored);ReadKey(s,&e->key);
 if(!(e->flags&0x40000000)) {if(e->flags<3)e->flags=(1u<<(e->flags+1))-1;e->flags|=0x40000000;}
 uintptr_t present=0;Read(s,&present);
 if(!present) {e->parent=nullptr;return 1;}
 auto p=CallGameFunction<Parent*,0x004E3F90>(96,"Animation/SceneEntity/SceneEntityParentRecord",0);
 if(p){p->key={0,0};p->slot=0;p->enabled=0;p->mode=0;}
 e->parent=p;ReadKey(s,&p->key);
 uintptr_t obj=reinterpret_cast<uintptr_t>(ResolveParent(p));Read(s,&obj);
 if(obj)Read(s,&p->slot);else p->slot=0;
 CallGameMethod<void,0x007D7990>(s,p->opaque+4);
 Check(s,CallGameFunction<uint8_t,0x004DD4C0>(s->data,&p->enabled));
 Check(s,CallGameFunction<uint8_t,0x004DD4C0>(s->data,&p->mode));return 1;
}
uintptr_t DestroyMap(Map* s) {
 auto p=s->begin;auto end=s->end;s->vtable=Table(0x00FCB82C);
 for(;p!=end;++p) CallGameVirtual<void,2>(p->value);
 uintptr_t result=reinterpret_cast<uintptr_t>(s->begin);
 if(s->begin) result=CallGameFunction<uintptr_t,0x004E3F70>(s->begin);
 s->vtable=Table(0x0107396C);return result;
}
uint8_t SaveMap(Map* m,Stream* s,uint32_t flags) {
 Write(s,m->begin?static_cast<uintptr_t>(m->end-m->begin):0);
 auto end=m->end;
 for(auto p=m->begin;p!=end;++p){Write(s,p->key);Write(s,reinterpret_cast<uintptr_t>(p->value));if(p->value)SaveEntity(p->value,s,flags);}
 return 1;
}
Map* DeleteMap(Map* s,uint32_t flags) {DestroyMap(s);if(flags&1)Free(s);return s;}
Map* ConstructMap(Map* s) {
 s->vtable=Table(0x0107396C);std::atomic_ref<int32_t>(s->refs).exchange(0);
 s->vtable=Table(0x00FCB82C);s->begin=s->end=s->capacity=nullptr;
 s->descriptor=static_cast<const char*>(Table(0x00FCB840));
 auto p=CallGameMethod<Entry*,0x0089D5A0>(&s->begin,8,s->begin,s->end);
 s->begin=s->end=p;s->capacity=p+8;return s;
}
}
