#include "ts3/misc/clu_c7e680_Misc.h"

namespace ts3::misc {
// 0x00C7E680: signed jle/jge, not unsigned comparisons suggested by Hex-Rays.
OverlapItem* MarkOverlaps(OverlapList* self, const int32_t* rectangle, OverlapItem* incoming)
{
    OverlapItem* result = incoming;
    const auto end = self->end; // ESI snapshots the end pointer before any writes.
    for (auto cursor = self->begin; cursor != end; ++cursor) {
        result = *cursor;
        if (result->right > rectangle[0] && result->left < rectangle[2] &&
            result->bottom > rectangle[1] && result->top < rectangle[3]) {
            result->marked = 1;
        }
    }
    return result;
}
#if defined(_M_IX86)
// The only assembly is ABI glue for the nonstandard incoming-EAX contract.
__declspec(naked) OverlapItem* __fastcall MarkOverlapEntry(OverlapList*, void*, const int32_t*)
{
    __asm {
        push eax
        push dword ptr [esp + 8]
        push ecx
        call MarkOverlaps
        add esp, 12
        ret 4
    }
}
#endif
} // namespace ts3::misc
