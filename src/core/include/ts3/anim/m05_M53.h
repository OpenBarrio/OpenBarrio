#pragma once
#include "ts3/misc/cls_5044c0_x.h"
namespace ts3::anim::m53 {
using Motion = ts3::misc::ProceduralMotionController;
struct Ref { void* table; int32_t refs; };
struct Event { void* source; Ref* resource; void* controller; uint32_t arg0,arg1; uint8_t flags[3]; };
TS3_X86_OFFSET(Event,flags,0x14);
// TS3W.exe 0x00570120: Construct motion accumulation, retaining and subscribing to its controller; zero duration is not guarded.
Motion* Construct(Motion*,void*,void*,const float*,const float*);
// TS3W.exe 0x00570550: Unsubscribe and release the retained controller, restoring one before deletion, then destroy the base.
uintptr_t Destruct(Motion*);
// TS3W.exe 0x005705B0: Configure procedural velocity and forward an event from the secondary interface; preserves two rsqrt refinements.
uintptr_t MotionEvent(void*,const Event*,uint32_t);
// TS3W.exe 0x00570870: Forward an event from the secondary interface while retaining its source and resource.
uintptr_t ForwardEvent(void*,const Event*,uint32_t);
// TS3W.exe 0x005709D0: Construct the first evaluator in supplied storage, or return null for null storage.
void* __stdcall FactoryA(void*,void*);
// TS3W.exe 0x005709F0: Construct the second evaluator in supplied storage, or return null for null storage.
void* __stdcall FactoryB(void*,void*);
#if defined(_M_IX86)
Motion* __fastcall HookConstruct(Motion*,void*,void*,void*,const float*,const float*);
uintptr_t __fastcall HookDestruct(Motion*,void*);
uintptr_t __fastcall HookMotionEvent(void*,void*,const Event*,uint32_t);
uintptr_t __fastcall HookForwardEvent(void*,void*,const Event*,uint32_t);
#endif
}
