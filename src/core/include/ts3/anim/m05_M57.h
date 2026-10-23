#pragma once
#include "ts3/layout.h"
namespace ts3::anim::m57 {
// TS3W.exe 0x00570D80: Return the factory's fixed object size, 0x48.
uint32_t Size();
// TS3W.exe 0x00570D90: Construct a paired-key evaluator in supplied storage, or return null.
void* __stdcall ConstructPair(void* host,void* storage);
// TS3W.exe 0x00570DB0: Construct a scale evaluator in supplied storage, or return null.
void* __stdcall ConstructScale(void* host,void* storage);
// TS3W.exe 0x00570DD0: Allocate and construct a clip controller with positive-zero time, returning null on allocation failure.
void* __stdcall CreateClip(void* host);
// TS3W.exe 0x00570E10: Allocate and construct the 0x48-byte controller, returning null on allocation failure.
void* __stdcall CreateController(void* host);
// TS3W.exe 0x00570E40: Allocate and construct the two-controller wrapper, returning null on allocation failure.
void* __stdcall CreatePair(void* host);
}
