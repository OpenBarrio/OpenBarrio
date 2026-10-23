// Cluster unit clu_70d020_CAS: maps each input dword to its index in an object's table.
#pragma once

#include <cstdint>
#include "ts3/layout.h"

namespace ts3::cas {

struct CasValueMap {
    uint8_t pad00[0x28];
    uint32_t count;
    const uint32_t* values;

    // TS3W.exe 0x0070D020: replace each input value with its first table index, or -1.
    void MapValues(int32_t* indices, uint32_t length) const;
};
#if defined(_M_IX86)
TS3_X86_OFFSET(CasValueMap, count, 0x28);
TS3_X86_OFFSET(CasValueMap, values, 0x2C);
#endif

}  // namespace ts3::cas

// Hook-compatible thiscall replacement entry (fastcall carries the this pointer in ECX).
void __fastcall MapValues(ts3::cas::CasValueMap* self, void* edx, int32_t* indices, uint32_t length);
