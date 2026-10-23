#include "ts3/ui/m08_B31.h"
#include <xmmintrin.h>
#include "ts3/math/m04_B04.h"
namespace ts3::ui::b31 {
namespace {
template<class T> T* Back(void* p,size_t n) {return reinterpret_cast<T*>(static_cast<uint8_t*>(p)-n);}
template<class T> T Image(uintptr_t address) {
#if defined(_M_IX86)
 return reinterpret_cast<T>(address);
#else
 (void)address; AbortNotRewritten("m08_B31 original image datum"); return T{};
#endif
}
float& F(DialogState* s,unsigned offset) {return s->geometry[(offset-0x24)/4];}
float Add(float a,float b) {return _mm_cvtss_f32(_mm_add_ss(_mm_set_ss(a),_mm_set_ss(b)));}
float Sub(float a,float b) {return _mm_cvtss_f32(_mm_sub_ss(_mm_set_ss(a),_mm_set_ss(b)));}
struct Child {void** vtable; void* next;};
Child* Owner(void* iterator) {return Back<Child>(iterator,sizeof(void*));}
}
void* __fastcall DeleteDialogState(DialogState* s,void*,uint32_t f) {
 return CallGameMethod<void*,0x00521dc0>(Back<Dialog>(s,offsetof(Dialog,state)),f);
}
void* __fastcall DeleteDialogSecondary(void* s,void*,uint32_t f) {
 return CallGameMethod<void*,0x00521dc0>(Back<Dialog>(s,offsetof(Dialog,secondary)),f);
}
void* __fastcall DeleteDrawableSecondary(void* s,void*,uint32_t f) {
 return CallGameMethod<void*,0x00522190>(Back<Drawable>(s,offsetof(Drawable,secondary)),f);
}
void* __fastcall DeleteDrawableTertiary(void* s,void*,uint32_t f) {
 return CallGameMethod<void*,0x00522190>(Back<Drawable>(s,offsetof(Drawable,tertiary)),f);
}
void* __fastcall QueryDrawableSecondary(void* s,void*,uint32_t id) {
 return CallGameMethod<void*,0x0051f880>(Back<Drawable>(s,offsetof(Drawable,secondary)),id);
}
uint32_t __fastcall ForwardTertiary(void* s,void*) {
 return CallGameMethod<uint32_t,0x0053d0d0>(Back<uint8_t>(s,3*sizeof(void*)));
}
void* __fastcall QueryDrawableTertiary(void* s,void*,uint32_t id) {
 return CallGameMethod<void*,0x0051f880>(Back<Drawable>(s,offsetof(Drawable,tertiary)),id);
}
Dialog* __fastcall ConstructDialog(Dialog* s,void*) {
 CallGameMethod<void,0x00515c80>(s);
 s->vtable=Image<void**>(0x00fbc380); s->secondary=Image<void**>(0x00fbc258);
 auto& d=s->state; d.vtable=Image<void**>(0x00fbc1e8);
 d.flags=0x3f; d.zero[0]=d.zero[1]=0; d.active=0; d.selection=-1;
 for(auto& p:d.resources) p=nullptr;
 for(unsigned off=0xd4;off<=0x110;off+=4) {
 float value=3.0f;
 if(off==0xe8||off==0xec||off==0xf0||off==0x10c||off==0x110) value=0;
 if(off==0x104||off==0x108) value=10000.0f;
 F(&d,off)=value;
 }
 d.index=-1; d.resource=nullptr; d.tag=Image<const char*>(0x00fbc4bc);
 d.begin=d.end=Image<char16_t*>(0x011c7e54);d.capacity=Image<char16_t*>(0x011c7e56);
 if(!d.active) {d.active=1;CallGameVirtual<void,29>(&s->secondary,8,1);}
 CallGameMethod<void,0x005147d0>(&s->secondary,4,1);
 return s;
}
uintptr_t __fastcall LayoutDialog(DialogState* s,void*) {
 auto base=Back<Dialog>(s,offsetof(Dialog,state));auto secondary=&base->secondary;
 CallGameVirtual<void,40>(base);
 float bounds[4]={0,0,0,0};void* it;void* end;
 CallGameVirtual<void,52>(secondary,&it);
 auto sentinel=CallGameVirtual<void**,53>(secondary,&end);
 while(it!=*sentinel) {
 auto child=Owner(it);
 if(CallGameVirtual<uint32_t,7>(child)!=0x4f0dd487 && CallGameVirtual<uint32_t,7>(child)!=0xef3d61c5) {
 if(Sub(bounds[2],bounds[0])==0 && Sub(bounds[3],bounds[1])==0) {
 auto r=CallGameVirtual<const float*,14>(child);for(int i=0;i<4;++i)bounds[i]=r[i];
 } else {
 auto r=CallGameVirtual<const float*,14>(child);
 ts3::math::RectUnion(bounds,r,bounds,0);
 }
 }
 it=*static_cast<void**>(it);sentinel=CallGameVirtual<void**,53>(secondary,&end);
 }
 float width=Add(Add(Sub(bounds[2],bounds[0]),F(s,0xdc)),F(s,0xd4));
 float height=Add(Add(Sub(bounds[3],bounds[1]),F(s,0xe0)),F(s,0xd8));
 if(s->flags&0x20) {
 width=Add(Add(Sub(F(s,0x5c),F(s,0x54)),Sub(F(s,0x4c),F(s,0x44))),width);
 height=Add(Add(Sub(F(s,0x80),F(s,0x78)),Sub(F(s,0x70),F(s,0x68))),height);
 }
 if(s->flags&8) height=Add(Sub(F(s,0x40),F(s,0x38)),height);
 float x=F(s,0xd4),y=F(s,0xd8);
 if(s->flags&0x20) {x=Add(Sub(F(s,0x4c),F(s,0x44)),x);y=Add(Sub(F(s,0x70),F(s,0x68)),y);}
 if(s->flags&8)y=Add(Sub(F(s,0x40),F(s,0x38)),y);
 x=Sub(x,bounds[0]);y=Sub(y,bounds[1]);
 CallGameVirtual<void,52>(secondary,&it);sentinel=CallGameVirtual<void**,53>(secondary,&end);
 while(it!=*sentinel) {
 auto child=Owner(it);
 if(CallGameVirtual<uint32_t,7>(child)!=0x4f0dd487) {
 auto r=CallGameVirtual<const float*,14>(child);
 float moved[4];moved[0]=Add(r[0],x);moved[2]=Add(Sub(r[2],r[0]),moved[0]);
 moved[1]=Add(r[1],y);moved[3]=Add(Sub(r[3],r[1]),moved[1]);
 CallGameVirtual<void,25>(child,moved);
 }
 it=*static_cast<void**>(it);sentinel=CallGameVirtual<void**,53>(secondary,&end);
 }
 return CallGameVirtual<uintptr_t,27>(secondary,width,height);
}
Drawable* __fastcall ConstructDrawable(Drawable* s,void*) {
 s->vtable=Image<void**>(0x00fbc49c);s->secondary=Image<void**>(0x00fbc484);
 s->refs=0;s->tertiary=Image<void**>(0x00fbc45c);s->color=0xff7f7f7f;s->value=0;
 for(auto& p:s->resources)p=nullptr;
 return s;
}
void* __stdcall CreateDialog(uint32_t,void* allocator) {
 auto p=CallGameFunction<Dialog*,0x00504e70>(0x380,0x10,Image<const char*>(0x00fbc4d0),allocator);
 if(!p)return nullptr;
 auto s=ConstructDialog(p,nullptr);
 return s?&s->secondary:nullptr;
}
Drawable* __stdcall CreateDrawable(uint32_t,void* allocator) {
 auto p=CallGameFunction<Drawable*,0x00504e70>(0x40,4,Image<const char*>(0x00fbc4f8),allocator);
 if(!p)return nullptr;
 return ConstructDrawable(p,nullptr);
}
uint8_t __fastcall ForwardValue(ForwardView* s,void*,void* target,void* argument) {
 if(!s->value)return 0;
 CallGameVirtual<uintptr_t,22>(target,argument,s->value);
 return 1;
}
uint32_t __fastcall ControlType(void*,void*) {return 0x4f8772aa;}
uint32_t __fastcall RegisterControl(void* s,void*,Record* r) {
 r->owner=Back<void>(s,sizeof(void*));r->descriptor=Image<const void*>(0x00fbc540);r->flags=1;
 auto result=CallGameVirtual<uint32_t,5>(s);r->kind=result;return result;
}
}
