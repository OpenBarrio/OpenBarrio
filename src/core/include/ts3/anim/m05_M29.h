#pragma once
#include "ts3/anim/m05_layouts.h"
#include "ts3/anim/m05_B23.h"
namespace ts3::anim::m29 {
struct Clip {
 Controller base;
 void* begin; void* end; void* capacity;
 unsigned char opaque34[4], storage[0x4b8];
 ClipIkInfo ik;
 b23::Resource* resource;
 void* stateD4;
 ResourcePrefix* retained;
 float time,total,elapsed,period;
 uint32_t mode,cycles;
 int32_t last;
 uint8_t flags[7],padding;
};
struct Event { Clip* owner; ResourcePrefix* retained; void* host; ClipIkInfo* ik; void* record; uint8_t last,flag1,flag2; };
TS3_X86_SIZE(Clip,0x600); TS3_X86_OFFSET(Clip,resource,0x5d0); TS3_X86_OFFSET(Clip,flags,0x5f8);
TS3_X86_SIZE(Event,24);
// TS3W.exe 0x00568820: Construct a clip controller, preserving unspecified storage bytes.
Clip* Construct(Clip*,void*,float);
// TS3W.exe 0x005688E0: Destroy and optionally free the controller, returning this even after free.
Clip* Delete(Clip*,uint32_t);
// TS3W.exe 0x00568900: Populate an event and exchange its retained reference before setting flags.
uint8_t FillEvent(Clip*,Event*);
// TS3W.exe 0x005689A0: Dispatch eligible clip events forward or backward, updating the last index after callbacks.
void Dispatch(Clip*,float,uint8_t);
// TS3W.exe 0x00568E10: Advance the controller once, process events and notifications, and release its temporary references.
void Update(Clip*,float);
// TS3W.exe 0x00569070: Flush remaining events for a started resource and notify controller release.
void Finish(Clip*);
#if defined(_M_IX86)
inline Clip* __fastcall HookConstruct(Clip* s,void*,void* h,float t){return Construct(s,h,t);}
inline Clip* __fastcall HookDelete(Clip* s,void*,uint32_t f){return Delete(s,f);}
inline uint8_t __fastcall HookFillEvent(Clip* s,void*,Event* e){return FillEvent(s,e);}
inline void __fastcall HookDispatch(Clip* s,void*,float t,uint8_t b){Dispatch(s,t,b);}
inline void __fastcall HookUpdate(Clip* s,void*,float t){Update(s,t);}
inline void __fastcall HookFinish(Clip* s,void*){Finish(s);}
#endif
}
