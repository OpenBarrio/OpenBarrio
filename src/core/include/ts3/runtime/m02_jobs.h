#pragma once
#include "ts3/runtime/m02_layouts.h"
namespace ts3::runtime::m02 {
using JobCallback = void (__fastcall*)(void*,void*);
using SchedulerMethod = uintptr_t (__fastcall*)(Scheduler*,void*,void*);
struct ServiceBoundary { void* context; void* (*timeManager)(void*); void (*schedule)(void*,void*,uint32_t); };
namespace slots {
constexpr unsigned schedulerCount=11, serviceCount=13;
constexpr unsigned registerBindings=18, registerHandle=50, lookupHandle=51, removeHandle=54;
}
using RegisterHandle = uint32_t (__fastcall*)(void*,void*,void*,uint32_t);
using LookupHandle = void* (__fastcall*)(void*,void*,uint32_t);
using RegisterBindings = uint32_t (__fastcall*)(void*,void*,void*);
}
