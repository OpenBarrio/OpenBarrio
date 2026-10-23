#include "ts3/render/m07_B21.h"
#include <cstring>
namespace ts3::render::m07::b21 {
Dependencies dependencies;
namespace {
Model* Parent(Interface* s) { return reinterpret_cast<Model*>(reinterpret_cast<uint8_t*>(s)-offsetof(Model,iface)); }
void Free(void* p) { Original(0x004E3F70,dependencies.free)(p); }
void Base(void* s,uint32_t a,Thiscall<void,void> f) { CallThis(Original(a,f),s); }
void* V(void* s,unsigned slot) { return (*static_cast<void***>(s))[slot]; }
int32_t Invoke(void* s,unsigned slot) {
 auto f=slot==0?dependencies.addRef:dependencies.release;
 if(!f) f=reinterpret_cast<Thiscall<int32_t,void>>(V(s,slot));
 return CallThis(f,s);
}
void* Manager() {
 if(dependencies.injectedManager) return dependencies.manager;
#if defined(_M_IX86)
 return *reinterpret_cast<void**>(0x011D1530);
#else
 throw std::runtime_error("B21 manager needs native injection");
#endif
}
bool Queued(void* s) { void* m=Manager(); return m && CallThis(Original(0x006E3BF0,dependencies.enqueue),m,s)!=0; }
void** Table(uint32_t a,void** injected) {
 if(injected) return injected;
#if defined(_M_IX86)
 return reinterpret_cast<void**>(uintptr_t(a));
#else
 throw std::runtime_error("B21 table needs native injection");
#endif
}
}
uint8_t __fastcall Descriptor(Interface* s,void*,uint32_t key,uint32_t* out) {
 const uint32_t* p=s->words;
 if(key) { int32_t i=CallThis(Original(0x006D8080,dependencies.find),static_cast<void*>(Parent(s)),key); if(i<0) return 0; p=s->entries[i].words; }
 for(unsigned j=0;j<4;++j) out[j]=p[j];
 return 1;
}
uint8_t __fastcall Acquire(Interface* s,void*,uint32_t key,void** out) {
 if(!key) return CallThis(reinterpret_cast<Thiscall<uint8_t,Interface,void**>>(s->table[5]),s,out);
 int32_t i=CallThis(Original(0x006D8080,dependencies.find),static_cast<void*>(Parent(s)),key);
 if(i<0) return 0;
 void* p=s->entries[i].resource; *out=p; if(p) Invoke(p,0); return p!=nullptr;
}
uint8_t __fastcall Dispose(Resource* s,void*) {
 if(!Queued(s)) CallThis(reinterpret_cast<Thiscall<void,Resource>>(s->table[6]),s);
 return 1;
}
int32_t __fastcall ModelAddRef(Interface* s,void*) { return CallThis(Original(0x00723130,dependencies.retain),static_cast<void*>(Parent(s))); }
void* __fastcall ModelQuery(Interface* s,void*,uint32_t id) { return CallThis(Original(0x006D7880,dependencies.query),static_cast<void*>(Parent(s)),id); }
void* __fastcall ModelDelete(Interface* s,void*,uint32_t flags) { return DestroyModel(Parent(s),nullptr,flags); }
int32_t __fastcall ModelRelease(Interface* s,void*) { return CallThis(Original(0x005B07A0,dependencies.release),static_cast<void*>(Parent(s))); }
void* __fastcall DerivedDelete(Derived* s,void*,uint32_t flags) {
 if(s->extra) Free(s->extra);
 Base(s,0x006DAFC0,dependencies.baseDerived); if(flags&1) Free(s); return s;
}
uint8_t __fastcall Decode(Resource* s,void*,Stream* stream,void*,uint8_t swap) {
 auto* data=stream->view ? static_cast<uint32_t*>(stream->view->data) : nullptr;
 uint32_t bytes=CallThis(reinterpret_cast<Thiscall<uint32_t,Stream>>(stream->table[7]),stream);
 if(bytes<20) return 0;
 if(swap) Original(0x00739270,dependencies.swap)(data,bytes>>2);
 uint32_t count=data[3],version=data[1];
 if(count>=0x10000) return 0;
 // EA under-validates expanded elements: still count*4, although the loop reads count*12.
 if(bytes<count*4+(version<2?16u:20u)) return 0;
 Temp temp{}; const char* tag="Renderer/ModelFactory/TempElements";
 CallThis(Original(0x00C05E00,dependencies.temp),&temp,count,&tag);
 auto* dst=static_cast<uint8_t*>(temp.begin);
 auto* src=data+(version<2?4:5);
 for(uint32_t i=0;i<count;++i) {
  uint16_t offset; uint8_t type,usage,index;
  if(version>=2 && (data[4]&1)) { offset=uint16_t(src[i*3]); type=uint8_t(src[i*3+1]); usage=uint8_t(src[i*3+2]); index=uint8_t(src[i*3+2]>>16); }
  else { uint32_t v=src[i]; offset=uint16_t(v>>24); type=uint8_t(v>>16); usage=uint8_t(v); index=uint8_t(v>>8); }
  uint16_t zero=0; std::memcpy(dst+i*8,&zero,2); std::memcpy(dst+i*8+2,&offset,2);
  dst[i*8+4]=type; dst[i*8+5]=0; dst[i*8+6]=usage; dst[i*8+7]=index;
 }
 // Factory lookup precedes release of the old format. The old slot is cleared before callback.
 void* factory=Original(0x0061A510,dependencies.factory)();
 if(s->payload) { void* old=s->payload; s->payload=nullptr; Invoke(old,2); }
 uint8_t ok=CallThis(Original(0x0061A9B0,dependencies.create),factory,temp.begin,count,&s->payload);
 if(ok) CallThis(Original(0x00731CC0,dependencies.notify),static_cast<void*>(s),uint32_t(0x30));
 if(temp.begin) Free(temp.begin);
 return ok?1:0;
}
void* __fastcall DestroyModel(Model* s,void*,uint32_t flags) {
 s->table=Table(0x00FF66DC,dependencies.modelTable); s->iface.table=Table(0x00FF66B0,dependencies.interfaceTable);
 Base(&s->iface.entries,0x00C8D6B0,dependencies.vectorDestroy);
 if(s->iface.resource) Invoke(s->iface.resource,1);
 s->iface.table=Table(0x00FA1B78,dependencies.baseTable);
 Base(s,0x006CAF20,dependencies.baseModel); if(flags&1) Free(s); return s;
}
void* __fastcall VertexQuery(Resource* s,void*,uint32_t id) {
 return id==0x0229684bu || id==0x02269ed1u || id==0xeb54b802u || id==0xee3f516eu?s:nullptr;
}
void* __fastcall BufferDelete(Resource* s,void*,uint32_t flags) {
 Free(s->payload); Base(s,0x00731CF0,dependencies.baseResource); if(flags&1) Free(s); return s;
}
void* __fastcall IndexQuery(Resource* s,void*,uint32_t id) {
 return id==0x01d0e70fu || id==0x02269ed1u || id==0xeb54b802u || id==0xee3f516eu?s:nullptr;
}
uint8_t __fastcall FormatDispose(Resource* s,void*) {
 if(!Queued(s) && s->payload) { void* p=s->payload; s->payload=nullptr; Invoke(p,2); } return 1;
}
void* __fastcall FormatDelete(Resource* s,void*,uint32_t flags) {
 if(s->payload) Invoke(s->payload,2);
 Base(s,0x00731CF0,dependencies.baseResource); if(flags&1) Free(s); return s;
}
}
