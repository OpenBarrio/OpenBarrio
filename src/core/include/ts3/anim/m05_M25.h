#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::m25 {
struct Record { Controller* child; void* key; uint32_t flags; float duration,weight; };
struct Node { ListLink links; Record value; };
struct Sequence {
    Controller base; void* secondary; Controller* transition; ListLink nodes; void* descriptor;
    Record current,next; float rate; uint32_t mode,flags;
};
struct Reader { unsigned char prefix[8]; void* stream; uint32_t unused,endian; uint8_t good; };
TS3_X86_OFFSET(Sequence,current,0x3c);
TS3_X86_OFFSET(Sequence,next,0x50);
TS3_X86_OFFSET(Sequence,rate,0x64);
TS3_X86_SIZE(Sequence,0x70);
// TS3W.exe 0x005668F0: promote a matching current controller when a next controller exists.
void Promote(Sequence*);
// TS3W.exe 0x00566960: serialize sequence state; success returns the raw good byte, not normalized bool.
uint8_t Save(Sequence*,Reader*);
// TS3W.exe 0x00566B70: start or blend a record; the temporary blend record retains EA's weight -1.
void Start(Sequence*,const Record*);
// TS3W.exe 0x00566CB0: enqueue or immediately start a controller; flags are additive.
bool Enqueue(Sequence*,Controller*,float,float,bool,void*,uint32_t,bool);
// TS3W.exe 0x00566F20: snapshot and clear queued nodes before dispatching their removal callbacks.
void Flush(Sequence*,uint32_t);
// TS3W.exe 0x00566FB0: pop an unblocked queued record and report its key notification.
bool Pop(Sequence*,uint8_t*);
#if defined(_M_IX86)
inline void __fastcall HookPromote(Sequence* s,void*) { Promote(s); }
inline uint8_t __fastcall HookSave(Sequence* s,void*,Reader* r) { return Save(s,r); }
inline void __fastcall HookStart(Sequence* s,void*,const Record* r) { Start(s,r); }
inline bool __fastcall HookEnqueue(Sequence* s,void*,Controller* c,float d,float w,bool o,void* k,uint32_t m,bool e) { return Enqueue(s,c,d,w,o,k,m,e); }
inline void __fastcall HookFlush(Sequence* s,void*,uint32_t f) { Flush(s,f); }
inline bool __fastcall HookPop(Sequence* s,void*,uint8_t* n) { return Pop(s,n); }
#endif
}
