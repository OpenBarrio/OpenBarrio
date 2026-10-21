#pragma once
#include "ts3/runtime/m02_stopwatch.h"
namespace ts3::runtime::m02::s1 {
PlatformDispatch* SetClock(PlatformDispatch*);
// TS3W.exe 0x004E3380: clear accumulation and sample a new origin; QPC returns its BOOL in EAX.
uint32_t __fastcall Restart(StopWatch*,void*);
// TS3W.exe 0x004F30E0: select mode and calibrated factor, retaining the raw EAX contract.
uint32_t __fastcall SetMode(StopWatch*,void*,uint32_t);
// TS3W.exe 0x004F32C0: read scaled elapsed units rounded with EA's near-half bias; does not restart.
uint64_t __fastcall ElapsedUInt(StopWatch*,void*);
// TS3W.exe 0x004F33C0: read floating elapsed units with unsigned wrap and minimum tick delta.
float __fastcall ElapsedFloat(StopWatch*,void*);
// TS3W.exe 0x004F3480: return a calibrated conversion factor, or one for other modes.
float __cdecl UnitFactor(uint32_t);
// TS3W.exe 0x004F34F0: set extended deadline from QPC, optionally starting the watch (low-byte flag).
uint32_t __fastcall SetDeadline(DeadlineTimer*,void*,uint32_t,uint32_t);
// TS3W.exe 0x004F35B0: initialize the watch and optionally start it (low-byte flag).
StopWatch* __fastcall Construct(StopWatch*,void*,uint32_t,uint32_t);
// TS3W.exe 0x00774AC0: look up a tagged handle and read floating elapsed units.
float __cdecl ManagedFloat(uint32_t);
// TS3W.exe 0x00774B80: clear registration singleton, restore base vptr, optionally free.
void* __fastcall DeleteRegistration(void*,void*,uint32_t);
// TS3W.exe 0x00774BF0: reset a tagged watch and return its pointer, or raw tag/null.
uintptr_t __cdecl Reset(uint32_t);
// TS3W.exe 0x00774C70: test start ticks; invalid tags clear AL only.
uint32_t __cdecl IsRunning(uint32_t);
// TS3W.exe 0x00774D00: remove a tagged handle and free its watch through the original allocator.
uintptr_t __cdecl Destroy(uint32_t);
}
