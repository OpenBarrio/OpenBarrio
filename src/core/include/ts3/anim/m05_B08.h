#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::b08 {
struct StreamState { void* vtable; uint32_t value4; void* handle; uint32_t valueC; void* extra; uint8_t good; };
struct DelegateNode { ListLink links; void* target; void* payload; };
struct DelegateList { void* vtable; ListLink links; uint32_t depth; };
// TS3W.exe 0x00565780: Write a full dword through the stream wrapper.
int32_t Write(StreamState*, uint32_t);
// TS3W.exe 0x005657A0: Save an optional controller and its type tag.
uint8_t SaveOptional(Controller*, StreamState*);
// TS3W.exe 0x00565830: Save controller base state; preserves the stack pointer's high bytes in the latch argument.
uint8_t Save(Controller*, StreamState*);
// TS3W.exe 0x005658C0: Load controller base state, rejecting versions above one.
uint8_t Load(Controller*, StreamState*);
// TS3W.exe 0x00565950: Initialize a retained controller pointer without releasing old storage.
Controller** Retain(Controller**, Controller*);
// TS3W.exe 0x00565990: Leave dispatch and purge tombstones only at depth zero.
void Leave(DelegateList*);
// TS3W.exe 0x005659D0: Destroy all delegate nodes and reset the sentinel.
void Clear(DelegateList*);
// TS3W.exe 0x00565A10: Clear and optionally free a delegate-list object, returning this.
DelegateList* DeleteList(DelegateList*, uint32_t);
// TS3W.exe 0x00565A30: Notify once, snapshotting next before each callback.
void Notify(Controller*, void*);
// TS3W.exe 0x00565A80: Initialize controller base with a postincremented global ID.
Controller* Construct(Controller*, void*);
// TS3W.exe 0x00565AE0: Return the controller interface type ID.
uint32_t Type(void*);
// TS3W.exe 0x00565AF0: Query the secondary interface and return its enclosing controller.
Controller* Query(void*, uint32_t);
// TS3W.exe 0x00565B30: Adjust delegate interface this and invoke deleting destruction.
Controller* DeleteInterface(void*, uint32_t);
// TS3W.exe 0x00565BB0: Dispatch destruction notification with temporary references and last-release restoration.
void ReleaseNotify(Controller*);
// TS3W.exe 0x00565C40: Destroy controller delegates and optionally free, returning this after free.
Controller* Delete(Controller*, uint32_t);
// Explicit ECX/dummy-EDX adapters preserve the original x86 thiscall stack cleanup.
#if defined(_M_IX86)
inline int32_t __fastcall HookWrite(StreamState* s, void*, uint32_t v) { return Write(s,v); }
inline uint8_t __fastcall HookSave(Controller* s, void*, StreamState* v) { return Save(s,v); }
inline uint8_t __fastcall HookLoad(Controller* s, void*, StreamState* v) { return Load(s,v); }
inline Controller** __fastcall HookRetain(Controller** s, void*, Controller* v) { return Retain(s,v); }
inline void __fastcall HookLeave(DelegateList* s, void*) { return Leave(s); }
inline void __fastcall HookClear(DelegateList* s, void*) { return Clear(s); }
inline DelegateList* __fastcall HookDeleteList(DelegateList* s, void*, uint32_t v) { return DeleteList(s,v); }
inline void __fastcall HookNotify(Controller* s, void*, void* v) { return Notify(s,v); }
inline Controller* __fastcall HookConstruct(Controller* s, void*, void* v) { return Construct(s,v); }
inline uint32_t __fastcall HookType(void* s, void*) { return Type(s); }
inline Controller* __fastcall HookQuery(void* s, void*, uint32_t v) { return Query(s,v); }
inline Controller* __fastcall HookDeleteInterface(void* s, void*, uint32_t v) { return DeleteInterface(s,v); }
inline void __fastcall HookReleaseNotify(Controller* s, void*) { return ReleaseNotify(s); }
inline Controller* __fastcall HookDelete(Controller* s, void*, uint32_t v) { return Delete(s,v); }
#endif
}
