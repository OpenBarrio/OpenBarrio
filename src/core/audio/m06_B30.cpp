#include "ts3/audio/m06_B30.h"
#include <algorithm>
#include <cstring>
namespace ts3::audio::b30 {
namespace {
uintptr_t Address(const void* p){return reinterpret_cast<uintptr_t>(p);}
uint8_t* Base(Queue* q){return q->begin==q->end?nullptr:q->begin;}
uint8_t* Add(uint8_t* p,uintptr_t n){return reinterpret_cast<uint8_t*>(Address(p)+n);}
uintptr_t Reset(Queue* q){
 for(auto& p:q->transient)p=nullptr;
 q->cursor=Base(q); q->read=q->record=q->length=q->count=nullptr;
 q->unknown40=q->items=q->unknown4c=0;
 return Address(q->cursor);
}
uintptr_t Finish(Queue* q){
 if(q->count){++q->items; std::memcpy(q->count,&q->items,4);}
 return Address(q->count);
}
void Word(Queue* q,uint32_t v){F89FEB0(q,nullptr,&v,4);}
uintptr_t Parameter(Queue* q,uint32_t key,uint32_t size,uint32_t type,const void* data){
 Word(q,key);Word(q,size);Word(q,type);F89FEB0(q,nullptr,data,size);return Finish(q);
}
}
uintptr_t __fastcall F89FD60(Queue* q,void*,uint32_t requested){
 uint32_t size=std::min(std::max(requested,q->capacity),q->limit);
 uint8_t* old=Base(q);
 uint8_t* pointers[]={q->cursor,q->read,q->record,q->length,q->count};
 intptr_t offsets[5];
 for(int i=0;i<5;++i)offsets[i]=pointers[i]?static_cast<intptr_t>(Address(pointers[i])-Address(old)):-1;
#if defined(_M_IX86)
 CallGameMethod<void,0x00405410>(q,size);
#else
 if(q->nativeResize)q->nativeResize(q,size);
 else AbortNotRewritten("m06_B30 vector resize 00405410");
#endif
 auto* base=Base(q);q->capacity=size;
 for(int i=0;i<5;++i){
  auto* p=offsets[i]<0?nullptr:Add(base,static_cast<uintptr_t>(offsets[i]));
  if(Address(p)<Address(base)||Address(p)>=Address(q->end))p=nullptr;
  pointers[i]=p;
 }
 q->cursor=pointers[0];q->read=pointers[1];q->record=pointers[2];q->length=pointers[3];q->count=pointers[4];
 return q->count?Address(base):0;
}
uintptr_t __fastcall F89FEB0(Queue* q,void*,const void* src,uint32_t size){
 auto available=[&](){return static_cast<uint32_t>(q->capacity-Address(q->cursor)+Address(Base(q)));};
 if(available()<size){
  if(q->capacity>=q->limit || size>q->limit){Reset(q);return 0;}
  F89FD60(q,nullptr,std::min(q->capacity*2,q->limit));
  if(available()<size)return Reset(q);
 }
 uintptr_t cursor=Address(q->cursor);
 if(cursor<Address(q->begin)||cursor>Address(q->end)-size){Reset(q);return 0;}
 uintptr_t result=0; // MOV EAX, source also makes reserve-only appends return zero.
 if(src){
  if(size<=4){std::memcpy(q->cursor,src,4);result=Address(src);}
  else {std::memcpy(q->cursor,src,size);result=cursor;}
 }
 q->cursor=Add(q->cursor,(size+3u)&~3u);
 return result;
}
uintptr_t __fastcall F89FFC0(Queue* q,void*,uint32_t k,uint32_t v){return Parameter(q,k,4,0,&v);}
uintptr_t __fastcall F8A0020(Queue* q,void*,uint32_t k,uint32_t v){return Parameter(q,k,4,1,&v);}
uintptr_t __fastcall F8A0080(Queue* q,void*,uint32_t k,uint32_t lo,uint32_t hi){uint32_t v[]={lo,hi};return Parameter(q,k,8,2,v);}
uintptr_t __fastcall F8A00E0(Queue* q,void*,uint32_t k,uint32_t v){return Parameter(q,k,4,4,&v);}
uintptr_t __fastcall F8A0140(Queue* q,void*,uint32_t size,uint32_t limit){
 q->cursor=nullptr;F89FD60(q,nullptr,size);auto result=Reset(q);q->limit=limit;
 return (result&~uintptr_t(255))|1;
}
uintptr_t __fastcall F8A01A0(Queue* q,void*,uint32_t key){
 if(q->length&&q->record){q->cursor=q->record;q->record=q->length=q->count=nullptr;}
 q->record=q->cursor;Word(q,key);q->length=q->cursor;Word(q,0);q->count=q->cursor;
 uint32_t zero=0;auto result=F89FEB0(q,nullptr,&zero,4);q->items=0;
 // Incidental EAX is the short write's source stack-slot address, not a boolean.
 return result;
}
uintptr_t __fastcall F8A0210(Queue* q,void*,uint32_t key,float value){
#if defined(_M_IX86)
 // FLD/FSTP also quiets signaling NaNs, unlike a pure integer bit-copy.
 __asm {
  fld value
  fstp value
 }
#endif
 uint32_t bits;std::memcpy(&bits,&value,4);
#if !defined(_M_IX86)
 // Match the x87 load/store's quiet bit without truncating native pointers or
 // depending on SSE's treatment of signaling NaNs during a plain move.
 if((bits&0x7f800000u)==0x7f800000u && (bits&0x007fffffu))bits|=0x00400000u;
#endif
 return F89FFC0(q,nullptr,key,bits);
}
uintptr_t __fastcall F8A0230(Queue* q,void*,uint32_t k,uint32_t v){return F8A0020(q,nullptr,k,v);}
uintptr_t __fastcall F8A0240(Queue* q,void*,uint32_t k,uint32_t lo,uint32_t hi){return F8A0080(q,nullptr,k,lo,hi);}
uintptr_t __fastcall F8A0260(Queue* q,void*,uint32_t k,const char* v,uint32_t n){if(!n)n=static_cast<uint32_t>(std::strlen(v));return Parameter(q,k,n+1,3,v);}
uintptr_t __fastcall F8A02E0(Queue* q,void*,uint32_t k,uint32_t v){return F8A00E0(q,nullptr,k,v);}
uintptr_t __fastcall F8A02F0(Queue* q,void*,uint32_t k,const void* v,uint32_t n){return Parameter(q,k,n,5,v);}
uintptr_t __fastcall F8A0350(Queue* q,void*,uint32_t k,const void* v){
 Word(q,k);Word(q,12);Word(q,6);
 auto* p=static_cast<const uint8_t*>(v);F89FEB0(q,nullptr,p,4);F89FEB0(q,nullptr,p+4,4);F89FEB0(q,nullptr,p+8,4);return Finish(q);
}
}
