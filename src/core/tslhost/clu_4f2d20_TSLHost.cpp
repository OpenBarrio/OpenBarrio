#include "ts3/tslhost/clu_4f2d20_TSLHost.h"
#include <cstring>
#include <xmmintrin.h>
namespace ts3::tslhost {
namespace {
template<unsigned Address> void* GameAddress() {
#if defined(_M_IX86)
 return reinterpret_cast<void*>(Address);
#else
 AbortNotRewritten("TSLHost game datum"); return nullptr;
#endif
}
template<unsigned Address> void* Global() { return *static_cast<void**>(GameAddress<Address>()); }
const Wide* End(const Wide* p) { const Wide* e=p; while (*e) ++e; return e; }
struct String { Wide* data; Wide* end; Wide* capacity; const char* tag; };
TS3_X86_SIZE(String,16);
void Release(String& s) {
 const auto span=static_cast<intptr_t>((reinterpret_cast<uintptr_t>(s.capacity)-reinterpret_cast<uintptr_t>(s.data)) & ~uintptr_t{1});
 if (span>2 && s.data) CallGameFunction<void,0x4e3f70>(s.data);
}
void Path(String& s, void* service, unsigned address) {
 s={nullptr,nullptr,nullptr,"TSLHost/NamedPipeServer/logPath"};
 CallGameMethod<void,0x40b700>(&s, CallGameVirtual<void*,9>(service));
 const Wide* suffix=nullptr;
#if defined(_M_IX86)
 suffix=reinterpret_cast<const Wide*>(address);
#else
 (void)address; AbortNotRewritten("TSLHost path suffix");
#endif
 // The original starts its scan at suffix+1, not suffix.
 const Wide* e=suffix+1; while(*e) ++e;
 CallGameMethod<void,0x408b90>(&s,suffix,e);
}
int SetPath(Logger* self, Wide** field, const Wide* path) {
 CallGameMethod<void,0x4e16f0>(self->mutex,GameAddress<0x1030ba0>());
 CallGameMethod<void,0x408e20>(field,path,End(path));
 if (!static_cast<uint8_t>(CallGameFunction<int,0x4dbba0>(*field)))
  CallGameFunction<void,0x4dbb20>(*field,0);
 return CallGameMethod<int,0x4e17b0>(self->mutex);
}
template<class... Args> int Read(void* reader, uintptr_t key, const Wide* name, void* type, Args... args) {
 // This vtable is unusual: slot 13 is cdecl with an explicit self argument.
#if defined(_M_IX86)
 using Fn=int(__cdecl*)(void*,uintptr_t,const Wide*,void*,Args...);
 return reinterpret_cast<Fn>((*static_cast<void***>(reader))[13])(reader,key,name,type,args...);
#else
 (void)reader;(void)key;(void)name;(void)type; ((void)args,...);
 AbortNotRewritten("TSLHost cdecl reader slot 13"); return 0;
#endif
}
}
// 0x004FD370
int VFormat(Wide* out,uint32_t capacity,const Wide* format,va_list args) {
 FormatSink sink{out,0,out?capacity:0};
 int result=CallGameFunction<int,0x4fcd80>(GameAddress<0x4fab50>(),&sink,format,args);
 if(out && result>=0) {
  if(static_cast<uint32_t>(result)<capacity) out[result]=0;
  else if(capacity) out[capacity-1]=0;
 }
 return result;
}
// 0x004F2D20
int Format(Wide* out,uint32_t capacity,const Wide* format,...) {
 va_list args; va_start(args,format);
 int result=VFormat(out,capacity,format,args); va_end(args); return result;
}
// 0x0060AE00
Logger* GetLogger() {
 auto p=static_cast<unsigned char*>(Global<0x11cec9c>());
 return p?reinterpret_cast<Logger*>(reinterpret_cast<uintptr_t>(p)-8):nullptr;
}
// 0x00898280
int LoggerApi::SetGamePath(const Wide* path) { return SetPath(this,&gamePath,path); }
// 0x008985B0
int LoggerApi::SetLogPath(const Wide* path) { return SetPath(this,&logPath,path); }
// 0x008982F0
int LoggerApi::SetFlags(int value) {
 CallGameMethod<void,0x4e16f0>(mutex,GameAddress<0x1030ba0>());
 flags=value;
 if(value) if(void* service=CallGameFunction<void*,0x72f1c0>()) {
  String s; Path(s,service,0x1030bc4); SetGamePath(s.data); Release(s);
 }
 return CallGameMethod<int,0x4e17b0>(mutex);
}
// 0x0060CB30
uint32_t HostApi::Init() {
 if(void* service=CallGameFunction<void*,0x72f1c0>()) {
  String s; Path(s,service,0xfe76f0);
  auto p=static_cast<unsigned char*>(Global<0x11cec98>());
  auto logger=reinterpret_cast<LoggerApi*>(p?reinterpret_cast<uintptr_t>(p)-8:0);
  logger->SetLogPath(s.data); Release(s);
  Wide filename[260]; filename[256]=0;
  const Wide* base=CallGameVirtual<const Wide*,9>(service);
  Format(filename,259,u"%ls%ls.ini",base,u"Sims3Launcher");
  if(static_cast<uint8_t>(CallGameFunction<int,0x4dbba0>(filename))) {
   alignas(void*) unsigned char ini[60];
   CallGameMethod<void,0x58d800>(ini,0);
   CallGameMethod<void,0x58d890>(ini,filename);
   Wide* text=nullptr;
   if(static_cast<uint8_t>(CallGameMethod<int,0x58c380>(ini,u"Sims3Launcher",u"GameLauncherLogging",&text))) {
    int value=0;
#if defined(_M_IX86)
    using Parse=long(__cdecl*)(const Wide*,Wide**,int);
    value=(*static_cast<Parse*>(GameAddress<0xf9575c>()))(text,nullptr,16);
#else
    (void)text; AbortNotRewritten("TSLHost wcstol import");
#endif
    if(value && Global<0x11cec9c>() && GetLogger()) static_cast<LoggerApi*>(GetLogger())->SetFlags(value);
   }
   CallGameMethod<void,0x58d770>(ini);
  }
 }
 if(thread) {
  struct ThreadOptions { void* first; uint32_t stack; uint32_t zero; unsigned char opaque[8]; const char* name; } options;
  TS3_X86_SIZE(ThreadOptions,24);
  CallGameMethod<void,0x4e2960>(&options); options.stack=4096; options.zero=0; options.name="NamedPipeListenerThread";
  auto allocator=CallGameFunction<void*,0x4e2980>();
  uint32_t r=CallGameMethod<uint32_t,0x4e2f30>(thread,GameAddress<0x60cb00>(),0,&options,allocator);
  if(!r) return r; // EAX is zero on the failure path, not just AL.
 }
 CallGameMethod<void,0x60c6d0>(this,0);
 uint32_t r=CallGameFunction<uint32_t,0x898620>(2,46,u"NamedPipeServer",u"Init",0,0);
 return (r&0xffffff00u)|1; // EA only overwrites AL.
}
// 0x006ACFC0
void ReadVectors(void* reader,uintptr_t key,const Wide* prefix,Vec* out,int count,uint8_t normalize) {
 for(int i=0;i<count;++i) {
  Wide name[256]; float x=0,y=0,z=0;
  Format(name,256,u"%s%d",prefix,i);
  int r=Read(reader,key,name,Global<0x1158bac>(),&x,&y,&z);
  out[i]={x,y,z,x};
  if(r==3 && normalize) {
   __m128 v=_mm_load_ps(&out[i].x), sq=_mm_mul_ps(v,v);
   __m128 sum=_mm_add_ps(_mm_add_ps(_mm_shuffle_ps(sq,sq,2),_mm_shuffle_ps(sq,sq,1)),sq);
   sum=_mm_shuffle_ps(sum,sum,0);
   __m128 inv=_mm_rsqrt_ps(sum),one=_mm_set1_ps(1.0f),half=_mm_set1_ps(0.5f);
   for(int j=0;j<2;++j) inv=_mm_add_ps(_mm_mul_ps(_mm_sub_ps(one,_mm_mul_ps(_mm_mul_ps(inv,inv),sum)),_mm_mul_ps(half,inv)),inv);
   _mm_store_ps(&out[i].x,_mm_mul_ps(inv,v));
  }
 }
}
// 0x006AD100 (zero-count return is undefined in EA; no caller consumes it).
int ScaleVectors(void* reader,uintptr_t key,const Wide* prefix,Vec* out,int count) {
 int result=0;
 for(int i=0;i<count;++i) {
  Wide name[256]; float scale=1.0f; Format(name,256,u"%s%d",prefix,i);
  result=Read(reader,key,name,Global<0x1158bb0>(),&scale);
  _mm_store_ps(&out[i].x,_mm_mul_ps(_mm_set1_ps(scale),_mm_load_ps(&out[i].x)));
 }
 return result;
}
// 0x006AF9B0
uint8_t ReadLights(uintptr_t key,uintptr_t unused,LightContext* context) {
 (void)unused; int model=0; void* reader=context->reader; LightState* state=context->state;
 Read(reader,key,u"ModelHash",Global<0x1158bb4>(),&model);
 uint32_t hash;
 if(model) hash=CallGameFunction<uint32_t,0x7cfca0>(CallGameFunction<uint32_t,0x4fa640>(key,0x811c9dc5u,1));
 else hash=CallGameFunction<uint32_t,0x7cfda0>(key);
 if(state->enabled) {
  LightRecord record; record.hash=hash;
  ReadVectors(reader,key,u"Direction",record.direction,4,true);
  ReadVectors(reader,key,u"Color",record.color,4,false);
  ReadVectors(reader,key,u"VertexDirection",record.vertexDirection,4,true);
  ReadVectors(reader,key,u"VertexColor",record.vertexColor,4,false);
  ScaleVectors(reader,key,u"Intensity",record.color,4);
  ScaleVectors(reader,key,u"VertexIntensity",record.vertexColor,4);
  if(reinterpret_cast<uintptr_t>(state->end)>=reinterpret_cast<uintptr_t>(state->capacity))
   CallGameMethod<void,0x6af090>(&state->begin,state->end,&record);
  else {
   auto target=state->end;
   state->end=reinterpret_cast<LightRecord*>(reinterpret_cast<uintptr_t>(target)+sizeof(LightRecord));
   if(target) CallGameMethod<void,0x6ad5f0>(target,&record);
  }
 }
 return 1;
}
}
