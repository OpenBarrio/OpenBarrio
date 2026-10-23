#pragma once
#include "ts3/layout.h"
namespace ts3::resource::m03::g06 {
// TS3W.exe 0x0059B860: map the service's inline locale to a static table locale; exact case, aliases and English fallback retained.
const char16_t* TableLocale(void* self);
#if defined(_M_IX86)
const char16_t* __fastcall HookTableLocale(void* self, void*);
#endif
}
