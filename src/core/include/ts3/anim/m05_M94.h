#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::m94 {
struct State { State* next; State* previous; void* descriptor; unsigned char opaque[0x24]; float value; };
struct Output { unsigned char opaque[0x40]; uint32_t low,high,index; };
TS3_X86_OFFSET(State,value,0x30);
// TS3W.exe 0x00580510: Copy a target vector unless enable bit zero is set; EAX is not a return value.
void SetVector(LookAtTask*,const Float4Value*);
// TS3W.exe 0x00580530: Allocate and retain a task, rejecting signed-negative mode; null allocation still faults.
LookAtTask* __stdcall Create(void*,int32_t,uint32_t);
// TS3W.exe 0x005807C0: Free the owned target and optionally the task, returning this even after free.
LookAtTask* Destroy(LookAtTask*,uint32_t);
// TS3W.exe 0x00580A70: Initialize a sentinel, descriptor and scalar, leaving opaque storage untouched.
State* Initialize(State*);
// TS3W.exe 0x00580D20: Resolve the identifier via the manager and release its temporary retained resource.
void* Resolve(LookAtTarget*);
// TS3W.exe 0x00581090: Populate optional output fields; preserve duplicate queries and AL-only test.
void Populate(LookAtTask*,Output*);
}
namespace ts3::anim {
struct m94_hooks {
 void SetVector(const void* p) { m94::SetVector(reinterpret_cast<LookAtTask*>(this),static_cast<const Float4Value*>(p)); }
 void* Destroy(uint32_t f) { return m94::Destroy(reinterpret_cast<LookAtTask*>(this),f); }
 void* Initialize() { return m94::Initialize(reinterpret_cast<m94::State*>(this)); }
 void* Resolve() { return m94::Resolve(reinterpret_cast<LookAtTarget*>(this)); }
 void Populate(void* p) { m94::Populate(reinterpret_cast<LookAtTask*>(this),static_cast<m94::Output*>(p)); }
};
}
