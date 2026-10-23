#pragma once
#include "ts3/anim/m05_B01.h"
namespace ts3::anim::m23 {
struct Evaluator { b01::Node base; uint32_t opaque24; uint32_t key[2]; uint32_t first,second; uint32_t tail[2]; };
TS3_X86_OFFSET(Evaluator,key,0x28);
struct Subject { unsigned char opaque[0x40]; void* rig; unsigned char rest[0x10]; uint8_t flags; };
TS3_X86_OFFSET(Subject,flags,0x54);
struct Sequence { Controller base; void* interface28; Controller* transition; ListLink nodes; void* descriptor; void* pending; unsigned char opaque40[0x10]; void* pending50; };
TS3_X86_OFFSET(Sequence,pending,0x3c); TS3_X86_OFFSET(Sequence,pending50,0x50);
// TS3W.exe 0x005654D0: Load four fields from the signed stream cursor, skipping failed reads.
void Load(Evaluator*,b01::Cursor*);
// TS3W.exe 0x00565570: Reset evaluator base and optionally free, returning this even after free.
Evaluator* Destroy(Evaluator*,uint32_t);
// TS3W.exe 0x005655A0: Dispatch the evaluator then apply root rotation and translation with exact SSE ordering.
void Evaluate(Subject*,void*);
// TS3W.exe 0x00565B40: Tear down controller delegate subobject and restore resource base vtable.
void DestroyController(Controller*);
// TS3W.exe 0x00565D30: Erase a global controller registration and set only AL to one.
uint32_t Unregister(uint32_t);
// TS3W.exe 0x00565F40: Check queued nodes and pending state before querying the transition's low byte.
uint32_t Active(Sequence*);
}
#if defined(_M_IX86)
namespace ts3::anim::m23::hooks {
void __fastcall Load(Evaluator*,void*,b01::Cursor*);
Evaluator* __fastcall Destroy(Evaluator*,void*,uint32_t);
void __stdcall Evaluate(Subject*,void*);
void __fastcall DestroyController(Controller*,void*);
uint32_t __cdecl Unregister(uint32_t);
uint32_t __fastcall Active(Sequence*,void*);
}
#endif
