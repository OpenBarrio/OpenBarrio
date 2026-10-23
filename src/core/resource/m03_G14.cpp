#include "ts3/resource/m03_G14.h"
#include "ts3/resource/m03_A09.h"
#include "ts3/resource/m03_D05.h"
#include "ts3/resource/m03_G02.h"
#include <atomic>
namespace ts3::resource::m03::g14 {
namespace {
template<class T> T* Image(uintptr_t address) {
#if defined(_M_IX86)
 return reinterpret_cast<T*>(address);
#else
 (void)address; AbortNotRewritten("G14 image datum"); return nullptr;
#endif
}
void* RefView(void* p) { return static_cast<uint8_t*>(p)+sizeof(void*); }
void Release(void*& p) { if(p) { auto* old=p; p=nullptr; CallGameVirtual<void,2>(old); } }
uint8_t Children(Loader* s,Load* l) {
#if defined(_M_IX86)
 return a09::Children(reinterpret_cast<a09::Loader*>(s),reinterpret_cast<a09::LoadInfo*>(l));
#else
 (void)s; (void)l; AbortNotRewritten("G14 child scheduling native layout adapter"); return 0;
#endif
}
struct Stream { void* vtable; void* backing; };
void* Buffer(Stream* s) { return s->backing?static_cast<void**>(s->backing)[2]:nullptr; }
struct Lock {
 void* mutex;
 explicit Lock(void* m):mutex(m) { CallGameMethod<void,0x004E16F0>(m,Image<char>(0x00FB217C)); }
 ~Lock() { CallGameMethod<void,0x004E17B0>(mutex); }
};
}
Load* __fastcall DeleteLoad(Load* self,void*,uint32_t flags) {
 a06::Destroy(&self->context);
 CallGameMethod<void,0x00401040>(&self->request);
 if(self->factory) CallGameVirtual<void,1>(RefView(self->factory));
 if(self->resource) CallGameVirtual<void,2>(self->resource);
 if(self->record) CallGameVirtual<void,2>(self->record);
 self->vtable=Image<void>(0x00FA1B78);
 if(flags&1) CallGameFunction<void,0x004E3F70>(self);
 return self;
}
void* __fastcall DeleteMonitor(void* self,void*,uint32_t flags) {
 // Adapted from verified ResourceManDestroy (004A6030), with this destructor/free family.
 CallGameMethod<void,0x00733880>(self);
 if(flags&1) CallGameFunction<void,0x004E3F70>(self);
 return self;
}
uint8_t __fastcall Complete(Loader* self,void*,Completion* completion,uint32_t status) {
 auto* l=completion->load;
 if(status!=4) {
  if(l && l->record) { CallGameVirtual<void,8>(l->record); Release(l->record); }
  return 1;
 }
 std::atomic_ref<int32_t>(self->active).fetch_sub(1);
 std::atomic_ref<int32_t>(self->bytes).fetch_sub(static_cast<int32_t>(l->size));
 auto* manager=CallGameFunction<void*,0x004AFD20>();
 if(l->record) return Children(self,l);
 uint32_t info[4];
 if(!CallGameVirtual<uint8_t,13>(l->factory,l->key,&l->record,1,6,2,info)) { l->failed=1; return 0; }
 l->flag95=0; l->flag96=0;
 if(!info[3]) {
  auto* stream=CallGameVirtual<void*,6>(l->record);
  if(l->forceCopy || CallGameVirtual<uint32_t,3>(stream)!=0x347223d2u) {
   if(!CallGameFunction<uint8_t,0x00727540>(&l->record)) { l->failed=1; return 0; }
   l->flag96=1;
  }
  { Lock lock(self->mutex); CallGameMethod<void,0x008825E0>(&l->resource,&l->record); Release(l->record); }
  return Children(self,l);
 }
 if(!CallGameFunction<uint8_t,0x00727540>(&l->record)) { l->failed=1; return 0; }
 auto* input=CallGameVirtual<Stream*,6>(l->record);
 void* source=Buffer(input); auto sourceSize=CallGameVirtual<uint32_t,7>(input);
 auto* allocation=CallGameFunction<void*,0x004E3F90>(44,Image<char>(0x00FFD308),0,0,0,0);
 if(!allocation) { l->failed=1; return 0; }
 auto* output=CallGameMethod<Stream*,0x004DF0C0>(allocation,0,0,Image<char>(0x00FFD308));
 if(!output) { l->failed=1; return 0; }
 CallGameMethod<void,0x004DEAC0>(output,1,1.0f);
 if(!CallGameVirtual<uint8_t,8>(output,info[2])) { CallGameVirtual<void,0>(output,1); l->failed=1; return 0; }
 CallGameMethod<void,0x004DEAC0>(output,1,0.0f);
 CallGameVirtual<void,1>(output);
 auto* storage=CallGameFunction<g02::Record*,0x004E41F0>(40,Image<char>(0x00FFD2E0),0,0,0,0);
 g02::Record* record=nullptr;
 if(storage) {
  auto* key=CallGameVirtual<const Key*,4>(l->record);
  record=g02::Construct(storage,nullptr,nullptr,output,key,1,1);
 }
 struct Owner { void* p; } owner{};
 CallGameMethod<void,0x005C9100>(&owner,record);
 if(!owner.p) { CallGameVirtual<void,0>(output,1); l->failed=1; return 0; }
 auto* dest=Buffer(output); auto destSize=CallGameVirtual<uint32_t,7>(output);
 struct Codec { void* vtable; void* allocator; } codec{};
 CallGameMethod<void,0x004EBFD0>(&codec,CallGameFunction<void*,0x00931020>());
 void* decoder=nullptr;
 if(info[3]==0xffff) {
  auto type=CallGameVirtual<uint32_t,3>(l->factory);
  if(type==0x2a43bd44 || type==0x03c4b9a4 || type==0x03db0260 || type==0x05b5d180) decoder=&codec;
 }
 if(!decoder) decoder=CallGameVirtual<void*,27>(manager,info[3]);
 const bool ok=decoder && CallGameVirtual<uint32_t,2>(decoder,source,sourceSize,dest,destSize,1)==destSize;
 if(ok) {
  CallGameVirtual<void,8>(l->record); Release(l->record);
  { Lock lock(self->mutex); CallGameMethod<void,0x007C8D00>(&l->resource,owner.p); l->flag96=1; }
 } else l->failed=1;
 codec.vtable=Image<void>(0x01055008);
 CallGameVirtual<void,2>(owner.p);
 return ok?Children(self,l):0;
}
Handle* __fastcall Open(Driver* self,void*,const char* path,uint32_t mode,void** out) {
 auto* manager=CallGameFunction<void*,0x004AFD20>();
 d05::Key key{},resolved;
 auto* pathRoot=static_cast<uint8_t*>(self->path)+sizeof(void*);
 if(!CallGameFunction<uint8_t,0x0072DF80>(path,pathRoot,&key,manager)) return nullptr;
 d05::Resolve(&resolved,&key);
 auto* factory=CallGameVirtual<void*,17>(manager,&resolved);
 if(!factory) return nullptr;
 auto* refs=RefView(factory); CallGameVirtual<void,0>(refs);
 int access,share,create; CallGameFunction<void,0x0072DFF0>(mode,&access,&share,&create);
 uint32_t info[4];
 if(!CallGameVirtual<uint8_t,13>(factory,&resolved,nullptr,access,share,create,info)) { CallGameVirtual<void,1>(refs); return nullptr; }
 auto* database=CallGameVirtual<void*,3>(refs,0x03c4b995u);
 if(database) { CallGameVirtual<void,0>(RefView(database)); *out=CallGameMethod<void*,0x0072CAA0>(database,&resolved); }
 bool fallback=false;
 if(!*out || info[3]) {
  auto n=CallGameFunction<uint32_t,0x004FE0D0>(pathRoot);
  // EA fixed_string is borrowed inline storage here; it is not destroyed on this path.
  alignas(void*) uint8_t text[0x104]{};
  CallGameMethod<void,0x0072E4E0>(text,self->prefix);
  CallGameMethod<void,0x0072E4B0>(text,path+n);
  a07::Request request;
  CallGameMethod<void,0x00401000>(&request);
  CallGameMethod<void,0x00401500>(&request,*reinterpret_cast<char**>(text),mode,0,0,0);
  *out=CallGameMethod<void*,0x004012C0>(&request);
  fallback=*out!=nullptr;
  CallGameMethod<void,0x00401040>(&request);
  if(!fallback) {
   if(database) CallGameVirtual<void,1>(RefView(database));
   CallGameVirtual<void,1>(refs); return nullptr;
  }
 }
 CallGameMethod<void,0x004E16F0>(self->mutex,Image<char>(0x00FFD9A0));
 auto* result=CallGameMethod<Handle*,0x004E8530>(self->pool);
 CallGameMethod<void,0x004E17B0>(self->mutex);
 if(result) {
  result->size=info[2]; result->offset=fallback?0:info[0];
  result->file=fallback?*out:nullptr; result->database=fallback?nullptr:database;
  if(result->database) { CallGameVirtual<void,20>(result->database); database=nullptr; }
  else if(!fallback) database=nullptr;
 }
 if(database) CallGameVirtual<void,1>(RefView(database));
 CallGameVirtual<void,1>(refs);
 return result;
}
}
