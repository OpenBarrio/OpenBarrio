#include "ts3/ui/m08_B11.h"
#include <atomic>
#include <cmath>
#include <xmmintrin.h>
#include <intrin.h>
namespace ts3::ui::b11 {
uint8_t __fastcall Ancestor(View* s,void*,View* p) {
 if (!p) return 0;
 p=CallGameVirtual<View*,4>(p);
 while(p) { if(p==(reinterpret_cast<uintptr_t>(s)==sizeof(void*)?nullptr:s)) return 1; p=CallGameVirtual<View*,4>(p); }
 return 0;
}
uint32_t __fastcall Focus(View* s,void*,uint32_t c) { return s->manager && CallGameVirtual<View*,21>(s->manager,c)==s; }
uint32_t __fastcall Capture(View* s,void*,uint32_t c) { return s->manager && CallGameVirtual<View*,24>(s->manager,c)==s; }
uintptr_t __fastcall Forward(View* s,void*) { return CallGameVirtual<uintptr_t,36>(s->target); }
uint8_t __fastcall AncestorFlags(View* s,void*,uint32_t b) {
 const uint32_t mask=static_cast<uint8_t>(b)?1:3;
 for(auto p=s->manager;p;p=p->manager) {
 auto secondary=reinterpret_cast<unsigned char*>(p)+sizeof(void*);
 if(!(CallGameVirtual<uint32_t,10>(secondary)&mask)) return 0;
 } return 1;
}
static uint32_t Dec(int32_t* p) {
 return static_cast<uint32_t>(_InterlockedExchangeAdd(reinterpret_cast<volatile long*>(p),-1))-1;
}
uint32_t __fastcall Release(View* s,void*) {
 auto n=Dec(&s->refs); if(n) return n;
 _InterlockedExchange(reinterpret_cast<volatile long*>(&s->refs),1);
 CallGameVirtual<void,7>(s); n=Dec(&s->refs);
 if(!n) CallGameVirtual<void,2>(s,1);
 return n;
}
void* __fastcall Data(View* s,void*) { return s->data; }
void __fastcall SetValue(View* s,void*,uint32_t v) {
 auto old=s->value; if(old==v) return;
 s->value=v; uint32_t message[7]; message[2]=0x12; message[3]=v; message[4]=old;
 CallGameVirtual<void,70>(s,message);
 if(s->active) CallGameVirtual<void,36>(s);
}
void __fastcall SetOrder(View* s,void*,uint32_t v) {
 if(s->order==v) return; auto m=s->manager; s->order=v;
 if(m) { auto base=reinterpret_cast<unsigned char*>(s)-sizeof(void*);
 CallGameMethod<void,0x0050b400>(m,base); CallGameMethod<void,0x0050aef0>(m,base); }
}
void __fastcall SetResource(View* s,void*,void* v) {
 auto old=s->resource; if(old==v) return;
 if(v) CallGameVirtual<void,0>(v); s->resource=v;
 if(old) CallGameVirtual<void,1>(old);
}
void __fastcall SetRect(View* s,void*,const float* p) {
 float first=p[0]; bool changed=false;
 for(int i=0;i<4;++i) if(s->rect[i]!=p[i]) { changed=true; break; }
 if(!changed) return;
 s->rect[0]=first;
 for(int i=1;i<4;++i) {
#if defined(_M_IX86)
 float* dst=&s->rect[i]; const float* src=p+i;
 __asm { mov eax,src
 fld dword ptr[eax]
 mov eax,dst
 fstp dword ptr[eax] }
#else
 s->rect[i]=p[i];
#endif
 }
 CallGameVirtual<void,37>(s);
}
static float Add(float a,float b) {return _mm_cvtss_f32(_mm_add_ss(_mm_set_ss(a),_mm_set_ss(b)));}
static float Sub(float a,float b) {return _mm_cvtss_f32(_mm_sub_ss(_mm_set_ss(a),_mm_set_ss(b)));}
void __fastcall Move(View* s,void*,float x,float y) { float r[]={x,y,Add(Sub(s->rect[2],s->rect[0]),x),Add(Sub(s->rect[3],s->rect[1]),y)}; CallGameVirtual<void,25>(s,r); }
void __fastcall Resize(View* s,void*,float x,float y) { float r[]={s->rect[0],s->rect[1],Add(s->rect[0],x),Add(s->rect[1],y)}; CallGameVirtual<void,25>(s,r); }
}
