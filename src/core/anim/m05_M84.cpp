#include "ts3/anim/m05_M84.h"
#include "ts3/anim/m05_B21.h"
#include "ts3/resource/m03_A11.h"
namespace ts3::anim::m84impl {
namespace {
void* Address(uint32_t ea) {
#if defined(_M_IX86)
 return reinterpret_cast<void*>(ea);
#else
 (void)ea; NotRewritten("M84 original data");
#endif
}
}
void* __fastcall CreateClip(void*,void*) {
 auto p=CallGameFunction<void*,0x004E3F90>(0x68,Address(0xfc314c),0,0,0,0);
 if(!p) return nullptr;
 return CallGameMethod<void*,0x0057AB10>(p);
}
void* __fastcall DeleteClip(Clip* self,void*,uint32_t flags) {
 self->base.vtable=Address(0xfc3130);
 CallGameMethod<void,0x0057A970>(self);
 CallGameMethod<void,0x0057FF30>(&self->events);
 CallGameMethod<void,0x006A0850>(&self->map,self->map.allocation);
 CallGameMethod<void,0x00731CF0>(self);
 if(flags&1) CallGameFunction<void,0x004E3F70>(self);
 return self;
}
void* __fastcall BindClip(Clip* self,void*,void* key) {
 auto lock=Address(0x11cd8f8);
 CallGameMethod<void,0x004E16F0>(lock,Address(0xfc303c));
 auto name=static_cast<uint8_t*>(key)+8;
 void* result=nullptr; void* node;
 auto lookup=[&]() {
  CallGameMethod<void,0x0057A7D0>(&self->map,&node,name);
  if(node!=&self->map.sentinel[0]) result=*reinterpret_cast<void**>(static_cast<uint8_t*>(node)+0x20);
 };
 lookup(); lookup();
 if(node==&self->map.sentinel[0] && self->view) {
  auto clip=*reinterpret_cast<void**>(static_cast<uint8_t*>(self->view)+0xc);
  if(clip && key) {
   auto bytes=*reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(clip)+0x18)*4u;
   auto allocator=*static_cast<void**>(Address(0x11cb864));
   auto buffer=CallGameMethod<void*,0x004E69D0>(allocator,bytes,0);
   CallGameMethod<void,0x005843F0>(clip,key,buffer,bytes);
   auto slot=CallGameMethod<void**,0x0057AD50>(&self->map,name);
   *slot=buffer; result=buffer;
  }
 }
 CallGameMethod<void,0x004E17B0>(lock);
 return result;
}
void* __fastcall ConstructFactory(void* self,void*) {
 ts3::resource::A11Construct(static_cast<ts3::resource::A11Factory*>(self),0x8eaf13deu,Address(0x81d94df4));
 *static_cast<void**>(self)=Address(0xfc3230); return self;
}
int32_t __fastcall ResolveRig(RigResource* self,void*) {
 auto indices=reinterpret_cast<int32_t*>(self->opaquebc);
 auto find=[&](uint32_t name) { return CallGameMethod<int32_t,0x00575E30>(&self->skeleton,Address(name)); };
 int32_t r;
 indices[0]=find(0xfc16d0); indices[1]=find(0xfc14dc); indices[2]=find(0xfc14a0);
 indices[4]=find(0xfc3274); indices[5]=find(0xfc148c); r=indices[6]=find(0xfc147c);
 r=indices[2];
 if(r!=-1) r=indices[3]=ts3::anim::b21::ChainIndex(reinterpret_cast<const ts3::anim::b21::SkeletonView*>(&self->skeleton),r);
 if(indices[1]==-1) {
  r=indices[7]=find(0xfc14d0);
  if(r!=-1) { indices[8]=find(0xfc14c4); indices[9]=find(0xfc14b8); r=indices[10]=find(0xfc14ac); }
 }
 return r;
}
void* __fastcall DeleteFactory(void* self,void*,uint32_t flags) {
 *static_cast<void**>(self)=Address(0xfc3230);
 CallGameMethod<void,0x0072F180>(self);
 if(flags&1) CallGameFunction<void,0x004E4230>(self);
 return self;
}
}
