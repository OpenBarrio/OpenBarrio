#include "ts3/anim/m05_M90.h"
#include <cstring>
namespace ts3::anim::m90 {
namespace {
void* Table(uintptr_t address) {
#if defined(_M_IX86)
 return reinterpret_cast<void*>(address);
#else
 (void)address; static void* native[4]{}; return native;
#endif
}
Event* Init(Event* s,uint32_t owner,uint32_t type,uint32_t flags,float tolerance,uintptr_t table) {
 s->vtable=Table(table);s->owner=owner;s->type=type;s->subtype=0;
 s->time=0;s->tolerance=tolerance;s->duration=0x1.111112p-5f;
 s->flags=flags;s->payloadBytes=0;s->payload=nullptr;return s;
}
uint32_t Word(const uint8_t* p) { uint32_t v;std::memcpy(&v,p,4);return v; }
float Float(const uint8_t* p) { float v;std::memcpy(&v,p,4);return v; }
}
Event* __fastcall ConstructEffect(Event* s,void*,uint32_t owner) {
 Init(s,owner,0xc1e40005,0,0,0x00FCB518);
 reinterpret_cast<ExtendedEvent*>(s)->words[0]=0;return s;
}
Event* __fastcall ConstructStopEffect(Event* s,void*,uint32_t owner) {
 return Init(s,owner,0xc1e4000a,2,0,0x00FCB528);
}
Event* __fastcall ConstructVisibility(Event* s,void*,uint32_t owner) {
 Init(s,owner,0xc1e40006,2,0x1.111112p-7f,0x00FCB538);
 reinterpret_cast<ExtendedEvent*>(s)->words[0]=0x3f800000;return s;
}
Event* __fastcall ConstructCreateProp(Event* s,void*,uint32_t owner) {
 Init(s,owner,0xc1e40008,0,0x1.111112p-7f,0x00FCB548);
 auto* e=reinterpret_cast<ExtendedEvent*>(s);e->words[0]=0;
 e->words[2]=0;e->words[3]=0;e->words[4]=0;e->words[5]=0;return s;
}
Event* __fastcall ConstructDestroyProp(Event* s,void*,uint32_t owner) {
 Init(s,owner,0xc1e40009,2,0x1.111112p-7f,0x00FCB558);
 reinterpret_cast<ExtendedEvent*>(s)->words[0]=0;return s;
}
Event* ParseEvent(uint32_t version,uint32_t owner,uint8_t** cursor,uint32_t* remaining,uint32_t swap) {
 if(*remaining<32) return nullptr;
 uint8_t* data=*cursor;
 if(uint8_t(swap)) CallGameFunction<void,0x00739270>(data,uint32_t(7));
 uint32_t type=Word(data),index=type-0xc1e40001;
 if(index>9 || index==6 || (index==4 && version<0x101)) return nullptr;
 static constexpr uint32_t sizes[]={128,44,168,40,72,44,0,64,44,48};
 static constexpr uintptr_t names[]={0xfcb664,0xfcb644,0xfcb628,0xfcb60c,0xfcb5f0,0xfcb5b0,0,0xfcb590,0xfcb570,0xfcb5d0};
 auto* s=CallGameFunction<Event*,0x004E3F90>(sizes[index],reinterpret_cast<const char*>(names[index]),uint32_t(0),uint32_t(0),uint32_t(0),uint32_t(0));
 if(!s) return nullptr;
 switch(index) {
 case 0:s=CallGameMethod<Event*,0x0057F330>(s,owner);break;
 case 1:s=CallGameMethod<Event*,0x0057F420>(s,owner);break;
 case 2:s=CallGameMethod<Event*,0x0057F470>(s,owner);break;
 case 3:s=CallGameMethod<Event*,0x0057F2E0>(s,owner);break;
 case 4:s=ConstructEffect(s,nullptr,owner);break;
 case 5:s=ConstructVisibility(s,nullptr,owner);break;
 case 7:s=ConstructCreateProp(s,nullptr,owner);break;
 case 8:s=ConstructDestroyProp(s,nullptr,owner);break;
 case 9:s=ConstructStopEffect(s,nullptr,owner);break;
 }
 if(!s) return nullptr;
 s->subtype=Word(data+4);
 // The original FLD/FSTP round-trip quiets signaling NaNs in the time field.
 uint32_t timeBits=Word(data+8);
 if((timeBits&0x7f800000)==0x7f800000 && (timeBits&0x007fffff)) timeBits|=0x00400000;
 std::memcpy(&s->time,&timeBits,4);
 if(version>=0x102) s->flags=Word(data+20);else s->flags|=Word(data+20);
 float tolerance=Float(data+12);
 if(tolerance>0x1p-16f) s->tolerance=tolerance;
 if(s->tolerance>s->time) s->tolerance=s->time;
 float duration=Float(data+16);
 if(duration>0x1p-16f) s->duration=duration;
 s->payloadBytes=Word(data+24);s->payload=data+28;
 uint32_t words=(s->payloadBytes>>2)+8;
 *cursor+=uint32_t(words*4);*remaining-=words;
 auto accepted=CallGameVirtual<uint8_t,0>(s,version,cursor,remaining,swap);
 if(!accepted) { CallGameVirtual<void,1>(s,uint32_t(1));return nullptr; }
 if(version<0x104) {
  if(s->type==0xc1e40001 || s->type==0xc1e40002 || s->type==0xc1e40004) s->flags|=0x104;
  else s->flags&=0xfffffefb;
 }
 return s;
}
}
