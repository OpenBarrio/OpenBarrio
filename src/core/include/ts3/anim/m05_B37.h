#pragma once
#include <cstdint>
#include <cstddef>
namespace ts3::anim::b37 {
// Only consumed fields are named; opaque storage is not a complete class contract.
struct Controller {
 void* vtable;
 unsigned char opaque04[0x14];
 void* secondaryVtable;
 unsigned char opaque1C[0x8c];
 float valueA8, valueAC;
};
#if defined(_M_IX86)
static_assert(offsetof(Controller,secondaryVtable)==0x18);
static_assert(offsetof(Controller,valueA8)==0xa8);
static_assert(offsetof(Controller,valueAC)==0xac);
#endif
// TS3W.exe 0x008557E0: Query the secondary interface identifier.
void* __fastcall QueryInterface(void* self, void*, uint32_t id);
// TS3W.exe 0x00855800: Query the base controller interfaces.
void* __fastcall QueryBase(void* self, void*, uint32_t id);
// TS3W.exe 0x00855830: Read the float at x86 offset AC.
float __fastcall GetAC(Controller* self, void*);
// TS3W.exe 0x00855840: Read the float at x86 offset A8.
float __fastcall GetA8(Controller* self, void*);
// TS3W.exe 0x00855850: Return the turn-controller type identifier.
uint32_t __fastcall TypeTurn(void*, void*);
// TS3W.exe 0x00855860: Query turn-controller interfaces.
void* __fastcall QueryTurn(void* self, void*, uint32_t id);
// TS3W.exe 0x008558B0: Adjust the secondary interface and return the turn type.
uint32_t __fastcall TypeTurnInterface(void* self, void*);
// TS3W.exe 0x008558C0: Adjust the secondary interface and query turn interfaces.
void* __fastcall QueryTurnInterface(void* self, void*, uint32_t id);
// TS3W.exe 0x008558D0: Write sixteen zero bytes; preserve pointer high bits in the return register.
uintptr_t __fastcall ZeroVector(void*, void*, float* output);
// TS3W.exe 0x00856610: Return the vehicle-controller type identifier.
uint32_t __fastcall TypeVehicle(void*, void*);
// TS3W.exe 0x00856620: Query vehicle-controller interfaces.
void* __fastcall QueryVehicle(void* self, void*, uint32_t id);
// TS3W.exe 0x00856660: Adjust the secondary interface and query vehicle interfaces.
void* __fastcall QueryVehicleInterface(void* self, void*, uint32_t id);
// TS3W.exe 0x00856670: Adjust the secondary interface and return the vehicle type.
uint32_t __fastcall TypeVehicleInterface(void* self, void*);
// TS3W.exe 0x00856B00: Return the alternate vehicle-controller type identifier.
uint32_t __fastcall TypeOtherVehicle(void*, void*);
// TS3W.exe 0x00856B10: Query alternate vehicle-controller interfaces.
void* __fastcall QueryOtherVehicle(void* self, void*, uint32_t id);
}
