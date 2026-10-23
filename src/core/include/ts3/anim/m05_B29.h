#pragma once
#include "ts3/layout.h"
namespace ts3::anim::b29 {
struct Ref { void** vtable; int32_t refs; };
struct Link { Link* next; Link* previous; };
struct Entry { Link link; Ref* controller; uint32_t first,second; float time,weight; uint8_t flags[2]; };
struct Group { void* vtable; Link list; void* descriptor; Ref* master; };
struct Observer { void* vtable; void* target; uintptr_t opaque[2]; uint32_t payload; };
TS3_X86_OFFSET(Group,list,4);
TS3_X86_OFFSET(Group,master,0x10);
TS3_X86_SIZE(Entry,0x20);
TS3_X86_OFFSET(Observer,payload,0x10);
// TS3W.exe 0x00582270: Append a retained controller entry; EAX combines first's upper bytes with the final flag.
uint32_t __fastcall Append(Group*,void*,Ref*,uint32_t,uint32_t,float,float,uint32_t,uint32_t);
// TS3W.exe 0x00582520: Construct an empty sequenced controller group without touching trailing bytes.
Group* __fastcall Sequence(Group*,void*);
// TS3W.exe 0x00582540: Destroy a sequenced group and optionally free it, returning this even after free.
Group* __fastcall DeleteSequence(Group*,void*,uint32_t);
// TS3W.exe 0x00582570: Construct an empty synchronized group with a null master.
Group* __fastcall Synchronize(Group*,void*);
// TS3W.exe 0x00582590: Release the master before destroying slaves, restoring last-release refs to one.
Group* __fastcall DeleteSynchronized(Group*,void*,uint32_t);
// TS3W.exe 0x005825E0: Forward five arguments to the scene operation with two zero flags.
uint32_t __fastcall Forward(void*,void*,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t);
// TS3W.exe 0x00582610: Forward five arguments to the scene operation with enable set and a zero final flag.
uint32_t __fastcall ForwardEnabled(void*,void*,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t);
// TS3W.exe 0x00582640: Notify the optional target with the embedded payload address.
void __fastcall Notify(Observer*,void*);
// TS3W.exe 0x00582690: Look up the selected binding using this address's distinct global selection mask.
void* __cdecl Find(const void*);
// TS3W.exe 0x00582830: Query slot11 on nonnull controllers until a nonzero low byte is returned.
uint8_t __fastcall Any11(Group*,void*);
// TS3W.exe 0x00582870: Query slot12 on nonnull controllers until a nonzero low byte is returned.
uint8_t __fastcall Any12(Group*,void*);
// TS3W.exe 0x005833A0: Count intrusive nodes, including nodes whose controller is null.
uint32_t __fastcall Count(const Group*,void*);
}
