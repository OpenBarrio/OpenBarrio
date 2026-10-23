#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::anim::b39 {
// Only the fields accessed by this batch; opaque bytes are never initialized.
struct Vehicle {
    unsigned char prefix[0x14]; void* host;
    unsigned char gap18[0x81-0x18]; uint8_t mode;
    unsigned char gap82[0xac-0x82]; float speed, target;
    unsigned char gapB4[0xc8-0xb4]; void* route; uint8_t flags;
};
TS3_X86_OFFSET(Vehicle, host, 0x14);
TS3_X86_OFFSET(Vehicle, target, 0xb0);
TS3_X86_OFFSET(Vehicle, route, 0xc8);
struct Range { void* begin; void* end; };
struct StringRecord { const char16_t* begin; const char16_t* end; const char16_t* capacity; const void* descriptor; };
// TS3W.exe 0x00859280: set the target speed without canonicalizing its bits.
void __fastcall SetTarget(Vehicle*, void*, float);
// TS3W.exe 0x008596B0: stop movement and notify the host in original order.
uintptr_t __fastcall Stop(Vehicle*, void*);
// TS3W.exe 0x00859720: replace the current route, skipping an empty range.
uintptr_t __fastcall SetRoute(Vehicle*, void*, const Range*, uint32_t);
// TS3W.exe 0x00859770: add the quaternion-rotated lateral offset to the host position.
uintptr_t __fastcall Position(void*, void*, float*);
// TS3W.exe 0x00859840: copy the aligned four-lane interface value and return the output.
float* __fastcall CopyValue(void*, void*, float*);
// TS3W.exe 0x00859860: initialize Cruise and Decelerate descriptor string records.
StringRecord* __fastcall InitNames(StringRecord*, void*);
// TS3W.exe 0x00859940: return the vehicle controller type identifier.
uint32_t __fastcall VehicleType(void*, void*);
// TS3W.exe 0x00859960: return the same identifier from the secondary interface.
uint32_t __fastcall SecondaryType(void*, void*);
// TS3W.exe 0x0085A7E0: forward to vtable slot three and preserve its return.
uintptr_t __fastcall Forward(void*, void*);
// TS3W.exe 0x0085A800: return the accumulator type identifier.
uint32_t __fastcall AccumulatorType(void*, void*);
// TS3W.exe 0x0085A810: query either supported interface without adjusting this.
void* __fastcall Query(void*, void*, uint32_t);
// TS3W.exe 0x0085A840: clear all four accumulator lanes.
void __fastcall Clear(void*, void*);
// TS3W.exe 0x0085A850: accumulate a scaled vector and return the input pointer, including aliases.
const float* __fastcall Accumulate(void*, void*, const float*, float);
// TS3W.exe 0x0085A880: reset extrema, priority and latch bytes; return AL=false.
bool __fastcall Reset(void*, void*);
// TS3W.exe 0x0085A8C0: latch the low byte of the request and minimize signed priority.
int32_t __fastcall Request(void*, void*, int32_t, uint32_t);
}
