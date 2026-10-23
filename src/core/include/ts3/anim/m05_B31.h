#pragma once
#include "ts3/layout.h"
namespace ts3::anim::b31 {
struct Link { Link* next; Link* previous; };
struct Observer { void** vtable; Link link; };
struct Dispatcher { void** vtable; void** refVtable; uint32_t refs; Link listeners; };
struct Proxy { void** vtable; unsigned char opaque[0x24]; void* target; };
struct EntityView { unsigned char prefix[0xdc]; void* entity; unsigned char opaque[0x20]; uint8_t active; };
TS3_X86_OFFSET(EntityView,entity,0xdc);
TS3_X86_OFFSET(EntityView,active,0x100);
struct SlaveState {
    void** vtable; uint32_t opaque; void* resource; uint32_t value;
    void** begin; void** end; void** capacity; const void* descriptor;
    uint8_t flag; void* first; void* second; uint32_t tail; uint8_t enabled;
};
TS3_X86_OFFSET(Dispatcher,listeners,0xc);
TS3_X86_OFFSET(Proxy,target,0x28);
TS3_X86_OFFSET(SlaveState,first,0x24);
TS3_X86_OFFSET(SlaveState,enabled,0x30);
// TS3W.exe 0x00839850: notify listeners then self, retaining through callbacks.
uintptr_t __fastcall NotifyAndSelf(Dispatcher*,void*);
// TS3W.exe 0x008399E0: append an intrusive listener and return its link.
Link* __fastcall Attach(Dispatcher*,void*,Observer*);
// TS3W.exe 0x00839A10: unlink a listener and return its successor; links remain stale.
Link* __fastcall Detach(Dispatcher*,void*,Observer*);
// TS3W.exe 0x00839B10: notify listener slot four, retaining the dispatcher.
uintptr_t __fastcall Notify(Dispatcher*,void*);
// TS3W.exe 0x00839B60: notify listener slot five, retaining the dispatcher.
uintptr_t __fastcall NotifyOther(Dispatcher*,void*);
// TS3W.exe 0x00839BB0: return the pointer at DC only when byte 100 is set.
void* __fastcall ActiveEntity(void*,void*);
// TS3W.exe 0x00839BD0: query the target interface or use fallback slot seven.
uintptr_t __fastcall ForwardFirst(Proxy*,void*,void*);
// TS3W.exe 0x00839C10: query the target interface or use fallback slot nine.
uintptr_t __fastcall ForwardSecond(Proxy*,void*,void*);
// TS3W.exe 0x00839C50: obtain target weight, defaulting to one.
float __fastcall Weight(Proxy*,void*);
// TS3W.exe 0x00839C70: obtain the target vector or clear four lanes with an aligned store.
void __fastcall Vector(Proxy*,void*,void*);
// TS3W.exe 0x00839DF0: initialize slave state, preserving padding.
SlaveState* __fastcall Construct(SlaveState*,void*);
// TS3W.exe 0x00839E30: return the slave-state type id.
uint32_t __fastcall Type(void*,void*);
// TS3W.exe 0x00839E40: return this for the exact slave-state type id only.
void* __fastcall Query(void*,void*,uint32_t);
// TS3W.exe 0x00839E60: release second, first, storage, then atomic resource.
void __fastcall Destruct(SlaveState*,void*);
// TS3W.exe 0x0083A630: destroy and optionally free, returning this even after free.
SlaveState* __fastcall Delete(SlaveState*,void*,uint32_t);
}
