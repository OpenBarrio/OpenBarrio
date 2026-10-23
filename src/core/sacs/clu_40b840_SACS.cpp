#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <cstdint>

#include "ts3/layout.h"
#include "ts3/sacs/clu_40b840_SACS.h"

namespace ts3::sacs {
namespace {

struct Utf16String {
    char16_t* begin;
    char16_t* end;
    char16_t* capacity;
    void* allocator;
};
TS3_X86_OFFSET(Utf16String, begin, 0x00);
TS3_X86_OFFSET(Utf16String, end, 0x04);
TS3_X86_OFFSET(Utf16String, capacity, 0x08);
TS3_X86_OFFSET(Utf16String, allocator, 0x0C);
TS3_X86_SIZE(Utf16String, 0x10);

struct ClipboardState {
    uint8_t enabled;
    uint8_t pad01[3];
    int32_t activePasteCount;
    int32_t blocked;
};
TS3_X86_OFFSET(ClipboardState, activePasteCount, 0x04);
TS3_X86_OFFSET(ClipboardState, blocked, 0x08);

}  // namespace

// 0x0040B840: Ensure space for one additional UTF-16 code unit, preserving the
// game's return convention (requested count when capacity suffices, allocator
// result when growth is required).
uintptr_t __fastcall ReserveClipboardString(void* string, void*, uint32_t requested)
{
    auto* value = static_cast<Utf16String*>(string);
    const uintptr_t begin = reinterpret_cast<uintptr_t>(value->begin);
    const uint32_t size = static_cast<uint32_t>((reinterpret_cast<uintptr_t>(value->end) - begin) / sizeof(char16_t));
    const uint32_t capacity =
        static_cast<uint32_t>((reinterpret_cast<uintptr_t>(value->capacity) - begin) / sizeof(char16_t));
    const uint32_t count = (requested < size ? size : requested) + 1;
    if (count > capacity) {
        char16_t* grown = ts3::CallGameMethod<char16_t*, 0x0040AB90>(value, count);
        return reinterpret_cast<uintptr_t>(grown);
    }
    return count;
}

// 0x004E9AA0: Import CF_UNICODETEXT into the destination string. GlobalSize
// includes the trailing NUL; the original loop uses that size as a limit and
// separately stops at the first NUL.
uint8_t __fastcall PasteClipboardText(void* state, void*, void* destination)
{
    auto* text = static_cast<Utf16String*>(destination);
    if (text->begin != text->end) {
        *text->begin = u'\0';
        text->end = text->begin;
    }

    auto* owner = static_cast<ClipboardState*>(state);
    uint8_t pasted = 0;
    if (!owner->enabled || owner->blocked)
        return pasted;

    ++owner->activePasteCount;
    if (OpenClipboard(nullptr)) {
        if (IsClipboardFormatAvailable(CF_UNICODETEXT)) {
            HANDLE data = GetClipboardData(CF_UNICODETEXT);
            if (data) {
                const auto* source = static_cast<const char16_t*>(GlobalLock(data));
                if (source) {
                    const SIZE_T length = GlobalSize(data) / sizeof(char16_t);
                    ReserveClipboardString(destination, nullptr, static_cast<uint32_t>(length));
                    for (SIZE_T i = 0; i < length && source[i] != u'\0'; ++i)
                        ts3::CallGameMethod<char16_t*, 0x00412CC0>(destination, source[i]);
                    GlobalUnlock(data);
                    pasted = 1;
                }
            }
        }
        CloseClipboard();
    }
    --owner->activePasteCount;
    return pasted;
}

}  // namespace ts3::sacs
