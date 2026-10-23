#include "ts3/pathplan/clu_66e120_PathPlan.h"
#include <cstring>

namespace ts3::pathplan {
// 0x0066E120: stdcall, ret 8; no globals, callees, or writes. Copying also
// accommodates keys at addresses only dword-aligned (no uint64_t alignment assumed).
#if defined(_M_IX86)
int32_t __stdcall CompareU64Pair(const void* a, const void* b)
#else
int32_t CompareU64Pair(const void* a, const void* b)
#endif
{
    PathPlanKey left, right;
    std::memcpy(&left, a, sizeof(left));
    std::memcpy(&right, b, sizeof(right));
    if (left.firstHigh != right.firstHigh) return left.firstHigh < right.firstHigh;
    if (left.firstLow != right.firstLow) return left.firstLow < right.firstLow;
    if (left.secondHigh != right.secondHigh) return left.secondHigh < right.secondHigh;
    return left.secondLow < right.secondLow;
}
} // namespace ts3::pathplan
