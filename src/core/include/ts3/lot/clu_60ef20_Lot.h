#pragma once

#include <cstdint>

namespace ts3::lot {

// TS3W.exe 0x0060EF20, __thiscall (this, arg, optional record) -> int.
int32_t __fastcall UpdateLot(void* self, void* edx, void* argument, const void* record);

// TS3W.exe 0x0060EF70, __thiscall (this, optional record) -> int, ret 4.
int32_t __fastcall QueryLot(void* self, void* edx, const void* record);

}  // namespace ts3::lot
