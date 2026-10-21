#pragma once
#include "ts3/runtime/m02_layouts.h"
namespace ts3::runtime::m02 {
using WatchUInt = uint64_t (__fastcall*)(StopWatch*,void*);
using WatchFloat = float (__fastcall*)(StopWatch*,void*);
using WatchSetUInt = uint64_t (__fastcall*)(StopWatch*,void*,uint64_t);
using WatchSetFloat = uint64_t (__fastcall*)(StopWatch*,void*,float);
using ManagedUInt = uint32_t (__cdecl*)(uint32_t);
using ManagedFloat = float (__cdecl*)(uint32_t);
constexpr uint32_t stopWatchTag=0x10;
}
