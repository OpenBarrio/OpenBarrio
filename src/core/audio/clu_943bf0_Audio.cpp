#include "ts3/audio/clu_943bf0_Audio.h"

#include <cstdint>

namespace ts3::audio {
namespace {
constexpr uintptr_t kSoundNameAllocator = 0x010435D0;
}

// TS3W.exe 0x00943BF0. The embedded EASTL string assignment remains delegated to
// the unrewritten game helper at 0x00405C00.
AudioComponentSoundName* __fastcall ConstructAudioComponentSoundName(
    AudioComponentSoundName* self, void*, int32_t id, const char* soundName)
{
    self->id = id;
    self->name = {nullptr, nullptr, nullptr,
                  reinterpret_cast<const char*>(kSoundNameAllocator)};
    CallGameMethod<void, 0x00405C00>(&self->name, soundName);
    return self;
}

// TS3W.exe 0x00943CE0. This short cdecl shim returns null without touching the
// object, otherwise forwards to the adjacent, not-yet-rewritten constructor.
void* __cdecl ConstructAudioInstancesIfPresent(void* self, uint32_t value)
{
    return self ? CallGameMethod<void*, 0x00943C20>(self, value) : nullptr;
}

}  // namespace ts3::audio
