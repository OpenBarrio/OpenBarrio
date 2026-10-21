#pragma once
#include "ts3/runtime/m02_layouts.h"
namespace ts3::runtime::m02::c1 {
struct CallbackOwner { uint8_t prefix[16]; void* callback; };
TS3_X86_OFFSET(CallbackOwner,callback,0x10);
// TS3W.exe 0x004E2980: return the default thread wrapper pointer.
void* __cdecl DefaultWrapper();
// TS3W.exe 0x004E2990: return the alternate thread wrapper pointer.
void* __cdecl AlternateWrapper();
// TS3W.exe 0x004E2AE0: return the native HANDLE, or null for an empty facade.
void* __fastcall NativeHandle(Thread*,void*);
// TS3W.exe 0x004E3360: install the base vtable and optionally delete, returning self.
void* __fastcall DeleteThreadBase(void*,void*,uint32_t);
// TS3W.exe 0x00599160: return the scheduler singleton pointer.
void* __cdecl SchedulerSingleton();
// TS3W.exe 0x00599260: retain incoming callback before releasing old, even for self assignment; EAX is unspecified if both are null.
void __fastcall ReplaceCallback(CallbackOwner*,void*,void*);
// TS3W.exe 0x005993E0: return the secondary Service singleton pointer.
void* __cdecl ServiceSingleton();
// TS3W.exe 0x005994E0: store configuration through the secondary interface and return the argument in full EAX.
uint32_t __fastcall SetSecondaryConfig(void*,void*,uint32_t);
// TS3W.exe 0x007749C0: return the registration name address, not its contents.
const void* __fastcall RegistrationName(void*,void*);
// TS3W.exe 0x00774A40: stop a tag-0x10 managed watch; preserve invalid-tag EAX and callee EAX.
uint32_t __cdecl Stop(uint32_t);
// TS3W.exe 0x00774A80: return low EAX of elapsed integer ticks, zero on invalid/missing handle.
uint32_t __cdecl Elapsed(uint32_t);
// TS3W.exe 0x00774BB0: start a tag-0x10 managed watch; preserve raw EAX on every path.
uint32_t __cdecl Start(uint32_t);
// TS3W.exe 0x00774C30: restart a tag-0x10 managed watch; preserve raw EAX on every path.
uint32_t __cdecl Restart(uint32_t);
// TS3W.exe 0x00774D40: write the native binding table address, returning the output pointer.
const void** __fastcall RegistrationTable(void*,void*,const void**);
}
