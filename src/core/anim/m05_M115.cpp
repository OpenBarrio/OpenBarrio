#include "ts3/anim/m05_M115.h"
#include <cstring>
#include <atomic>
namespace ts3::anim::m115 {
namespace {
void* Table(uintptr_t address) {
#if defined(_M_IX86)
 return reinterpret_cast<void*>(address);
#else
 (void)address; AbortNotRewritten("M115 original vtable"); return nullptr;
#endif
}
Builder* Construct(Builder* p,uintptr_t base,uintptr_t derived) {
 p->vtable=Table(0x107396c);
 reinterpret_cast<std::atomic<int32_t>*>(&p->refs)->exchange(0);
 p->vtable=Table(base);
 CallGameMethod<void,0x83e190>(p->state);
 p->value=0.0f;
 p->vtable=Table(derived);
 return p;
}
String Empty(uintptr_t descriptor) { auto b=static_cast<char*>(Table(0x11c7e54)); return {b,b,b+1,static_cast<const char*>(Table(descriptor))}; }
void Assign(String& s,const char* b,const char* e) { CallGameMethod<void,0x405a50>(&s,b,e); }
void Append(String& s,const char* b,const char* e) { CallGameMethod<void,0x405790>(&s,b,e); }
void Free(String& s) { if(s.capacity-s.begin>1 && s.begin) CallGameFunction<void,0x4e3f70>(s.begin); }
const char* Suffix(uintptr_t table,uint32_t i) {
#if defined(_M_IX86)
 return reinterpret_cast<const char* const*>(table)[i];
#else
 (void)table;(void)i;AbortNotRewritten("M115 suffix table"); return nullptr;
#endif
}
}
Builder* ConstructF5B0(Builder* p) { return Construct(p,0x10232a4,0x1023434); }
Builder* ConstructF610(Builder* p) { return Construct(p,0x10232d4,0x1023464); }
Builder* ConstructF660(Builder* p) { return Construct(p,0x10232a4,0x1023494); }
Builder* ConstructF6A0(Builder* p) { return Construct(p,0x1023274,0x10234c4); }
Builder* Delete(Builder* p,uint32_t flags) {
 CallGameMethod<void,0x83d7c0>(p->state);
 p->vtable=Table(0x107396c);
 if(flags&1) CallGameFunction<void,0x4e3f70>(p);
 return p;
}
void __stdcall Register(Registry* p,const char* name,uint32_t a,uint32_t b,uint32_t c,uint32_t d) {
 if(!p->enabled) return;
 String prefix=Empty(0x102308c), temporary=Empty(0x102308c);
 auto converted=CallGameFunction<String*,0x4f38c0>(&temporary,name,-1,temporary.descriptor);
 if(converted!=&prefix) Assign(prefix,converted->begin,converted->end);
 Free(temporary);
 temporary=Empty(0x102308c); Assign(temporary,prefix.begin,prefix.end);
 Key key{{a,b,c,d}};
 auto target=CallGameMethod<String*,0x83df60>(p,&key);
 if(target!=&temporary) Assign(*target,temporary.begin,temporary.end);
 String combined=Empty(0x1023520);
 for(uint32_t weather=0;weather<3;++weather) {
  for(uint32_t index=0,style=0;style<p->count;style=uint16_t(++index)) {
   if(combined.begin!=combined.end) { *combined.begin=0; combined.end=combined.begin; }
   Append(combined,prefix.begin,prefix.end);
   auto w=Suffix(0x1166204,weather); Append(combined,w,w+std::strlen(w));
   auto s=Suffix(0x1166210,style); Append(combined,s,s+std::strlen(s));
   Record r; r.key=key;r.weather=uint16_t(1u<<weather);r.style=uint16_t(1u<<(style&31));
   r.name=Empty(0x10230ac);Assign(r.name,combined.begin,combined.end);r.flag=0;
   if(p->end<p->capacity) {
    auto out=p->end++;
    if(out) {
     out->key=r.key;out->weather=r.weather;out->style=r.style;
     auto length=r.name.end-r.name.begin;
     out->name.descriptor=r.name.descriptor;
     if(length+1>1) {
      out->name.begin=CallGameFunction<char*,0x4e3f90>(size_t(length+1),r.name.descriptor,0,0,static_cast<const char*>(Table(0xf98980)),0xe7);
      out->name.end=out->name.begin;out->name.capacity=out->name.begin+length+1;
     } else out->name=Empty(0x10230ac);
     std::memmove(out->name.begin,r.name.begin,length);out->name.end=out->name.begin+length;*out->name.end=0;out->flag=0;
    }
   } else CallGameMethod<void,0x83eb00>(&p->begin,p->end,&r);
   Free(r.name);
  }
 }
 Free(combined);Free(temporary);Free(prefix);
}
}
