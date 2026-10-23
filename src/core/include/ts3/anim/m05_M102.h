#pragma once
#include <cstdint>
namespace ts3::anim::m102 {
// TS3W.exe 0x00584E70: Decode neighboring compressed keys; hold the first value at flagged or out-of-range successors.
uintptr_t Sample6(uint32_t key, float* first, uint16_t* firstFlags, uint32_t limit, float* second, uint16_t* secondFlags, const void* table, const unsigned char* records, uint32_t count);
// TS3W.exe 0x00584F30: Decode neighboring compressed keys; hold the first value at flagged or out-of-range successors.
uintptr_t Sample8(uint32_t key, float* first, uint16_t* firstFlags, uint32_t limit, float* second, uint16_t* secondFlags, const void* table, const unsigned char* records, uint32_t count);
// TS3W.exe 0x00584FF0: Decode neighboring compressed keys; hold the first value at flagged or out-of-range successors.
uintptr_t Sample10(uint32_t key, float* first, uint16_t* firstFlags, uint32_t limit, float* second, uint16_t* secondFlags, const void* table, const unsigned char* records, uint32_t count);
// TS3W.exe 0x005850B0: Decode neighboring compressed keys; hold the first value at flagged or out-of-range successors.
uintptr_t Sample12(uint32_t key, float* first, uint16_t* firstFlags, uint32_t limit, float* second, uint16_t* secondFlags, const void* table, const unsigned char* records, uint32_t count);
// TS3W.exe 0x00585170: Decode neighboring compressed keys; hold the first value at flagged or out-of-range successors.
uintptr_t Sample6B(uint32_t key, float* first, uint16_t* firstFlags, uint32_t limit, float* second, uint16_t* secondFlags, const void* table, const unsigned char* records, uint32_t count);
// TS3W.exe 0x00585230: Decode neighboring compressed keys; hold the first value at flagged or out-of-range successors.
uintptr_t Sample6C(uint32_t key, float* first, uint16_t* firstFlags, uint32_t limit, float* second, uint16_t* secondFlags, const void* table, const unsigned char* records, uint32_t count);
}
