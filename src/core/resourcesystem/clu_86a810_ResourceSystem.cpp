#include "ts3/resourcesystem/clu_86a810_ResourceSystem.h"
#include "ts3/world/clu_4ec290_World.h"
#include <cstring>
namespace ts3::resourcesystem {
namespace {
struct DirectoryEntry { DirectoryEntry *next, *previous; uint32_t type; const char16_t* path; };
struct DirectoryList { DirectoryEntry *first, *last; void* allocator; uint32_t size; const char* category; };
TS3_X86_SIZE(DirectoryEntry, 16);
TS3_X86_SIZE(DirectoryList, 20);
struct DirectoryIterator { void* state[4]; };
}
// 0x0086A810. The typed decompile loses the receiver on several thiscall edges
// and truncates the formatting argument to AL; the assembly passes the whole path.
uintptr_t EnumerateCustomSongs(CustomSongVector* songs)
{
    ts3::CallGameMethod<void, 0x0077F690>(songs, songs->begin, songs->end);
    songs->end = songs->begin;
    ts3::world::WorldPath root;
    root.Clear();
    auto* manager = ts3::CallGameFunction<void*, 0x0072F1C0>();
    auto* directory = ts3::CallGameVirtual<const uint16_t*, 9>(manager);
    root.Assign(directory);
#if defined(_M_IX86)
    const auto* suffix = *reinterpret_cast<const char16_t* const*>(uintptr_t(0x01168A58));
    auto* empty = reinterpret_cast<char16_t*>(uintptr_t(0x011C7E54));
    auto* emptyEnd = reinterpret_cast<char16_t*>(uintptr_t(0x011C7E56));
#else
    ts3::AbortNotRewritten("custom music directory globals");
    const char16_t* suffix = nullptr;
    char16_t* empty = nullptr;
    char16_t* emptyEnd = nullptr;
#endif
    ts3::CallGameMethod<void, 0x004EDB00>(&root, suffix);
    const uintptr_t exists = ts3::CallGameFunction<uintptr_t, 0x004DCFF0>(root.text);
    if (!static_cast<uint8_t>(exists)) return exists;
    DirectoryList entries;
    DirectoryIterator iterator{};
    auto* sentinel = reinterpret_cast<DirectoryEntry*>(&entries);
    entries.first = entries.last = sentinel;
    entries.allocator = ts3::CallGameFunction<void*, 0x004D9AD0>();
    entries.size = 0;
    entries.category = "ResourceSystem/ResourceMan/EntryList";
    ts3::CallGameMethod<void, 0x004DA3C0>(&iterator, root.text, &entries, nullptr, 2, 0x100000);
    for (auto* entry = entries.first; entry != sentinel; entry = entry->next) {
        ts3::world::WorldPath extension;
        ts3::CallGameMethod<void, 0x004EDA20>(&extension, entry->path);
        const auto* ext = ts3::CallGameMethod<const char16_t*, 0x004ED5E0>(&extension);
        const char16_t* wanted = u".mp3";
        bool matches = true;
        for (;;) {
            char16_t a = *ext++, b = *wanted++;
            if (a >= u'A' && a <= u'Z') a = static_cast<char16_t>(a + (u'a' - u'A'));
            if (a != b) { matches = false; break; }
            if (!a) break;
        }
        if (!matches) continue;
        CustomSong song;
        song.filename = {empty, empty, emptyEnd, "Audio/Playlist/Filename"};
        std::memset(song.tag, 0, sizeof song.tag);
        ts3::CallGameFunction<void, 0x0040E920>(&song.filename, u"%ls%ls", root.text, entry->path);
        auto* storage = ts3::CallGameFunction<void*, 0x004E3F90>(552, "Audio/CustomSongData/FileStream", 0, 0, 0, 0);
        if (storage) {
            auto* stream = ts3::CallGameMethod<void*, 0x004DB970>(storage, song.filename.begin);
            if (stream) {
                ts3::CallGameVirtual<void, 1>(stream);
                if (ts3::CallGameVirtual<uint8_t, 19>(stream, 1, 3, 1, 0)) {
                    ts3::CallGameVirtual<void, 10>(stream, 0, 2);
                    const uint32_t size = ts3::CallGameVirtual<uint32_t, 7>(stream);
                    if (size > 128) {
                        ts3::CallGameVirtual<void, 10>(stream, size - 128, 0);
                        ts3::CallGameVirtual<void, 12>(stream, song.tag, 128);
                        ts3::CallGameMethod<void, 0x0086A7C0>(songs, &song);
                    }
                    ts3::CallGameVirtual<void, 6>(stream);
                }
                ts3::CallGameVirtual<void, 2>(stream);
            }
        }
        const uintptr_t span = (reinterpret_cast<uintptr_t>(song.filename.capacity) - reinterpret_cast<uintptr_t>(song.filename.begin)) & ~uintptr_t(1);
        if (static_cast<intptr_t>(span) > 2 && song.filename.begin)
            ts3::CallGameFunction<void, 0x004E3F70>(song.filename.begin);
    }
    return ts3::CallGameMethod<uintptr_t, 0x00456CC0>(&entries);
}
}
