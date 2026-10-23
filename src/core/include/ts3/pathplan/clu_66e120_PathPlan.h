#pragma once
#include "ts3/layout.h"

namespace ts3::pathplan {
struct PathPlanKey {
    uint32_t firstLow, firstHigh, secondLow, secondHigh;
};
TS3_X86_SIZE(PathPlanKey, 16);
TS3_X86_OFFSET(PathPlanKey, firstLow, 0);
TS3_X86_OFFSET(PathPlanKey, firstHigh, 4);
TS3_X86_OFFSET(PathPlanKey, secondLow, 8);
TS3_X86_OFFSET(PathPlanKey, secondHigh, 12);
// TS3W.exe 0x0066E120: lexicographically compare two unsigned 64-bit key pairs; return 0 or 1.
#if defined(_M_IX86)
int32_t __stdcall CompareU64Pair(const void* a, const void* b);
#else
int32_t CompareU64Pair(const void* a, const void* b);
#endif
} // namespace ts3::pathplan
