#pragma once
#include <cstdint>
namespace ts3::deviceconfig {
// TS3W.exe 0x004FE990: FormatProperty bridge.
char16_t* FormatProperty(int32_t value, void* buffer, int32_t radix);
// TS3W.exe 0x00740F00: GetProperty bridge.
uintptr_t GetProperty(uint32_t key, int32_t value);
}
