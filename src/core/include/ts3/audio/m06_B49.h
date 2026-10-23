#pragma once
#include "ts3/layout.h"
namespace ts3::audio {
// TS3W.exe 0x008AB310: Return the stored sound-state pointer at +AD4.
void* __fastcall B49310(void* self, void* unused);
// TS3W.exe 0x008AB320: Return the response scalar at +330.
float __fastcall B49320(void* self, void* unused);
// TS3W.exe 0x008AB330: Test indexed activity, applying the guarded +800 flag to the low byte of the guard argument.
uint8_t __fastcall B49330(void* self, void* unused, uint32_t index, uint32_t guarded);
// TS3W.exe 0x008AB370: Test signed activity minus the indexed byte, retaining the upper difference bytes in EAX.
uint32_t __fastcall B49370(void* self, void* unused, uint32_t index);
// TS3W.exe 0x008AB390: Test the +80 state bit, retaining upper flag bytes in EAX.
uint32_t __fastcall B49390(void* self, void* unused);
// TS3W.exe 0x008AB3B0: Return the double timestamp at +AC0.
double __fastcall B493B0(void* self, void* unused);
// TS3W.exe 0x008AB3C0: Evaluate virtual slot C0 with stored curve inputs and multiply by the +70 scalar.
double __fastcall B493C0(void* self, void* unused);
// TS3W.exe 0x008AB400: Evaluate virtual slot C0 only for mode 15, otherwise return one.
double __fastcall B49400(void* self, void* unused);
// TS3W.exe 0x008AB450: Forward two words through slot 2 of the embedded +F4 interface.
uint32_t __fastcall B49450(void* self, void* unused, uint32_t a, uint32_t b);
// TS3W.exe 0x008AB470: Publish the one-shot state before notifying the command system; already-set returns 128.
uint32_t __fastcall B49470(void* self, void* unused, uint32_t value, uint32_t flag);
// TS3W.exe 0x008AB4D0: Return one when either the +AD or +ACD state byte is nonzero.
uint32_t __fastcall B494D0(void* self, void* unused);
// TS3W.exe 0x008AB4F0: Return the complete raw byte at +B0 without boolean normalization.
uint8_t __fastcall B494F0(void* self, void* unused);
// TS3W.exe 0x008AB500: Return bit three of the mode byte.
uint8_t __fastcall B49500(void* self, void* unused);
// TS3W.exe 0x008AB510: Return the response scalar at +84.
float __fastcall B49510(void* self, void* unused);
}
