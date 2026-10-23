#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::resource::m03 {
struct d01 {
struct Leaves {
 void* (*service)()=nullptr;
 void* (*allocate)(size_t)=nullptr;
 uintptr_t (*update)(void*)=nullptr;
};
static Leaves& Dependencies();
// TS3W.exe 0x00733140: register cache debug controls, preserving repeated service lookups and final raw return.
static uintptr_t RegisterCache(void* self);
// TS3W.exe 0x00733D20: update the monitor at +0x30, ignoring both caller arguments.
static uintptr_t UpdateFirst(void* self,uintptr_t a,uintptr_t b);
// TS3W.exe 0x00733D30: update the monitor at +0xB0, ignoring both caller arguments.
static uintptr_t UpdateSecond(void* self,uintptr_t a,uintptr_t b);
// Raw thiscall hook adapter; this is a view, not an allocated C++ object.
uintptr_t UpdateSecond(uintptr_t a,uintptr_t b) { return UpdateSecond(this,a,b); }
};
}
