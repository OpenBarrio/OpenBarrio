// Return comparison policy only. No entry is certified without its disassembly.
#pragma once
#include <cstdint>
#include "ts3/resource/m03_layouts.h"
namespace ts3::resource::m03 {
enum class ReturnMask : uint64_t { AL=0xff, AX=0xffff, EAX=0xffffffffULL, EDX_EAX=~0ULL };
constexpr bool SameReturn(uint64_t a,uint64_t b,ReturnMask mask) {
    return ((a^b)&static_cast<uint64_t>(mask))==0;
}
#if defined(_M_IX86)
// TS3W.exe 004AFD20: mov eax,[011C78D8]; ret (full pointer, cdecl).
using OriginalManager = ResourceManager* (__cdecl*)();
// TS3W.exe 0072EC00: allocator creates 0x14, returns saved pointer after unlock,
// plain ret at 0072EC60. See canonical DirectoryManager and verified accessor.
using OriginalDirectory = DirectoryManager* (__cdecl*)();
struct OriginalDispatch {
    OriginalManager manager=nullptr;
    OriginalDirectory directory=nullptr;
};
#endif
// No default mask: AL-vs-EAX and cleanup returns must be established per entry.
// 00726DB7 is an internal tail, explicitly has NO callable original typedef.
}
