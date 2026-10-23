#pragma once
#include "ts3/layout.h"
namespace ts3::audio::b16 {
struct Modifier { void* vtable; uint32_t state; int32_t property; };
TS3_X86_OFFSET(Modifier, state, 4);
TS3_X86_OFFSET(Modifier, property, 8);
// TS3W.exe 0x0086B9D0: evaluate the event modifier; preserves EA truncation and fallback behavior.
uintptr_t __fastcall F86B9D0(Modifier* self, void*, void* context, char* output, uint32_t capacity, int32_t extra);
// TS3W.exe 0x0086BAB0: initialize the modifier, retaining untouched tail bytes.
Modifier* __fastcall F86BAB0(Modifier* self, void*);
// TS3W.exe 0x0086BAC0: evaluate the event modifier; preserves EA truncation and fallback behavior.
uintptr_t __fastcall F86BAC0(Modifier* self, void*, void* context, char* output, uint32_t capacity, int32_t extra);
// TS3W.exe 0x0086BB80: evaluate the event modifier; preserves EA truncation and fallback behavior.
uintptr_t __fastcall F86BB80(Modifier* self, void*, void* context, char* output, uint32_t capacity, int32_t extra);
// TS3W.exe 0x0086BE50: initialize the modifier, retaining untouched tail bytes.
Modifier* __fastcall F86BE50(Modifier* self, void*);
// TS3W.exe 0x0086BE60: initialize the modifier, retaining untouched tail bytes.
Modifier* __fastcall F86BE60(Modifier* self, void*);
// TS3W.exe 0x0086BE70: evaluate the event modifier; preserves EA truncation and fallback behavior.
uintptr_t __fastcall F86BE70(Modifier* self, void*, void* context, char* output, uint32_t capacity, int32_t extra);
// TS3W.exe 0x0086BFC0: initialize the modifier, retaining untouched tail bytes.
Modifier* __fastcall F86BFC0(Modifier* self, void*);
// TS3W.exe 0x0086BFD0: evaluate the event modifier; preserves EA truncation and fallback behavior.
uint32_t F86BFD0(char* output, const char* text, uint32_t capacity);
// TS3W.exe 0x0086C000: evaluate the event modifier; preserves EA truncation and fallback behavior.
uintptr_t __fastcall F86C000(Modifier* self, void*, void* context, char* output, uint32_t capacity, int32_t extra);
// TS3W.exe 0x0086C1A0: initialize the modifier, retaining untouched tail bytes.
Modifier* __fastcall F86C1A0(Modifier* self, void*, int32_t property);
// TS3W.exe 0x0086C1C0: evaluate the event modifier; preserves EA truncation and fallback behavior.
uintptr_t __fastcall F86C1C0(Modifier* self, void*, void* context, char* output, uint32_t capacity, int32_t extra);
// TS3W.exe 0x0086C300: initialize the modifier, retaining untouched tail bytes.
Modifier* __fastcall F86C300(Modifier* self, void*);
}
