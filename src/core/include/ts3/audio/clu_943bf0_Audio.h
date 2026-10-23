#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::audio {

// TS3W.exe 0x00943BF0 constructs the sound-name component used by Audio.
struct AudioComponentString {
    char* begin;
    char* end;
    char* capacity;
    const char* allocatorName;
};

struct AudioComponentSoundName {
    int32_t id;
    AudioComponentString name;
};

TS3_X86_OFFSET(AudioComponentSoundName, name, 4);
TS3_X86_SIZE(AudioComponentString, 16);
TS3_X86_SIZE(AudioComponentSoundName, 20);

// __thiscall(this, id, soundName), ret 8.
AudioComponentSoundName* __fastcall ConstructAudioComponentSoundName(
    AudioComponentSoundName* self, void* edx, int32_t id, const char* soundName);

// TS3W.exe 0x00943CE0 is cdecl(this, value), ret 0. The helper it conditionally
// forwards to (0x00943C20) is not part of this rewrite unit.
void* __cdecl ConstructAudioInstancesIfPresent(void* self, uint32_t value);

}  // namespace ts3::audio
