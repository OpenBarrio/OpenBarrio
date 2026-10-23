#pragma once
#include "ts3/resource/m03_manager_contracts.h"
namespace ts3::resource::m03 {
using namespace manager;
namespace a03 {
#if defined(_M_IX86)
struct Vector { void* begin; void* end; void* capacity; void* allocator; const char* name; uint32_t flags; };
TS3_X86_SIZE(Vector,24);
// TS3W.exe 0x004B0960: notify filters with the database lock dropped around callbacks; returns final unlock EAX.
int32_t Notify(ResourceManager*,void*,const KeyRange*);
// TS3W.exe 0x004B0A60: release database references and free capacity rounded down to stride8, leaving the vector header stale.
uint32_t DestroyDatabases(Vector*);
// TS3W.exe 0x004B0AB0: release primary callback references and free capacity, leaving the header stale.
uint32_t DestroyCallbacks(Vector*);
// TS3W.exe 0x004B0B40: copy entries, retaining the inline interlocked count; null cursor still advances.
CallbackEntry** CopyInline(CallbackEntry**,const CallbackEntry*,const CallbackEntry*,CallbackEntry*);
// TS3W.exe 0x004B0B90: assign database entries forward, retaining before publishing and releasing.
DatabaseEntry* AssignDatabases(const DatabaseEntry*,const DatabaseEntry*,DatabaseEntry*);
// TS3W.exe 0x004B0BF0: assign callback entries forward, retaining before publishing and releasing.
CallbackEntry* AssignCallbacks(const CallbackEntry*,const CallbackEntry*,CallbackEntry*);
// TS3W.exe 0x004B0C50: assign database entries backward and return destination begin.
DatabaseEntry* BackwardDatabases(const DatabaseEntry*,const DatabaseEntry*,DatabaseEntry*);
// TS3W.exe 0x004B0CB0: assign callback entries backward and return destination begin.
CallbackEntry* BackwardCallbacks(const CallbackEntry*,const CallbackEntry*,CallbackEntry*);
// TS3W.exe 0x004B0D10: scan UTF16 type names case-insensitively under mutex, returning FFFFFFFF on miss.
uint32_t FindType(ResourceManager*,const char16_t*);
// TS3W.exe 0x004B0DD0: erase a filter node, free 32 bytes, and return successor iterator.
FilterNode** Erase(FilterSet*,FilterNode**,FilterNode*);
// TS3W.exe 0x004B0E10: release inline counts, resetting last reference to one before deleting; empty EAX unspecified.
uintptr_t ReleaseInline(const CallbackEntry*,const CallbackEntry*);
// TS3W.exe 0x004B0E60: assign inline-owned entry, retaining before publication and copying flags last.
CallbackEntry* AssignInline(CallbackEntry*,const CallbackEntry*);
#endif
}
#if defined(_M_IX86)
// TS3W.exe 0x004B0960: thiscall-compatible notification hook.
int32_t __fastcall A03Notify(ResourceManager*,void*,void*,const manager::KeyRange*);
// TS3W.exe 0x004B0A60: thiscall-compatible database vector destruction hook.
uint32_t __fastcall A03DestroyDatabases(a03::Vector*,void*);
// TS3W.exe 0x004B0AB0: thiscall-compatible callback vector destruction hook.
uint32_t __fastcall A03DestroyCallbacks(a03::Vector*,void*);
// TS3W.exe 0x004B0D10: thiscall-compatible type-name lookup hook.
uint32_t __fastcall A03FindType(ResourceManager*,void*,const char16_t*);
// TS3W.exe 0x004B0DD0: thiscall-compatible filter erasure hook.
manager::FilterNode** __fastcall A03Erase(manager::FilterSet*,void*,manager::FilterNode**,manager::FilterNode*);
// TS3W.exe 0x004B0E10: thiscall-compatible release hook; unused ECX is discarded.
uintptr_t __fastcall A03Release(void*,void*,const manager::CallbackEntry*,const manager::CallbackEntry*);
// TS3W.exe 0x004B0E60: thiscall-compatible inline assignment hook.
manager::CallbackEntry* __fastcall A03Assign(manager::CallbackEntry*,void*,const manager::CallbackEntry*);
#endif
}
