#pragma once
#include "ts3/anim/m05_M07.h"
#include "ts3/anim/m05_M10.h"
#include "ts3/anim/m05_M16.h"
#include "ts3/anim/m05_M17.h"
namespace ts3::anim::m55 {
// TS3W.exe 0x00570B10: Construct a scalar evaluator in nonnull storage, otherwise return null.
void* __stdcall CreateScalar(void* host, void* storage);
// TS3W.exe 0x00570B40: Construct a list blend evaluator in nonnull storage, otherwise return null.
void* __stdcall CreateBlend(void* host, void* storage);
// TS3W.exe 0x00570BC0: Construct a scalar evaluator and replace its vtable with the first specialized table.
void* __stdcall CreateFirst(void* host, void* storage);
// TS3W.exe 0x00570C40: Construct a scalar evaluator and replace its vtable with the second specialized table.
void* __stdcall CreateSecond(void* host, void* storage);
// TS3W.exe 0x00570C70: Construct a buffer evaluator in nonnull storage, otherwise return null.
void* __stdcall CreateBuffer(void* host, void* storage);
// TS3W.exe 0x00570CA0: Construct a constant transform evaluator in nonnull storage, otherwise return null.
void* __stdcall CreateConstant(void* host, void* storage);
}
