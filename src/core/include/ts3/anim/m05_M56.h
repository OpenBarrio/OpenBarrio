#pragma once
#include <cstdint>
namespace ts3::anim::m56 {
// TS3W.exe 0x00570CD0: Construct in non-null storage, otherwise return null.
void* __stdcall CreateCD0(void* context, void* storage);
// TS3W.exe 0x00570CF0: Construct in non-null storage, otherwise return null.
void* __stdcall CreateCF0(void* context, void* storage);
// TS3W.exe 0x00570D10: Construct in non-null storage, otherwise return null.
void* __stdcall CreateD10(void* context, void* storage);
// TS3W.exe 0x00570D30: Construct in non-null storage, otherwise return null.
void* __stdcall CreateD30(void* context, void* storage);
// TS3W.exe 0x00570D60: Construct in non-null storage, otherwise return null.
void* __stdcall CreateD60(void* context, void* storage);
// TS3W.exe 0x00570D50: Return the Scratch object size (0x34).
uint32_t __fastcall Size(void* self, void* unused);
}
