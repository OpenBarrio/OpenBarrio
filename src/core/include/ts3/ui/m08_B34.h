#pragma once
#include <cstdint>
#include <cstddef>
namespace ts3::ui::b34 {
// Sparse native grid interface: original +0 vptr, +4 flags; subsequent scalar offsets grow with vptr.
struct Grid { void** table; uint32_t flags; uint8_t rest[0x180]; };
struct Record { const void* descriptor; void* owner; uint32_t count, type; };
struct Glide { void** primary; void** secondary; uint8_t gap08[8]; void** effect; uint8_t gap14[0x5c]; void** tail; float xyz[3]; };
struct FactoryServices { void* (*allocate)(size_t,int,const char*,uint32_t); void (*construct)(void*); const void* const* tables; };
// TS3W.exe 0x005235A0: populate a registration record and return slot 5's type.
uint32_t __fastcall Register(void*,void*,Record*);
// TS3W.exe 0x00523650: allocate and construct a GlideEffect; allocation failure returns null.
void* __stdcall Create(uint32_t,uint32_t);
void SetFactoryServices(const FactoryServices*);
// TS3W.exe 0x005237C0: query the grid interface or delegate to the original base.
void* __fastcall Query(void*,void*,uint32_t);
// TS3W.exe 0x005237F0: return the grid class identifier.
uint32_t __fastcall Type(void*,void*);
// TS3W.exe 0x00523830: modify flag bits through slot 10; only the low enable byte is tested.
uint32_t __fastcall Modify(Grid*,void*,uint32_t,uint32_t);
// TS3W.exe 0x00523870: read the scalar at +160; preserve incidental upper EAX bits.
uint32_t __fastcall Read(Grid*,void*,uint32_t*);
// TS3W.exe 0x00523890: set one of eight scalar entries; unsigned bound and AL-only result preserved.
uint32_t __fastcall SetEntry(Grid*,void*,uint32_t,uint32_t);
// TS3W.exe 0x005238B0: read one of eight scalar entries; invalid indices leave output untouched.
uint32_t __fastcall GetEntry(Grid*,void*,uint32_t*,uint32_t);
// TS3W.exe 0x005238D0: forward the 800 flag query to slot 12.
uint32_t __fastcall Enabled(Grid*,void*);
// TS3W.exe 0x005238E0: copy four floats in load/store order, including aliased outputs.
uint32_t __fastcall RectA(Grid*,void*,float*,float*,float*,float*);
// TS3W.exe 0x00523910: copy the second four-float group, preserving aliasing and AL-only result.
uint32_t __fastcall RectB(Grid*,void*,float*,float*,float*,float*);
// TS3W.exe 0x00523940: copy the third four-float group, preserving aliasing and AL-only result.
uint32_t __fastcall RectC(Grid*,void*,float*,float*,float*,float*);
// TS3W.exe 0x00523970: call slot 11 then store +80 and return the stored value.
uint32_t __fastcall SetMode(Grid*,void*,uint32_t,uint32_t);
// TS3W.exe 0x005239B0: store +168 and replace only AL with one.
uint32_t __fastcall SetValue(Grid*,void*,uint32_t);
}
