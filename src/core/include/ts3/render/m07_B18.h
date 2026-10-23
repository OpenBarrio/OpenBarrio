#pragma once
#include "ts3/render/m07_abi.h"
namespace ts3::render::m07::b18 {
// Relocated originals/native fakes; null uses the scaffold's x86-only bridge.
extern void* (*resolve)(uint32_t);
struct Vector { void* begin; void* end; void* capacity; const char* tag; };
struct String { void* begin; void* end; void* capacity; const char* tag; };
struct Field { uint32_t word; String name; uint8_t tail[12]; };
struct Set { void* vtable; int32_t refs; uint32_t unknown; Vector modes,bytecode,fields; String name; };
struct Param { uint32_t a,b,type,min,max,epsilon; String name,semantic; uint32_t tail; };
struct Params { Vector array,data; String names[3]; };
struct Factory { uint8_t prefix[0x4c]; Params* params; };
struct Watcher { uint8_t mutexes[0x30]; int32_t refs; uint32_t gap34; void* first; void* last; String name; Vector notifiers; };
struct Shader { uint32_t key[2]; uint8_t gap08[0x20]; uint32_t flags; uint8_t tail[0xc]; };
struct Package { void* resource; void* second; void* third; uint8_t gap0c[0x10]; Vector shaders; };
struct Reader { uint8_t prefix[8]; void* stream; uint32_t gap0c; uint32_t endian; uint8_t good; };
struct State { uint8_t prefix[0x24]; void* zeros[9]; uint32_t arrays0[16],arrays1[16]; uint32_t zeroC8,zeroCC; uint32_t arrays2[16],arrays3[16]; void* zero150; void* zero154; uint8_t gap158[8]; void* zero160; uint32_t gap164; Vector vertex,pixel; void* owner; };
// TS3W.exe 0x0079C100: Read filtered shader blocks, retaining bad-stream and incidental EAX behavior.
uintptr_t __fastcall ReadPackage(Package*,void*,Reader*,uint32_t);
// TS3W.exe 0x0079C340: Reload matching shader identities and swap the three resource words.
uintptr_t __fastcall Reload(Package*,void*,Package*);
// TS3W.exe 0x0079C770: Resize 32-byte set records; preserve helper-dependent returns.
uintptr_t __fastcall Resize(Vector*,void*,uint32_t);
// TS3W.exe 0x0079E2C0: Initialize shader state while leaving all unaccessed holes intact.
State* __fastcall InitState(State*,void*,void*);
// TS3W.exe 0x0079FB80: Construct a refcounted shader set with empty tagged storage.
Set* __fastcall InitSet(Set*,void*);
// TS3W.exe 0x0079FBF0: Assign through a temporary wide string then destroy the temporary.
uintptr_t __fastcall SetName(Set*,void*,const void*);
// TS3W.exe 0x0079FC60: Destroy set storage in name/fields/bytecode/modes order.
uintptr_t __fastcall DestroySet(Set*,void*);
// TS3W.exe 0x0079FCE0: Destroy, optionally free for flags bit zero, and return the receiver.
Set* __fastcall DeleteSet(Set*,void*,uint32_t);
// TS3W.exe 0x0079FD80: Initialize a shader parameter with exact float bits and two empty strings.
Param* __fastcall InitParam(Param*,void*);
// TS3W.exe 0x007A0150: Initialize the shader factory parameter arrays and names.
Params* __fastcall InitParams(Params*,void*);
// TS3W.exe 0x007A04E0: Read parameter metadata and byte payload with versioned bounds.
uint8_t __fastcall ReadParams(Factory*,void*,void*,uint32_t,uint32_t);
// TS3W.exe 0x007A0E00: Initialize watcher mutexes, atomic reference and notifier storage.
Watcher* __fastcall InitWatcher(Watcher*,void*);
#if defined(_M_IX86)
static_assert(sizeof(Set)==0x4c && sizeof(Param)==0x3c && sizeof(Params)==0x50);
static_assert(offsetof(State,vertex)==0x168 && offsetof(State,owner)==0x188);
static_assert(sizeof(Watcher)==0x60 && offsetof(Package,shaders)==0x1c);
#endif
}
