#include "ts3/anim/m05_M152.h"
#include <cstring>
namespace ts3::anim::m152 {
namespace {
template<unsigned A> uint32_t Global() {
#if defined(_M_IX86)
 return *reinterpret_cast<const uint32_t*>(uintptr_t(A));
#else
 AbortNotRewritten("m152 game global"); return 0;
#endif
}
void* Interface(void* object) {
#if defined(_M_IX86)
 return CallGameVirtual<void*,3>(object,reinterpret_cast<void*>(uintptr_t(0x017F1D88)));
#else
 AbortNotRewritten("m152 interface identifier"); return nullptr;
#endif
}
__m128 Transform(__m128 p,const Matrix& m,bool reverse=false) {
 auto x=_mm_mul_ps(_mm_shuffle_ps(p,p,0),m.row[0]);
 auto y=_mm_mul_ps(_mm_shuffle_ps(p,p,85),m.row[1]);
 return _mm_add_ps(_mm_add_ps(reverse?_mm_add_ps(y,x):_mm_add_ps(x,y),
   _mm_mul_ps(_mm_shuffle_ps(p,p,170),m.row[2])),
   _mm_mul_ps(_mm_shuffle_ps(p,p,255),m.row[3]));
}
__m128 Direction(__m128 p,const Matrix& m,bool reverse=false) {
 auto x=_mm_mul_ps(_mm_shuffle_ps(p,p,0),m.row[0]);
 auto y=_mm_mul_ps(_mm_shuffle_ps(p,p,85),m.row[1]);
 return _mm_add_ps(reverse?_mm_add_ps(y,x):_mm_add_ps(x,y),_mm_mul_ps(_mm_shuffle_ps(p,p,170),m.row[2]));
}
uintptr_t Line(void* canvas,const __m128* a,const __m128* b,uint32_t c) {
 return CallGameMethod<uintptr_t,0x006C8680>(canvas,a,b,c,c);
}
}
void* Query(Lookup* self,uint32_t a,uint32_t b) {
 auto object=CallGameMethod<void*,0x006FE3B0>(self->renderer,a,b);
 return object?Interface(object):nullptr;
}
uint8_t Visible(Entity* e) {
 Entity* state=e?CallGameMethod<Entity*,0x00935770>(e,Global<0x011E9158>()):nullptr;
 // No null guard: the original immediately reads state+0x30.
 if(state->parent&&(state->parent->flags&Global<0x011E90FC>())&&Visible(state->parent)) return 1;
 if((CallGameFunction<uintptr_t,0x007D9A10>()&255)!=0) {
  auto renderer=CallGameFunction<void*,0x006FDE10>();
  if(renderer) {
   auto object=CallGameMethod<void*,0x006FE3B0>(renderer,e->idLow,e->idHigh);
   if(object) { auto p=Interface(object); if(p) return static_cast<uint8_t>(CallGameMethod<uintptr_t,0x006F4F50>(p)); }
  }
 }
 alignas(16) unsigned char bounds[32];
 auto other=e?CallGameMethod<void*,0x00935770>(e,Global<0x011E8FC4>()):nullptr;
 if(other) {
  auto b=CallGameMethod<void*,0x009594F0>(state,bounds);
  auto p=CallGameMethod<void*,0x00949DB0>(other);
  return static_cast<uint8_t>(CallGameFunction<uintptr_t,0x00980F70>(p,b));
 }
 auto b=CallGameMethod<void*,0x00959640>(state,bounds);
 return static_cast<uint8_t>(CallGameFunction<uintptr_t,0x00981070>(b));
}
void DrawSkeleton(void* object) {
 if(!object) return;
 Matrix world;
 CallGameMethod<void,0x006FAF40>(object,&world);
 auto pose=CallGameMethod<Pose*,0x006F4A50>(object);
 if(!pose||!pose->rig) return;
 auto skeleton=static_cast<unsigned char*>(pose->rig)+offsetof(Rig,skeleton);
 auto joints=CallGameMethod<unsigned char*,0x009F0DE0>(skeleton);
 auto canvas=static_cast<unsigned char*>(CallGameFunction<void*,0x006E8330>())+0x328;
 auto count=CallGameMethod<uint32_t,0x00575650>(skeleton);
 for(uint32_t i=0;i<count;++i) {
  if(!(joints[96+112*i]&0x40)) continue;
  Matrix bone,combined;
  CallGameMethod<void,0x005734E0>(pose,i,&bone);
  for(unsigned r=0;r<4;++r) combined.row[r]=Transform(bone.row[r],world,r==0);
  auto origin=combined.row[3];
  const auto scale=_mm_set1_ps(0.03f),head=_mm_set1_ps(0.05f);
  __m128 axes[3]={Direction(_mm_mul_ps(_mm_set_ps(0,0,0,1),scale),combined),
   Direction(_mm_mul_ps(_mm_set_ps(0,0,1,0),scale),combined),
   Direction(_mm_mul_ps(_mm_set_ps(0,1,0,0),scale),combined,true)};
  const uint32_t colors[3]={0x88AA0000,0x8800AA00,0x880000AA};
  // Exact packed colors from the three original line groups.
  for(unsigned a=0;a<3;++a) {
   auto end=_mm_add_ps(origin,axes[a]); Line(canvas,&origin,&end,colors[a]);
   unsigned b=a==0?1:0, c=a==2?1:2;
   auto h1=_mm_add_ps(_mm_mul_ps(head,axes[b]),origin); Line(canvas,&h1,&end,colors[a]);
   auto h2=_mm_add_ps(_mm_mul_ps(head,axes[c]),origin); Line(canvas,&h2,&end,colors[a]);
  }
 }
}
uintptr_t DrawAxes(void* canvas,const Matrix* m,const Matrix* transform,float size) {
 alignas(16) float p[4]; _mm_store_ps(p,m->row[3]);
 auto origin=_mm_set_ps(1,p[2],p[1],p[0]);
 if(transform) origin=Transform(origin,*transform,true);
 auto x=_mm_set_ps(size,0,0,size),y=_mm_set_ps(0,0,size,0),z=_mm_set_ps(0,size,0,0);
 auto a=_mm_add_ps(Direction(x,*m,true),origin);
 auto b=_mm_add_ps(Direction(y,*m),origin);
 auto c=_mm_add_ps(Direction(z,*m,true),origin);
 Line(canvas,&origin,&a,0xffff0000); Line(canvas,&origin,&b,0xff00ff00);
 return Line(canvas,&origin,&c,0xff0000ff);
}
uintptr_t ProjectText(const int32_t* rect,const __m128* clip,void* renderer,const char* text,void* font,uint32_t style,const uint32_t* color) {
 alignas(16) float p[4]; _mm_store_ps(p,*clip);
 if(!(p[2]>0.0f)) return reinterpret_cast<uintptr_t>(rect);
 auto width=static_cast<int32_t>(uint32_t(rect[2])-uint32_t(rect[0]));
 auto height=static_cast<int32_t>(uint32_t(rect[3])-uint32_t(rect[1]));
 float x=float(width)*((p[0]/p[3])*0.5f+0.5f);
 float y=(1.0f-((p[1]/p[3])*0.5f+0.5f))*float(height);
 auto ink=reinterpret_cast<uint32_t*>(static_cast<unsigned char*>(renderer)+0x224);
 *ink=0xff000000;
 CallGameMethod<uintptr_t,0x00692080>(font,text,static_cast<uint32_t>(std::strlen(text)),x+1.0f,y+1.0f,style,renderer);
 *ink=*color;
 return CallGameMethod<uintptr_t,0x00692080>(font,text,static_cast<uint32_t>(std::strlen(text)),x,y,style,renderer);
}
uintptr_t UpdateLookAt(Component* c) {
 uintptr_t r=reinterpret_cast<uintptr_t>(c->rig);
 if(!r) return r;
 r=CallGameFunction<uintptr_t,0x00936C90>(19,c->entity,c->rig);
 if(!c->lookAt&&(c->entity->flags&Global<0x011E90FC>())) {
  r=CallGameMethod<uintptr_t,0x0057B360>(c->rig);
  if(r&255) {
#if defined(_M_IX86)
   auto memory=CallGameFunction<void*,0x004E3F90>(0x34,reinterpret_cast<const char*>(uintptr_t(0x01043454)),0,0,0,0);
#else
   auto memory=CallGameFunction<void*,0x004E3F90>(0x34,"Animation/LookAtController",0,0,0,0);
#endif
   c->lookAt=memory?CallGameMethod<void*,0x00580A70>(memory):nullptr;
   r=reinterpret_cast<uintptr_t>(c->lookAt);
  }
 }
 return r;
}
#if defined(_M_IX86)
namespace hooks {
void* __fastcall Query(Lookup* s,void*,uint32_t a,uint32_t b) { return m152::Query(s,a,b); }
uintptr_t __fastcall UpdateLookAt(Component* s,void*) { return m152::UpdateLookAt(s); }
__declspec(naked) void DrawAxes() {
 __asm {
  sub esp,4
  movss [esp],xmm6
  push ecx
  push eax
  push esi
  call m152::DrawAxes
  add esp,16
  ret
 }
}
__declspec(naked) void ProjectText() {
 __asm {
  push ebp
  mov ebp,esp
  push [ebp+16]
  push [ebp+12]
  push [ebp+8]
  push esi
  push edi
  push ecx
  push eax
  call m152::ProjectText
  add esp,28
  pop ebp
  ret
 }
}
}
#endif
}
