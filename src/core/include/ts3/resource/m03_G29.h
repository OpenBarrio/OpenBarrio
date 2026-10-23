#pragma once
#include <cstdint>
#include "ts3/layout.h"
namespace ts3::resource::m03::g29 {
struct Cache { void** vtable; void* database; };
TS3_X86_OFFSET(Cache,database,4);
// TS3W.exe 0x0073C910: Read a cached managed byte array; only AL is defined as the boolean result.
uint8_t __fastcall ReadBytes(Cache*,void*,uintptr_t name,uintptr_t* out);
// TS3W.exe 0x0073CA50: Open a cached stream, validate it and export a handle, retaining cleanup order.
uintptr_t __fastcall ReadHandle(Cache*,void*,uintptr_t name);
// TS3W.exe 0x0073CB60: Write the unsigned minimum of requested and managed-array bytes; returns incidental cleanup EAX.
uintptr_t __fastcall WriteBytes(Cache*,void*,uintptr_t name,uintptr_t array,uint32_t count);
// TS3W.exe 0x0073CC70: Copy a handle stream in 4096-byte blocks; roll back partial output and return cleanup EAX.
uintptr_t __fastcall WriteHandle(Cache*,void*,uintptr_t name,uintptr_t handle);
}
