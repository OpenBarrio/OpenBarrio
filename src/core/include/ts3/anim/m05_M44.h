#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::m44 {
struct Range { void* begin; void* end; void* capacity; void* descriptor; };
struct RandomController {
    Controller base;
    void* interface28;
    ResourcePrefix* sequence;
    ResourcePrefix* current;
    void* listener;
    Range clips,keys,repeats,weights;
    float time,delay;
    int32_t pending;
    uint8_t busy;
};
TS3_X86_OFFSET(RandomController,sequence,0x2c);
TS3_X86_OFFSET(RandomController,clips,0x38);
TS3_X86_OFFSET(RandomController,pending,0x80);
TS3_X86_OFFSET(RandomController,busy,0x84);
// TS3W.exe 0x0056D560: Construct a random controller and retain a new sequence.
RandomController* Construct(RandomController*,void* host);
// TS3W.exe 0x0056D660: Return the random-controller interface identifier.
uint32_t TypeId();
// TS3W.exe 0x0056D670: Query either supported identifier from the +8 interface.
void* Query(void*,uint32_t);
// TS3W.exe 0x0056D690: Forward deleting destruction from the +0C interface.
void* DeleteInterface(void*,uint32_t);
// TS3W.exe 0x0056D6A0: Destroy members in reverse order and then the base, preserving EAX.
uintptr_t Destruct(RandomController*);
// TS3W.exe 0x0056D770: Decrement pending and dispatch from the +28 interface; ignore both arguments.
uintptr_t Notify(void*,uintptr_t,uintptr_t);
#if defined(_M_IX86)
RandomController* __fastcall HookConstruct(RandomController*,void*,void*);
uint32_t __fastcall HookTypeId(void*,void*);
void* __fastcall HookQuery(void*,void*,uint32_t);
void* __fastcall HookDelete(void*,void*,uint32_t);
uintptr_t __fastcall HookDestruct(RandomController*,void*);
uintptr_t __fastcall HookNotify(void*,void*,uintptr_t,uintptr_t);
#endif
}
