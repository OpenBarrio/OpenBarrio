// Unit clu_4dfd60_UI (UI): the last-path-component locator over a word range.
// Semantics from the TS3W.exe 1.67.2 listing of 0x004DFD60 (129 bytes, cdecl, leaf).
#include "ts3/ui/clu_4dfd60_UI.h"

namespace ts3::ui {

namespace {

// The original compares `end` against the 32-bit value of the game global
// 0x011C7E2C. In the real game image that value is (WORD*)-1 (all 0xFF) and
// means "the NUL terminator of the string at begin"; the raw file holds 0 there,
// so read the global from the mapped image to match the original in every
// environment (the project pattern for game globals, cf. cls_723c70_UI).
constexpr uintptr_t kSentinelGlobal = 0x11C7E2C;

inline bool IsSentinel(uint16_t* end)
{
    return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end)) ==
           *reinterpret_cast<const uint32_t*>(kSentinelGlobal);
}

}  // namespace

uint16_t* FindLastPathComponent(uint16_t* begin, uint16_t* end)
{
    // TS3W.exe 0x004DFD60: `end` == 0x011C7E2C means "the NUL terminator of the
    // string at begin"; otherwise `end` is the range end. The string is a word
    // array (each character a 16-bit word, char in the low byte, 0 in the high
    // byte), so the scan advances whole words.
    uint16_t* result = end;
    if (IsSentinel(end))
    {
        result = begin;
        while (*result != 0)
            ++result;
    }
    // 0x4DFD89-0x4DFD9B: skip the backward scan when begin >= result, or when the last
    // word of the range is already a separator ('\\' or '/' - ':' is not checked here).
    if (begin >= result || (*(result - 1) != 0x5C) && (*(result - 1) != 0x2F))
    {
        // 0x4DFD9D-0x4DFDC4: scan the words of [begin, result) from the back; i ends just
        // after the last separator ('\\', '/' or ':'), or at begin when there is none.
        uint16_t* i = result;
        while (i > begin)
        {
            const uint16_t v = *(i - 1);
            if (v == 0x5C || v == 0x2F || v == 0x3A)
                break;
            --i;
        }
        // 0x4DFDC5-0x4DFDDB: return i, except when i is just after a leading "\\" and the
        // range reaches past it - in that case the original returns result (end).
        if (i != begin + 2 || begin + 2 > result || *begin != 0x5C || begin[1] != 0x5C)
            return i;
    }
    return result;
}

}  // namespace ts3::ui
