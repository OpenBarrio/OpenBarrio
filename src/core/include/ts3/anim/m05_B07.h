#pragma once
#include "ts3/layout.h"
#include "ts3/math/float_values.h"
namespace ts3::anim {
// ABI facade: member overloads are used by the registered thiscall adapters.
struct b07 {
struct State {
    void** vtable; unsigned char opaque[0x24-sizeof(void*)];
    void* first; void* second;
};
TS3_X86_OFFSET(State,first,0x24);
TS3_X86_OFFSET(State,second,0x28);
struct StreamState {
    int32_t cursor; uint32_t field4,field8;
    unsigned char* readBase; unsigned char* writeBase;
};
// TS3W.exe 0x0055DF40: Return evaluator class ID.
static uint32_t TypeDF40(void*);
// TS3W.exe 0x0055DF50: Query evaluator or base interface.
static void* QueryDF50(void*,uint32_t);
// TS3W.exe 0x0055DFF0: Replace retained resource at +24, restoring last ref before delete.
static uintptr_t SetDFF0(State*,void*);
// TS3W.exe 0x0055E040: Replace retained resource at +28, restoring last ref before delete.
static uintptr_t SetE040(State*,void*);
// TS3W.exe 0x0055E090: Evaluate across controller boundaries, retaining the active controller.
static math::Float4Value* AdvanceE090(math::Float4Value*,void*,uint32_t,uint32_t,float,uint8_t,math::Float4Value*,uint32_t);
// TS3W.exe 0x00561BB0: Return evaluator class ID.
static uint32_t Type1BB0(void*);
// TS3W.exe 0x00561BC0: Query evaluator or base interface.
static void* Query1BC0(void*,uint32_t);
// TS3W.exe 0x00561BF0: Save fields in original order, including raw flag bytes.
static uintptr_t Save1BF0(void*,StreamState*);
// TS3W.exe 0x00561D10: Load fields and canonicalize flags; preserve partial AL return.
static uintptr_t Load1D10(void*,StreamState*);
// TS3W.exe 0x005623F0: Apply axis-angle rotation in bind-local coordinates using original SSE order.
static int32_t Rotate23F0(void*,const void*,int32_t,const math::Float4Value*,float);
// TS3W.exe 0x00565100: Return evaluator class ID.
static uint32_t Type5100(void*);
// TS3W.exe 0x00565110: Query evaluator or base interface.
static void* Query5110(void*,uint32_t);
// TS3W.exe 0x00565140: Save two raw scalar fields.
static uintptr_t Save5140(void*,StreamState*);
// TS3W.exe 0x005651A0: Load two raw scalar fields.
static uintptr_t Load51A0(void*,StreamState*);
// TS3W.exe 0x005653F0: Return evaluator class ID.
static uint32_t Type53F0(void*);

uint32_t TypeDF40() { return TypeDF40(this); }
uint32_t Type1BB0() { return Type1BB0(this); }
uint32_t Type5100() { return Type5100(this); }
uint32_t Type53F0() { return Type53F0(this); }
void* QueryDF50(uint32_t id) { return QueryDF50(this,id); }
void* Query1BC0(uint32_t id) { return Query1BC0(this,id); }
void* Query5110(uint32_t id) { return Query5110(this,id); }
uintptr_t SetDFF0(void* p) { return SetDFF0(reinterpret_cast<State*>(this),p); }
uintptr_t SetE040(void* p) { return SetE040(reinterpret_cast<State*>(this),p); }
uintptr_t Save1BF0(void* s) { return Save1BF0(this,static_cast<StreamState*>(s)); }
uintptr_t Load1D10(void* s) { return Load1D10(this,static_cast<StreamState*>(s)); }
uintptr_t Save5140(void* s) { return Save5140(this,static_cast<StreamState*>(s)); }
uintptr_t Load51A0(void* s) { return Load51A0(this,static_cast<StreamState*>(s)); }
static int32_t Rotate23F0(void* out,void* bind,int32_t index,void* axis,float angle) { return Rotate23F0(out,static_cast<const void*>(bind),index,static_cast<const math::Float4Value*>(axis),angle); }
};
}
