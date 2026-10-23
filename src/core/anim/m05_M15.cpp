#include "ts3/anim/m05_M15.h"
#include <cstring>
namespace ts3::anim::m15 {
namespace {
template<unsigned A> void* Address() {
#if defined(_M_IX86)
 return reinterpret_cast<void*>(uintptr_t(A));
#else
 AbortNotRewritten("m15 game data"); return nullptr;
#endif
}
void Table(Node* n,void* p) { n->base.vtable=static_cast<void**>(p); }
uintptr_t Word(Writer* w,uint32_t value) {
 w->offset=static_cast<int32_t>(static_cast<uint32_t>(w->offset)+4);
 uintptr_t r;
 if(w->offset>=0) r=CallGameMethod<uintptr_t,0x00594140>(w,4);
 else r=0;
 if(w->offset<0) {
  r=reinterpret_cast<uintptr_t>(w->buffer);
  std::memcpy(w->buffer+w->offset-3,&value,4);
 }
 return r;
}
}
Node* ConstructOrient(Node* n,void* host) {
 b01::Construct(&n->base,host);
 Table(n,Address<0x00FC141C>());
 n->begin=n->end=n->capacity=nullptr; n->descriptor=Address<0x00FC13F8>();
 return n;
}
Node* ConstructPlain(Node* n,void* host) {
 b01::Construct(&n->base,host); Table(n,Address<0x00FC1448>()); return n;
}
Node* DeleteOrient(Node* n,uint32_t flags) {
 Table(n,Address<0x00FC141C>()); n->end=n->begin;
 if(n->begin) CallGameFunction<void,0x004E3F70>(n->begin);
 b01::Reset(&n->base);
 if(flags&1) CallGameFunction<void,0x004E3F70>(n);
 return n;
}
Node* DeletePlain(Node* n,uint32_t flags) {
 Table(n,Address<0x00FC1448>()); b01::Reset(&n->base);
 if(flags&1) CallGameFunction<void,0x004E3F70>(n);
 return n;
}
uintptr_t Evaluate(Node* n,Context* c,Pose* p) {
 c->state[0x29e]|=2;
 uintptr_t r=reinterpret_cast<uintptr_t>(&n->base.children);
 if(n->base.children.previous!=&n->base.children) {
  auto link=n->base.children.next;
  auto child=link?reinterpret_cast<b01::Node*>(reinterpret_cast<unsigned char*>(link)-offsetof(b01::Node,link)):nullptr;
  r=CallGameVirtual<uintptr_t,2>(child,c,p);
  if((c->flags&2)&&!p->weights) r=CallGameMethod<uintptr_t,0x005741B0>(p,1.0f);
 } else {
  r=CallGameMethod<uintptr_t,0x005743A0>(p);
  if(c->flags&2) {
   CallGameMethod<void,0x005741E0>(p);
   r=CallGameMethod<uintptr_t,0x005741B0>(p,0.0f);
  }
 }
 // The range load overwrites EAX even when the override loop is skipped.
 r=reinterpret_cast<uintptr_t>(n->begin);
 if(n->begin!=n->end&&(c->flags&2)) {
  float* weights=p->weights;
  auto srt=CallGameMethod<SrtValue*,0x00574190>(p);
  r=reinterpret_cast<uintptr_t>(srt);
  for(auto e=n->begin;e!=n->end;++e) {
   if(e->index>=0&&static_cast<uint32_t>(e->index)<p->boneCount) {
    weights[e->index]=1.0f;
    std::memcpy(reinterpret_cast<unsigned char*>(srt)+48*e->index+16,e->value,16);
   }
  }
 }
 return r;
}
uintptr_t Save(Node* n,Writer* w) {
 uintptr_t r=Word(w,static_cast<uint32_t>(n->end-n->begin));
 for(auto e=n->begin;e!=n->end;++e) {
  bool write=true;
  if(w->offset>=-16) {
   r=CallGameMethod<uintptr_t,0x00594140>(w,0);
   if(!(r&255)) { w->failed=1; write=false; }
  }
  if(write) {
   auto out=w->buffer+w->offset+1;
   r=reinterpret_cast<uintptr_t>(out);
   std::memcpy(out,e->value,16); w->offset+=16;
  }
  r=Word(w,static_cast<uint32_t>(e->index));
 }
 return r;
}
#if defined(_M_IX86)
namespace hooks {
uintptr_t __fastcall Evaluate(Node* n,void*,Context* c,Pose* p) { return m15::Evaluate(n,c,p); }
uintptr_t __fastcall Save(Node* n,void*,Writer* w) { return m15::Save(n,w); }
Node* __fastcall ConstructOrient(Node* n,void*,void* h) { return m15::ConstructOrient(n,h); }
Node* __fastcall ConstructPlain(Node* n,void*,void* h) { return m15::ConstructPlain(n,h); }
Node* __fastcall DeleteOrient(Node* n,void*,uint32_t f) { return m15::DeleteOrient(n,f); }
Node* __fastcall DeletePlain(Node* n,void*,uint32_t f) { return m15::DeletePlain(n,f); }
}
#endif
}
