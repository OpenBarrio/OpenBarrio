#pragma once

#include <cstdint>

namespace ts3::usertool {

// TS3W.exe 0x00B99400: __thiscall (this, output pointer vector), ret 4.
int32_t __fastcall AppendUserToolValues(void* self, void* edx, void* output);

// TS3W.exe 0x00C04050: __cdecl (integer type), ret 0.
int32_t __cdecl IsAcceptedType(int32_t type);

}  // namespace ts3::usertool
