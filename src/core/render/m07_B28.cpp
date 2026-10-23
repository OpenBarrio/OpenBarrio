#include "ts3/render/m07_B28.h"
#include <cstring>
#include <intrin.h>
#include <xmmintrin.h>
namespace ts3::render::m07::b28 {
Dependencies dependencies;
namespace {
template<class T> T Data(uint32_t address,T native) {
#if defined(_M_IX86)
 return reinterpret_cast<T>(uintptr_t(address));
#else
 (void)address; return native;
#endif
}
void** Table(uint32_t a) { static void* native[34]{}; return Data(a,native); }
template<class R,class S,class... A> R Virtual(S* s,unsigned slot,A... a) {
 auto f=reinterpret_cast<Thiscall<R,S,A...>>((*reinterpret_cast<void***>(s))[slot]);
 return CallThis(f,s,a...);
}
uintptr_t Free(void* p) {return Original(0x004E3F70,dependencies.free)(p);}
void Identity(float* m) {for(int i=0;i<16;++i) m[i]=i%5==0?1.0f:0.0f;}
void Multiply(float* out,const float* a,const float* b) {
 __m128 rows[4]; for(int i=0;i<4;++i) rows[i]=_mm_loadu_ps(b+i*4);
 for(int i=0;i<4;++i) {
  __m128 v=_mm_loadu_ps(a+i*4);
  __m128 r=_mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v,v,0),rows[0]),_mm_mul_ps(_mm_shuffle_ps(v,v,85),rows[1]));
  r=_mm_add_ps(r,_mm_mul_ps(_mm_shuffle_ps(v,v,170),rows[2]));
  r=_mm_add_ps(r,_mm_mul_ps(_mm_shuffle_ps(v,v,255),rows[3])); _mm_storeu_ps(out+i*4,r);
 }
}
float Mul(float a,float b) {return _mm_cvtss_f32(_mm_mul_ss(_mm_set_ss(a),_mm_set_ss(b)));}
}
uintptr_t __fastcall Bind(BindOwner* s,void*,void* value,uint32_t key) {
 if(!value) return Virtual<uintptr_t>(s,33,key);
 Node* n; auto* head=&s->list;
 Original(0x006F8780,dependencies.find)(&n,head->next,head,key);
 Node* selected=n; // EA overwrites the stack key with the returned iterator.
 if(n==head) {CallThis(Original(0x006F92B0,dependencies.append),head);n=head->prev;}
 if(n->value!=value) {
  void* old=n->value; Virtual<void>(value,0);n->value=value;
  if(old) Virtual<void>(old,1);
 }
 return Original(0x006F97C0,dependencies.finish)(head->next,head,selected,selected);
}
Objects* __fastcall ConstructObjects(Objects* s,void*) {
 s->table=Table(0x00FF39F8);_InterlockedExchange(reinterpret_cast<volatile long*>(&s->refs),0);
 s->table=Table(0x00FF9C94);s->owner=nullptr;s->children.next=s->children.prev=&s->children;
 s->begin=s->end=s->capacity=nullptr;s->tag=Data(0x00FF9C78,"Renderer/SceneNode/Objects");
 s->selector=9;s->d34=s->d38=s->d48=s->d4c=0;s->resource=nullptr;
 if(auto* m=Original(0x006FDE10,dependencies.manager)()) s->handle=CallThis(Original(0x00701960,dependencies.insert),m,s);
 return s;
}
uintptr_t __fastcall DestroyObjects(Objects* s,void*) {
 s->table=Table(0x00FF9C94);
 if(auto* m=Original(0x006FDE10,dependencies.manager)()) CallThis(Original(0x006FE3F0,dependencies.erase),m,s->handle);
 if(s->resource && --s->resource->refs==0) {s->resource->refs=1;Virtual<void>(s->resource,0,1u);}
 uintptr_t r=s->begin?Free(s->begin):0;s->table=Table(0x00FF39F8);return r;
}
uintptr_t __fastcall VisitObjects(Objects* s,void*) {
 uintptr_t result=0; // No defined EAX contract on the null-owner path.
 if(s->owner) {
  CallThis(Original(0x006E40B0,dependencies.flush),s->owner,static_cast<void*>(&s->begin));
  result=reinterpret_cast<uintptr_t>(&s->children);
  for(auto* n=s->children.next;n!=&s->children;) {
   auto* child=n?reinterpret_cast<uint8_t*>(n)-2*sizeof(void*):nullptr;
   result=Virtual<uintptr_t>(child,4); n=reinterpret_cast<Link*>(*reinterpret_cast<void**>(child+2*sizeof(void*)));
  }
 }
 return result;
}
Objects* __fastcall DeleteObjects(Objects* s,void*,uint32_t flags) {
 DestroyObjects(s,nullptr);if(flags&1) Free(s);return s;
}
void* __fastcall Query(SceneObject* s,void*,uint32_t id) {
 // Donor 004AF100 differs only in this second interface ID.
 return id==0xEE3F516Eu || id==0x30648997u?s:nullptr;
}
uint32_t __fastcall SetPair(SceneObject* s,void*,uint32_t a,uint32_t b) {
 if(s->a!=a || s->b!=b) {s->flags|=1;s->a=a;s->b=b;}return a;
}
uint32_t __fastcall SetThird(SceneObject* s,void*,uint32_t c) {
 if(s->c!=c) {s->flags|=1;s->c=c;}return c;
}
uintptr_t __fastcall Rebase(SceneObject* s,void*,const float* input) {
 alignas(16) float inverse[16],tmp[16],out[16];alignas(16) uint8_t scratch[16];
 Original(0x00561E60,dependencies.inverse)(inverse,s->transform,scratch);
 Multiply(tmp,input,inverse);Multiply(out,tmp,s->matrix);std::memcpy(s->matrix,out,64);
 // Keep EA's sequential row copies when input aliases the receiver.
 for(int i=0;i<4;++i) {__m128 row=_mm_loadu_ps(input+4*i);_mm_storeu_ps(s->transform+4*i,row);}
 return CallThis(Original(0x006FC9E0,dependencies.notify),s);
}
uintptr_t __fastcall Opacity(SceneObject* s,void*) {
 bool forced=(s->flags&0x20)!=0;
 float old=forced?1.0f:Mul(s->inherited,Mul(s->color[3],s->scale));
 s->inherited=1.0f;
 for(auto* p=s->parent;p && (p->flags&0x10);p=p->parent) s->inherited=Mul(s->inherited,p->color[3]);
 float now=forced?1.0f:Mul(s->inherited,Mul(s->color[3],s->scale));
 return CallThis(Original(0x006FCAB0,dependencies.opacity),s,old,now);
}
uintptr_t __fastcall Update(SceneObject* s,void*) {
 alignas(16) float m[16];CallThis(Original(0x006FCBD0,dependencies.update),s);
 void* value=CallThis(Original(0x006FAF40,dependencies.getMatrix),s,static_cast<void*>(m));
 uintptr_t r=CallThis(Original(0x006FCD80,dependencies.setBounds),static_cast<void*>(&s->d40),value);
 if(s->children.prev!=&s->children) for(auto* n=s->children.next;n!=&s->children;n=n->next) {
  auto* child=n?reinterpret_cast<SceneObject*>(reinterpret_cast<uint8_t*>(n)-offsetof(SceneObject,link20)):nullptr;
  r=Virtual<uintptr_t>(child,31);
 }
 return r;
}
SceneObject* __fastcall ConstructScene(SceneObject* s,void*,void* context,void* owner) {
 s->table=Table(0x00FF5EE8);_InterlockedExchange(reinterpret_cast<volatile long*>(&s->refs),0);
 s->table=Table(0x00FF9D00);s->p30=s->p34=s->p38=nullptr;s->identity.Init();
 s->children.next=s->children.prev=&s->children;
 s->alpha=s->scale=s->inherited=1.0f;s->registration=nullptr;s->d1f0=s->d1f4=0;
 s->rigsBegin=s->rigsEnd=s->rigsCapacity=nullptr;s->rigsTag=Data(0x00FF9CDC,"Renderer/SceneObject/mLightRigs");
 s->flags2&=~1u;s->flags=4;s->context=context;s->owner=owner;
 s->link18.next=s->link18.prev=&s->link18;s->link20.next=s->link20.prev=&s->link20;
 s->d28=s->d2c=s->d40=0;s->d44=0x80000205;s->d48=0x7f7fffff;
 CallGameMethod<void,0x0068BB50>(s->bounds);
 Identity(s->matrix);Identity(s->transform);s->identity.tag=0;s->parent=nullptr;
 s->d19c=s->d1a0=s->a=s->b=0;s->c=0x7fffffff;s->f1bc=s->f1c0=-1.0f;s->f1c4=0;
 for(auto& v:s->color)v=1.0f;return s;
}
uintptr_t __fastcall DestroyScene(SceneObject* s,void*) {
 s->table=Table(0x00FF9D00);
 if(s->children.prev!=&s->children) {
  ts3::ui::UiIdentity blank;blank.Init();blank.tag=0;
  for(auto* n=s->children.next;n!=&s->children;) {
   auto* child=n?reinterpret_cast<SceneObject*>(reinterpret_cast<uint8_t*>(n)-offsetof(SceneObject,link20)):nullptr;
   n=n->next;CallThis(Original(0x006FC860,dependencies.detach),child,0,&blank);
  }
 }
 if(s->registration) {CallThis(Original(0x00706200,dependencies.unregister),s->registration,s);s->registration=nullptr;}
 s->p30=nullptr;s->link20.prev->next=s->link20.next;s->link20.next->prev=s->link20.prev;
 auto r=CallThis(Original(0x00A8F270,dependencies.clearRigs),static_cast<void*>(&s->rigsBegin));
 s->table=Table(0x00FA1B78);return r;
}
uint8_t __fastcall Prepare(Instance* s,void*,InstanceModel* input,InstanceModel** output,int a4,int a5) {
 InstanceModel* model=input;if(input) Virtual<void>(input,0);
 if(void* first=*input->meshes) {
  auto* resource=CallGameMethod<void*,0x00444710>(first);
  if(resource) if(auto* shader=CallGameMethod<void*,0x008AB2B0>(resource)) {
   const char* names[]={"SingleObject","InstancedObject","ShadowCaster","InstancedShadowCaster"};
   for(int i=0;i<4;++i) s->names[i]=CallGameMethod<uint32_t,0x0079A550>(shader,names[i]);
  }
 }
 void* host=nullptr;
#if defined(_M_IX86)
 host=*reinterpret_cast<void**>(0x011CECC8);
#endif
 uint8_t hardware=CallGameMethod<uint8_t,0x0060F1C0>(host);
 if(hardware) {
  if(!model->instanced) {
   model=nullptr;Virtual<void>(input,1);
   if(!CallGameMethod<uint8_t,0x006DB6E0>(input,&model,0,a5)) {if(model) Virtual<void>(model,1);return 0;}
  }
  auto* renderer=CallGameFunction<void*,0x0061A510>();
  uint32_t count=CallGameMethod<uint32_t,0x006C93D0>(model);
  struct Vector {uint64_t* begin;uint64_t* end;uint64_t* capacity;const char* tag;uint64_t* fixed;uint64_t storage[16];} v;
  v.begin=v.end=v.fixed=v.storage;v.capacity=v.storage+16;v.tag="EASTL fixed_vector";
  auto append=[&](uint64_t item) {
   if(v.end>=v.capacity) CallGameMethod<void,0x006F9430>(&v,v.end,&item);
   else {auto* to=v.end++;if(to)*to=item;}
  };
  struct Declaration {uint8_t pad[0x18];uint64_t* begin;uint32_t count;};
  for(uint32_t i=0;i<count;++i) {
   void* mesh=model->meshes[i];auto* decl=CallGameMethod<Declaration*,0x0093ECC0>(mesh);
   if(!decl) continue;
   CallGameMethod<void,0x006F9850>(&v,decl->begin,decl->begin+decl->count,count);
   bool one=false,two=false;
   for(auto* it=v.begin;it!=v.end;++it) {
    auto* b=reinterpret_cast<uint8_t*>(it);
    if(*reinterpret_cast<uint16_t*>(b)==1 && b[6]==0) {if(b[7]==1) one=true;if(b[7]==2)two=true;}
   }
   if(!one) append(0x0100000200000001ull);
   if(!two) append(0x02000003000c0001ull);
   if(!one || !two) {
    void* format=nullptr;
    if(CallGameMethod<uint8_t,0x0061A9B0>(renderer,v.begin,int32_t(v.end-v.begin),&format)) CallGameMethod<void,0x0068EA80>(mesh,format);
    if(format) Virtual<void>(format,2);
   }
  }
  if(v.begin && v.begin!=v.fixed) Free(v.begin);
 }
 uint8_t success=1;
 if(s->list.next==&s->list) { *output=model;model=nullptr; }
 else for(auto* n=s->list.next;n!=&s->list && success;n=n->next) success&=Virtual<uint8_t>(n->value,4,model,output,a4,a5);
 if(success && !hardware) {
  if(!s->emulator) {
   auto* memory=CallGameFunction<void*,0x004E3F90>(28,"Renderer/InstanceEmulator",0,0,0,0);
   s->emulator=memory?CallGameMethod<void*,0x00697480>(memory):nullptr;
  }
  CallGameMethod<void,0x00696B80>(s->emulator,int32_t((s->end-s->begin)/96));
  CallGameMethod<void,0x006974F0>(s->emulator,reinterpret_cast<uint8_t*>(s)-0x2d0,*output);
 }
 if(model) Virtual<void>(model,1);return success;
}

}
