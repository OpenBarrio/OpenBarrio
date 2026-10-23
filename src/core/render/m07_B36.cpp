#include "ts3/render/m07_B36.h"
#include "ts3/renderer/clu_718cb0_Renderer.h"
#include <cstring>
#include <xmmintrin.h>
#include <intrin.h>
namespace ts3::render::m07::b36 {
Resolver resolve=nullptr;
Globals globals{};
namespace {
template<class R,uint32_t A,class... T> R M(void* s,T... a) {
 if(resolve) if(auto p=resolve(A)) return reinterpret_cast<R(__fastcall*)(void*,void*,T...)>(p)(s,nullptr,a...);
 return CallGameMethod<R,A>(s,a...);
}
template<class R,uint32_t A,class... T> R F(T... a) {
 if(resolve) if(auto p=resolve(A)) return reinterpret_cast<R(__cdecl*)(T...)>(p)(a...);
 return CallGameFunction<R,A>(a...);
}
template<class R,int S,class... T> R V(void* s,T... a) {
 return reinterpret_cast<R(__fastcall*)(void*,void*,T...)>((*static_cast<void***>(s))[S])(s,nullptr,a...);
}
void* Address(uint32_t a) {
 if(resolve) if(auto p=resolve(a)) return p;
#if defined(_M_IX86)
 return reinterpret_cast<void*>(uintptr_t(a));
#else
 throw std::runtime_error("m07_B36 native data address requires injection");
#endif
}
Globals G() {
 if(globals.parameters) return globals;
#if defined(_M_IX86)
 return {*reinterpret_cast<void**>(0x011CECC8),*reinterpret_cast<Parameters**>(0x011D7530),*reinterpret_cast<void**>(0x011D753C)};
#else
 throw std::runtime_error("m07_B36 native globals require injection");
#endif
}
void* Allocate(uint32_t n,const char* tag) { return F<void*,0x004E3F90>(n,tag,0u,0u,0u,0u); }
uintptr_t Low(uintptr_t r,uint8_t b) { return (r&~uintptr_t(255))|b; }
struct Format { uint8_t pad[0x20]; uint32_t* stride; };
struct Context {
 void* query; uint32_t count; void* argument; void* other;
 uint8_t flag,flag2,pad[6]; void* storage; uint32_t capacity; uint32_t size;
 float one,two; uint32_t zero; const char* tag;
};
struct Query { uint32_t count; uint32_t id; uint32_t unused[6]; void* value; uint32_t one; };
TS3_X86_OFFSET(Context,storage,0x18); TS3_X86_OFFSET(Context,capacity,0x1c);
}
void* __fastcall CopyBounds(void* self,void*,void* out) {
 auto s=static_cast<float*>(self); auto d=static_cast<float*>(out);
 _mm_store_ps(d,_mm_load_ps(s+0x14));
 _mm_store_ps(d+4,_mm_load_ps(s+0x18));
 return out;
}
// Twin 0054C820 (m05_B02::QueryC820): only the two interface IDs differ.
void* __fastcall QueryGenie(Genie* s,void*,uint32_t id) {
 if(id==0xee3f516eu || id==0x0c929e36u) return s;
 return nullptr;
}
// Twin 005A71D0: cached interface is +0C rather than +18; vtables/free target differ.
Genie* __fastcall DeleteGenie(Genie* s,void*,uint32_t flags) {
 s->table=static_cast<void**>(Address(0x00FF28B0));
 if(s->cached) V<void,1>(s->cached);
 s->table=static_cast<void**>(Address(0x00FA1B78));
 if(flags&1) F<void,0x004E3F70>(s);
 return s;
}
ShadowManager* __fastcall ConstructManager(ShadowManager* s,void*) {
 M<void,0x00699850>(s);
 s->table=static_cast<void**>(Address(0x00FF265C));
 s->positions={nullptr,nullptr,nullptr,static_cast<const char*>(Address(0x00FF262C))};
 return s;
}
// Twin 0055BD00 (m05_M15::DeleteOrient): vector offset, vtable and base destructor differ.
ShadowManager* __fastcall DeleteManager(ShadowManager* s,void*,uint32_t flags) {
 s->table=static_cast<void**>(Address(0x00FF265C));
 s->positions.end=s->positions.begin;
 if(s->positions.begin) F<void,0x004E3F70>(s->positions.begin);
 M<void,0x00699210>(s);
 if(flags&1) F<void,0x004E3F70>(s);
 return s;
}
uintptr_t __fastcall DescribeGenie(Genie* s,void*,void* out) {
 return F<uintptr_t,0x00567460>(out,"GenieFixupLOD (enabled = %d)",uint32_t(s->enabled));
}
void __fastcall ResizeInstances(Instancing* s,void*,uint32_t count) {
 if(count<=s->capacity) { s->size=count; return; }
 // The doubling wraps before the unsigned multiply overflow check.
 uint64_t bytes=uint64_t(uint32_t(count*2))*16;
 auto p=static_cast<uint8_t*>(Allocate(bytes>0xffffffffu?0xffffffffu:uint32_t(bytes),"Renderer/InstancingData"));
 if(!p) return;
 uint32_t copied=s->size*16;
 if(copied) std::memcpy(p,s->first,copied);
 auto second=p+uint32_t(count*16);
 if(copied) std::memcpy(second,s->second,copied);
 F<void,0x004E3F70>(s->first);
 s->second=second; s->first=p; s->capacity=count; s->size=count;
}
void __fastcall InitializeGamma(Gamma* s,void*) {
 if(s->ready) return;
 auto g=G();
 s->blit=M<uint32_t,0x0079A160>(g.parameters,"BlitData",3u,2u);
 auto p=G().parameters;
 if(s->blit<p->count) {
  auto& b=p->bindings[s->blit]; ++b.revision; b.stamp=p->stamp; b.data=s->data;
 }
 s->shader=M<void*,0x0079B840>(g.shaders,"GammaBlit",0u,0u);
 s->enable=M<uint32_t,0x0079A160>(g.parameters,"GammaEnable",1u,2u);
 s->ready=1;
}
uint8_t __fastcall CreateVertices(Shadow* s,void*) {
 if(s->buffer) return 1;
 auto p=Allocate(32,"Renderer/InteriorShadow/VertexBuffer");
 s->buffer=p?M<void*,0x0061A380>(p):nullptr;
 if(!s->buffer || !s->pool) return 0;
 auto size=600u*(*static_cast<Format*>(s->format)->stride);
 if(!M<uint8_t,0x0061A1C0>(s->buffer,size,0u,"InteriorShadow")) {
  if(s->buffer) V<void,0>(s->buffer,1u);
  s->buffer=nullptr; return 0;
 }
 auto data=M<uint8_t*,0x00619E80>(s->buffer,0u,0u,0u);
 if(data) {
  const float x[6]={-1,-1,1,-1,1,1}; const float y[6]={-1,1,1,-1,-1,1};
  for(uint32_t i=0;i<100;++i) for(unsigned j=0;j<6;++j) {
   // x87 stores convert these exact +/-1 constants without changing bits.
   std::memcpy(data,&x[j],4); std::memcpy(data+4,&y[j],4); std::memcpy(data+8,&i,4); data+=12;
  }
  M<void,0x00616C10>(s->buffer);
 }
 return 1;
}
uint8_t __fastcall InitializeShadow(Shadow* s,void*) {
 V<void,18>(s);
 auto system=F<void*,0x006E8330>();
 if(system) { M<void,0x006E84B0>(system,s,0u); s->pool=M<void*,0x006E83F0>(system,1u); }
 if(!M<uint8_t,0x006923E0>(s)) return 0;
 if(!CreateVertices(s,nullptr)) return 0;
 auto g=G();
 s->shader=M<void*,0x0079B840>(g.shaders,"ExteriorSimDropShadow",0u,0u);
 if(!s->shader) return 0;
 uint32_t key[4]{};
 if(!F<uint8_t,0x004AECE0>(key,L"GenericBlobShadow",Address(0x00B2D882),0u,0u,0u)) return 0;
 s->textureId=M<uint32_t,0x0079A160>(g.parameters,"simBlobShadowTexture",1u,5u);
 M<void,0x00722F50>(s->texture,key,0u,0u,0u);
 s->locations=M<uint32_t,0x0079A160>(g.parameters,"ShadowLocations",1u,2u);
 return 1;
}
uintptr_t __fastcall CollectPositions(ShadowManager* s,void*) {
 auto node=s->headRight; auto sentinel=reinterpret_cast<Node*>(&s->headLeft);
 s->positions.end=s->positions.begin;
 auto sys=F<void*,0x006E8330>(); auto world=M<void*,0x006E8400>(sys);
 uintptr_t result=M<uintptr_t,0x00961490>(world);
 if(!(result&255)) return result;
 result=reinterpret_cast<uintptr_t>(sentinel);
 unsigned accepted=0;
 while(node!=sentinel && accepted<100) {
  if(M<uint8_t,0x00692B80>(s,node->value)) {
   alignas(16) Vector tmp;
   auto value=M<Vector*,0x00697E10>(s,&tmp,node->value);
   if(reinterpret_cast<uintptr_t>(s->positions.end)>=reinterpret_cast<uintptr_t>(s->positions.capacity))
    M<void,0x00692C00>(&s->positions,s->positions.end,value);
   else { auto out=s->positions.end++; if(out) _mm_store_ps(reinterpret_cast<float*>(out),_mm_load_ps(reinterpret_cast<float*>(value))); }
   ++accepted;
  }
  node=F<Node*,0x004E0B20>(node); result=reinterpret_cast<uintptr_t>(node);
 }
 return result;
}
uintptr_t __fastcall FixupGenie(Genie* s,void*,void* input,void** out,void* argument,void* flags) {
 if(!s->enabled) {
  uintptr_t r;
  if(s->cached) { r=reinterpret_cast<uintptr_t>(s->cached);s->cached=nullptr;*out=reinterpret_cast<void*>(r); }
  else { *out=input;r=V<uintptr_t,0>(input); }
  return Low(r,1);
 }
 Query q; std::memset(q.unused+2,0,16);
 q.id=F<uint32_t,0x007CFE70>("IsGenie"); q.value=Address(0x011589A0); q.one=1; q.count=1;
 Context c; M<void,0x006DB3B0>(&c); c.count=1;c.query=&q;c.argument=argument;c.flag=1;
 void* value=nullptr;
 if(*(static_cast<uint8_t*>(input)+0x72)) {
  V<void,0>(input);value=input; M<void,0x006DB480>(input,input,&c,flags);
 } else if(M<uint8_t,0x006DB6E0>(input,&value,&c,flags)) {
  M<void,0x009DC510>(&s->cached,input);
 } else {
  if(value) V<void,1>(value);
  return Low(M<uintptr_t,0x00687440>(&c),0);
 }
 *out=value; V<void,0>(value); if(value) V<void,1>(value);
 auto r=M<uintptr_t,0x007D3AF0>(&c.capacity,c.storage,c.capacity); c.size=0;
 if(c.capacity>1) r=F<uintptr_t,0x004E3F70>(c.storage);
 return Low(r,1);
}
uintptr_t __fastcall CreateTargets(void* s,void*,void* registry) {
 auto p=Allocate(40,"Renderer/RenderSystem/CubeStripRT");
 auto color=p?M<void*,0x00618060>(p):nullptr;
 if(color) { V<void,1>(color); if(M<uint8_t,0x00617BB0>(color,32u,192u,61u,0u)) M<void,0x00617580>(registry,0x8fbdade5u,color); }
 auto host=G().host; auto device=M<void*,0x0060EE40>(host);
 if(M<uint8_t,0x00617A20>(device)) {
  p=Allocate(40,"Renderer/RenderSystem/CubeStripDepthRT");
  auto depth=p?M<void*,0x00618060>(p):nullptr;
  if(depth) { V<void,1>(depth); if(M<uint8_t,0x00617BB0>(depth,128u,768u,141u,0u)) M<void,0x00617580>(registry,0x04eb8d9fu,depth); V<void,2>(depth); }
 }
 M<void,0x006E1B80>(s,128u,256u,0xc0f1e869u,0x58730d63u,61u,0u);
 auto r=M<uintptr_t,0x006E1D00>(s,registry,64u,0xd336669bu);
 if(color) r=V<uintptr_t,2>(color);
 return r;
}
System* __fastcall ConstructSystem(System* s,void*) {
 s->primary=Address(0x00FE441C);s->secondary=Address(0x00FEFEC0);
 _InterlockedExchange(reinterpret_cast<volatile long*>(&s->refs),0);
 s->primary=Address(0x00FF7D7C);s->secondary=Address(0x00FF7D6C);s->ready=0;
 s->shadowPool=nullptr;s->shadowFormat=nullptr;s->dropPool=nullptr;s->dropFormat=nullptr;s->instanceVB=nullptr;
 s->zero108=0;s->invalid=0x80000000u;
 for(auto& p:s->zero110) p=nullptr;
 reinterpret_cast<ts3::renderer::Blob20*>(s->blob148)->Clear();
 reinterpret_cast<ts3::renderer::Blob20*>(s->blob160)->Clear();
 for(unsigned i:{1u,5u,6u,7u,8u}) s->flags178[i]=0;
 s->zero184[0]=s->zero184[1]=nullptr;
 M<void,0x004E17E0>(s->mutex190,0u,1u);
 for(auto& p:s->jobs) p=nullptr;
 s->jobsTag=static_cast<const char*>(Address(0x00FF7D48));
 M<void,0x00730090>(s->jobs,0u);
 M<void,0x004E17E0>(s->mutex1f0,0u,1u);
 s->condemnedNext=s->condemnedPrev=&s->condemnedNext;
 s->condemnedCount=0;s->condemnedStorage=nullptr;s->condemnedCapacity=0;
 s->condemnedTag=static_cast<const char*>(Address(0x00FF7D1C));
 s->settings={nullptr,nullptr,nullptr,static_cast<const char*>(Address(0x00FF7CF0))};s->zero248=0;
 M<void,0x00693240>(s->tail);
 return s;
}
}
