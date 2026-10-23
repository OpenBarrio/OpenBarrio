#pragma once

#include <cstdint>

namespace ts3::sacs {

// TS3W.exe 0x00582090: __thiscall (this, key, object), ret 8.
uint8_t __fastcall AddUniqueObject(void* self, void* edx, void* key, void* object);

// TS3W.exe 0x00582320: __thiscall (this, source, values, floats, flags), ret 0x1c.
uint8_t __fastcall UpdateEntry(void* self, void* edx, void* source, uint32_t value20, uint32_t value24,
                               float value28, float value32, uint8_t flag36, uint8_t flag37);

}  // namespace ts3::sacs
