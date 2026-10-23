#include "ts3/ui/m08_B02.h"
#include <cstring>
#include <stdexcept>
namespace ts3::ui::b02 {
namespace {
template<class T> T& Global(uintptr_t a) {
#if defined(_M_IX86)
 return *reinterpret_cast<T*>(a);
#else
 (void)a; throw std::runtime_error("m08_B02: original global unavailable on native host");
#endif
}
void Copy(float* d,const float* s) {
 // FLD/FSTP is observable for signaling NaNs and the x87 exception flags.
 for(int i=0;i<4;++i) {
#if defined(_M_IX86)
  const float* source=s+i; float* destination=d+i;
  __asm { mov eax, source }
  __asm { fld dword ptr [eax] }
  __asm { mov eax, destination }
  __asm { fstp dword ptr [eax] }
#else
  uint32_t bits; std::memcpy(&bits,s+i,4);
  if((bits&0x7f800000u)==0x7f800000u && (bits&0x007fffffu)) bits|=0x00400000u;
  std::memcpy(d+i,&bits,4);
#endif
 }
}
}
uintptr_t Select(Selector* s) {
 void* mode=s->overrideValue?s->overrideValue:s->fallback;
 uintptr_t result=reinterpret_cast<uintptr_t>(s->overrideValue);
 s->selected=nullptr;
 for(Entry* p=s->begin;p!=s->end;++p) {
  Ref* r=p->object;
  if(!s->selected && CallGameVirtual<uint8_t,4>(r,mode)) {
   CallGameVirtual<void,6>(r,&s->value);
   result=CallGameVirtual<uintptr_t,5>(r,mode); s->selected=r;
  } else result=CallGameVirtual<uintptr_t,5>(r,static_cast<void*>(nullptr));
 }
 return result;
}
Ref* SetCurrent(Ref* next) {
 auto* manager=Global<uint8_t*>(0x011CC9A4);
 auto& slot=*reinterpret_cast<Ref**>(manager+0x10);
 Ref* old=slot;
 if(next!=old) { if(next) CallGameVirtual<void,0>(next); slot=next; if(old) CallGameVirtual<void,1>(old); }
 return old;
}
Ref* Current() { return *reinterpret_cast<Ref**>(Global<uint8_t*>(0x011CC9A4)+0x10); }
uintptr_t Override(Selector* s,void* p) { s->overrideValue=p; return Select(s); }
void* Take(Transfer* s) { void* p=s->value; s->value=nullptr; return p; }
void Initialize() {
 if(!Global<void*>(0x011CC9A8)||Global<uint8_t>(0x011CC9AC)) return;
 Global<uint8_t>(0x011CC9AC)=1;
 auto copy=[](uintptr_t d,uintptr_t s) { auto* out=&Global<char16_t>(d); auto* in=&Global<char16_t>(s); do { *out++=*in; } while(*in++); };
 copy(0x011CCD70,0x00FBA2D4); copy(0x011CCDB0,0x00FBA2B0);
 Global<float>(0x011CCF70)=Global<float>(0x01045CCC); Global<uint32_t>(0x011CCF84)=0;
 void* font=CallGameFunction<void*,0x0044D900>(1);
 CallGameMethod<void,0x004662F0>(&Global<uint8_t>(0x011CC9F0),font);
 Global<uint8_t>(0x011CCD61)=1;
 CallGameMethod<void,0x0045A0B0>(&Global<uint8_t>(0x011CC9F0),&Global<char16_t>(0x011CCD70),0,0);
}
void* Singleton() { return Global<void*>(0x011CC9BC); }
void Adjust(Stack* s,uint32_t delta) {
 StackRecord* r=s->end-1; r->count+=delta;
 if(r->count==0) {
  if(r->first) CallGameVirtual<void,1>(r->first);
  r=s->end-1;
  if(r->second) CallGameVirtual<void,1>(r->second);
  --s->end;
 }
}
uintptr_t GetClip(Buffer* s,float* out) { Copy(out,s->rect); return (reinterpret_cast<uintptr_t>(out)&~uintptr_t(255))|s->clipping; }
uintptr_t SetClip(Buffer* s,uint32_t flag,const float* r) {
 s->clipping=uint8_t(flag);
 if(s->clipping) { Copy(s->rect,r); return reinterpret_cast<uintptr_t>(r); }
 return 0;
}
const float* Intersect(Buffer* s,const float* r) {
 if(!s->clipping) { s->clipping=1; Copy(s->rect,r); return r; }
 float* d=s->rect;
 if(r[0]>=d[2] || d[0]>=r[2] || r[1]>=d[3] || d[1]>=r[3]) {
  d[0]=d[1]=d[2]=d[3]=0.0f;
 } else {
  d[0]=d[0]>r[0]?d[0]:r[0]; d[1]=d[1]>r[1]?d[1]:r[1];
  d[2]=r[2]>d[2]?d[2]:r[2]; d[3]=r[3]>d[3]?d[3]:r[3];
 }
 return r;
}
uintptr_t DrawText(Buffer* s,Text* text) {
 int32_t bytes=static_cast<int32_t>(reinterpret_cast<uintptr_t>(text->end)-reinterpret_cast<uintptr_t>(text->begin));
 return CallGameVirtual<uintptr_t,12>(s,text->begin,text->style,bytes>>1);
}
Buffer* Construct(Buffer* s,void* owner) {
#if defined(_M_IX86)
 s->table=reinterpret_cast<void**>(0x00FBA310); s->label=reinterpret_cast<const char*>(0x00FBA2F8);
#else
 s->table=nullptr; s->label="UI/UTFWin/GDEBuffer";
#endif
 s->owner=owner; s->color=0xffffffff; s->renderable=nullptr; s->clipping=0;
 for(auto& p:s->words) p=nullptr;
 for(float& f:s->rect) f=0;
 return s;
}
Ref* Acquire(Pool* s) {
#if defined(_M_IX86)
 const char* lockName=reinterpret_cast<const char*>(0x00FBA244);
 const char* label=reinterpret_cast<const char*>(0x00FBA3FC);
#else
 const char* lockName=""; const char* label="UI/UTFWin/Renderable2D";
#endif
 CallGameMethod<void,0x004E16F0>(s->lock,lockName);
 uintptr_t thread=CallGameFunction<uintptr_t,0x004E1270>();
 void* list=thread==Global<uintptr_t>(0x011CC9B4)?static_cast<void*>(s->primary):static_cast<void*>(s->secondary);
 Ref* r;
 if(s->begin==s->end) {
  void* mem=CallGameFunction<void*,0x004E3F90>(0x50,label,0,0,0,0);
  r=mem?CallGameMethod<Ref*,0x00506550>(mem):nullptr;
  CallGameVirtual<void,0>(r); // EA dereferences null on allocation failure.
 } else { r=s->end[-1]; --s->end; }
 // Renderable +14 is a pointer to the chosen pool list, not a 32-bit handle.
 struct Renderable { void** table; uintptr_t pad[4]; void* list; };
 reinterpret_cast<Renderable*>(r)->list=list;
 ++s->active;
 CallGameMethod<void,0x004E17B0>(s->lock);
 return r;
}
void Border(Buffer* s,float x1,float y1,float x2,float y2,float thickness) {
 if(s->color<0x01000000) return;
 if(!s->renderable) s->renderable=Acquire(Global<Pool*>(0x011CC9B0));
 float dx=x2-x1,dy=y2-y1;
 float w=dx>thickness?thickness:dx, h=dy>thickness?thickness:dy;
 // COMISS/JAE treats unordered as false: a NaN extent is still dispatched.
 if(w<=0 || h<=0) return;
 float bottom=y1+h;
 CallGameVirtual<void,14>(s,x1,y1,x2,bottom);
 float top=y2-h;
 CallGameVirtual<void,14>(s,x1,top,x2,y2);
 CallGameVirtual<void,14>(s,x1,bottom,x1+w,top);
 CallGameVirtual<void,14>(s,x2-w,bottom,x2,top);
}
}

namespace ts3::ui::b02::hooks {
uintptr_t __fastcall Select(Selector* s,void*) { return b02::Select(s); }
uintptr_t __fastcall Override(Selector* s,void*,void* p) { return b02::Override(s,p); }
void* __fastcall Take(Transfer* s,void*) { return b02::Take(s); }
void __fastcall Adjust(Stack* s,void*,uint32_t d) { return b02::Adjust(s,d); }
uintptr_t __fastcall GetClip(Buffer* s,void*,float* p) { return b02::GetClip(s,p); }
uintptr_t __fastcall SetClip(Buffer* s,void*,uint32_t f,const float* p) { return b02::SetClip(s,f,p); }
const float* __fastcall Intersect(Buffer* s,void*,const float* p) { return b02::Intersect(s,p); }
uintptr_t __fastcall DrawText(Buffer* s,void*,Text* t) { return b02::DrawText(s,t); }
Buffer* __fastcall Construct(Buffer* s,void*,void* p) { return b02::Construct(s,p); }
Ref* __fastcall Acquire(Pool* s,void*) { return b02::Acquire(s); }
void __fastcall Border(Buffer* s,void*,float a,float b,float c,float d,float t) { return b02::Border(s,a,b,c,d,t); }
}
