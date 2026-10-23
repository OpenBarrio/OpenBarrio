#pragma once
#include "ts3/anim/m05_B08.h"
namespace ts3::anim::m28 {
// The scaffold's opaque5d0 contains three pointers, widened here for native builds.
struct Clip {
    Controller base;
    void* begin; void* end; void* capacity;
    unsigned char opaque34[4]; unsigned char inlineStorage[0x4b8];
    ClipIkInfo ik;
    void* resource0; void* resource1; ResourcePrefix* retained;
    float stateDC,stateE0,stateE4,period;
    uint32_t mode,valueF0; int32_t valueF4;
    uint8_t flags[7],opaque5ff;
};
TS3_X86_OFFSET(Clip,ik,0x4f0);
TS3_X86_OFFSET(Clip,resource0,0x5d0);
TS3_X86_OFFSET(Clip,retained,0x5d8);
TS3_X86_SIZE(Clip,0x600);
// TS3W.exe 0x00568620: Dispatch slot zero, snapshotting next before callbacks and purging at depth zero.
void Dispatch0(Controller*,void*);
// TS3W.exe 0x00568670: Dispatch slot one with deferred tombstone cleanup.
void Dispatch1(Controller*,void*);
// TS3W.exe 0x005686C0: Dispatch slot two with deferred tombstone cleanup.
void Dispatch2(Controller*,void*);
// TS3W.exe 0x00568710: Dispatch slot four (not three) with deferred tombstone cleanup.
void Dispatch4(Controller*,void*);
// TS3W.exe 0x00568760: Destroy clip state, releasing retained resource first and restoring its last reference.
void Destroy(Clip*);
// TS3W.exe 0x00568810: Adjust the delegate interface by minus 0xC and tail-call deleting destruction.
Clip* DeleteInterface(void*,uint32_t);
#if defined(_M_IX86)
inline void __fastcall HookDispatch0(Controller* s,void*,void* e){Dispatch0(s,e);}
inline void __fastcall HookDispatch1(Controller* s,void*,void* e){Dispatch1(s,e);}
inline void __fastcall HookDispatch2(Controller* s,void*,void* e){Dispatch2(s,e);}
inline void __fastcall HookDispatch4(Controller* s,void*,void* e){Dispatch4(s,e);}
inline void __fastcall HookDestroy(Clip* s,void*){Destroy(s);}
inline Clip* __fastcall HookDeleteInterface(void* s,void*,uint32_t f){return DeleteInterface(s,f);}
#endif
}
