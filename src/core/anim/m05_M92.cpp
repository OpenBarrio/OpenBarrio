#include "ts3/anim/m05_M92.h"
#include "ts3/layout.h"
#include "ts3/scratch/clu_4e3f70_Scratch.h"
namespace ts3::anim::m92 {
TS3_X86_OFFSET(Events,cached,0x10);
TS3_X86_OFFSET(KeyEvent,key,0x38);
uint32_t __fastcall EqualPair(PairEvent* s,void*,const PairEvent* b) {
 uint32_t r=b25::Equal(&s->base,nullptr,&b->base);
 if(!uint8_t(r)) return r;
 r=s->keys[0];
 bool equal=r==b->keys[0] && s->keys[1]==b->keys[1];
 return (r&0xffffff00u)|uint32_t(equal);
}
int32_t __fastcall Count(Events* s,void*) {
 // Adapt the verified Index::Count twin's arithmetic shift to pointer stride.
 auto span=static_cast<intptr_t>(reinterpret_cast<uintptr_t>(s->end)-reinterpret_cast<uintptr_t>(s->begin));
 return static_cast<int32_t>(span >> (sizeof(void*)==4?2:3));
}
int32_t __fastcall FindFlagged(Events* s,void*) {
 if(s->cached==-2) {
  s->cached=-1;
  uint32_t n=static_cast<uint32_t>(Count(s,nullptr));
  for(uint32_t i=0;i<n;++i) if(s->begin[i] && (s->begin[i]->flags&4)==4) s->cached=static_cast<int32_t>(i);
 }
 return s->cached;
}
b25::Event* __fastcall At(Events* s,void*,uint32_t i) { return s->begin[i]; }
void __fastcall Destroy(Events* s,void*) {
 const uint32_t n=static_cast<uint32_t>(Count(s,nullptr));
 for(uint32_t i=0;i<n;++i) {
  auto* p=s->begin[i]; if(p) CallGameVirtual<void,1>(p,uint32_t(1));
 }
 if(s->begin) ts3::scratch::Free4E3F70(s->begin);
}
void* __fastcall CollectKey(KeyEvent* s,void*,void* set) {
 struct Iterator { void* node; void* bucket; uint32_t inserted; } iterator;
 uint32_t key=s->key;
 return CallGameMethod<void*,0x00B7AAD0>(set,&iterator,&key,uint32_t(0));
}
}
