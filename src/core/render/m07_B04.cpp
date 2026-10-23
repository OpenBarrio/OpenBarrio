#include "ts3/render/m07_B04.h"
#include <algorithm>
#include <cstring>
namespace ts3::render::m07::b04 {
namespace {
template<unsigned Offset,class... A> int32_t Com(void* self,A... args) {
 using F=Stdcall<int32_t,void*,A...>;
 auto table=*static_cast<void***>(self);
 return reinterpret_cast<F>(table[Offset/4])(self,args...);
}
uint32_t Word(const void* p,size_t n) { uint32_t v; std::memcpy(&v,static_cast<const uint8_t*>(p)+n,4);return v; }
uint32_t SampleFormat(uint32_t n) {
#if defined(_M_IX86)
 return reinterpret_cast<const uint32_t*>(0xfe7a60)[n];
#else
 // D3DMULTISAMPLE_TYPE table is the identity mapping (proved image data).
 return n;
#endif
}
}
bool Check(Host* h,uint32_t mode) {
 if(!h->d3d9 || mode>=17) return false;
 return Com<0x2c>(h->d3d9,h->adapter,Word(h->privateData,0),Word(h->privateData,0x154),uint32_t(h->windowed),SampleFormat(mode),static_cast<void*>(nullptr))>=0;
}
uint32_t Find(Host* h,uint32_t mode) { while(mode && !Check(h,mode)) --mode;return mode; }
float Scalar(Host* h) { return h->scalar; }
uintptr_t Instancing(Host* h) {
 bool supported=Word(h->privateData,0xe0)>=0xfffe0300u;h->instancing=uint8_t(supported);
 uintptr_t result=(reinterpret_cast<uintptr_t>(h->privateData)&~uintptr_t(255))|uintptr_t(supported);
 if(!supported) {
  int32_t hr=Com<0x28>(h->d3d9,0u,1u,22u,0u,1u,0x54534e49u);result=uint32_t(hr);
  if(hr==0) { result=uint32_t(Com<0xe4>(h->device,154u,0x54534e49u));h->instancing=1; }
 }
 return result;
}
Host* RenderState(Host* h,uint32_t state,uint32_t value) {
 uint32_t n=state-7,bit=1u<<(n&31),group=n>>5,old=h->desired[n];
 if(old!=value) { h->desired[n]=value;h->dirty[group]|=bit;
  if(!(h->invalid[group]&bit)) { h->invalid[group]|=bit;
   if(h->capacity-h->index>=2) { h->stack[h->index++]=old;h->stack[h->index++]=n+0x100000; }
  }
 }
 return h;
}
Host* SamplerState(Host* h,uint32_t sampler,uint32_t state,uint32_t value) {
 auto& s=h->samplers[sampler];uint32_t n=state-1,bit=1u<<(n&31),old=s.desired[n];
 if(old!=value) { s.desired[n]=value;s.dirty|=bit;
  if(!(s.invalid&bit)) { s.invalid|=bit;
   if(h->capacity-h->index>=2) {h->stack[h->index++]=old;h->stack[h->index++]=(sampler<<16)+n;}
  }
 }
 return h;
}
void* Flush(Host* h) {
 // Seven 32-bit groups are scanned, including the 21-word overlap with applied.
 auto words=reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(h)+offsetof(Host,desired));
 auto applied=reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(h)+offsetof(Host,applied));
 for(uint32_t g=0;g<7;++g) {
  uint32_t bits=h->dirty[g]&~h->unused[g];
  if(!bits) continue;
  for(uint32_t j=0;j<32;++j,bits>>=1) {uint32_t n=g*32+j;if((bits&1)&&applied[n]!=words[n]) {applied[n]=words[n];Com<0xe4>(h->device,n+7,words[n]);}}
  h->dirty[g]=0;
 }
 for(uint32_t i=0;i<16;++i) {auto& s=h->samplers[i];if(!(h->samplerDirty&(1u<<i))) continue;
  uint32_t bits=s.dirty&~s.unused;if(!bits) continue;
  for(uint32_t j=0;j<13;++j,bits>>=1) if((bits&1)&&s.applied[j]!=s.desired[j]) {s.applied[j]=s.desired[j];Com<0x114>(h->device,i,j+1,s.desired[j]);}
  s.dirty=0;
 }
 return reinterpret_cast<uint8_t*>(&h->samplers[16])+0x34;
}
Host* Push(Host* h) {
 if(h->capacity-h->index<25) return h;
 for(auto& s:h->samplers) {h->stack[h->index++]=s.invalid;s.invalid=0;}
 for(uint32_t v:h->invalid) h->stack[h->index++]=v;
 for(auto& v:h->invalid) v=0;
 h->stack[h->index++]=0xffffffffu;return h;
}
void* Invalidate(Host* h) {h->index=0;for(auto& v:h->invalid)v=~0u;for(auto& s:h->samplers)s.invalid=~0u;return reinterpret_cast<uint8_t*>(&h->samplers[16])+0x70;}
void Unbind(Host* h,void* texture) {for(uint32_t i=0;i<16;++i) if(h->textures[i]==texture) {h->textures[i]=nullptr;Com<0x104>(h->device,i,static_cast<void*>(nullptr));}}
uint32_t IndexFormat(Host* h) {return ts3::CallGameMethod<uint32_t,0x616c10>(h->indexBuffer);}
void* Handle(Host* h) {return h->handle94;}
int32_t Scissor(Host* h,const int32_t* rect) {int32_t copy[4];for(int i=0;i<4;++i)copy[i]=rect[i];return Com<0x12c>(h->device,static_cast<const int32_t*>(copy));}
void Restore(Host* h,Saved* s) {
 uint32_t count=std::min(Word(h->privateData,0x10c),1u);
 for(uint32_t i=0;i<count;++i) {void* target=s->target;s->target=nullptr;Com<0x94>(h->device,i,target);if(target)Com<8>(target);}
 Com<0x9c>(h->device,s->depth);if(s->depth) {Com<8>(s->depth);s->depth=nullptr;}
 if(s->width>0) {uint32_t rect[4]={uint32_t(s->x),uint32_t(s->y),uint32_t(s->x)+uint32_t(s->width),uint32_t(s->y)+uint32_t(s->height)};ts3::CallGameMethod<void,0x60ffe0>(h,rect,s->minZ,s->maxZ);}
 if(s->scissor[0]>=0) Scissor(h,s->scissor);
}
}
