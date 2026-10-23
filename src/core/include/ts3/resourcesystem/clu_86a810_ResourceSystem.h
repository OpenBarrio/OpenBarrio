#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::resourcesystem {
struct SongFilename { char16_t* begin; char16_t* end; char16_t* capacity; const char* category; };
struct CustomSong { SongFilename filename; uint8_t tag[128]; };
struct CustomSongVector {
    CustomSong *begin, *end, *capacity;
    void* allocator; uint32_t flags; const char* category;
};
TS3_X86_SIZE(SongFilename, 16);
TS3_X86_SIZE(CustomSong, 144);
TS3_X86_OFFSET(CustomSong, tag, 16);
TS3_X86_SIZE(CustomSongVector, 24);
// TS3W.exe 0x0086A810: enumerate custom MP3 files and append songs with a 128-byte trailing tag.
uintptr_t EnumerateCustomSongs(CustomSongVector* songs);
}
