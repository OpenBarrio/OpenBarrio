#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::audio::b14impl {
struct String { char16_t *begin, *end, *capacity; const char* label; };
struct Track { String name; uint8_t data[128]; };
struct Tracks { Track *begin, *end, *capacity; const char* label; };
struct Playlist {
 void *vtable, *secondary, *notification; int32_t refs; uint32_t flags;
 void *owner, *p18, *p1c, *p20, *p24, *p28, *p2c;
 float f30,f34,f38,f3c,f40,f44,f48,f4c,f50,f54,f58;
 void* player; uint8_t buffer[0x4e200]; Tracks tracks; uint32_t index; String name;
};
TS3_X86_OFFSET(Playlist, tracks, 0x4e260);
TS3_X86_OFFSET(Playlist, name, 0x4e274);
TS3_X86_SIZE(Track, 0x90);
// TS3W.exe 0x0086A030: Assigns the playlist name only for the recognized property.
bool SetName(Playlist*, uint32_t, const char16_t*, uint32_t);
// TS3W.exe 0x0086A080: Initializes playlist state without touching the PCM buffer.
Playlist* Construct(Playlist*);
// TS3W.exe 0x0086A170: Adjusts notification this by two pointers before deleting.
Playlist* DeleteNotification(void*, uint32_t);
// TS3W.exe 0x0086A180: Releases player, strings, tracks and owner in original order.
void Destruct(Playlist*);
// TS3W.exe 0x0086A220: Destroys the playlist and optionally frees its allocation.
Playlist* Delete(Playlist*, uint32_t);
// TS3W.exe 0x0086A240: Runs the original forward Fisher-Yates swaps.
void Shuffle(Track*, Track*, uint32_t (**)(uint32_t));
// TS3W.exe 0x0086A2B0: Inserts a track, preserving aliased source adjustment.
void Insert(Tracks*, Track*, const Track*);
// TS3W.exe 0x0086A7C0: Publishes the advanced end before copying the new track.
void Append(Tracks*, const Track*);
// TS3W.exe 0x0086AA80: Refreshes and shuffles tracks, resetting the current index.
bool Refresh(Playlist*);
// TS3W.exe 0x0086AAD0: Selects an override or advances the circular track index.
bool NextName(Playlist*, String*);
// TS3W.exe 0x0086AB60: Attempts each filename until the player accepts one.
bool OpenNext(Playlist*);
// TS3W.exe 0x0086AC60: Creates the packet player and binds its inline buffer.
bool Initialize(Playlist*);
// TS3W.exe 0x0086ACF0: Registers the playlist and starts the selected file.
bool Start(Playlist*, uint32_t, uint32_t);
// TS3W.exe 0x0086AD40: Validates float bit classes and updates the recognized property. Unknown IDs return true.
bool SetFloat(Playlist*, uint32_t, float, uint32_t);
// TS3W.exe 0x0086A450: Seeds the custom music directory from default and configured paths.
void PrepareDirectory();
}
namespace ts3::audio {
struct b14 {

 b14impl::Playlist* Construct() { return b14impl::Construct(reinterpret_cast<b14impl::Playlist*>(this)); }
 void Destruct() { b14impl::Destruct(reinterpret_cast<b14impl::Playlist*>(this)); }
 b14impl::Playlist* Delete(uint32_t flags) { return b14impl::Delete(reinterpret_cast<b14impl::Playlist*>(this),flags); }
 b14impl::Playlist* DeleteNotification(uint32_t flags) { return b14impl::DeleteNotification(this,flags); }
 bool Start(uint32_t a,uint32_t b) { return b14impl::Start(reinterpret_cast<b14impl::Playlist*>(this),a,b); }
 bool SetName(uint32_t id,void* text,uint32_t arg) { return b14impl::SetName(reinterpret_cast<b14impl::Playlist*>(this),id,static_cast<char16_t*>(text),arg); }
 void Insert(void* pos,void* source) { b14impl::Insert(reinterpret_cast<b14impl::Tracks*>(this),static_cast<b14impl::Track*>(pos),static_cast<b14impl::Track*>(source)); }
 void Append(void* source) { b14impl::Append(reinterpret_cast<b14impl::Tracks*>(this),static_cast<b14impl::Track*>(source)); }
 bool Refresh() { return b14impl::Refresh(reinterpret_cast<b14impl::Playlist*>(this)); }
 bool NextName(void* out) { return b14impl::NextName(reinterpret_cast<b14impl::Playlist*>(this),static_cast<b14impl::String*>(out)); }
 bool OpenNext() { return b14impl::OpenNext(reinterpret_cast<b14impl::Playlist*>(this)); }
 bool Initialize() { return b14impl::Initialize(reinterpret_cast<b14impl::Playlist*>(this)); }
 bool SetFloat(uint32_t id,float value,uint32_t arg) { return b14impl::SetFloat(reinterpret_cast<b14impl::Playlist*>(this),id,value,arg); }
 static void Shuffle(void* first,void* last,void* random) { b14impl::Shuffle(static_cast<b14impl::Track*>(first),static_cast<b14impl::Track*>(last),static_cast<uint32_t(**)(uint32_t)>(random)); }
 static void PrepareDirectory() { b14impl::PrepareDirectory(); }
};
}
